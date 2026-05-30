$enc = [System.Text.Encoding]::GetEncoding(949)
$bytes = [System.IO.File]::ReadAllBytes('client\SystemEN\itemInfomoon.lua')
$text = $enc.GetString($bytes)
$lines = $text -split "`n"
$lines[0..40] -join "`n"
