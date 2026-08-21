cd ..
$srcFiles = Get-ChildItem -Recurse -Filter *.cpp -Path src | ForEach-Object { $_.FullName }
New-Item -ItemType Directory -Force -Path build/examples | Out-Null
$outName = "build/examples/SerialXNetExample" + ($IsWindows ? ".exe" : "")
g++ -Wall -std=c++11 -Isrc -Isrc/net net_examples/SerialXNetCompleteEsample.cpp $srcFiles -o $outName
