. tools\generate_descriptions.ps1 | Out-Null

# Reload just the functions by dot-sourcing a stripped version
# Instead, inline the YAML load and test directly

$ItemFiles = @(
    "db\import\item_db.yml",
    "db\import\items\item_equip.yml",
    "db\import\items\item_etc.yml",
    "db\import\items\item_card.yml",
    "db\import\item_cash.yml"
)

$targetId = 13345
foreach ($f in $ItemFiles) {
    if (-not (Test-Path $f)) { continue }
    $lines = [System.IO.File]::ReadAllLines($f, [System.Text.Encoding]::UTF8)
    $cid = $null; $inScript = $false; $sc = ""
    foreach ($line in $lines) {
        if ($line -match '^\s{2}-\s+Id:\s+(\d+)') { $cid = [int]$Matches[1]; $inScript = $false; $sc = "" }
        if ($cid -eq $targetId) {
            if ($line -match '^\s{4}Script:\s*\|$') { $inScript = $true; continue }
            if ($line -match '^\s{4}Script:\s+"(.+)"') { $sc = $Matches[1] -replace '\\n',"`n" -replace '\\t',"`t"; $inScript = $false; continue }
            if ($inScript) {
                if ($line -match '^\s{6}') { $sc += $line.TrimStart() + "`n"; continue }
                else { $inScript = $false }
            }
        }
    }
    if ($sc) {
        Write-Host "=== Found in $f ==="
        Write-Host "SCRIPT: $($sc -replace '\n','|')"
        break
    }
}
