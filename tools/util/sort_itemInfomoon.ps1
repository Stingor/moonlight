# sort_itemInfomoon.ps1
# Réorganise les entrées de itemInfomoon.lua par ordre croissant d'ID
# Encodage CP949 (EUC-KR) préservé

$luaPath = 'client\SystemEN\itemInfomoon.lua'
$enc     = [System.Text.Encoding]::GetEncoding(949)

Write-Host "Lecture de $luaPath..."
$bytes = [System.IO.File]::ReadAllBytes($luaPath)
$text  = $enc.GetString($bytes)
$lines = $text -split "`n"

# --- 1. Extraire l'en-tête (avant le premier [id]) ---
$headerLines = [System.Collections.Generic.List[string]]::new()
$i = 0
while ($i -lt $lines.Count -and $lines[$i] -notmatch '^\s*\[(\d+)\]\s*=\s*\{') {
    $headerLines.Add($lines[$i])
    $i++
}

# --- 2. Extraire chaque bloc [id] = { ... } ---
# Un bloc commence sur la ligne "[id] = {" et se termine sur la ligne "	}," ou "	}"
$items = [System.Collections.Generic.SortedDictionary[int, string]]::new()

$blockLines = [System.Collections.Generic.List[string]]::new()
$currentId  = -1
$braceDepth = 0

while ($i -lt $lines.Count) {
    $line = $lines[$i]

    # Début d'un nouveau bloc item
    if ($line -match '^\s*\[(\d+)\]\s*=\s*\{') {
        $currentId  = [int]$Matches[1]
        $braceDepth = 1
        $blockLines = [System.Collections.Generic.List[string]]::new()
        $blockLines.Add($line)
        $i++
        continue
    }

    if ($currentId -ge 0) {
        # Compter les accolades pour savoir quand le bloc se termine
        $opens  = ([regex]::Matches($line, '\{')).Count
        $closes = ([regex]::Matches($line, '\}')).Count
        $braceDepth += $opens - $closes

        $blockLines.Add($line)

        if ($braceDepth -le 0) {
            # Fin du bloc
            $items[$currentId] = ($blockLines -join "`n")
            $currentId  = -1
            $braceDepth = 0
        }
    }
    # (lignes hors bloc ignorées — ne devraient pas exister entre items)

    $i++
}

Write-Host "Items trouvés : $($items.Count)"

# --- 3. Reconstruire le fichier ---
$out = [System.Collections.Generic.List[string]]::new()

# En-tête (ex: "tbl = {")
foreach ($hl in $headerLines) {
    $out.Add($hl)
}

# Blocs triés
foreach ($kvp in $items.GetEnumerator()) {
    $out.Add($kvp.Value)
}

# Pied de fichier : fermeture "}" si l'en-tête contient "tbl = {"
$out.Add("}")

# --- 4. Écriture CP949 ---
$finalText  = $out -join "`n"
$finalBytes = $enc.GetBytes($finalText)
[System.IO.File]::WriteAllBytes($luaPath, $finalBytes)

$lineCount = ($finalText -split "`n").Count
Write-Host "Fichier réécrit : $luaPath ($lineCount lignes)"
