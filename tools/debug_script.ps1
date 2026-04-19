$enc = [System.Text.Encoding]::GetEncoding(949)

# Simuler le parse du script inline de l'item 1196
$scriptRaw = 'bonus bUnbreakableWeapon; bonus bAgi,3; bonus bMaxHPrate,-10;'
$scriptText = $scriptRaw -replace '\\n', "`n" -replace '\\t', "`t"

Write-Host "=== Script brut ==="
Write-Host $scriptText

# Expand inline
$expandedLines = [System.Collections.Generic.List[string]]::new()
$lines = ($scriptText -replace '//[^\n]*','') -split '\n'
foreach ($rawLine in $lines) {
    $t = $rawLine.Trim()
    if ($t -match ';.*bonus|;.*itemheal|;.*percentheal') {
        foreach ($part in ($t -split ';')) {
            $p = $part.Trim()
            if ($p) { $expandedLines.Add($p) }
        }
    } else { $expandedLines.Add($t) }
}

Write-Host "=== Lignes expandées ==="
foreach ($l in $expandedLines) { Write-Host "  [$l]" }

# Tester les regex de ConvertTo-EffectText
$NoValueText = @{
    bUnbreakableWeapon='Indestructible weapon'; bUnbreakableArmor='Indestructible armor'
}
$StatDisplayNames = @{
    bAgi='AGI'; bMaxHPrate='MaxHP'
}
$PercentStats = [System.Collections.Generic.HashSet[string]]@('bMaxHPrate')

foreach ($s in $expandedLines) {
    $s = ($s.Trim() -replace ';$','').Trim()
    if (-not $s) { continue }
    Write-Host "--- Test: [$s]"
    if ($s -match '^bonus\s+b(\w+)\s*$')                    { Write-Host "  NoValue: $($NoValueText[$Matches[1]])" }
    elseif ($s -match '^bonus\s+b(\w+)\s*,\s*(.+)$') {
        $stat = $Matches[1]; $val = $Matches[2].Trim()
        $disp = $StatDisplayNames[$stat]
        $isPct = $PercentStats.Contains($stat)
        $suf = if ($isPct) {'%'} else {''}
        if ($val -match '^(-?\d+)$') {
            $v=[int]$val; $sign=if($v -ge 0){'+'}else{''}
            Write-Host "  Stat: $disp $sign$v$suf"
        } else { Write-Host "  Val non-numérique: $val" }
    } else { Write-Host "  Aucun match" }
}
