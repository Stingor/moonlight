  do {
      $empty = Get-ChildItem "datatwro" -Recurse -Directory | Where-Object {
  $_.GetFiles('*','AllDirectories').Count -eq 0 }
      $empty | Remove-Item -Force
  } while ($empty.Count -gt 0)