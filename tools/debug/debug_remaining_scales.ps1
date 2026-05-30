# Find items in the output lua that still have "scales with refine" and trace back to their YAML

$enc = [System.Text.Encoding]::GetEncoding(949)
$lua = $enc.GetString([System.IO.File]::ReadAllBytes('client\SystemEN\itemInfomoon.lua'))
$lines = $lua -split "`r?`n"

# Find IDs of items with "scales with refine"
$badIds = @{}
$currentId = $null
foreach ($line in $lines) {
    if ($line -match '^\s*\[(\d+)\]\s*=\s*\{') { $currentId = [int]$Matches[1] }
    if ($line -match 'scales with refine' -and $currentId) {
        if (-not $badIds.ContainsKey($currentId)) { $badIds[$currentId] = @() }
        $badIds[$currentId] += $line.Trim()
    }
}

Write-Host "Items with 'scales with refine': $($badIds.Count)"

# Load YAML and find their scripts
$ItemFiles = @(
    "db\import\item_db.yml",
    "db\import\items\item_equip.yml",
    "db\import\items\item_etc.yml",
    "db\import\items\item_card.yml"
)

$shown = 0
foreach ($f in $ItemFiles) {
    if (-not (Test-Path $f)) { continue }
    $ylines = [System.IO.File]::ReadAllLines($f, [System.Text.Encoding]::UTF8)
    $cid = $null; $inScript = $false; $script = ""
    foreach ($yl in $ylines) {
        if ($yl -match '^\s{2}-\s+Id:\s+(\d+)') {
            if ($cid -and $badIds.ContainsKey($cid) -and $shown -lt 15) {
                Write-Host "`n=== ID $cid ==="
                foreach ($l in $badIds[$cid]) { Write-Host "  OUTPUT: $l" }
                Write-Host "  SCRIPT: $($script -replace '\n',' | ')"
                $shown++
            }
            $cid = [int]$Matches[1]; $inScript = $false; $script = ""
        }
        if ($cid -and $badIds.ContainsKey($cid)) {
            if ($yl -match '^\s{4}Script:\s*\|$') { $inScript = $true; continue }
            if ($yl -match '^\s{4}Script:\s+"(.+)"') { $script = $Matches[1]; $inScript = $false; continue }
            if ($inScript) {
                if ($yl -match '^\s{6}') { $script += $yl.Trim() + " | "; continue }
                else { $inScript = $false }
            }
        }
    }
}
