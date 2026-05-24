$grfcl  = "D:\Mes documents\GitHub\GRFEditor\GRFEditor\GrfCL.exe"
$sprDir = "D:\Mes documents\GitHub\moonlightsite\ressources\sprite\npcs_mobs"
$outDir = "D:\Mes documents\GitHub\moonlightsite\ressources\sprite\npcs_mobs_gif"

New-Item -ItemType Directory -Force $outDir | Out-Null

$acts = Get-ChildItem $sprDir -Filter "*.act"
$total = $acts.Count
$i = 0

foreach ($act in $acts) {
    $i++
    $spr = [System.IO.Path]::ChangeExtension($act.FullName, '.spr')
    if (-not (Test-Path $spr)) {
        Write-Warning "[$i/$total] SPR manquant : $($act.Name)"
        continue
    }

    $gifName = $act.BaseName + '.gif'
    $gifDest = Join-Path $outDir $gifName

    if (Test-Path $gifDest) {
        Write-Host "[$i/$total] Déjà existant : $gifName" -ForegroundColor DarkGray
        continue
    }

    Write-Host "[$i/$total] Export : $($act.Name)"

    # Appel via Start-Process pour passer les chemins Unicode correctement
    $psi = [System.Diagnostics.ProcessStartInfo]::new($grfcl)
    $psi.Arguments = "-gif `"$outDir`" `"$($act.FullName)`" 0 -ignore true"
    $psi.UseShellExecute = $false
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $psi.StandardOutputEncoding = [System.Text.Encoding]::UTF8
    $p = [System.Diagnostics.Process]::Start($psi)
    $p.WaitForExit()

    if (-not (Test-Path $gifDest)) {
        Write-Warning "[$i/$total] GIF non créé pour : $($act.Name)"
        continue
    }

    # Renommage : strip le suffixe coréen
    # ex: "poring_책가방.gif" → "poring.gif"
    # ex: "leaf_cat_초록복주머니.gif" → "leaf_cat.gif"
    # Fichiers 100% ASCII (ex: "4_m_03.gif") restent inchangés
    if ($act.BaseName -match '^([a-zA-Z0-9]+(?:_[a-zA-Z0-9]+)*)_\P{IsBasicLatin}') {
        $cleanName = $Matches[1] + '.gif'
        $cleanDest = Join-Path $outDir $cleanName
        if (-not (Test-Path $cleanDest)) {
            Move-Item $gifDest $cleanDest
            Write-Host "  → renommé en $cleanName" -ForegroundColor Cyan
        } else {
            Remove-Item $gifDest
        }
    }
}

Write-Host "`nTerminé. GIFs dans : $outDir" -ForegroundColor Green
