#!/usr/bin/env pwsh

New-Item -ItemType Directory build/ -Force | Out-Null

$out = Join-Path build ("main" + ($IsWindows ? ".exe" : ""))

# L'accesso alla porta seriale è implementato separatamente per piattaforma
# (Win32 vs POSIX/termios): qui si sceglie quale dei due compilare.
# Il percorso/nome della porta usata a runtime si imposta con la variabile
# d'ambiente SERIALX_TEST_PORT (es. SERIALX_TEST_PORT=COM4 su Windows,
# SERIALX_TEST_PORT=/dev/pts/4 su Linux), letta da tests/emulator/serial_port_*.cpp.
$serialImpl = $IsWindows `
    ? "tests/emulator/serial_port_win.cpp" `
    : "tests/emulator/serial_port_posix.cpp"

g++ -std=c++17 -Isrc -Itests/emulator -Itests/sketch `
    tests/runner/main.cpp `
    tests/sketch/sketch.ino.cpp `
    tests/emulator/Arduino.cpp `
    $serialImpl `
    src/SerialXShell.cpp `
    -o $out -static -static-libstdc++

if ($LASTEXITCODE -eq 0) {
    Write-Host "✅ Ok: $out" -ForegroundColor Green
} else {
    Write-Warning "❌ Errore"
}
