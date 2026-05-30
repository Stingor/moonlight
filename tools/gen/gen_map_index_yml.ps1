# Generate db/map_index.yml from db/map_index.txt and client/data/mapnametable.txt

# Build display name lookup from mapnametable.txt (first-occurrence wins)
$nameTable = @{}
foreach ($line in (Get-Content 'client/data/mapnametable.txt' -Encoding UTF8)) {
    if ($line.TrimStart() -match '^//') { continue }
    if ($line.Trim() -eq '') { continue }
    if ($line -match '^([^.#\s]+)\.(rsw|gat)#([^#]*)#') {
        $key = $matches[1].ToLower()
        $val = $matches[3].Trim()
        if ($val -ne '' -and -not $nameTable.ContainsKey($key)) {
            $nameTable[$key] = $val
        }
    }
}

# Helper: quote a YAML string value if needed
function YamlQuote($s) {
    # Always double-quote display names for safety (they can contain ": " etc.)
    $escaped = $s.Replace('\', '\\').Replace('"', '\"')
    return "`"$escaped`""
}

# Parse map_index.txt and build entries
$entries = [System.Collections.Generic.List[hashtable]]::new()
$lastIndex = -1
foreach ($line in (Get-Content 'db/map_index.txt' -Encoding UTF8)) {
    $trimmed = $line.Trim()
    if ($trimmed -match '^//') { continue }
    if ($trimmed -eq '') { continue }

    $mapName = $null
    $index = -1

    if ($trimmed -match '^(\S+)\s+(\d+)') {
        $mapName = $matches[1]
        $index = [int]$matches[2]
    } elseif ($trimmed -match '^(\S+)') {
        $mapName = $matches[1]
        $index = $lastIndex + 1
    } else {
        continue
    }

    $lastIndex = $index
    $displayName = $nameTable[$mapName.ToLower()]

    $entries.Add(@{
        Map = $mapName
        Id = $index
        DisplayName = $displayName
    })
}

# Build YAML lines
$lines = [System.Collections.Generic.List[string]]::new()
$lines.Add('# This file is a part of rAthena.')
$lines.Add('#   Copyright(C) 2021 rAthena Development Team')
$lines.Add('#   https://rathena.org - https://github.com/rathena')
$lines.Add('#')
$lines.Add('# This program is free software: you can redistribute it and/or modify')
$lines.Add('# it under the terms of the GNU General Public License as published by')
$lines.Add('# the Free Software Foundation, either version 3 of the License, or')
$lines.Add('# (at your option) any later version.')
$lines.Add('#')
$lines.Add('# This program is distributed in the hope that it will be useful,')
$lines.Add('# but WITHOUT ANY WARRANTY; without even the implied warranty of')
$lines.Add('# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the')
$lines.Add('# GNU General Public License for more details.')
$lines.Add('#')
$lines.Add('# You should have received a copy of the GNU General Public License')
$lines.Add('# along with this program. If not, see <http://www.gnu.org/licenses/>.')
$lines.Add('#')
$lines.Add('###########################################################################')
$lines.Add('# Map Index Database')
$lines.Add('###########################################################################')
$lines.Add('#')
$lines.Add('# Contains the list of maps with their respective IDs for inter-server use.')
$lines.Add('# IDs must never change, therefore any new maps need to be added at the end,')
$lines.Add('# and old ones must not be removed, but may be replaced.')
$lines.Add('#')
$lines.Add('###########################################################################')
$lines.Add('# - Map:  Internal map name. (Required)')
$lines.Add('#   Id:   Map index ID. If omitted, uses previous Id + 1. (Optional)')
$lines.Add('#   Name: Display name for the map. (Optional)')
$lines.Add('###########################################################################')
$lines.Add('')
$lines.Add('Header:')
$lines.Add('  Type: MAP_INDEX_DB')
$lines.Add('  Version: 1')
$lines.Add('')
$lines.Add('Body:')

foreach ($entry in $entries) {
    $lines.Add("  - Map: $($entry.Map)")
    $lines.Add("    Id: $($entry.Id)")
    if ($entry.DisplayName) {
        $lines.Add("    Name: $(YamlQuote $entry.DisplayName)")
    }
}

$lines.Add('')
$lines.Add('Footer:')
$lines.Add('  Imports:')
$lines.Add('    - Path: db/import/map_index.yml')

[System.IO.File]::WriteAllLines('db/map_index.yml', $lines, [System.Text.UTF8Encoding]::new($false))
Write-Host "Done! Generated $($entries.Count) entries."
Write-Host "Maps with display names: $(($entries | Where-Object { $_.DisplayName }).Count)"
