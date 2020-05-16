#include "Swapchain/DXSwapchain.h"
#include <Adapter/DXAdapter.h>
#include <Device/DXDevice.h>
#include <Instance/DXInstance.h>
#include <Resource/DXResource.h>
#include <Utilities/DXUtility.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

DXSwapchain::DXSwapchain(DXDevice& device, GLFWwindow* window, uint32_t width, uint32_t height, uint32_t frame_count, bool vsync)
    : m_device(device)
    , m_vsync(vsync)
{
    DXInstance& instance = device.GetAdapter().GetInstance();
    DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
    swap_chain_desc.Width = width;
    swap_chain_desc.Height = height;
    swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.BufferCount = frame_count;
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    ComPtr<IDXGISwapChain1> tmp_swap_chain;
    ASSERT_SUCCEEDED(instance.GetFactory()->CreateSwapChainForHwnd(device.GetCommandQueue().Get(), glfwGetWin32Window(window), &swap_chain_desc, nullptr, nullptr, &tmp_swap_chain));
    tmp_swap_chain.As(&m_swap_chain);

    for (size_t i = 0; i < frame_count; ++i)
    {
        std::shared_ptr<DXResource> res = std::make_shared<DXResource>(m_device);
        ComPtr<ID3D12Resource> back_buffer;
        ASSERT_SUCCEEDED(m_swap_chain->GetBuffer(i, IID_PPV_ARGS(&back_buffer)));
        res->m_format = GetFormat();
        res->state = D3D12_RESOURCE_STATE_PRESENT;
        res->default_res = back_buffer;
        res->desc = back_buffer->GetDesc();
        m_back_buffers.emplace_back(res);
    }
}

gli::format DXSwapchain::GetFormat() const
{
    return gli::FORMAT_RGBA8_UNORM_PACK8;
}

std::shared_ptr<Resource> DXSwapchain::GetBackBuffer(uint32_t buffer)
{
    return m_back_buffers[buffer];
}

uint32_t DXSwapchain::NextImage(const std::shared_ptr<Semaphore>& semaphore)
{
    m_device.Signal(semaphore);
    return m_swap_chain->GetCurrentBackBufferIndex();
}

void DXSwapchain::Present(const std::shared_ptr<Semaphore>& semaphore)
{
    m_device.Wait(semaphore);
    if (m_vsync)
    {
        ASSERT_SUCCEEDED(m_swap_chain->Present(1, 0));
    }
    else
    {
        ASSERT_SUCCEEDED(m_swap_chain->Present(0, DXGI_PRESENT_ALLOW_TEARING));
    }
}
