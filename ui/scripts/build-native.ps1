param(
    [string]$QtPrefix = $env:CMAKE_PREFIX_PATH,
    [ValidateSet('Debug', 'Release')][string]$Configuration = 'Release'
)
$ErrorActionPreference = 'Stop'
$uiRoot = Split-Path $PSScriptRoot -Parent
if (-not $QtPrefix) { $QtPrefix = Join-Path $uiRoot '.tools\Qt\6.8.3\msvc2022_64' }
if (-not (Test-Path -LiteralPath (Join-Path $QtPrefix 'lib\cmake\Qt6\Qt6Config.cmake'))) {
    throw 'Pass -QtPrefix with the path to a Qt 6 MSVC x64 installation containing Widgets and Svg.'
}
$cmakeCommand = Get-Command cmake -ErrorAction SilentlyContinue
$cmakePath = if ($cmakeCommand) { $cmakeCommand.Source } else { $null }
if (-not $cmakePath) {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path -LiteralPath $vswhere) {
        $vsPath = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        if ($vsPath) { $cmakePath = Join-Path $vsPath 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' }
    }
}
if (-not $cmakePath -or -not (Test-Path -LiteralPath $cmakePath)) { throw 'CMake 3.21+ and Visual Studio 2022 C++ build tools are required.' }
$buildRoot = Join-Path $uiRoot 'native\build-windows'
$packageRoot = Join-Path $uiRoot 'dist\windows'
& $cmakePath -S (Join-Path $uiRoot 'native') -B $buildRoot -G 'Visual Studio 17 2022' -A x64 "-DCMAKE_PREFIX_PATH=$QtPrefix"
if ($LASTEXITCODE -ne 0) { throw 'Native configure failed.' }
& $cmakePath --build $buildRoot --config $Configuration --parallel 2 -- /nodeReuse:false
if ($LASTEXITCODE -ne 0) { throw 'Native build failed.' }
& $cmakePath --install $buildRoot --config $Configuration --prefix $packageRoot
if ($LASTEXITCODE -ne 0) { throw 'Native package installation failed.' }
$deployQt = Join-Path $QtPrefix 'bin\windeployqt.exe'
$qtMode = if ($Configuration -eq 'Debug') { '--debug' } else { '--release' }
& $deployQt $qtMode --no-translations --no-compiler-runtime (Join-Path $packageRoot 'bin\StackJunkieShowcase.exe')
if ($LASTEXITCODE -ne 0) { throw 'Qt runtime packaging failed.' }
$offscreenPlugin = if ($Configuration -eq 'Debug') { 'qoffscreend.dll' } else { 'qoffscreen.dll' }
Copy-Item -LiteralPath (Join-Path $QtPrefix "plugins\platforms\$offscreenPlugin") -Destination (Join-Path $packageRoot "bin\platforms\$offscreenPlugin")
Write-Output "Native showcase: $(Join-Path $packageRoot 'bin\StackJunkieShowcase.exe')"
