#include "Shader/ShaderBase.h"
#include <HLSLCompiler/Compiler.h>

ShaderBase::ShaderBase(const ShaderDesc& desc, ShaderBlobType blob_type)
    : m_shader_type(desc.type)
    , m_blob_type(blob_type)
{
    m_blob = Compile(desc, blob_type);
    m_reflection = CreateShaderReflection(blob_type, m_blob.data(), m_blob.size());
    m_bindings = m_reflection->GetBindings();
    for (uint32_t i = 0; i < m_bindings.size(); ++i)
    {
        BindKey bind_key = { m_shader_type, m_bindings[i].type, m_bindings[i].slot, m_bindings[i].space, m_bindings[i].count };
        m_bind_keys[m_bindings[i].name] = bind_key;
        m_mapping[bind_key] = i;
        m_binding_keys.emplace_back(bind_key);
    }

    decltype(auto) input_parameters = m_reflection->GetInputParameters();
    for (uint32_t i = 0; i < input_parameters.size(); ++i)
    {
        decltype(auto) layout = m_input_layout_descs.emplace_back();
        layout.slot = i;
        layout.location = input_parameters[i].location;
        layout.format = input_parameters[i].format;
        layout.stride = gli::detail::bits_per_pixel(layout.format) / 8;

        m_locations[input_parameters[i].semantic_name] = layout.location;
        m_semantic_names[layout.location] = input_parameters[i].semantic_name;
    }
}

ShaderType ShaderBase::GetType() const
{
    return m_shader_type;
}

const std::vector<uint8_t>& ShaderBase::GetBlob() const
{
    return m_blob;
}

const BindKey& ShaderBase::GetBindKey(const std::string& name) const
{
    auto it = m_bind_keys.find(name);
    if (it != m_bind_keys.end())
        return it->second;
    assert(m_blob_type == ShaderBlobType::kSPIRV);
    return m_bind_keys.at("type_" + name);
}

const std::vector<ResourceBindingDesc>& ShaderBase::GetResourceBindings() const
{
    return m_bindings;
}

const ResourceBindingDesc& ShaderBase::GetResourceBinding(const BindKey& bind_key) const
{
    return m_bindings.at(m_mapping.at(bind_key));
}

const std::vector<InputLayoutDesc>& ShaderBase::GetInputLayouts() const
{
    return m_input_layout_descs;
}

uint32_t ShaderBase::GetInputLayoutLocation(const std::string& semantic_name) const
{
    return m_locations.at(semantic_name);
}

const std::string& ShaderBase::GetSemanticName(uint32_t location) const
{
    return m_semantic_names.at(location);
}

const std::vector<BindKey>& ShaderBase::GetBindings() const
{
    return m_binding_keys;
}

const std::shared_ptr<ShaderReflection>& ShaderBase::GetReflection() const
{
    return m_reflection;
}