#include "mainwindow.h"
MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent)
{
    physWorldView = new PhysWorldView(this);
    setCentralWidget(physWorldView);
}
MainWindow::~MainWindow()
{

}
