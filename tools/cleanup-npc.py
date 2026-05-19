#!/usr/bin/env python3
"""
NPC/Script cleanup utility for rAthena files
- Normalizes spacing in control structures (if, for, while, switch)
- Removes trailing whitespace
- Fixes label indentation
- Encodes in ANSI (latin-1)
"""

import sys
import re
from pathlib import Path


def cleanup_npc(filepath):
    """Clean up NPC/script file with proper formatting"""

    filepath = Path(filepath)
    if not filepath.exists():
        print(f"ERROR: File not found: {filepath}")
        return False

    try:
        # Read in latin-1 encoding
        with open(filepath, 'r', encoding='latin-1') as f:
            content = f.read()
    except Exception as e:
        print(f"ERROR reading file: {e}")
        return False

    original_content = content

    # 1. Add space after control structures: if( -> if (
    content = re.sub(r'\b(if|else if|while|for|switch)\(', r'\1 (', content)

    # 2. Remove trailing whitespace from all lines
    lines = content.split('\n')
    lines = [line.rstrip() for line in lines]
    content = '\n'.join(lines)

    # 3. Move labels to column 0 (remove leading whitespace)
    content = re.sub(r'\n\s+(L_\w+):', r'\n\1:', content)

    # 4. Fix spacing issues
    # Remove space before closing ) in conditions
    content = re.sub(r'\)\s+\{', ') {', content)

    # 5. Ensure valid latin-1 encoding
    try:
        content.encode('latin-1')
    except UnicodeEncodeError as e:
        print(f"ERROR: File contains characters not encodable in latin-1: {e}")
        return False

    # Check for UTF-8 replacement sequences
    content_bytes = content.encode('latin-1')
    if b'\xef\xbf\xbd' in content_bytes:
        print("WARNING: File contains invalid UTF-8 sequences")

    # 6. Write back in latin-1
    try:
        with open(filepath, 'w', encoding='latin-1') as f:
            f.write(content)
    except Exception as e:
        print(f"ERROR writing file: {e}")
        return False

    # Report changes
    if content != original_content:
        print(f"✓ Cleaned: {filepath}")
        print("  - Added space after control structures (if, for, while, switch)")
        print("  - Removed trailing whitespace")
        print("  - Fixed label indentation")
        print("  - Encoded in ANSI (latin-1)")
        return True
    else:
        print(f"✓ No changes needed: {filepath}")
        return True


def main():
    if len(sys.argv) < 2:
        print("Usage: python cleanup-npc.py <file1> [file2] [file3] ...")
        print("\nYou can also drag and drop files onto this script")
        print("\nSupported formats:")
        print("  - .npc files (rAthena NPC scripts)")
        print("  - .txt files (rAthena text files)")
        print("  - .cpp files (rAthena C++ scripts)")
        sys.exit(1)

    all_success = True
    for filepath in sys.argv[1:]:
        success = cleanup_npc(filepath)
        if not success:
            all_success = False

    if all_success:
        print("\n✓ All files processed successfully")
    else:
        print("\n✗ Some files had errors")
        sys.exit(1)


if __name__ == '__main__':
    main()
