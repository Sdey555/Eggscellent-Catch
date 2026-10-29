param(
    [string]$Compiler = 'g++',
    [string]$OutputPath = (Join-Path $PSScriptRoot '..\EggscellentCatch-Portable.exe')
)

$ErrorActionPreference = 'Stop'
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$releaseDirectory = Join-Path $projectRoot 'release'
$launcherSource = Join-Path $PSScriptRoot 'portable_launcher.cpp'
$outputFile = [System.IO.Path]::GetFullPath($OutputPath)

if (-not (Test-Path -LiteralPath (Join-Path $releaseDirectory 'EggscellentCatch.exe'))) {
    throw "Release build not found at '$releaseDirectory'. Build the Qt release version first."
}

$compilerCommand = Get-Command $Compiler -ErrorAction Stop
$workDirectory = Join-Path ([System.IO.Path]::GetTempPath()) (
    'EggscellentCatch-package-' + [guid]::NewGuid().ToString('N')
)
$stageDirectory = Join-Path $workDirectory 'app'
$archivePath = Join-Path $workDirectory 'game.zip'
$launcherPath = Join-Path $workDirectory 'launcher.exe'
$null = New-Item -ItemType Directory -Path $stageDirectory -Force

try {
    Get-ChildItem -LiteralPath $releaseDirectory -File -Recurse |
        Where-Object { $_.Extension -in '.dll', '.exe', '.qm' } |
        ForEach-Object {
            $relativePath = $_.FullName.Substring($releaseDirectory.Length).TrimStart('\')
            $destination = Join-Path $stageDirectory $relativePath
            $destinationDirectory = Split-Path -Parent $destination
            $null = New-Item -ItemType Directory -Path $destinationDirectory -Force
            Copy-Item -LiteralPath $_.FullName -Destination $destination
        }

    Push-Location $workDirectory
    try {
        & $compilerCommand.Source -std=c++17 -O2 -static -s -mwindows -municode `
            $launcherSource -o $launcherPath -luser32
    }
    finally {
        Pop-Location
    }
    if ($LASTEXITCODE -ne 0) {
        throw "The portable launcher build failed with exit code $LASTEXITCODE."
    }

    Compress-Archive -Path (Join-Path $stageDirectory '*') `
        -DestinationPath $archivePath -CompressionLevel Optimal

    $outputDirectory = Split-Path -Parent $outputFile
    $null = New-Item -ItemType Directory -Path $outputDirectory -Force
    $outputStream = [System.IO.File]::Create($outputFile)
    try {
        $launcherStream = [System.IO.File]::OpenRead($launcherPath)
        try {
            $launcherStream.CopyTo($outputStream)
        }
        finally {
            $launcherStream.Dispose()
        }

        $archiveStream = [System.IO.File]::OpenRead($archivePath)
        try {
            $archiveStream.CopyTo($outputStream)
            $magic = [System.Text.Encoding]::ASCII.GetBytes('EGGPKG01')
            $archiveSize = [BitConverter]::GetBytes([UInt64]$archiveStream.Length)
            $outputStream.Write($magic, 0, $magic.Length)
            $outputStream.Write($archiveSize, 0, $archiveSize.Length)
        }
        finally {
            $archiveStream.Dispose()
        }
    }
    finally {
        $outputStream.Dispose()
    }

    $sizeInMegabytes = [math]::Round((Get-Item -LiteralPath $outputFile).Length / 1MB, 1)
    Write-Host "Created $outputFile ($sizeInMegabytes MB)"
}
finally {
    Remove-Item -LiteralPath $workDirectory -Recurse -Force -ErrorAction SilentlyContinue
}
