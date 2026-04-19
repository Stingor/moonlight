$enc = [System.Text.Encoding]::GetEncoding(949)
$bytes = [System.IO.File]::ReadAllBytes('client\SystemEN\itemInfomoon.lua')
$text = $enc.GetString($bytes)
$lines = $text -split "`r?`n"
$start = ($lines | Select-String '^\s*\[1275\]').LineNumber - 1
$lines[($start)..($start+18)] -join "`n"
