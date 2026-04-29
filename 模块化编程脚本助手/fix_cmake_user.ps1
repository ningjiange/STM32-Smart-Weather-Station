# STM32 CMakeLists.txt 自动配置脚本
# 功能：自动添加User/Src下的所有.c文件到CMakeLists.txt中

# 确定项目根目录
# 如果脚本在子文件夹中（如"模块化编程脚本助手"），则向上一级查找
$projectRoot = $PSScriptRoot

# 检查是否需要向上级查找项目根目录
while (-not (Test-Path (Join-Path $projectRoot "cmake\stm32cubemx\CMakeLists.txt")) -and 
      -not (Test-Path (Join-Path $projectRoot "CMakeLists.txt"))) {
    $parentDir = Split-Path -Parent $projectRoot
    if ($parentDir -eq $projectRoot) {
        # 已经到达根目录，还未找到，尝试当前工作目录
        $projectRoot = Get-Location
        break
    }
    $projectRoot = $parentDir
}

$cmakeFile = Join-Path $projectRoot "cmake\stm32cubemx\CMakeLists.txt"
$userSrcDir = Join-Path $projectRoot "User\Src"
$userIncPath = '    ${CMAKE_CURRENT_SOURCE_DIR}/../../User/Inc'

Write-Host "Project Root: $projectRoot" -ForegroundColor Gray
Write-Host ""

# 检查文件和目录
if (-not (Test-Path $cmakeFile)) {
    Write-Error "Cannot find: $cmakeFile"
    Write-Error "Project root detection failed. Please ensure the script is run from the project directory."
    exit 1
}

if (-not (Test-Path $userSrcDir)) {
    Write-Error "Cannot find: $userSrcDir"
    Write-Error "Project root detection failed. Please ensure the script is run from the project directory."
    exit 1
}

# 读取CMakeLists.txt内容
$content = Get-Content $cmakeFile

# 第一步：确保User/Inc在MX_Include_Dirs中
Write-Host "Step 1: Checking include paths..."
$newContent = @()
$userIncAdded = $false
$inIncludeDirs = $false

foreach ($line in $content) {
    if ($line -match "^set\(MX_Include_Dirs") {
        $inIncludeDirs = $true
    }
    
    if ($inIncludeDirs -and $line -match "^\)") {
        if (-not $userIncAdded) {
            $newContent += $userIncPath
            Write-Host "✓ Added User/Inc to include paths"
            $userIncAdded = $true
        }
        $inIncludeDirs = $false
    }
    
    if ($line -eq $userIncPath) {
        $userIncAdded = $true
    }
    
    $newContent += $line
}

$content = $newContent

# 第二步：添加User/Src下的所有.c文件
Write-Host "Step 2: Scanning User/Src directory..."
$cFiles = Get-ChildItem $userSrcDir -Filter "*.c" | Sort-Object Name

if ($cFiles.Count -eq 0) {
    Write-Warning "No .c files found in User/Src"
} else {
    Write-Host "Found $($cFiles.Count) source file(s): $($cFiles.Name -join ', ')"
}

# 获取需要添加的源文件列表
$newContent = @()
$sourcesAdded = @()
$inApplicationSrc = $false

foreach ($line in $content) {
    if ($line -match "^set\(MX_Application_Src") {
        $inApplicationSrc = $true
    }
    
    if ($inApplicationSrc -and $line -match "^\)") {
        # 找到MX_Application_Src结束位置，插入User源文件
        foreach ($cFile in $cFiles) {
            $sourcePathLine = "    `${CMAKE_CURRENT_SOURCE_DIR}/../../User/Src/$($cFile.Name)"
            
            # 检查是否已经存在
            if (-not ($content -contains $sourcePathLine)) {
                $newContent += $sourcePathLine
                $sourcesAdded += $cFile.Name
            }
        }
        $inApplicationSrc = $false
    }
    
    $newContent += $line
}

# 输出添加结果
if ($sourcesAdded.Count -gt 0) {
    Write-Host "✓ Added source files: $($sourcesAdded -join ', ')"
} else {
    Write-Host "ℹ️  All User source files already exist in CMakeLists.txt"
}

# 写入更新后的内容
Set-Content $cmakeFile $newContent -Encoding UTF8

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "CMakeLists.txt has been successfully fixed!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Configuration summary:"
Write-Host "  - Include path: User/Inc [OK]"
Write-Host "  - Source files: $($cFiles.Count) files [OK]"
Write-Host ""