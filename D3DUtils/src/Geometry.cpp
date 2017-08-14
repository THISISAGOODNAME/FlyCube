#include "Geometry.h"
#include <vector>
#include <tuple>
#include <fstream>
#include <set>
#include <FileUtility.h>

glm::vec3 aiVector3DToVec3(const aiVector3D& x)
{
    return glm::vec3(x.x, x.y, x.z);
}

ModelLoader::ModelLoader(const std::string& file, IModel& meshes)
    : m_path(GetAssetFullPath(file))
    , m_directory(SplitFilename(m_path))
    , m_model(meshes)
{
    LoadModel();
}

std::string ModelLoader::SplitFilename(const std::string& str)
{
    return str.substr(0, str.find_last_of("/"));
}

void ModelLoader::LoadModel()
{
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(m_path, aiProcess_FlipUVs | aiProcess_FlipWindingOrder | aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_OptimizeMeshes | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace);
    assert(scene && scene->mFlags != AI_SCENE_FLAGS_INCOMPLETE && scene->mRootNode);
    ProcessNode(scene->mRootNode, scene);
}

void ModelLoader::ProcessNode(aiNode* node, const aiScene* scene)
{
    for (uint32_t i = 0; i < node->mNumMeshes; ++i)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        ProcessMesh(mesh, scene);
    }

    for (uint32_t i = 0; i < node->mNumChildren; ++i)
    {
        ProcessNode(node->mChildren[i], scene);
    }
}

void ModelLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
    IMesh& cur_mesh = m_model.GetNextMesh();
    // Walk through each of the mesh's vertices
    for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
    {
        IMesh::Vertex vertex;

        if (mesh->HasPositions())
        {
            vertex.position.x = mesh->mVertices[i].x;
            vertex.position.y = mesh->mVertices[i].y;
            vertex.position.z = mesh->mVertices[i].z;

            m_model.GetBoundBox().x_min = std::min(m_model.GetBoundBox().x_min, vertex.position.x);
            m_model.GetBoundBox().x_max = std::max(m_model.GetBoundBox().x_max, vertex.position.x);

            m_model.GetBoundBox().y_min = std::min(m_model.GetBoundBox().y_min, vertex.position.y);
            m_model.GetBoundBox().y_max = std::max(m_model.GetBoundBox().y_max, vertex.position.y);

            m_model.GetBoundBox().z_min = std::min(m_model.GetBoundBox().z_min, vertex.position.z);
            m_model.GetBoundBox().z_max = std::max(m_model.GetBoundBox().z_max, vertex.position.z);
        }

        if (mesh->HasNormals())
        {
            vertex.normal = aiVector3DToVec3(mesh->mNormals[i]);
        }

        if (mesh->HasTangentsAndBitangents())
        {
            vertex.tangent = aiVector3DToVec3(mesh->mTangents[i]);
            vertex.bitangent = aiVector3DToVec3(mesh->mBitangents[i]);
        }

        // A vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
        // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
        if (mesh->HasTextureCoords(0))
        {
            vertex.texCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }
        else
        {
            vertex.texCoords = glm::vec2(0.0f, 0.0f);
        }

        cur_mesh.vertices.push_back(vertex);
    }
    // Now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace face = mesh->mFaces[i];
        // Retrieve all indices of the face and store them in the indices vector
        for (uint32_t j = 0; j < face.mNumIndices; ++j)
        {
            cur_mesh.indices.push_back(face.mIndices[j]);
        }
    }

    // Process materials
    if (mesh->mMaterialIndex >= 0)
    {
        std::vector<IMesh::Texture> textures;
        aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
        LoadMaterialTextures(mat, aiTextureType_AMBIENT, textures);
        LoadMaterialTextures(mat, aiTextureType_DIFFUSE, textures);
        LoadMaterialTextures(mat, aiTextureType_SPECULAR, textures);
        LoadMaterialTextures(mat, aiTextureType_HEIGHT, textures);
        LoadMaterialTextures(mat, aiTextureType_OPACITY, textures);

        FindSimilarTextures(textures);

        auto comparator = [&](const IMesh::Texture& lhs, const IMesh::Texture& rhs)
        {
            return std::tie(lhs.type, lhs.path) < std::tie(rhs.type, rhs.path);
        };

        std::set<IMesh::Texture, decltype(comparator)> unique_textures(comparator);

        for (IMesh::Texture& texture : textures)
        {
            unique_textures.insert(texture);
        }

        for (const IMesh::Texture& texture : unique_textures)
        {
            cur_mesh.textures.push_back(texture);
        }

        mat->Get(AI_MATKEY_SHININESS, cur_mesh.material.shininess);
        mat->Get(AI_MATKEY_COLOR_AMBIENT, cur_mesh.material.amb);
        mat->Get(AI_MATKEY_COLOR_DIFFUSE, cur_mesh.material.dif);
        mat->Get(AI_MATKEY_COLOR_SPECULAR, cur_mesh.material.spec);
        mat->Get(AI_MATKEY_NAME, cur_mesh.material.name);
    }
}

void ModelLoader::FindSimilarTextures(std::vector<IMesh::Texture>& textures)
{
    static std::pair<std::string, aiTextureType> map_from[] = {
        { "_s", aiTextureType_SPECULAR },
        { "_color", aiTextureType_DIFFUSE }
    };

    static std::pair<std::string, aiTextureType> map_to[] = {
        { "_g", aiTextureType_SHININESS },
        { "_gloss", aiTextureType_SHININESS },
        { "_rough", aiTextureType_SHININESS },
        { "_nmap", aiTextureType_HEIGHT }
    };

    std::vector<IMesh::Texture> added_textures;
    for (auto& from_type : map_from)
    {
        for (auto& cur_texture : textures)
        {
            std::string path = cur_texture.path;

            size_t loc = path.find(from_type.first);
            if (loc == std::string::npos)
                continue;

            for (auto& to_type : map_to)
            {
                std::string cur_path = path;
                cur_path.replace(loc, from_type.first.size(), to_type.first);
                if (!std::ifstream(cur_path).good())
                    continue;

                IMesh::Texture texture;
                texture.type = to_type.second;
                texture.path = cur_path;
                added_textures.push_back(texture);
            }
        }
    }

    textures.insert(textures.end(), added_textures.begin(), added_textures.end());
}

void ModelLoader::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::vector<IMesh::Texture>& textures)
{
    for (uint32_t i = 0; i < mat->GetTextureCount(type); ++i)
    {
        aiString texture_name;
        mat->GetTexture(type, i, &texture_name);
        std::string texture_path = m_directory + "/" + texture_name.C_Str();
        if(!std::ifstream(texture_path).good())
            continue;

        IMesh::Texture texture;
        texture.type = type;
        texture.path = texture_path;
        textures.push_back(texture);
    }
}
