#pragma once

#include <Context/DX12Context.h>
#include "Program/ProgramApi.h"
#include <algorithm>
#include <deque>
#include <array>
#include <set>
#include <unordered_map>

#include "Context/DescriptorPool.h"

class DX12ProgramApi : public ProgramApi
{
public:
    DX12ProgramApi(DX12Context& context);

    virtual void SetMaxEvents(size_t count) override;
    virtual void UseProgram() override;
    virtual void ApplyBindings() override;
    virtual void OnCompileShader(ShaderType type, const ComPtr<ID3DBlob>& blob) override;
    virtual void AttachSRV(ShaderType type, const std::string& name, uint32_t slot, const Resource::Ptr& res) override;
    virtual void AttachUAV(ShaderType type, const std::string& name, uint32_t slot, const Resource::Ptr& res) override;
    virtual void AttachCBuffer(ShaderType type, UINT slot, BufferLayout& buffer) override;
    virtual void AttachSampler(ShaderType type, uint32_t slot, const SamplerDesc& desc) override;
    virtual void AttachRTV(uint32_t slot, const Resource::Ptr& ires) override;
    virtual void AttachDSV(const Resource::Ptr& ires) override;
    virtual void ClearRenderTarget(uint32_t slot, const FLOAT ColorRGBA[4]) override;
    virtual void ClearDepthStencil(UINT ClearFlags, FLOAT Depth, UINT8 Stencil) override;
    virtual void SetRasterizeState(const RasterizerDesc& desc) override;
    virtual void SetBlendState(const BlendDesc& desc) override;
    virtual void SetDepthStencilState(const DepthStencilDesc& desc) override;

    void OnPresent();

private:
    void AttachCBV(ShaderType type, uint32_t slot, DX12Resource::Ptr& res);
    DescriptorHeapRange CreateSrv(ShaderType type, const std::string& name, uint32_t slot, const Resource::Ptr& ires);
    DescriptorHeapRange CreateUAV(ShaderType type, const std::string& name, uint32_t slot, const Resource::Ptr& ires);
    DescriptorHeapRange CreateCBV(ShaderType type, uint32_t slot, DX12Resource::Ptr& res);
    DescriptorHeapRange CreateSampler(ShaderType type, uint32_t slot, const SamplerDesc& desc);
    DescriptorHeapRange CreateRTV(uint32_t slot, const Resource::Ptr& ires);
    DescriptorHeapRange CreateDSV(const Resource::Ptr& ires);
    DX12Resource::Ptr CreateCBuffer(size_t buffer_size);

    void SetRootSignature(ID3D12RootSignature* pRootSignature);
    void SetRootDescriptorTable(UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor);
    void SetRootConstantBufferView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);
    std::vector<D3D12_INPUT_ELEMENT_DESC> GetInputLayout(ComPtr<ID3D12ShaderReflection> reflector);
    void CreateGraphicsPSO();
    void CreateComputePSO();
    void ParseShaders();
    void OMSetRenderTargets();

private:
    decltype(&::D3DReflect) _D3DReflect = &::D3DReflect;

    size_t m_num_cbv = 0;
    size_t m_num_srv = 0;
    size_t m_num_uav = 0;
    size_t m_num_rtv = 0;
    size_t m_num_sampler = 0;

    std::map<std::tuple<ShaderType, ResourceType, uint32_t>, DescriptorHeapRange> m_heap_ranges;
    std::map<ShaderType, ComPtr<ID3DBlob>> m_blob_map;
    bool m_is_compute = false;

    D3D12_ROOT_DESCRIPTOR_TABLE DescriptorTable;
    D3D12_ROOT_CONSTANTS Constants;
    D3D12_ROOT_DESCRIPTOR Descriptor;

    struct BindingLayout
    {
        D3D12_ROOT_PARAMETER_TYPE type;
        size_t root_param_index;
        union
        {
            struct
            {
                size_t root_param_heap_offset;
                size_t heap_offset;
            } table;

            struct
            {
                size_t root_param_num;
            } view;
        };
    };

    std::map<std::tuple<ShaderType, D3D12_DESCRIPTOR_RANGE_TYPE>, BindingLayout> m_binding_layout;
    std::map<std::tuple<ShaderType, size_t>, std::reference_wrapper<BufferLayout>> m_cbv_layout;
    PerFrameData<std::map<std::tuple<ShaderType, size_t>, std::vector<DX12Resource::Ptr>>> m_cbv_buffer;
    PerFrameData<std::map<std::tuple<ShaderType, size_t>, size_t>> m_cbv_offset;
    ComPtr<ID3D12RootSignature> m_root_signature;
    bool m_changed_pso_desc = false;
    bool m_changed_binding = false;
    bool m_changed_om = false;
    D3D12_GRAPHICS_PIPELINE_STATE_DESC m_pso_desc = {};
    D3D12_COMPUTE_PIPELINE_STATE_DESC m_compute_pso_desc = {};

    template<typename T>
    struct Hasher
    {
        std::size_t operator()(const T& oid) const
        {
            const uint8_t* data = reinterpret_cast<const uint8_t*>(&oid);
            auto size = sizeof(T);
            std::size_t prime = 31;
            std::size_t p_pow = 1;
            std::size_t hash = 0;
            for (size_t i = 0; i < size; ++i)
            {
                hash += (*data + 1ll) * p_pow;
                p_pow *= prime;
                ++data;
            }
            return hash;
        }
    };

    template<typename T>
    class EqualFn
    {
    public:
        bool operator() (const T& t1, const T& t2) const
        {
            return memcmp(&t1, &t2, sizeof(T)) == 0;
        }
    };

    std::unordered_map<D3D12_GRAPHICS_PIPELINE_STATE_DESC, ComPtr<ID3D12PipelineState>,
        Hasher<D3D12_GRAPHICS_PIPELINE_STATE_DESC>, 
        EqualFn<D3D12_GRAPHICS_PIPELINE_STATE_DESC>> m_pso;
    ComPtr<ID3D12PipelineState> m_current_pso;

    std::vector<D3D12_INPUT_ELEMENT_DESC> m_input_layout;
    ComPtr<ID3D12ShaderReflection> m_input_layout_reflector;

    const bool m_use_cbv_table = false;

    DX12Context& m_context;
    size_t m_program_id;
};
