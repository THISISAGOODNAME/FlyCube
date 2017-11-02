#pragma once

#include <Scene/SceneBase.h>
#include <Context/Context.h>
#include <Geometry/DX11Geometry.h>
#include <d3d11.h>
#include <DXGI1_4.h>
#include <wrl.h>
#include <string>

#include <Program/Program.h>
#include <ProgramRef/GeometryPassPS.h>
#include <ProgramRef/GeometryPassVS.h>
#include <ProgramRef/LightPassPS.h>
#include <ProgramRef/LightPassVS.h>

using namespace Microsoft::WRL;

class DX11Scene : public SceneBase
{
public:
    DX11Scene(int width, int height);

    static IScene::Ptr Create(int width, int height);

    virtual void OnUpdate() override;
    void GeometryPass();
    void LightPass();
    virtual void OnRender() override;
    virtual void OnResize(int width, int height) override;
    virtual glm::mat4 StoreMatrix(const glm::mat4& m) override;

private:
    void CreateRT();
    void CreateViewPort();
    void CreateSampler();

    std::unique_ptr<Model<DX11Mesh>> CreateGeometry(const std::string & path);

    void CreateRTV(ComPtr<ID3D11RenderTargetView>& rtv, ComPtr<ID3D11ShaderResourceView>& srv);
    void SetVertexBuffer(UINT slot, ID3D11Buffer* buffer, UINT stride, UINT offset);

    void InitGBuffer();


    ComPtr<ID3D11RenderTargetView> m_render_target_view;
    ComPtr<ID3D11DepthStencilView> m_depth_stencil_view;
    ComPtr<ID3D11Texture2D> m_depth_stencil_buffer;

    D3D11_VIEWPORT m_viewport;

    ComPtr<ID3D11SamplerState> m_texture_sampler;

    std::unique_ptr<Program<GeometryPassPS, GeometryPassVS>> m_shader_geometry_pass;
    std::unique_ptr<Program<LightPassPS, LightPassVS>> m_shader_light_pass;

    std::unique_ptr<Model<DX11Mesh>> m_model_of_file;
    std::unique_ptr<Model<DX11Mesh>> m_model_square;

    ComPtr<ID3D11RenderTargetView> m_position_rtv;
    ComPtr<ID3D11RenderTargetView> m_normal_rtv;
    ComPtr<ID3D11RenderTargetView> m_ambient_rtv;
    ComPtr<ID3D11RenderTargetView> m_diffuse_rtv;
    ComPtr<ID3D11RenderTargetView> m_specular_rtv;

    ComPtr<ID3D11ShaderResourceView> m_position_srv;
    ComPtr<ID3D11ShaderResourceView> m_normal_srv;
    ComPtr<ID3D11ShaderResourceView> m_ambient_srv;
    ComPtr<ID3D11ShaderResourceView> m_diffuse_srv;
    ComPtr<ID3D11ShaderResourceView> m_specular_srv;

    int m_width;
    int m_height;
    Context m_context;
};
