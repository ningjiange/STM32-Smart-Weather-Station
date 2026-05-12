import re, os, sys

def find_project_root(script_dir):
    """向上查找包含 Core/ 目录的项目根目录"""
    d = os.path.dirname(script_dir)
    while d and d != os.path.dirname(d):
        if os.path.isdir(os.path.join(d, 'Core')):
            return d
        d = os.path.dirname(d)
    return script_dir


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

    with open(freertos_file, 'r', encoding='utf-8-sig') as f:
        content = f.read()

    patterns = [
        r'#include\s+"stm32f1xx_hal_gpio\.h"',
        r'#include\s+"stm32f1xx_hal_uart\.h"',
        r'#include\s+"stm32f103xb\.h"',
        r'#include\s+"stm32f1xx_hal\.h"'
    ]

    # 只匹配自动生成区（USER CODE BEGIN Includes 之前）的 include
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

    # 需要补回的宏：(宏名, 值, 在哪个宏后面插入)
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
    script_dir = os.path.dirname(os.path.abspath(__file__))
    root = find_project_root(script_dir)
    print('Project Root:', root)
    print()

    fix_cmake(root)
    fix_freertos_c(root)
    fix_freertos_config(root)

    print()
    print('Done! You can build now.')
