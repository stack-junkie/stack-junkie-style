# Builds the stub-guard sample in Release twice against the installed StackJunkie package:
# with its Stubs the build must fail and name each Stub's file and line; without them it must pass.
param([string]$QtPrefix = $env:CMAKE_PREFIX_PATH)
$ErrorActionPreference = 'Stop'
$sampleRoot = $PSScriptRoot
$nativeRoot = Split-Path (Split-Path $sampleRoot -Parent) -Parent
if (-not $QtPrefix) { $QtPrefix = Join-Path (Split-Path $nativeRoot -Parent) '.tools\Qt\6.8.3\msvc2022_64' }
if (-not (Test-Path -LiteralPath (Join-Path $QtPrefix 'lib\cmake\Qt6\Qt6Config.cmake'))) { throw 'Pass -QtPrefix with the path to a Qt 6 MSVC x64 installation.' }
$cmakeCommand = Get-Command cmake -ErrorAction SilentlyContinue
$cmake = if ($cmakeCommand) { $cmakeCommand.Source } else { $null }
if (-not $cmake) {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path -LiteralPath $vswhere) {
        $vsPath = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        if ($vsPath) { $cmake = Join-Path $vsPath 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' }
    }
}
if (-not $cmake -or -not (Test-Path -LiteralPath $cmake)) { throw 'CMake 3.21+ and Visual Studio 2022 C++ build tools are required.' }

# Short folder names keep MSBuild tracking paths under the Windows path limit.
$work = Join-Path $nativeRoot 'build-stub'
$package = Join-Path $work 'pkg'
& $cmake -S $nativeRoot -B (Join-Path $work 'lib') -G 'Visual Studio 17 2022' -A x64 "-DCMAKE_PREFIX_PATH=$QtPrefix" -DSTACKJUNKIE_BUILD_SHOWCASE=OFF | Out-Null
if ($LASTEXITCODE -ne 0) { throw 'Library configure failed.' }
& $cmake --build (Join-Path $work 'lib') --config Release --parallel 2 -- /nodeReuse:false | Out-Null
if ($LASTEXITCODE -ne 0) { throw 'Library build failed.' }
& $cmake --install (Join-Path $work 'lib') --config Release --prefix $package | Out-Null
if ($LASTEXITCODE -ne 0) { throw 'Library install failed.' }

function Build-Sample([bool]$withStub) {
    $dir = Join-Path $work ($(if ($withStub) { 'on' } else { 'off' }))
    $flag = if ($withStub) { 'ON' } else { 'OFF' }
    & $cmake -S $sampleRoot -B $dir -G 'Visual Studio 17 2022' -A x64 "-DCMAKE_PREFIX_PATH=$package;$QtPrefix" "-DSTUB_GUARD_WITH_STUB=$flag" | Out-Null
    if ($LASTEXITCODE -ne 0) { throw 'Sample configure failed.' }
    $ErrorActionPreference = 'Continue'
    $output = & $cmake --build $dir --config Release -- /nodeReuse:false 2>&1 | Out-String
    return @{ Code = $LASTEXITCODE; Output = $output }
}

$failed = $false
$stubLines = Select-String -LiteralPath (Join-Path $sampleRoot 'main.cpp') -Pattern 'STACKJUNKIE_STUB\(' | ForEach-Object LineNumber
$with = Build-Sample $true
$named = @($stubLines | Where-Object { $with.Output -match ('main\.cpp\(' + $_ + '[,)].*Stub remains') })
if ($with.Code -ne 0 -and $named.Count -eq $stubLines.Count) {
    Write-Output "PASS: Release build with Stubs failed and named each one:"
    $with.Output -split "`r?`n" | Where-Object { $_ -match 'Stub remains' } | ForEach-Object { '  ' + $_.Trim() } | Select-Object -Unique
} else {
    $failed = $true
    Write-Output "FAIL: Release build with Stubs exited $($with.Code) and named $($named.Count) of $($stubLines.Count) Stubs."
    Write-Output $with.Output
}
$without = Build-Sample $false
if ($without.Code -eq 0) { Write-Output 'PASS: Release build without Stubs succeeded.' }
else { $failed = $true; Write-Output "FAIL: Release build without Stubs exited $($without.Code)."; Write-Output $without.Output }
if ($failed) { exit 1 }
