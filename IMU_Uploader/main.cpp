/**
 * @file main.cpp
 * @brief 应用程序入口文件
 * 
 * 初始化Qt应用程序，加载样式表，创建并显示主窗口
 */

#include "src/mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QDir>
#include <QFont>
#include <QStyleFactory>

/**
 * @brief 主函数
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * @return 应用程序退出码
 * 
 * 程序启动流程:
 * 1. 创建QApplication对象
 * 2. 加载暗色主题样式表
 * 3. 设置应用程序字体
 * 4. 创建并显示主窗口
 * 5. 进入事件循环
 */
int main(int argc, char *argv[])
{
    // 创建Qt应用程序对象
    QApplication a(argc, argv);
    
    // 设置应用程序信息
    a.setApplicationName("IMU Uploader");
    a.setApplicationVersion("1.0.0");
    a.setOrganizationName("IMU Developer");
    
    // 设置字体
    QFont font("Microsoft YaHei", 9);
    a.setFont(font);
    
    // 加载暗色主题样式表
    QFile styleFile(":/themes/dark.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        a.setStyleSheet(styleSheet);
        styleFile.close();
    }
    
    // 创建主窗口
    MainWindow w;
    w.setWindowTitle("IMU姿态传感器上位机 v1.0");
    w.resize(1400, 900);  // 设置默认窗口大小
    w.show();             // 显示主窗口
    
    // 进入Qt事件循环
    return a.exec();
}
