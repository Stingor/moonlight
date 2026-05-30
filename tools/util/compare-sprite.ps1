$FolderA = "D:\TWRO\Nouveau dossier\data"
$FolderB = "D:\TWRO\Nouveau dossier\datatwro"

$sw = [System.Diagnostics.Stopwatch]::StartNew()
function Log($msg) { Write-Host "[$($sw.Elapsed.ToString('mm\:ss\.ff'))] $msg" }

# --- Scan ---
Log "Scan de A: $FolderA"
$filesA = @(Get-ChildItem $FolderA -File -Recurse)
Log "Scan de B: $FolderB"
$filesB = @(Get-ChildItem $FolderB -File -Recurse)
Log "$($filesA.Count) fichiers dans A, $($filesB.Count) fichiers dans B"

# --- Indexation par nom ---
Log "Indexation par nom de fichier..."

$IndexA = [System.Collections.Generic.Dictionary[string, System.Collections.Generic.List[string]]]::new([System.StringComparer]::OrdinalIgnoreCase)
foreach ($f in $filesA) {
    if (-not $IndexA.ContainsKey($f.Name)) { $IndexA[$f.Name] = [System.Collections.Generic.List[string]]::new() }
    $IndexA[$f.Name].Add($f.FullName)
}

$IndexB = [System.Collections.Generic.Dictionary[string, System.Collections.Generic.List[string]]]::new([System.StringComparer]::OrdinalIgnoreCase)
foreach ($f in $filesB) {
    if (-not $IndexB.ContainsKey($f.Name)) { $IndexB[$f.Name] = [System.Collections.Generic.List[string]]::new() }
    $IndexB[$f.Name].Add($f.FullName)
}

# --- Intersection ---
$commonNames = [System.Collections.Generic.List[string]]::new()
foreach ($name in $IndexA.Keys) {
    if ($IndexB.ContainsKey($name)) { $commonNames.Add($name) }
}
Log "$($commonNames.Count) noms identiques trouves"

# --- Suppression ---
Log "Suppression des fichiers en commun..."
$deletedA = 0; $deletedB = 0

foreach ($name in $commonNames) {
    foreach ($path in $IndexA[$name]) {
        if (Test-Path $path) { Remove-Item $path -Force; $deletedA++ }
    }
    foreach ($path in $IndexB[$name]) {
        if (Test-Path $path) { Remove-Item $path -Force; $deletedB++ }
    }
}

Log "Suppression terminee — $deletedA supprimes de A, $deletedB supprimes de B"
Log "Duree totale: $($sw.Elapsed.ToString('mm\:ss\.ff'))"
