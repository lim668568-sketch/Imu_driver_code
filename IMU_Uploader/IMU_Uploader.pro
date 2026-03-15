QT += core gui widgets serialport opengl network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = IMU_Uploader
TEMPLATE = app

SOURCES += \
    main.cpp \
    src/serial/SerialPort.cpp \
    src/serial/ProtocolParser.cpp \
    src/visualization/ThreeDView.cpp \
    src/visualization/AttitudeIndicator.cpp \
    src/visualization/WaveformPlot.cpp \
    src/data/DataBuffer.cpp \
    src/data/Statistics.cpp \
    src/data/FFTAnalyzer.cpp \
    src/config/ConfigManager.cpp \
    src/config/DeviceManager.cpp \
    src/utils/CRC.cpp \
    src/utils/Logger.cpp \
    src/mainwindow.cpp

HEADERS += \
    src/serial/SerialPort.h \
    src/serial/ProtocolParser.h \
    src/visualization/ThreeDView.h \
    src/visualization/AttitudeIndicator.h \
    src/visualization/WaveformPlot.h \
    src/data/DataBuffer.h \
    src/data/Statistics.h \
    src/data/FFTAnalyzer.h \
    src/config/ConfigManager.h \
    src/config/DeviceManager.h \
    src/utils/CRC.h \
    src/utils/Logger.h \
    src/mainwindow.h

FORMS += \
    src/mainwindow.ui

RESOURCES += \
    resources/resources.qrc

RC_ICONS = resources/icons/app_icon.ico

QMAKE_LFLAGS += -Wl,--enable-stdcall-fixup
