$FolderA = "E:\Nouveau dossier\Moonlight-Destiny\data\sprite"
$FolderB = "E:\Nouveau dossier\Moonlight-Destiny\data\spritenew"

$sw = [System.Diagnostics.Stopwatch]::StartNew()
function Log($msg) { Write-Host "[$($sw.Elapsed.ToString('mm\:ss\.ff'))] $msg" }

# --- Scan ---
Log "Scan de A: $FolderA"
$filesA = @(Get-ChildItem $FolderA -File -Recurse)
Log "Scan de B: $FolderB"
$filesB = @(Get-ChildItem $FolderB -File -Recurse)
Log "$($filesA.Count) fichiers dans A, $($filesB.Count) fichiers dans B"

# --- Pre-filtre par taille (elimine les fichiers sans correspondance possible) ---
Log "Pre-filtrage par taille de fichier..."

$sizeSetB = [System.Collections.Generic.HashSet[long]]::new()
foreach ($f in $filesB) { [void]$sizeSetB.Add($f.Length) }

$candidatesA = [System.Collections.Generic.List[object]]::new()
foreach ($f in $filesA) { if ($sizeSetB.Contains($f.Length)) { $candidatesA.Add($f) } }

$sizeSetA = [System.Collections.Generic.HashSet[long]]::new()
foreach ($f in $candidatesA) { [void]$sizeSetA.Add($f.Length) }

$candidatesB = [System.Collections.Generic.List[object]]::new()
foreach ($f in $filesB) { if ($sizeSetA.Contains($f.Length)) { $candidatesB.Add($f) } }

$totalCandidates = $candidatesA.Count + $candidatesB.Count
Log "$($candidatesA.Count) candidats dans A, $($candidatesB.Count) candidats dans B apres pre-filtre ($totalCandidates a hasher)"

# --- Hash par lots via runspace pool (un lot par CPU, zero overhead par fichier) ---
$cpuCount = [Environment]::ProcessorCount
Log "Hashing sur $cpuCount threads (runspace pool)..."

$allCandidates = [object[]]::new($totalCandidates)
$i = 0
foreach ($f in $candidatesA) { $allCandidates[$i++] = [PSCustomObject]@{ FullName = $f.FullName; Folder = 'A' } }
foreach ($f in $candidatesB) { $allCandidates[$i++] = [PSCustomObject]@{ FullName = $f.FullName; Folder = 'B' } }

$chunkSize = [Math]::Max(1, [Math]::Ceiling($allCandidates.Count / $cpuCount))
$chunks = [System.Collections.Generic.List[object[]]]::new()
for ($j = 0; $j -lt $allCandidates.Count; $j += $chunkSize) {
    $end = [Math]::Min($j + $chunkSize - 1, $allCandidates.Count - 1)
    $chunks.Add($allCandidates[$j..$end])
}

$pool = [runspacefactory]::CreateRunspacePool(1, $cpuCount)
$pool.Open()

$hashScript = {
    param($files)
    $md5 = [System.Security.Cryptography.MD5]::Create()
    foreach ($f in $files) {
        try {
            $stream = [System.IO.File]::OpenRead($f.FullName)
            $hash = [System.BitConverter]::ToString($md5.ComputeHash($stream)).Replace('-','')
            $stream.Dispose()
            [PSCustomObject]@{ Folder = $f.Folder; FullName = $f.FullName; Hash = $hash }
        } catch { }
    }
    $md5.Dispose()
}

$jobs = foreach ($chunk in $chunks) {
    $ps = [powershell]::Create()
    $ps.RunspacePool = $pool
    [void]$ps.AddScript($hashScript).AddArgument($chunk)
    [PSCustomObject]@{ PS = $ps; Handle = $ps.BeginInvoke() }
}

$results = [System.Collections.Generic.List[object]]::new()
foreach ($job in $jobs) {
    $out = $job.PS.EndInvoke($job.Handle)
    if ($out) { $results.AddRange([object[]]$out) }
    $job.PS.Dispose()
}
$pool.Close(); $pool.Dispose()

Log "$($results.Count) hashes calcules. Indexation..."

# --- Indexation ---
$IndexA = [System.Collections.Generic.Dictionary[string, System.Collections.Generic.List[string]]]::new()
$IndexB = [System.Collections.Generic.Dictionary[string, System.Collections.Generic.List[string]]]::new()

foreach ($r in $results) {
    $idx = if ($r.Folder -eq 'A') { $IndexA } else { $IndexB }
    if (-not $idx.ContainsKey($r.Hash)) { $idx[$r.Hash] = [System.Collections.Generic.List[string]]::new() }
    $idx[$r.Hash].Add($r.FullName)
}

# --- Intersection ---
$commonHashes = [System.Collections.Generic.List[string]]::new()
foreach ($hash in $IndexA.Keys) {
    if ($IndexB.ContainsKey($hash)) { $commonHashes.Add($hash) }
}
Log "$($commonHashes.Count) hashes identiques trouves"

# --- Suppression ---
Log "Suppression des fichiers identiques..."
$deletedA = 0; $deletedB = 0

foreach ($hash in $commonHashes) {
    foreach ($path in $IndexA[$hash]) {
        if (Test-Path $path) { Remove-Item $path -Force; $deletedA++ }
    }
    foreach ($path in $IndexB[$hash]) {
        if (Test-Path $path) { Remove-Item $path -Force; $deletedB++ }
    }
}

Log "Suppression terminee — $deletedA supprimes de A, $deletedB supprimes de B"
Log "Duree totale: $($sw.Elapsed.ToString('mm\:ss\.ff'))"
