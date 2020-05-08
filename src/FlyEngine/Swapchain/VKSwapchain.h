#include "Swapchain.h"
#include <vulkan/vulkan.hpp>
#include <memory>

class VKDevice;
class VKResource;

class VKSwapchain : public Swapchain
{
public:
    VKSwapchain(VKDevice& device, GLFWwindow* window, uint32_t width, uint32_t height, uint32_t frame_count);
private:
    vk::Format m_swapchain_color_format = vk::Format::eB8G8R8Unorm;
    vk::UniqueSwapchainKHR m_swapchain;
    std::vector<std::shared_ptr<VKResource>> m_back_buffers;
};
