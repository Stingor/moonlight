# NPC/Script cleanup utility for rAthena files
# - Normalizes spacing in control structures (if, for, while, switch)
# - Removes trailing whitespace
# - Fixes label indentation
# - Encodes in ANSI (latin-1)
#
# Usage: Drag and drop .npc/.txt/.cpp files onto this script, or:
#   .\cleanup-npc.ps1 file1.npc file2.npc

param(
    [Parameter(ValueFromRemainingArguments=$true)]
    [string[]]$Files
)

function Clean-NPCFile {
    param([string]$FilePath)

    $file = Get-Item $FilePath -ErrorAction SilentlyContinue
    if (-not $file) {
        Write-Host "ERROR: File not found: $FilePath" -ForegroundColor Red
        return $false
    }

    try {
        # Read as latin-1 (Windows-1252)
        $content = [System.IO.File]::ReadAllText($FilePath, [System.Text.Encoding]::GetEncoding('iso-8859-1'))
        $originalContent = $content
    }
    catch {
        Write-Host "ERROR reading file: $_" -ForegroundColor Red
        return $false
    }

    # 1. Add space after control structures: if( -> if (
    $content = $content -replace '\b(if|else if|while|for|switch)\(', '$1 ('

    # 2. Remove trailing whitespace from all lines
    $lines = $content -split "`n"
    $lines = $lines | ForEach-Object { $_.TrimEnd() }
    $content = $lines -join "`n"

    # 3. Move labels to column 0 (remove leading whitespace)
    $content = $content -replace "`n\s+(L_\w+):", "`n`$1:"

    # 4. Fix spacing issues
    # Remove space before closing ) in conditions
    $content = $content -replace '\)\s+\{', ') {'

    # 5. Verify latin-1 encoding
    try {
        [System.Text.Encoding]::GetEncoding('iso-8859-1').GetBytes($content) > $null
    }
    catch {
        Write-Host "ERROR: File contains characters not encodable in latin-1: $_" -ForegroundColor Red
        return $false
    }

    # 6. Write back in latin-1
    try {
        [System.IO.File]::WriteAllText($FilePath, $content, [System.Text.Encoding]::GetEncoding('iso-8859-1'))
    }
    catch {
        Write-Host "ERROR writing file: $_" -ForegroundColor Red
        return $false
    }

    # Report
    if ($content -ne $originalContent) {
        Write-Host "✓ Cleaned: $FilePath" -ForegroundColor Green
        Write-Host "  - Added space after control structures (if, for, while, switch)"
        Write-Host "  - Removed trailing whitespace"
        Write-Host "  - Fixed label indentation"
        Write-Host "  - Encoded in ANSI (latin-1)"
        return $true
    }
    else {
        Write-Host "✓ No changes needed: $FilePath" -ForegroundColor Green
        return $true
    }
}

# Main
if ($Files.Count -eq 0) {
    Write-Host "Usage: Drag and drop .npc/.txt/.cpp files onto this script" -ForegroundColor Yellow
    Write-Host "   Or: .\cleanup-npc.ps1 file1.npc file2.npc" -ForegroundColor Yellow
    Read-Host "Press Enter to exit"
    exit 1
}

$allSuccess = $true
foreach ($file in $Files) {
    if (-not (Clean-NPCFile $file)) {
        $allSuccess = $false
    }
}

if ($allSuccess) {
    Write-Host "`n✓ All files processed successfully" -ForegroundColor Green
}
else {
    Write-Host "`n✗ Some files had errors" -ForegroundColor Red
}

# Pause so user can see output when dragging files
Read-Host "Press Enter to exit"
