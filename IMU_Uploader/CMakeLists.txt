cmake_minimum_required(VERSION 3.16)

project(IMU_Uploader VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS 
    Core 
    Gui 
    Widgets 
    SerialPort 
    OpenGL 
    Network
)

set(SOURCES
    main.cpp
    src/serial/SerialPort.cpp
    src/serial/ProtocolParser.cpp
    src/visualization/ThreeDView.cpp
    src/visualization/AttitudeIndicator.cpp
    src/visualization/WaveformPlot.cpp
    src/data/DataBuffer.cpp
    src/data/Statistics.cpp
    src/data/FFTAnalyzer.cpp
    src/config/ConfigManager.cpp
    src/config/DeviceManager.cpp
    src/utils/CRC.cpp
    src/utils/Logger.cpp
    src/mainwindow.cpp
)

set(HEADERS
    src/serial/SerialPort.h
    src/serial/ProtocolParser.h
    src/visualization/ThreeDView.h
    src/visualization/AttitudeIndicator.h
    src/visualization/WaveformPlot.h
    src/data/DataBuffer.h
    src/data/Statistics.h
    src/data/FFTAnalyzer.h
    src/config/ConfigManager.h
    src/config/DeviceManager.h
    src/utils/CRC.h
    src/utils/Logger.h
    src/mainwindow.h
)

set(FORMS
    src/mainwindow.ui
)

set(RESOURCES
    resources/resources.qrc
)

add_executable(IMU_Uploader
    ${SOURCES}
    ${HEADERS}
    ${FORMS}
    ${RESOURCES}
)

target_link_libraries(IMU_Uploader PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::SerialPort
    Qt6::OpenGL
    Qt6::Network
)

if(WIN32)
    set_target_properties(IMU_Uploader PROPERTIES
        WIN32_EXECUTABLE TRUE
        RC_ICONS "resources/icons/app_icon.ico"
    )
endif()

install(TARGETS IMU_Uploader
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
