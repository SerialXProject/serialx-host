#!/usr/bin/env pwsh

# Imposta la directory di lavoro sulla cartella in cui risiede lo script
Set-Location $PSScriptRoot

$buildDir = Join-Path $PSScriptRoot "build"
New-Item -ItemType Directory -Path $buildDir -Force | Out-Null

$out = Join-Path $buildDir ("main" + ($IsWindows ? ".exe" : ""))

# Selezione dell'implementazione seriale specifica per OS
$serialImpl = $IsWindows `
    ? (Join-Path $PSScriptRoot "emulator/serial_port_win.cpp") `
    : (Join-Path $PSScriptRoot "emulator/serial_port_posix.cpp")

# Recupera tutti i file .cpp dentro la cartella src e le sue sottocartelle
$srcPath = Resolve-Path "$PSScriptRoot/../../src"
$srcFiles = (Get-ChildItem -Recurse -Filter *.cpp -Path $srcPath).FullName

# Compilazione g++ con splatting (@srcFiles) per espandere tutti gli argomenti
g++ -std=c++17 -DARDUINO=1 `
    -I"$srcPath" `
    -I"$PSScriptRoot/emulator" `
    -I"$PSScriptRoot/sketch" `
    "$PSScriptRoot/runner/main.cpp" `
    "$PSScriptRoot/sketch/sketch.ino.cpp" `
    "$PSScriptRoot/emulator/Arduino.cpp" `
    "$serialImpl" `
    @srcFiles `
    -o "$out" -static -static-libstdc++

if ($LASTEXITCODE -eq 0) {
    Write-Host "✅ Ok: $out" -ForegroundColor Green
} else {
    Write-Warning "❌ Errore"
}