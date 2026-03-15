# 构建说明

## Windows平台

### 方法1: 使用Qt Creator (推荐)
1. 安装 Qt 5.15+ 或 Qt 6.x
2. 打开 Qt Creator
3. 文件 -> 打开文件或项目
4. 选择 `IMU_Uploader.pro`
5. 配置编译套件
6. 点击 "构建" -> "构建项目 IMU_Uploader"
7. 点击 "运行" 测试

### 方法2: 使用CMake
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### 生成独立exe文件
1. 编译Release版本
2. 在构建目录中找到 `IMU_Uploader.exe`
3. 使用 `windeployqt` 工具:
```bash
cd build/Release
windeployqt IMU_Uploader.exe
```
4. 将整个文件夹打包分发

## Linux平台

```bash
mkdir build
cd build
cmake ..
make
```

## macOS平台

```bash
mkdir build
cd build
cmake ..
make
macdeployqt IMU_Uploader.app
```

## 依赖项

- Qt 5.15+ 或 Qt 6.x
- C++17 编译器
- OpenGL 支持

## 常见问题

### 1. 找不到Qt
确保Qt安装路径已添加到PATH环境变量

### 2. OpenGL错误
更新显卡驱动程序

### 3. 串口无法打开
检查串口权限(Linux)或串口占用(Windows)
