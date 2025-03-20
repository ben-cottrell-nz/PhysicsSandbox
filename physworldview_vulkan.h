#ifndef PHYSWORLDVIEW_VULKAN_H
#define PHYSWORLDVIEW_VULKAN_H
// Qt headers
#include <QWidget>
#include <QVulkanWindow>
#include <QVulkanWindowRenderer>
// Skia headers
#include <include/gpu/vk/VulkanBackendContext.h>
#include <include/gpu/vk/VulkanExtensions.h>
#include <include/gpu/ganesh/GrDirectContext.h>
#include <include/gpu/ganesh/vk/GrVkDirectContext.h>
#include <include/gpu/ganesh/SkSurfaceGanesh.h>
#include <include/core/SkData.h>
#include <include/core/SkImage.h>
#include <include/core/SkStream.h>
#include <include/core/SkSurface.h>
#include <include/core/SkCanvas.h>

class VulkanRenderer : public QVulkanWindowRenderer
{
public:
    VulkanRenderer(QVulkanWindow *w);
    ~VulkanRenderer() { qDebug() << __FUNCTION__; }
    void initResources() override;
    void initSwapChainResources() override;
    void releaseSwapChainResources() override;
    void releaseResources() override;
    void startNextFrame() override;
    void setVkInstance(QVulkanInstance* vkInst) {
        //m_vkInstance = vkInst;
    }

private:
    QVulkanWindow *m_window;
    QVulkanDeviceFunctions *m_devFuncs;
    float m_green = 0;
    skgpu::VulkanBackendContext m_backendContext;
    std::unique_ptr<skgpu::VulkanExtensions> m_extensions;
    //std::unique_ptr<VkPhysicalDeviceFeatures2> m_features;
    sk_sp<GrDirectContext> m_grContext;
    sk_sp<SkSurface> m_gpuSurface;
    SkCanvas* m_skCanvas;
};

static uint64_t PhysWorldViewVulkan_refcnt = 0;

class PhysWorldViewVulkan : public QVulkanWindow
{
public:
    //explicit PhysWorldViewVulkan(QWidget *parent = nullptr);
    PhysWorldViewVulkan();
    ~PhysWorldViewVulkan();
    QVulkanWindowRenderer *createRenderer() override;
};

#endif // PHYSWORLDVIEW_VULKAN_H
