#include "mainwindow.h"
#include <QApplication>
#include <QVulkanInstance>
#include "physworldview_vulkan.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QVulkanInstance inst;
    inst.setLayers({"VK_LAYER_KHRONOS_validation"});
    if (!inst.create()) {
        qFatal("Failed to create the Vulkan instance: %d", inst.errorCode());
    }
    qDebug("Started app");
    std::shared_ptr<QVulkanInstance> pVkInst(&inst);
    PhysWorldViewVulkan vkw;
    vkw.setVulkanInstance(pVkInst.get());
    QMainWindow w;
    // freed by QMainWindow
    QWidget* wrapper(QWidget::createWindowContainer(&vkw, &w));
    w.setCentralWidget(wrapper);
    w.resize(1024,768);
    w.show();
    auto r = a.exec();
    qDebug() << "PhysWorldViewVulkan_refcnt: " << PhysWorldViewVulkan_refcnt;
    return r;
}
