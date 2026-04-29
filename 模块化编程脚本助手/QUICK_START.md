# 快速开始指南

## 🚀 一行命令快速修复 CMakeLists.txt

STM32CubeMX 重新生成代码后，只需运行一行命令就能恢复所有自定义模块：

### PowerShell 命令（推荐）

```powershell
cd D:\STM32CubeFile\按键交通路口信号灯报警模块; powershell -ExecutionPolicy Bypass -File fix_cmake_user.ps1
```

### 或者更简单的方式 - 双击启动

直接双击项目目录中的 **`fix_cmake_user.bat`** 文件

---

## 📋 工作流程

### 步骤 1️⃣：创建或修改你的模块

```
User/
├── Inc/
│   ├── key.h
│   ├── pwm.h
│   ├── oled.h
│   └── led_ctrl.h
└── Src/
    ├── key.c
    ├── pwm.c
    ├── oled.c
    └── led_ctrl.c
```

### 步骤 2️⃣：在 STM32CubeMX 中重新生成代码

- 打开 `.ioc` 文件
- 做出必要的配置更改
- 点击"Generate Code"按钮

### 步骤 3️⃣：修复 CMakeLists.txt

执行以下任一方式：

**方式 A - 直接双击（最简单）**

```
📁 项目文件夹
 └─ 📄 fix_cmake_user.bat  ← 直接双击这个文件
```

**方式 B - PowerShell 终端（在 VS Code 中）**

1. 在 VS Code 中打开集成终端（Ctrl + `）
2. 输入：

```powershell
powershell -ExecutionPolicy Bypass -File fix_cmake_user.ps1
```

3. 按 Enter 执行

**方式 C - Windows PowerShell 独立运行**

1. Win + R 打开"运行"对话框
2. 输入 `powershell`
3. 导航到项目目录
4. 输入命令并执行

### 步骤 4️⃣：重新构建项目

```powershell
cmake --build build/Debug
```

---

## ✨ 脚本做了什么

脚本自动：

- ✅ 扫描 `User/Src` 中的所有 `.c` 文件
- ✅ 将它们添加到 CMakeLists.txt 中
- ✅ 确保 `User/Inc` 包含路径已添加
- ✅ 避免重复添加相同的文件
- ✅ 提供清晰的操作日志

---

## 📝 当前配置的模块

| 模块名 | 源文件     | 头文件     | 功能            |
| ------ | ---------- | ---------- | --------------- |
| Key    | key.c      | key.h      | 按键输入处理    |
| PWM    | pwm.c      | pwm.h      | PWM 信号控制    |
| OLED   | oled.c     | oled.h     | OLED 显示屏驱动 |
| LED    | led_ctrl.c | led_ctrl.h | LED 控制        |

---

## 🔧 添加新模块

要添加一个新模块（例如 UART 通讯）：

1. 在 `User/Src` 中创建 `uart.c`
2. 在 `User/Inc` 中创建 `uart.h`
3. 运行脚本：`fix_cmake_user.bat` 或 PowerShell 命令
4. 新模块自动被添加到编译列表中

---

## ⚠️ 常见问题

**Q: 脚本无法运行，提示"无法加载文件"？**

```
A: 这是 Windows 执行策略问题。使用 fix_cmake_user.bat 文件可以绕过这个问题。
   或者在 PowerShell 中使用完整命令：
   powershell -ExecutionPolicy Bypass -File fix_cmake_user.ps1
```

**Q: 修改了源代码后需要重新运行脚本吗？**

```
A: 不需要。只有当：
   - 添加了新的 .c 文件到 User/Src
   - STM32CubeMX 重新生成了代码
   才需要重新运行脚本。
```

**Q: 如何删除某个模块？**

```
A:
   1. 删除对应的 .c 和 .h 文件
   2. 运行脚本（脚本不会添加不存在的文件）
   3. 重新编译
```

---

## 💡 提示

- 脚本是幂等的，运行多次不会产生重复
- 脚本采用 UTF-8 编码，完全支持中文路径
- 所有操作都有日志输出，便于调试

---

**有问题？** 检查 [README_CMAKE_SETUP.md](README_CMAKE_SETUP.md) 获取更详细的说明。
