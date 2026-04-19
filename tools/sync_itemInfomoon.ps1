# sync_itemInfomoon.ps1
# Synchronise client\SystemEN\itemInfomoon.lua avec les donnees de db\import
# - Met a jour identifiedDisplayName depuis les fichiers YAML
# - Supprime les indicateurs de slots [n] dans identifiedDisplayName
# - Ne touche JAMAIS identifiedResourceName
# - Preserve l'encodage EUC-KR (CP949) du fichier lua
#
# Usage : .\tools\sync_itemInfomoon.ps1

$LuaPath   = "client\SystemEN\itemInfomoon.lua"
$ItemFiles = @(
    "db\import\item_db.yml",
    "db\import\items\item_equip.yml",
    "db\import\items\item_etc.yml",
    "db\import\items\item_card.yml",
    "db\import\item_cash.yml"
)

# ------------------------------------------------------------------
# 1. Charger les noms depuis les YAML (Id -> Name)
# ------------------------------------------------------------------
$itemMap = @{}
foreach ($f in $ItemFiles) {
    if (-not (Test-Path $f)) { Write-Warning "Fichier introuvable : $f"; continue }
    $lines = Get-Content $f -Encoding UTF8
    $cid   = $null
    foreach ($line in $lines) {
        if ($line -match "^\s{2}-\s+Id:\s+(\d+)") {
            $cid = [int]$Matches[1]
        } elseif ($cid -and $line -match "^\s{4}Name:\s+(.+)$") {
            # Premier fichier qui definit l'Id gagne (import a priorite)
            if (-not $itemMap.ContainsKey($cid)) {
                $itemMap[$cid] = $Matches[1].Trim()
            }
            $cid = $null
        } elseif ($line -match "^\s{2}-\s+Id:") {
            $cid = $null
        }
    }
}
Write-Host "YAML : $($itemMap.Count) items charges."

# ------------------------------------------------------------------
# 2. Lire le fichier Lua en EUC-KR
# ------------------------------------------------------------------
$enc       = [System.Text.Encoding]::GetEncoding(949)
$bytes     = [System.IO.File]::ReadAllBytes($LuaPath)
$text      = $enc.GetString($bytes)
$lines     = $text -split "`r?`n"

# ------------------------------------------------------------------
# 3. Appliquer les modifications ligne par ligne
# ------------------------------------------------------------------
$newLines    = [System.Collections.Generic.List[string]]::new()
$currentId   = $null
$updatedNames = 0

foreach ($line in $lines) {
    # Detecter l'ID courant
    if ($line -match "^\s*\[(\d+)\]\s*=\s*\{") {
        $currentId = [int]$Matches[1]
    }

    # Mettre a jour identifiedDisplayName
    if ($line -match "^(\s*)identifiedDisplayName\s*=\s*""(.+)""") {
        $indent  = $Matches[1]
        $oldName = $Matches[2]

        if ($currentId -and $itemMap.ContainsKey($currentId)) {
            # Nom depuis YAML, sans indicateur de slot [n]
            $newName = $itemMap[$currentId] -replace ' \[\d+\]$', ''
        } else {
            # Pas dans le YAML : garder l'ancien nom, juste retirer [n]
            $newName = $oldName -replace ' \[\d+\]$', ''
        }

        if ($newName -ne $oldName) { $updatedNames++ }
        $newLines.Add("$indent`identifiedDisplayName = `"$newName`",")
    } else {
        $newLines.Add($line)
    }
}

Write-Host "Noms mis a jour : $updatedNames"

# ------------------------------------------------------------------
# 4. Ecrire en preservant l'encodage EUC-KR
# ------------------------------------------------------------------
$newText  = $newLines -join "`r`n"
$newBytes = $enc.GetBytes($newText)
[System.IO.File]::WriteAllBytes($LuaPath, $newBytes)
Write-Host "Fichier ecrit : $LuaPath"

# ------------------------------------------------------------------
# 5. Verification finale
# ------------------------------------------------------------------
$check = $enc.GetString([System.IO.File]::ReadAllBytes($LuaPath)) -split "`r?`n"
$slots = $check | Select-String 'identifiedDisplayName.*\[\d+\]"'
if ($slots) {
    Write-Warning "Des indicateurs [n] sont encore presents :"
    $slots | ForEach-Object { Write-Host "  $_" }
} else {
    Write-Host "OK - Aucun indicateur [n] restant."
}
$resCheck = $check | Select-String "identifiedResourceName" | Select-Object -First 3
Write-Host "Verification identifiedResourceName (doit etre lisible) :"
$resCheck | ForEach-Object { Write-Host "  $_" }
