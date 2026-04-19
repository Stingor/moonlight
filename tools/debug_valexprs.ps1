$ItemFiles = @(
    "db\import\item_db.yml",
    "db\import\items\item_equip.yml",
    "db\import\items\item_etc.yml",
    "db\import\items\item_card.yml",
    "db\import\item_cash.yml"
)

$patterns = [System.Collections.Generic.HashSet[string]]::new()

foreach ($f in $ItemFiles) {
    if (-not (Test-Path $f)) { continue }
    $lines = [System.IO.File]::ReadAllLines($f, [System.Text.Encoding]::UTF8)
    $inScript = $false
    foreach ($line in $lines) {
        if ($line -match '^\s{4}(Equip)?Script:\s*\|$') { $inScript = $true; continue }
        if ($line -match '^\s{4}(Equip)?Script:\s*"(.+)"') {
            $script = $Matches[2] -replace '\\n', "`n"
            foreach ($part in ($script -split '\n|;')) {
                $p = $part.Trim() -replace ';$',''
                if ($p -match '^bonus\s+b\w+\s*,\s*(.+)$') {
                    $val = $Matches[1].Trim()
                    if ($val -notmatch '^-?\d+$') { $patterns.Add($val) | Out-Null }
                }
            }
            $inScript = $false; continue
        }
        if ($inScript) {
            if ($line -match '^\s{6}') {
                $p = $line.Trim() -replace ';$',''
                if ($p -match '^bonus\s+b\w+\s*,\s*(.+)$') {
                    $val = $Matches[1].Trim()
                    if ($val -notmatch '^-?\d+$') { $patterns.Add($val) | Out-Null }
                }
            } else { $inScript = $false }
        }
    }
}

$patterns | Sort-Object | ForEach-Object { Write-Host $_ }
