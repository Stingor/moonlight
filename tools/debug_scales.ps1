$enc = [System.Text.Encoding]::GetEncoding(949)
$bytes = [System.IO.File]::ReadAllBytes('client\SystemEN\itemInfomoon.lua')
$text = $enc.GetString($bytes)
$lines = $text -split "`r?`n"
$lines | Select-String 'scales with refine' | Select-Object -First 30 | ForEach-Object { $_.Line.Trim() }
