# STM32 CMake 自动配置脚本使用指南

## 问题说明

每次使用 STM32CubeMX 重新生成代码时，`CMakeLists.txt` 中的自定义模块配置都会被重置，这使得模块化编程变得不方便。

## 解决方案

提供了 `fix_cmake_user.ps1` 脚本，可以自动扫描并添加 `User/Src` 目录下的所有 `.c` 文件到 CMakeLists.txt 中。

## 使用方法

### 方式一：直接在 PowerShell 中运行（推荐）

1. 打开 PowerShell 终端
2. 进入项目目录：
   ```powershell
   cd D:\STM32CubeFile\按键交通路口信号灯报警模块
   ```
3. 运行脚本：
   ```powershell
   powershell -ExecutionPolicy Bypass -File fix_cmake_user.ps1
   ```

### 方式二：使用 VS Code 集成终端

1. 在 VS Code 中打开该项目
2. 打开集成终端（Ctrl + `）
3. 运行命令：
   ```powershell
   powershell -ExecutionPolicy Bypass -File fix_cmake_user.ps1
   ```

### 方式三：创建快捷方式（可选）

在项目根目录创建 `run_cmake_setup.bat` 文件：

```batch
@echo off
cd /d "%~dp0"
powershell -ExecutionPolicy Bypass -File fix_cmake_user.ps1
pause
```

然后直接双击 `run_cmake_setup.bat` 即可运行。

## 工作流程

1. **修改完模块代码后**：
   - 在 `User/Src` 目录中添加新的 `.c` 文件
   - 在 `User/Inc` 目录中添加对应的 `.h` 文件

2. **使用 STM32CubeMX 重新生成代码**：
   - 打开 `.ioc` 文件
   - 点击"生成代码"

3. **恢复你的模块配置**：
   - 运行上述脚本

4. **重新编译项目**：
   ```powershell
   cmake --build build/Debug
   ```

## 脚本功能

该脚本会自动：

- ✓ 添加 `User/Inc` 到包含路径（如果未添加）
- ✓ 扫描 `User/Src` 目录下的所有 `.c` 文件
- ✓ 将它们添加到 `cmake/stm32cubemx/CMakeLists.txt` 的源文件列表中
- ✓ 避免重复添加已存在的文件

## 当前模块

项目中已配置的模块包括：

- `key.c` - 按键处理模块
- `pwm.c` - PWM 控制模块
- `oled.c` - OLED 显示模块
- `led_ctrl.c` - LED 控制模块

## 常见问题

### Q: 脚本报错"找不到文件"

A: 确保在项目根目录运行脚本，脚本需要访问 `cmake/stm32cubemx/CMakeLists.txt` 和 `User/Src` 目录。

### Q: 修改了 User 目录中的文件后需要重新运行脚本吗？

A: 不需要。脚本只需要在 STM32CubeMX 重新生成代码后运行一次。

### Q: 可以删除某个模块吗？

A: 可以。删除对应的 `.c` 和 `.h` 文件，然后重新运行脚本。脚本不会添加不存在的文件。

### Q: 脚本能否自动运行？

A: 可以。将脚本的执行命令添加到构建系统的预构建步骤中（在 CMakeLists.txt 中）。

## 扩展用法

### 添加新模块

要添加新模块（例如 `uart.c`）：

1. 在 `User/Src` 中创建 `uart.c`
2. 在 `User/Inc` 中创建 `uart.h`
3. 运行脚本
4. 重新编译项目

### 自动化集成（可选）

可以在 `CMakeLists.txt` 中添加自定义命令，使脚本在每次构建前自动运行：

```cmake
# 在 add_executable 之前添加
add_custom_target(fix_cmake_user
    COMMAND powershell -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/fix_cmake_user.ps1
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# 使你的目标依赖于此任务
add_dependencies(${CMAKE_PROJECT_NAME} fix_cmake_user)
```

## 技术细节

- **脚本语言**：PowerShell 5.1+
- **目标文件**：`cmake/stm32cubemx/CMakeLists.txt`
- **修改位置**：
  - `MX_Include_Dirs` 变量
  - `MX_Application_Src` 变量

## 备注

- 脚本采用 UTF-8 编码以支持中文路径
- 已测试支持 Windows PowerShell 和 PowerShell Core
- 脚本是幂等的（多次运行不会产生重复内容）
