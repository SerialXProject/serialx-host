#!/usr/bin/env pwsh

mkdir build -Force | Out-Null

$out = Join-Path build ("main" + ($IsWindows ? ".exe" : ""))

g++ -std=c++17 -Isrc -Itests tests/main.cpp tests/Arduino.cpp src/SerialXShell.cpp -o $out -static -static-libstdc++

if ($LASTEXITCODE -eq 0) {
    Write-Host "✅ Ok: $out" -ForegroundColor Green
} else {
    Write-Warning "❌ Errore"
}
