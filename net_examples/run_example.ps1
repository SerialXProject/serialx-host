cd ..
$srcFiles = Get-ChildItem -Recurse -Filter *.cpp -Path src | ForEach-Object { $_.FullName }
New-Item -ItemType Directory -Force -Path build/examples | Out-Null
$outName = "build/examples/SerialXNetExample" + ($IsWindows ? ".exe" : "")
g++ -Wall -std=c++11 -Isrc -Isrc/net net_examples/SerialXNetCompleteExample.cpp $srcFiles -o $outName

# Check if compilation succeeded before launching
if ($LASTEXITCODE -eq 0) {
    Start-Process $outName -Wait
} else {
    Write-Error "Compilation failed with exit code $LASTEXITCODE"
}
