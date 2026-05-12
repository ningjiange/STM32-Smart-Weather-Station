# 修复 freertos.c 中的头文件 include 顺序问题
# CubeMX 重新生成后，stm32f1xx_hal_xxx.h 会放在 main.h 之前，导致 HAL_StatusTypeDef 未定义
# 本脚本自动把这些 include 移到 USER CODE 区域

$projectRoot = $PSScriptRoot
while (-not (Test-Path (Join-Path $projectRoot "Core\Src\freertos.c"))) {
    $parentDir = Split-Path -Parent $projectRoot
    if ($parentDir -eq $projectRoot) { $projectRoot = Get-Location; break }
    $projectRoot = $parentDir
}

$freertosFile = Join-Path $projectRoot "Core\Src\freertos.c"

if (-not (Test-Path $freertosFile)) {
    Write-Error "Cannot find: $freertosFile"
    exit 1
}

$content = Get-Content $freertosFile -Raw

# 需要从自动生成区移除的 include 模式
$patterns = @(
    '#include\s+"stm32f1xx_hal_gpio\.h"',
    '#include\s+"stm32f1xx_hal_uart\.h"',
    '#include\s+"stm32f103xb\.h"',
    '#include\s+"stm32f1xx_hal\.h"'
)

$found = @()

foreach ($pattern in $patterns) {
    if ($content -match $pattern) {
        # 只匹配自动生成区（USER CODE BEGIN Includes 之前）的
        $autoGenSection = $content -split '/\*\s*USER CODE BEGIN Includes'
        if ($autoGenSection.Count -gt 1 -and $autoGenSection[0] -match $pattern) {
            $found += $matches[0]
        }
    }
}

if ($found.Count -eq 0) {
    Write-Host "No HAL includes to fix. freertos.c is OK." -ForegroundColor Green
    exit 0
}

Write-Host "Found HAL includes in auto-generated section:" -ForegroundColor Yellow
foreach ($f in $found) { Write-Host "  $f" -ForegroundColor Yellow }

# 从自动生成区删除这些 include
foreach ($pattern in $patterns) {
    $content = $content -replace "(?m)^\s*$pattern\s*\r?\n", ""
}

# 清理多余的空行（在 /* Includes -----*/ 和 /* USER CODE BEGIN Includes */ 之间）
$content = $content -replace "(/\*-+\*/\r?\n)\r?\n+(/\* USER CODE BEGIN Includes)", '$1$2'

Set-Content $freertosFile $content -NoNewline -Encoding UTF8

Write-Host ""
Write-Host "Fixed! Removed HAL includes from auto-generated section." -ForegroundColor Green
Write-Host "These includes are not needed because main.h already provides them." -ForegroundColor Green
