# export_kro_to_moon.ps1
# Compare itemInfokro.lua avec les donnees /db/import (YAML).
# Pour chaque item qui differe (nom ou stats), exporte l'entree corrigee
# dans itemInfomoon.lua si elle n'y est pas deja.
#
# Regles :
#  - identifiedResourceName : jamais modifie
#  - Items deja dans itemInfomoon.lua : ignores (conserves tels quels)
#  - Encodage EUC-KR (CP949) preserve en lecture et ecriture
#
# Usage : powershell -ExecutionPolicy Bypass -File "tools\export_kro_to_moon.ps1"

$KroPath   = "client\SystemEN\itemInfokro.lua"
$MoonPath  = "client\SystemEN\itemInfomoon.lua"
$ItemFiles = @(
    "db\import\item_db.yml",
    "db\import\items\item_equip.yml",
    "db\import\items\item_etc.yml",
    "db\import\items\item_card.yml",
    "db\import\item_cash.yml"
)
$Enc = [System.Text.Encoding]::GetEncoding(949)

# ------------------------------------------------------------------
# 1. Charger les donnees YAML (Id -> Name, Weight, Defense, Attack,
#    MagicAttack, Slots, Refineable)
# ------------------------------------------------------------------
$yamlItems = @{}
foreach ($f in $ItemFiles) {
    if (-not (Test-Path $f)) { Write-Warning "Introuvable : $f"; continue }
    $yamlLines = Get-Content $f -Encoding UTF8
    $cid = $null; $item = @{}
    foreach ($line in $yamlLines) {
        if ($line -match "^\s{2}-\s+Id:\s+(\d+)") {
            if ($cid -and $item.Count -and -not $yamlItems.ContainsKey($cid)) {
                $yamlItems[$cid] = $item
            }
            $cid = [int]$Matches[1]; $item = @{}
        } elseif ($cid) {
            if ($line -match "^\s{4}Name:\s+(.+)$")          { $item['Name']        = $Matches[1].Trim() }
            if ($line -match "^\s{4}Weight:\s+(\d+)")         { $item['Weight']      = [math]::Round([int]$Matches[1] / 10) }
            if ($line -match "^\s{4}Defense:\s+(\d+)")        { $item['Defense']     = [int]$Matches[1] }
            if ($line -match "^\s{4}Attack:\s+(\d+)")         { $item['Attack']      = [int]$Matches[1] }
            if ($line -match "^\s{4}MagicAttack:\s+(\d+)")    { $item['MagicAttack'] = [int]$Matches[1] }
            if ($line -match "^\s{4}Slots:\s+(\d+)")          { $item['Slots']       = [int]$Matches[1] }
            if ($line -match "^\s{4}Refineable:\s+true")      { $item['Refineable']  = $true }
        }
    }
    if ($cid -and $item.Count -and -not $yamlItems.ContainsKey($cid)) { $yamlItems[$cid] = $item }
}
Write-Host "YAML : $($yamlItems.Count) items charges."

# ------------------------------------------------------------------
# 2. Charger les IDs deja presents dans itemInfomoon.lua
# ------------------------------------------------------------------
$moonBytes = [System.IO.File]::ReadAllBytes($MoonPath)
$moonText  = $Enc.GetString($moonBytes)
$moonLines = $moonText -split "`r?`n"

$moonIds = @{}
foreach ($line in $moonLines) {
    if ($line -match "^\s*\[(\d+)\]\s*=\s*\{") { $moonIds[[int]$Matches[1]] = $true }
}
Write-Host "itemInfomoon.lua : $($moonIds.Count) items existants."

# ------------------------------------------------------------------
# 3. Parser itemInfokro.lua : extraire les blocs par ID
# ------------------------------------------------------------------
$kroBytes = [System.IO.File]::ReadAllBytes($KroPath)
$kroText  = $Enc.GetString($kroBytes)
$kroLines = $kroText -split "`r?`n"

# Helper : extraire une stat numerique depuis les lignes de description (sans balises couleur)
function Get-DescStat($descLines, $keyword) {
    foreach ($l in $descLines) {
        $clean = $l -replace '\^[0-9A-Fa-f]{6}', ''
        if ($clean -match ($keyword + '[^\d\-]*(\d+)')) { return [int]$Matches[1] }
    }
    return $null
}

# Helper : remplacer une valeur numerique dans une ligne contenant le keyword
function Set-DescStat($line, $keyword, $oldVal, $newVal) {
    $pattern = '(' + [regex]::Escape($keyword) + '[^"]*?)' + [regex]::Escape($oldVal.ToString()) + '(?=")'
    return [regex]::Replace($line, $pattern, '${1}' + $newVal.ToString())
}

# Parcourir kro bloc par bloc
$exportedCount = 0
$skippedMoon   = 0
$noYaml        = 0

# On va construire les nouvelles entrees a ajouter a moon
$newMoonEntries = [System.Collections.Generic.List[string]]::new()

