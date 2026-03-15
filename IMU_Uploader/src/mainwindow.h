#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>
#include <QTabWidget>
#include <QStatusBar>
#include <QTimer>
#include "src/serial/SerialPort.h"
#include "src/serial/ProtocolParser.h"
#include "src/visualization/ThreeDView.h"
#include "src/visualization/AttitudeIndicator.h"
#include "src/visualization/WaveformPlot.h"
#include "src/data/DataBuffer.h"
#include "src/config/DeviceManager.h"
#include "src/config/ConfigManager.h"

namespace Ui {
class MainWindow;
}

/**
 * @brief 主窗口类
 * 
 * IMU姿态传感器上位机的主界面，包含:
 * - 串口配置面板
 * - 3D姿态显示
 * - 姿态仪表盘
 * - 实时波形图
 * - 数据管理功能
 * 
 * 界面布局:
 * ┌─────────────────────────────────────────────┐
 * │  菜单栏  │  工具栏                           │
 * ├──────────┬──────────────────────────────────┤
 * │ 串口配置 │  3D姿态显示  │  姿态仪表盘       │
 * │ 实时数据 │              │                   │
 * │ 模型选择 │──────────────────────────────────│
 * │ 波形设置 │  实时波形图                      │
 * └──────────┴──────────────────────────────────┘
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
protected:
    /**
     * @brief 重写关闭事件
     * @param event 关闭事件对象
     * 
     * 在窗口关闭时保存设置并断开设备连接
     */
    void closeEvent(QCloseEvent *event) override;
    
private slots:
    // ========== 串口操作槽函数 ==========
    void onConnectClicked();        // 连接按钮点击
    void onDisconnectClicked();     // 断开按钮点击
    void onRefreshPorts();          // 刷新串口列表
    
    // ========== 设备状态槽函数 ==========
    void onDeviceConnected(const QString &portName);   // 设备已连接
    void onDeviceDisconnected();                       // 设备已断开
    void onConnectionError(const QString &error);      // 连接错误
    void onDataReceived(const IMUData &data);          // 接收到数据
    
    // ========== 数据管理槽函数 ==========
    void onExportData();    // 导出数据
    void onImportData();    // 导入数据
    void onClearData();     // 清空数据
    
    // ========== 界面设置槽函数 ==========
    void onModelChanged(int index);         // 3D模型切换
    void onTimeWindowChanged(int index);    // 时间窗口切换
    
    /**
     * @brief 更新状态栏
     * 
     * 每秒更新一次FPS和数据计数
     */
    void updateStatus();
    
private:
    // ========== UI初始化函数 ==========
    void setupUI();          // 初始化界面布局
    void setupMenuBar();     // 初始化菜单栏
    void setupToolBar();     // 初始化工具栏
    void setupStatusBar();   // 初始化状态栏
    void setupConnections(); // 初始化信号槽连接
    
    // ========== 设置管理函数 ==========
    void loadSettings();     // 加载设置
    void saveSettings();     // 保存设置
    
    Ui::MainWindow *ui;
    
    QWidget *m_centralWidget;    // 中央控件
    
    // ========== 串口配置控件 ==========
    QComboBox *m_portComboBox;        // 串口选择下拉框
    QComboBox *m_baudRateComboBox;    // 波特率选择下拉框
    QPushButton *m_connectButton;     // 连接按钮
    QPushButton *m_disconnectButton;  // 断开按钮
    QPushButton *m_refreshButton;     // 刷新按钮
    
    // ========== 可视化控件 ==========
    ThreeDView *m_threeDView;             // 3D姿态视图
    AttitudeIndicator *m_attitudeIndicator;  // 姿态仪表盘
    WaveformPlot *m_waveformPlot;        // 波形图
    
    // ========== 数据显示标签 ==========
    QLabel *m_rollLabel;    // 滚转角标签
    QLabel *m_pitchLabel;   // 俯仰角标签
    QLabel *m_yawLabel;     // 偏航角标签
    
    // ========== 状态栏标签 ==========
    QLabel *m_statusLabel;       // 连接状态标签
    QLabel *m_fpsLabel;          // FPS标签
    QLabel *m_dataCountLabel;    // 数据计数标签
    
    QTabWidget *m_tabWidget;     // 标签页控件
    
    // ========== 管理器对象 ==========
    DeviceManager *m_deviceManager;  // 设备管理器
    ConfigManager *m_configManager;  // 配置管理器
    
    // ========== 定时器和计数器 ==========
    QTimer *m_statusTimer;       // 状态更新定时器
    quint32 m_frameCount;        // 当前帧计数
    quint32 m_lastFrameCount;    // 上一秒帧计数
};

#endif
