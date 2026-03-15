/**
 * @file mainwindow.cpp
 * @brief 主窗口实现文件
 * 
 * 实现IMU姿态传感器上位机的主界面功能
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QMenuBar>
#include <QToolBar>
#include <QApplication>
#include <QDebug>

/**
 * @brief 构造函数
 * @param parent 父窗口指针
 * 
 * 初始化主窗口:
 * 1. 创建设备管理器和配置管理器实例
 * 2. 初始化UI界面
 * 3. 设置菜单栏、工具栏、状态栏
 * 4. 连接信号槽
 * 5. 加载用户设置
 * 6. 启动状态更新定时器(1秒间隔)
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_deviceManager(DeviceManager::instance())
    , m_configManager(ConfigManager::instance())
    , m_statusTimer(new QTimer(this))
    , m_frameCount(0)
    , m_lastFrameCount(0)
{
    ui->setupUi(this);
    
    // 初始化界面组件
    setupUI();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupConnections();
    loadSettings();
    
    // 启动状态更新定时器(每秒更新一次)
    m_statusTimer->start(1000);
}

/**
 * @brief 析构函数
 * 
 * 保存设置并释放资源
 */
MainWindow::~MainWindow()
{
    saveSettings();
    delete ui;
}

/**
 * @brief 重写关闭事件
 * @param event 关闭事件对象
 * 
 * 在窗口关闭时:
 * 1. 断开设备连接
 * 2. 保存用户设置
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_deviceManager->isConnected()) {
        m_deviceManager->disconnectDevice();
    }
    saveSettings();
    event->accept();
}

void MainWindow::setupUI()
{
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    QHBoxLayout *mainLayout = new QHBoxLayout(m_centralWidget);
    
    QWidget *leftPanel = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    
    QGroupBox *serialGroup = new QGroupBox("串口配置");
    QGridLayout *serialLayout = new QGridLayout(serialGroup);
    
    QLabel *portLabel = new QLabel("串口:");
    m_portComboBox = new QComboBox();
    m_refreshButton = new QPushButton("刷新");
    
    QLabel *baudLabel = new QLabel("波特率:");
    m_baudRateComboBox = new QComboBox();
    m_baudRateComboBox->addItems({"9600", "19200", "38400", "57600", "115200", "230400", "460800", "921600"});
    m_baudRateComboBox->setCurrentText("115200");
    
    m_connectButton = new QPushButton("连接");
    m_disconnectButton = new QPushButton("断开");
    m_disconnectButton->setEnabled(false);
    
    serialLayout->addWidget(portLabel, 0, 0);
    serialLayout->addWidget(m_portComboBox, 0, 1);
    serialLayout->addWidget(m_refreshButton, 0, 2);
    serialLayout->addWidget(baudLabel, 1, 0);
    serialLayout->addWidget(m_baudRateComboBox, 1, 1);
    serialLayout->addWidget(m_connectButton, 2, 0, 1, 2);
    serialLayout->addWidget(m_disconnectButton, 2, 2);
    
    leftLayout->addWidget(serialGroup);
    
    QGroupBox *dataGroup = new QGroupBox("实时数据");
    QVBoxLayout *dataLayout = new QVBoxLayout(dataGroup);
    
    m_rollLabel = new QLabel("Roll:  0.00°");
    m_rollLabel->setStyleSheet("color: rgb(255, 100, 100); font-size: 16px; font-weight: bold;");
    m_pitchLabel = new QLabel("Pitch: 0.00°");
    m_pitchLabel->setStyleSheet("color: rgb(100, 255, 100); font-size: 16px; font-weight: bold;");
    m_yawLabel = new QLabel("Yaw:   0.00°");
    m_yawLabel->setStyleSheet("color: rgb(100, 100, 255); font-size: 16px; font-weight: bold;");
    
    dataLayout->addWidget(m_rollLabel);
    dataLayout->addWidget(m_pitchLabel);
    dataLayout->addWidget(m_yawLabel);
    
    leftLayout->addWidget(dataGroup);
    
    QGroupBox *modelGroup = new QGroupBox("3D模型");
    QVBoxLayout *modelLayout = new QVBoxLayout(modelGroup);
    
    QComboBox *modelCombo = new QComboBox();
    modelCombo->addItems({"飞机", "汽车", "手机", "立方体"});
    connect(modelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onModelChanged);
    
    modelLayout->addWidget(modelCombo);
    leftLayout->addWidget(modelGroup);
    
    QGroupBox *plotGroup = new QGroupBox("波形设置");
    QVBoxLayout *plotLayout = new QVBoxLayout(plotGroup);
    
    QLabel *windowLabel = new QLabel("时间窗口:");
    QComboBox *windowCombo = new QComboBox();
    windowCombo->addItems({"5秒", "10秒", "30秒", "60秒"});
    windowCombo->setCurrentIndex(1);
    connect(windowCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onTimeWindowChanged);
    
    plotLayout->addWidget(windowLabel);
    plotLayout->addWidget(windowCombo);
    leftLayout->addWidget(plotGroup);
    
    leftLayout->addStretch();
    
    mainLayout->addWidget(leftPanel, 1);
    
    QWidget *rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    
    m_tabWidget = new QTabWidget();
    
    QWidget *monitorTab = new QWidget();
    QHBoxLayout *monitorLayout = new QHBoxLayout(monitorTab);
    
    m_threeDView = new ThreeDView();
    m_attitudeIndicator = new AttitudeIndicator();
    
    monitorLayout->addWidget(m_threeDView, 1);
    monitorLayout->addWidget(m_attitudeIndicator, 1);
    
    m_tabWidget->addTab(monitorTab, "实时监控");
    
    QWidget *waveformTab = new QWidget();
    QVBoxLayout *waveformLayout = new QVBoxLayout(waveformTab);
    
    m_waveformPlot = new WaveformPlot();
    m_waveformPlot->setDataBuffer(m_deviceManager->getDataBuffer());
    waveformLayout->addWidget(m_waveformPlot);
    
    m_tabWidget->addTab(waveformTab, "波形显示");
    
    rightLayout->addWidget(m_tabWidget);
    
    mainLayout->addWidget(rightPanel, 3);
    
    onRefreshPorts();
}

void MainWindow::setupMenuBar()
{
    QMenuBar *menuBar = this->menuBar();
    
    QMenu *fileMenu = menuBar->addMenu("文件(&F)");
    QAction *exportAction = fileMenu->addAction("导出数据(&E)");
    QAction *importAction = fileMenu->addAction("导入数据(&I)");
    fileMenu->addSeparator();
    QAction *exitAction = fileMenu->addAction("退出(&X)");
    
    connect(exportAction, &QAction::triggered, this, &MainWindow::onExportData);
    connect(importAction, &QAction::triggered, this, &MainWindow::onImportData);
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);
    
    QMenu *viewMenu = menuBar->addMenu("视图(&V)");
    QAction *resetViewAction = viewMenu->addAction("重置视图(&R)");
    connect(resetViewAction, &QAction::triggered, m_threeDView, &ThreeDView::resetView);
    
    QMenu *helpMenu = menuBar->addMenu("帮助(&H)");
    QAction *aboutAction = helpMenu->addAction("关于(&A)");
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, "关于", "IMU姿态传感器上位机 v1.0\n\n"
                                       "基于Qt开发的姿态传感器数据采集与可视化软件\n"
                                       "支持MPU6500/MPU6050/ICM42688等传感器");
    });
}

void MainWindow::setupToolBar()
{
    QToolBar *toolBar = addToolBar("工具栏");
    
    QAction *clearAction = toolBar->addAction("清空数据");
    connect(clearAction, &QAction::triggered, this, &MainWindow::onClearData);
    
    toolBar->addSeparator();
    
    QAction *pauseAction = toolBar->addAction("暂停");
    pauseAction->setCheckable(true);
    connect(pauseAction, &QAction::toggled, [this](bool checked) {
        m_waveformPlot->pause(checked);
    });
}

void MainWindow::setupStatusBar()
{
    QStatusBar *statusBar = this->statusBar();
    
    m_statusLabel = new QLabel("未连接");
    m_fpsLabel = new QLabel("FPS: 0");
    m_dataCountLabel = new QLabel("数据: 0");
    
    statusBar->addWidget(m_statusLabel, 1);
    statusBar->addPermanentWidget(m_fpsLabel);
    statusBar->addPermanentWidget(m_dataCountLabel);
}

void MainWindow::setupConnections()
{
    connect(m_connectButton, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(m_disconnectButton, &QPushButton::clicked, this, &MainWindow::onDisconnectClicked);
    connect(m_refreshButton, &QPushButton::clicked, this, &MainWindow::onRefreshPorts);
    
    connect(m_deviceManager, &DeviceManager::deviceConnected, this, &MainWindow::onDeviceConnected);
    connect(m_deviceManager, &DeviceManager::deviceDisconnected, this, &MainWindow::onDeviceDisconnected);
    connect(m_deviceManager, &DeviceManager::connectionError, this, &MainWindow::onConnectionError);
    connect(m_deviceManager, &DeviceManager::dataReceived, this, &MainWindow::onDataReceived);
    
    connect(m_statusTimer, &QTimer::timeout, this, &MainWindow::updateStatus);
}

void MainWindow::loadSettings()
{
    QByteArray geometry = m_configManager->getWindowGeometry();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
    
    QByteArray state = m_configManager->getWindowState();
    if (!state.isEmpty()) {
        restoreState(state);
    }
    
    QString portName = m_configManager->getPortName();
    if (!portName.isEmpty()) {
        int index = m_portComboBox->findText(portName);
        if (index >= 0) {
            m_portComboBox->setCurrentIndex(index);
        }
    }
    
    int baudRate = m_configManager->getBaudRate();
    m_baudRateComboBox->setCurrentText(QString::number(baudRate));
}

void MainWindow::saveSettings()
{
    m_configManager->setWindowGeometry(saveGeometry());
    m_configManager->setWindowState(saveState());
    
    QString portName = m_portComboBox->currentText().split(" - ").first();
    m_configManager->setSerialConfig(portName, m_baudRateComboBox->currentText().toInt());
}

void MainWindow::onConnectClicked()
{
    QString portName = m_portComboBox->currentText();
    if (portName.isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择串口!");
        return;
    }
    
    qint32 baudRate = m_baudRateComboBox->currentText().toInt();
    
    if (!m_deviceManager->connectDevice(portName, baudRate)) {
        QMessageBox::critical(this, "错误", "连接失败!");
    }
}

void MainWindow::onDisconnectClicked()
{
    m_deviceManager->disconnectDevice();
}

void MainWindow::onRefreshPorts()
{
    m_portComboBox->clear();
    QStringList ports = SerialPort().scanPorts();
    m_portComboBox->addItems(ports);
}

void MainWindow::onDeviceConnected(const QString &portName)
{
    m_connectButton->setEnabled(false);
    m_disconnectButton->setEnabled(true);
    m_portComboBox->setEnabled(false);
    m_baudRateComboBox->setEnabled(false);
    m_refreshButton->setEnabled(false);
    
    m_statusLabel->setText("已连接: " + portName);
    m_statusLabel->setStyleSheet("color: green;");
}

void MainWindow::onDeviceDisconnected()
{
    m_connectButton->setEnabled(true);
    m_disconnectButton->setEnabled(false);
    m_portComboBox->setEnabled(true);
    m_baudRateComboBox->setEnabled(true);
    m_refreshButton->setEnabled(true);
    
    m_statusLabel->setText("未连接");
    m_statusLabel->setStyleSheet("color: red;");
}

void MainWindow::onConnectionError(const QString &error)
{
    QMessageBox::critical(this, "连接错误", error);
}

void MainWindow::onDataReceived(const IMUData &data)
{
    m_frameCount++;
    
    m_rollLabel->setText(QString("Roll:  %1°").arg(data.roll, 6, 'f', 2));
    m_pitchLabel->setText(QString("Pitch: %1°").arg(data.pitch, 6, 'f', 2));
    m_yawLabel->setText(QString("Yaw:   %1°").arg(data.yaw, 6, 'f', 2));
    
    m_threeDView->setEulerAngles(data.roll, data.pitch, data.yaw);
    m_attitudeIndicator->setEulerAngles(data.roll, data.pitch, data.yaw);
}

void MainWindow::onExportData()
{
    QString filename = QFileDialog::getSaveFileName(this, "导出数据",
                                                    QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss") + ".csv",
                                                    "CSV文件 (*.csv)");
    if (!filename.isEmpty()) {
        if (m_deviceManager->getDataBuffer()->exportToCSV(filename)) {
            QMessageBox::information(this, "成功", "数据导出成功!");
        } else {
            QMessageBox::critical(this, "错误", "数据导出失败!");
        }
    }
}

void MainWindow::onImportData()
{
    QString filename = QFileDialog::getOpenFileName(this, "导入数据", "", "CSV文件 (*.csv)");
    if (!filename.isEmpty()) {
        if (m_deviceManager->getDataBuffer()->importFromCSV(filename)) {
            QMessageBox::information(this, "成功", "数据导入成功!");
        } else {
            QMessageBox::critical(this, "错误", "数据导入失败!");
        }
    }
}

void MainWindow::onClearData()
{
    m_deviceManager->clearBuffer();
    m_frameCount = 0;
}

void MainWindow::onModelChanged(int index)
{
    ThreeDView::ModelType type = static_cast<ThreeDView::ModelType>(index);
    m_threeDView->setModelType(type);
}

void MainWindow::onTimeWindowChanged(int index)
{
    int windows[] = {5, 10, 30, 60};
    m_waveformPlot->setTimeWindow(windows[index]);
}

void MainWindow::updateStatus()
{
    quint32 fps = m_frameCount - m_lastFrameCount;
    m_lastFrameCount = m_frameCount;
    
    m_fpsLabel->setText(QString("FPS: %1").arg(fps));
    m_dataCountLabel->setText(QString("数据: %1").arg(m_deviceManager->getDataBuffer()->size()));
}
