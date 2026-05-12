# 一键修复脚本 v3
# 调用 Python 完成实际修复工作
# 功能：1. 修复 CMakeLists.txt（添加 User/Src 和 User/Inc）
#       2. 修复 freertos.c include 顺序
#       3. 补回 FreeRTOSConfig.h 被覆盖的宏

$projectRoot = Split-Path -Parent $PSScriptRoot
if (-not (Test-Path (Join-Path $projectRoot "Core"))) {
    $projectRoot = $PSScriptRoot
    while ($projectRoot -and -not (Test-Path (Join-Path $projectRoot "Core"))) {
        $parentDir = Split-Path -Parent $projectRoot
        if ($parentDir -eq $projectRoot) { break }
        $projectRoot = $parentDir
    }
}

Write-Host "Project Root: $projectRoot" -ForegroundColor Gray
Write-Host ""

# 生成 Python 修复脚本并执行
$pyScript = @'
import re, os, sys

def fix_cmake(root):
    cmake_file = os.path.join(root, 'cmake', 'stm32cubemx', 'CMakeLists.txt')
    user_src_dir = os.path.join(root, 'User', 'Src')
    if not os.path.exists(cmake_file):
        print('[1/3] CMakeLists.txt not found, skip')
        return

    with open(cmake_file, 'r') as f:
        content = f.read()

    modified = False

    # 1. 添加 User/Inc 到 include 路径
    if 'User/Inc' not in content:
        content = re.sub(
            r'(set\(MX_Include_Dirs.*?)(\))',
            r'\1\n    ${CMAKE_CURRENT_SOURCE_DIR}/../../User/Inc\2',
            content, count=1, flags=re.DOTALL
        )
        print('  Added: User/Inc to include paths')
        modified = True

    # 2. 添加 User/Src 下的 .c 文件
    if os.path.isdir(user_src_dir):
        c_files = sorted([f for f in os.listdir(user_src_dir) if f.endswith('.c')])
        for c_file in c_files:
            if c_file not in content:
                source_line = '${CMAKE_CURRENT_SOURCE_DIR}/../../User/Src/' + c_file
                content = re.sub(
                    r'(set\(MX_Application_Src.*?)(\))',
                    r'\1\n    ' + source_line + r'\2',
                    content, count=1, flags=re.DOTALL
                )
                print('  Added: ' + c_file + ' to MX_Application_Src')
                modified = True

    if modified:
        with open(cmake_file, 'w') as f:
            f.write(content)
        print('[1/3] CMakeLists.txt fixed')
    else:
        print('[1/3] CMakeLists.txt OK, no fix needed')


def fix_freertos_c(root):
    freertos_file = os.path.join(root, 'Core', 'Src', 'freertos.c')
    if not os.path.exists(freertos_file):
        print('[2/3] freertos.c not found, skip')
        return

    with open(freertos_file, 'r') as f:
        content = f.read()

    patterns = [
        r'#include\s+"stm32f1xx_hal_gpio\.h"',
        r'#include\s+"stm32f1xx_hal_uart\.h"',
        r'#include\s+"stm32f103xb\.h"',
        r'#include\s+"stm32f1xx_hal\.h"'
    ]

    found = []
    auto_gen = content.split('/* USER CODE BEGIN Includes')
    if len(auto_gen) > 1:
        for p in patterns:
            if re.search(p, auto_gen[0]):
                found.append(p)

    if found:
        for p in patterns:
            content = re.sub(r'(?m)^\s*' + p + r'\s*\r?\n', '', content)
        content = re.sub(r'(/\*-+\*/\r?\n)\r?\n+(/\* USER CODE BEGIN Includes)', r'\1\2', content)
        with open(freertos_file, 'w') as f:
            f.write(content)
        print('[2/3] freertos.c includes fixed')
    else:
        print('[2/3] freertos.c includes OK, no fix needed')


def fix_freertos_config(root):
    config_file = os.path.join(root, 'Core', 'Inc', 'FreeRTOSConfig.h')
    if not os.path.exists(config_file):
        print('[3/3] FreeRTOSConfig.h not found, skip')
        return

    with open(config_file, 'r') as f:
        content = f.read()

    macros = [
        ('configUSE_STATS_FORMATTING_FUNCTIONS', '1', 'configUSE_TRACE_FACILITY'),
        ('INCLUDE_vTaskList', '1', 'INCLUDE_eTaskGetState'),
    ]

    modified = False
    for name, value, after in macros:
        if not re.search(r'#define\s+' + name + r'\s+', content):
            after_pattern = r'(#define\s+' + after + r'\s+.*)\r?\n'
            replacement = r'\1\n#define ' + name + '\t\t\t\t' + value + '\r\n'
            content = re.sub(after_pattern, replacement, content, count=1)
            print('  Added: ' + name)
            modified = True

    if modified:
        with open(config_file, 'w') as f:
            f.write(content)
        print('[3/3] FreeRTOSConfig.h macros fixed')
    else:
        print('[3/3] FreeRTOSConfig.h OK, no fix needed')


if __name__ == '__main__':
    root = r'''ROOT_DIR'''
    fix_cmake(root)
    fix_freertos_c(root)
    fix_freertos_config(root)
    print()
    print('Done! You can build now.')
'@

# 写入临时 Python 文件（替换 ROOT_DIR 占位符）
$pyScript = $pyScript.Replace('ROOT_DIR', $projectRoot)
$tempPy = Join-Path $env:TEMP "fix_all_project.py"
Set-Content $tempPy $pyScript -Encoding UTF8
python $tempPy
Remove-Item $tempPy -Force