$i = 0
while ($i -lt $kroLines.Count) {
    $line = $kroLines[$i]

    if ($line -match "^\s*\[(\d+)\]\s*=\s*\{") {
        $currentId = [int]$Matches[1]

        # Collecter toutes les lignes du bloc jusqu'a la fermeture },
        $blockLines = [System.Collections.Generic.List[string]]::new()
        $blockLines.Add($line)
        $depth = 1
        $i++
        while ($i -lt $kroLines.Count -and $depth -gt 0) {
            $bl = $kroLines[$i]
            $blockLines.Add($bl)
            if ($bl -match "\{") { $depth += ([regex]::Matches($bl, '\{')).Count }
            if ($bl -match "\}") { $depth -= ([regex]::Matches($bl, '\}')).Count }
            $i++
        }

        # Item deja dans moon : ignorer
        if ($moonIds.ContainsKey($currentId)) { $skippedMoon++; continue }

        # Pas dans le YAML : ignorer
        if (-not $yamlItems.ContainsKey($currentId)) { $noYaml++; continue }

        $yaml = $yamlItems[$currentId]

        # --- Verifier les differences ---
        $differs = $false

        # Nom
        $kroName = ""
        foreach ($bl in $blockLines) {
            if ($bl -match 'identifiedDisplayName\s*=\s*"(.+)"') { $kroName = $Matches[1]; break }
        }
        $yamlName = ($yaml['Name'] -replace ' \[\d+\]$', '')
        if ($kroName -ne $yamlName) { $differs = $true }

        # Stats dans la description
        $descLines = [System.Collections.Generic.List[string]]::new()
        $inDesc = $false
        foreach ($bl in $blockLines) {
            if ($bl -match "identifiedDescriptionName\s*=\s*\{") { $inDesc = $true }
            elseif ($inDesc -and $bl -match "^\s*\},?$")          { $inDesc = $false }
            elseif ($inDesc)                                        { $descLines.Add($bl) }
        }

        foreach ($statKey in @('Weight','Defense','Attack','MagicAttack','Slots')) {
            if ($yaml.ContainsKey($statKey)) {
                $luaVal = Get-DescStat $descLines ($statKey + ":")
                if ($null -ne $luaVal -and $luaVal -ne $yaml[$statKey]) { $differs = $true; break }
            }
        }

        if (-not $differs) { continue }

        # --- Appliquer les corrections sur le bloc ---
        $fixedBlock = [System.Collections.Generic.List[string]]::new()
        $inDescFix  = $false

        foreach ($bl in $blockLines) {
            # Corriger identifiedDisplayName
            if ($bl -match "^(\s*)identifiedDisplayName\s*=\s*""(.+)""") {
                $fixedBlock.Add("$($Matches[1])identifiedDisplayName = `"$yamlName`",")
                continue
            }

            # Corriger stats dans la description
            if ($bl -match "identifiedDescriptionName\s*=\s*\{") { $inDescFix = $true }
            elseif ($inDescFix -and $bl -match "^\s*\},?$")       { $inDescFix = $false }

            if ($inDescFix) {
                $fixedLine = $bl
                foreach ($statKey in @('Weight','Defense','Attack','MagicAttack','Slots')) {
                    if ($yaml.ContainsKey($statKey)) {
                        $clean = $fixedLine -replace '\^[0-9A-Fa-f]{6}', ''
                        if ($clean -match ($statKey + ':[^\d]*(\d+)')) {
                            $oldVal = [int]$Matches[1]
                            if ($oldVal -ne $yaml[$statKey]) {
                                $fixedLine = Set-DescStat $fixedLine ($statKey + ":") $oldVal $yaml[$statKey]
                            }
                        }
                    }
                }
                $fixedBlock.Add($fixedLine)
                continue
            }

            $fixedBlock.Add($bl)
        }

        $newMoonEntries.AddRange($fixedBlock)
        $exportedCount++
    } else {
        $i++
    }
}

Write-Host "Items ignores (deja dans moon) : $skippedMoon"
Write-Host "Items ignores (absent du YAML)  : $noYaml"
Write-Host "Items a exporter                : $exportedCount"

# ------------------------------------------------------------------
# 4. Inserer les nouveaux items dans itemInfomoon.lua
#    On insere avant la derniere ligne (fermeture du tbl = { ... })
# ------------------------------------------------------------------
if ($exportedCount -gt 0) {
    # Trouver la derniere ligne de fermeture "}"
    $insertIdx = $moonLines.Count - 1
    while ($insertIdx -gt 0 -and $moonLines[$insertIdx].Trim() -notmatch "^\}") {
        $insertIdx--
    }

    $resultLines = [System.Collections.Generic.List[string]]::new()
    for ($j = 0; $j -lt $insertIdx; $j++)              { $resultLines.Add($moonLines[$j]) }
    foreach ($el in $newMoonEntries)                    { $resultLines.Add($el) }
    for ($j = $insertIdx; $j -lt $moonLines.Count; $j++) { $resultLines.Add($moonLines[$j]) }

    $newText  = $resultLines -join "`r`n"
    $newBytes = $Enc.GetBytes($newText)
    [System.IO.File]::WriteAllBytes($MoonPath, $newBytes)
    Write-Host "itemInfomoon.lua mis a jour : $exportedCount items ajoutes."
} else {
    Write-Host "Aucun item a exporter."
}

# ------------------------------------------------------------------
# 5. Verification encodage
# ------------------------------------------------------------------
$check = $Enc.GetString([System.IO.File]::ReadAllBytes($MoonPath)) -split "`r?`n"
$resCheck = $check | Select-String "identifiedResourceName" | Select-Object -First 3
Write-Host "Verification identifiedResourceName :"
$resCheck | ForEach-Object { Write-Host "  $_" }
