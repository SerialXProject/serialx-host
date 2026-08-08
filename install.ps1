param(
    [string]$SourcePath = "."
)

Write-Host "Installazione SerialX..."

# Normalizza il percorso sorgente
$SourcePath = (Resolve-Path $SourcePath).Path

# Determina la cartella Arduino
if ($IsWindows) {
    $arduinoPath = Join-Path $env:USERPROFILE "Documents\Arduino\libraries"
} else {
    $arduinoPath = Join-Path $HOME "Arduino/libraries"
}

# Crea la cartella se non esiste
if (!(Test-Path $arduinoPath)) {
    New-Item -ItemType Directory -Path $arduinoPath -Force | Out-Null
}

# Nome libreria
$libraryName = "SerialX"
$destination = Join-Path $arduinoPath $libraryName

# Rimuove versione precedente
if (Test-Path $destination) {
    Write-Host "Rimozione versione esistente..."
    Remove-Item $destination -Recurse -Force
}

# Crea cartella destinazione
New-Item -ItemType Directory -Path $destination -Force | Out-Null

# Copia libreria ESCLUDENDO .git
Write-Host "Copia file..."

Get-ChildItem $SourcePath -Recurse -Force |
Where-Object { $_.FullName -notmatch "\\.git(\\|$)" } |
ForEach-Object {

    # Calcola percorso relativo
    $relativePath = $_.FullName.Substring($SourcePath.Length).TrimStart("\","/")

    # Costruisce percorso destinazione corretto
    $target = Join-Path $destination $relativePath

    if ($_.PSIsContainer) {
        New-Item -ItemType Directory -Path $target -Force | Out-Null
    } else {
        # Assicura che la directory esista
        $targetDir = Split-Path $target
        if (!(Test-Path $targetDir)) {
            New-Item -ItemType Directory -Path $targetDir -Force | Out-Null
        }

        Copy-Item $_.FullName -Destination $target -Force
    }
}

Write-Host "Installazione completata!"
Write-Host "Riavvia Arduino IDE."
pause