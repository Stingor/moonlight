# generate_descriptions.ps1
# Regenere les identifiedDescriptionName dans itemInfomoon.lua
# en parsant les scripts rAthena des fichiers YAML /db/import.
# Usage : powershell -ExecutionPolicy Bypass -File "tools\generate_descriptions.ps1"

$MoonPath  = "client\SystemEN\itemInfomoon.lua"
$KroPath   = "client\SystemEN\itemInfokro.lua"
$ItemFiles = @(
    "db\import\item_db.yml",
    "db\import\items\item_equip.yml",
    "db\import\items\item_etc.yml",
    "db\import\items\item_card.yml",
    "db\import\item_cash.yml"
)
$Enc = [System.Text.Encoding]::GetEncoding(949)

# ==============================================================
# TABLES
# ==============================================================
$StatDisplayNames = @{
    bStr='STR'; bAgi='AGI'; bVit='VIT'; bInt='INT'; bDex='DEX'; bLuk='LUK'
    bAtk='ATK'; bMatk='MATK'; bBaseAtk='ATK'; bWeaponAtk='ATK'
    bDef='DEF'; bDef2='VIT DEF'; bMdef='MDEF'; bMdef2='INT MDEF'
    bHit='HIT'; bFlee='FLEE'; bFlee2='Perfect FLEE'
    bCritical='CRI'; bAspd='ASPD'; bAspdRate='ASPD'
    bMaxHP='MaxHP'; bMaxSP='MaxSP'; bMaxHPrate='MaxHP'; bMaxSPrate='MaxSP'
    bHPrecovRate='HP Recovery'; bSPrecovRate='SP Recovery'
    bCastrate='Variable Cast Time'; bVariableCastrate='Variable Cast Time'
    bFixedCastrate='Fixed Cast Time'; bFixedCast='Fixed Cast Time'
    bDelayrate='After-cast Delay'
    bLongAtkRate='Ranged ATK'; bMatkRate='MATK'; bAtkRate='ATK'
    bNearAtkDef='Reduces Melee damage'; bLongAtkDef='Reduces Ranged damage'
    bMagicDef='Reduces Magic damage'; bMiscDef='Reduces Misc damage'
    bSPGainValue='SP recovered on attack'; bHPGainValue='HP recovered on attack'
    bAllStats='All Stats'; bAddMaxWeight='Max Weight'
    bSpeedAddRate='Move Speed'; bSpeedRate='Move Speed'
    bPerfectHit='Perfect HIT'; bExpAddRate='EXP'; bJobExpRate='Job EXP'
    bCritAtkRate='Critical damage'
    bShortWeaponDamageReturn='Melee damage reflect'
    bLongWeaponDamageReturn='Ranged damage reflect'
    bMagicDamageReturn='Magic damage reflect'
}

$PercentStats = [System.Collections.Generic.HashSet[string]]@(
    'bMaxHPrate','bMaxSPrate','bHPrecovRate','bSPrecovRate',
    'bCastrate','bVariableCastrate','bFixedCastrate','bDelayrate',
    'bLongAtkRate','bMatkRate','bAtkRate','bAspdRate',
    'bNearAtkDef','bLongAtkDef','bMagicDef','bMiscDef',
    'bSpeedAddRate','bSpeedRate','bPerfectHit','bExpAddRate','bJobExpRate','bCritAtkRate',
    'bShortWeaponDamageReturn','bLongWeaponDamageReturn','bMagicDamageReturn'
)

$NoValueText = @{
    bUnbreakableWeapon='Indestructible weapon'; bUnbreakableArmor='Indestructible armor'
    bUnbreakableHelm='Indestructible headgear'; bUnbreakableShield='Indestructible shield'
    bUnbreakableGarment='Indestructible garment'; bUnbreakableShoes='Indestructible footgear'
    bNoRegen='Prevents natural HP/SP regeneration'
    bNoMagicDamage='Nullifies Magic attacks'
    bNoCastCancel='Prevents Cast interruption'
    bNoGemStone='Removes Gemstone requirement'
}

$EleMap = @{
    Ele_Neutral='^777777Neutral^000000'; Ele_Water='^0000BBWater^000000'
    Ele_Earth='^996600Earth^000000';     Ele_Fire='^FF0000Fire^000000'
    Ele_Wind='^33CC00Wind^000000';       Ele_Poison='^009900Poison^000000'
    Ele_Holy='^FFCC00Holy^000000';       Ele_Dark='^777777Shadow^000000'
    Ele_Ghost='^777777Ghost^000000';     Ele_Undead='^777777Undead^000000'
}

$RaceMap = @{
    RC_Formless='Formless'; RC_Undead='Undead'; RC_Brute='Brute'; RC_Plant='Plant'
    RC_Insect='Insect'; RC_Fish='Fish'; RC_Demon='Demon'; RC_DemiHuman='Demi-Human'
    RC_Angel='Angel'; RC_Dragon='Dragon'; RC_Player='Player'
    RC_Player_Human='Human Player'; RC_Player_Doram='Doram Player'; RC_All='all races'
}

$SizeMap = @{
    Size_Small='Small'; Size_Medium='Medium'; Size_Large='Large'; Size_All='all sizes'
}

$EffMap = @{
    Eff_Stun='Stun'; Eff_Poison='Poison'; Eff_Silence='Silence'; Eff_Blind='Blind'
    Eff_Freeze='Freeze'; Eff_Curse='Curse'; Eff_Sleep='Sleep'; Eff_Stone='Stone'
    Eff_Bleeding='Bleeding'; Eff_Fear='Fear'; Eff_Burning='Burning'
    Eff_Crystallization='Crystallization'
}

$WeaponTypeMap = @{
    Sword='One-Handed Sword'; '2hSword'='Two-Handed Sword'; Dagger='Dagger'
    Axe='One-Handed Axe'; '2hAxe'='Two-Handed Axe'
    Spear='One-Handed Spear'; '2hSpear'='Two-Handed Spear'
    Mace='Mace'; '2hMace'='Two-Handed Mace'; Staff='Staff'; '2hStaff'='Two-Handed Staff'
    Bow='Bow'; Knuckle='Knuckle'; Musical='Musical Instrument'; Whip='Whip'
    Book='Book'; Katar='Katar'; Revolver='Revolver'; Rifle='Rifle'
    Gatling='Gatling Gun'; Shotgun='Shotgun'; Grenade='Grenade Launcher'
    Huuma='Huuma Shuriken'
}

$LocationMap = [ordered]@{
    Head_Top='Headgear'; Head_Mid='Headgear'; Head_Low='Headgear'
    Armor='Armor'; Left_Hand='Shield'; Garment='Garment'; Shoes='Footgear'
    Right_Accessory='Accessory'; Left_Accessory='Accessory'; Both_Accessory='Accessory'
    Both_Hand='Two-Handed Weapon'; Right_Hand='Weapon'
}

$JobDisplayMap = @{
    Novice='Novice'; Swordman='Swordman'; Mage='Mage'; Magician='Mage'
    Archer='Archer'; Acolyte='Acolyte'; Merchant='Merchant'; Thief='Thief'
    Knight='Knight'; Priest='Priest'; Wizard='Wizard'; Blacksmith='Blacksmith'
    Hunter='Hunter'; Assassin='Assassin'; Crusader='Crusader'; Monk='Monk'
    Sage='Sage'; Rogue='Rogue'; Alchemist='Alchemist'; Bard='Bard'; Dancer='Dancer'
    Lord_Knight='Lord Knight'; High_Priest='High Priest'; High_Wizard='High Wizard'
    Whitesmith='Whitesmith'; Sniper='Sniper'; Assassin_Cross='Assassin Cross'
    Paladin='Paladin'; Champion='Champion'; Scholar='Scholar'; Stalker='Stalker'
    Creator='Creator'; Clown='Clown'; Gypsy='Gypsy'
    Rune_Knight='Rune Knight'; Warlock='Warlock'; Ranger='Ranger'
    Archbishop='Archbishop'; Mechanic='Mechanic'; Guillotine_Cross='Guillotine Cross'
    Royal_Guard='Royal Guard'; Sorcerer='Sorcerer'; Minstrel='Minstrel'
    Wanderer='Wanderer'; Sura='Sura'; Genetic='Genetic'; Shadow_Chaser='Shadow Chaser'
    Ninja='Ninja'; Kagerou='Kagerou'; Oboro='Oboro'; Taekwon='Taekwon'
    Star_Gladiator='Star Gladiator'; Soul_Linker='Soul Linker'
    Gunslinger='Gunslinger'; Rebellion='Rebellion'; Summoner='Summoner'
}

# ==============================================================
# FONCTIONS DE CONVERSION
# ==============================================================
function Format-Stat($statName, $valExpr) {
    $displayName = $script:StatDisplayNames[$statName]
    if (-not $displayName) { return $null }
    $isPct   = $script:PercentStats.Contains($statName)
    $suffix  = if ($isPct) { "%" } else { "" }

    # Normalize: strip leading/trailing whitespace, replace getrefine() with .@r alias
    $e = $valExpr.Trim()
    $e = $e -replace 'getrefine\(\)','.@r'

    # Strip outer parens repeatedly while safe (no top-level ternary ?)
    while ($e -match '^\((.+)\)$') {
        $inner = $Matches[1]
        if ($inner -match '^[^()]*\?') { break }
        $e = $inner
    }

    # Helper closure for sign
    function s($n) { if ([int]$n -ge 0) { "+" } else { "" } }

    # Pure integer
    if ($e -match '^(-?\d+)$') {
        $v = [int]$e; return "$displayName $(s $v)$v$suffix"
    }
    # N - (.@r / M) with parens
    if ($e -match '^(-?\d+)\s*-\s*\(\.@r\s*/\s*(\d+)\)$') {
        $b=[int]$Matches[1]; $d=[int]$Matches[2]; return "$displayName $(s $b)$b$suffix, -1$suffix per $d refine levels"
    }
    # N - .@r  (decreases with refine)
    if ($e -match '^(-?\d+)\s*-\s*\.@r$') {
        $b=[int]$Matches[1]; return "$displayName $(s $b)$b$suffix, -1$suffix per refine level"
    }
    # N - .@r * M
    if ($e -match '^(-?\d+)\s*-\s*\.@r\s*\*\s*(\d+)$') {
        $b=[int]$Matches[1]; $p=[int]$Matches[2]
        return "$displayName $(s $b)$b$suffix, -$p$suffix per refine level"
    }
    # N - .@r / M
    if ($e -match '^(-?\d+)\s*-\s*\.@r\s*/\s*(\d+)$') {
        $b=[int]$Matches[1]; $d=[int]$Matches[2]
        return "$displayName $(s $b)$b$suffix, -1$suffix per $d refine levels"
    }
    # -(.@r) alone negative
    if ($e -match '^-\.@r$') { return "$displayName -1$suffix per refine level" }
    # -.@r/N
    if ($e -match '^-\.@r\s*/\s*(\d+)$') { return "$displayName -1$suffix per $($Matches[1]) refine levels" }
    # (.@r + N) / M  â€” e.g. (getrefine()+1)/2
    if ($e -match '^\.@r\s*\+\s*(\d+)\s*/\s*(\d+)$' -or $e -match '^\(\.@r\s*\+\s*(\d+)\)\s*/\s*(\d+)$') {
        $k=[int]$Matches[1]; $d=[int]$Matches[2]
        return "$displayName +(refine + $k) / $d$suffix"
    }
    # pow(min(.@r,N),2) or pow(.@r,2)  â€” quadratic scaling
    if ($e -match '^pow\(min\(\.@r,(\d+)\),2\)$') {
        return "$displayName +refine^2 (max $($Matches[1]))$suffix"
    }
    if ($e -match '^pow\(\.@r,2\)(?:\s*\*\s*(\d+))?(?:\s*/\s*(\d+))?$') {
        $k=if($Matches[1]){[int]$Matches[1]}else{1}; $d=if($Matches[2]){[int]$Matches[2]}else{1}
        if ($k -eq 1 -and $d -eq 1) { return "$displayName +refine^2$suffix" }
        if ($d -eq 100) { return "$displayName +refine^2 x $k%" }
        elseif ($d -eq 1) { return "$displayName +refine^2 x $k$suffix" }
        else { return "$displayName +refine^2 x $k/$d$suffix" }
    }
    # N + pow(.@r,2)  or  N + pow(min(.@r,M),2) â€” with optional *K/D
    if ($e -match '^(-?\d+)\s*\+\s*pow\((?:min\()?\.@r(?:,\d+\))?,2\)(?:\s*/\s*(\d+))?$') {
        $b=[int]$Matches[1]; $d=if($Matches[2]){[int]$Matches[2]}else{1}
        if ($d -eq 1) { return "$displayName $(s $b)$b$suffix + refine^2$suffix" }
        elseif ($d -eq 100) { return "$displayName $(s $b)$b$suffix + refine^2%" }
        else { return "$displayName $(s $b)$b$suffix + refine^2/$d$suffix" }
    }
    if ($e -match '^(-?\d+)\s*\+\s*pow\((?:min\()?\.@r(?:,\d+\))?,2\)\s*\*\s*(\d+)(?:\s*/\s*(\d+))?$') {
        $b=[int]$Matches[1]; $k=[int]$Matches[2]; $d=if($Matches[3]){[int]$Matches[3]}else{1}
        $desc = if ($d -eq 100) { "refine^2 x $k%" } elseif ($d -eq 1) { "refine^2 x $k$suffix" } else { "refine^2 x $k/$d$suffix" }
        return "$displayName $(s $b)$b$suffix + $desc"
    }
    # .@r alone
    if ($e -eq '.@r') { return "$displayName +1$suffix per refine level" }
    # N * .@r  or  .@r * N
    if ($e -match '^(-?\d+)\s*\*\s*\.@r$' -or $e -match '^\.@r\s*\*\s*(-?\d+)$') {
        $n=[int]$Matches[1]; return "$displayName $(s $n)$n$suffix per refine level"
    }
    # .@r / N
    if ($e -match '^\.@r\s*/\s*(\d+)$') {
        return "$displayName +1$suffix per $($Matches[1]) refine levels"
    }
    # (.@r / N) * M  or  M * (.@r / N)  â†’ +M per N refines (with optional double parens)
    if ($e -match '^\(?\(?\.@r\s*/\s*(\d+)\)?\)?\s*\*\s*(-?\d+)$') {
        $d=[int]$Matches[1]; $m=[int]$Matches[2]; return "$displayName $(s $m)$m$suffix per $d refine levels"
    }
    if ($e -match '^(-?\d+)\s*\*\s*\(?\(?\.@r\s*/\s*(\d+)\)?\)?$') {
        $m=[int]$Matches[1]; $d=[int]$Matches[2]; return "$displayName $(s $m)$m$suffix per $d refine levels"
    }
    # N + .@r * M  or  N + M * .@r  (with optional parens around .@r*M)
    if ($e -match '^(-?\d+)\s*\+\s*\(?(-?\d+)\s*\*\s*\.@r\)?$' -or
        $e -match '^(-?\d+)\s*\+\s*\(?\.@r\s*\*\s*(-?\d+)\)?$') {
        $b=[int]$Matches[1]; $p=[int]$Matches[2]
        return "$displayName $(s $b)$b$suffix, $(s $p)$p$suffix per refine level"
    }
    # N + (.@r >= M) â€” boolean addition: +1 bonus at refine threshold
    if ($e -match '^(-?\d+)\s*\+\s*\(\.@r\s*>=\s*(\d+)\)$') {
        $b=[int]$Matches[1]; $thr=[int]$Matches[2]
        return "$displayName $(s $b)$b$suffix (+1$suffix at refine $thr+)"
    }
    # N - (.@r >= M) boolean
    if ($e -match '^(-?\d+)\s*-\s*\(\.@r\s*>=\s*(\d+)\)$') {
        $b=[int]$Matches[1]; $thr=[int]$Matches[2]
        return "$displayName $(s $b)$b$suffix (-1$suffix at refine $thr+)"
    }
    # N + (.@r / M)  â€” with inner parens
    if ($e -match '^(-?\d+)\s*\+\s*\(\.@r\s*/\s*(\d+)\)$') {
        $b=[int]$Matches[1]; $d=[int]$Matches[2]
        return "$displayName $(s $b)$b$suffix + 1$suffix per $d refine levels"
    }
    # N + (.@r * M) or N + (M * .@r) with inner parens
    if ($e -match '^(-?\d+)\s*\+\s*\(\.@r\s*\*\s*(-?\d+)\)$' -or $e -match '^(-?\d+)\s*\+\s*\((-?\d+)\s*\*\s*\.@r\)$') {
        $b=[int]$Matches[1]; $p=[int]$Matches[2]
        return "$displayName $(s $b)$b$suffix, $(s $p)$p$suffix per refine level"
    }
    # N + (M * (.@r / D))  â€” e.g. 1000+(100*(.@r/2))
    if ($e -match '^(-?\d+)\s*\+\s*\((-?\d+)\s*\*\s*\(?\.@r\s*/\s*(\d+)\)?\)$') {
        $b=[int]$Matches[1]; $m=[int]$Matches[2]; $d=[int]$Matches[3]
        return "$displayName $(s $b)$b$suffix, $(s $m)$m$suffix per $d refine levels"
    }
    # N + BaseLevel
    if ($e -match '^(-?\d+)\s*\+\s*BaseLevel$') {
        $b=[int]$Matches[1]; return "$displayName $(s $b)$b$suffix + Base Level$suffix"
    }
    # N + .@r / M  â€” without parens
    if ($e -match '^(-?\d+)\s*\+\s*\.@r\s*/\s*(\d+)$') {
        $b=[int]$Matches[1]; $d=[int]$Matches[2]; $sign=s $b
        return "$displayName $sign$b$suffix + 1$suffix per $d refine levels"
    }
    # N + .@r  (multiplier=1)
    if ($e -match '^(-?\d+)\s*\+\s*\.@r$') {
        $b=[int]$Matches[1]; return "$displayName $(s $b)$b$suffix, +1$suffix per refine level"
    }
    # -(.@r * N) or -(.@r / N)  negative refine scaling
    if ($e -match '^-\(?\.@r\s*\*\s*(\d+)\)?$') {
        $n=[int]$Matches[1]; return "$displayName -$n$suffix per refine level"
    }
    if ($e -match '^-\(?\.@r\s*/\s*(\d+)\)?$') {
        $d=[int]$Matches[1]; return "$displayName -1$suffix per $d refine levels"
    }
    # (.@r / N) + M  or M + (.@r / N)
    if ($e -match '^\(?\.@r\s*/\s*(\d+)\)?\s*\+\s*(-?\d+)$') {
        $d=[int]$Matches[1]; $b=[int]$Matches[2]; return "$displayName $(s $b)$b$suffix + 1$suffix per $d refine levels"
    }
    # (1 + (.@r / N)) style â€” e.g. (1+(.@r/2))
    if ($e -match '^1\s*\+\s*\(?\.@r\s*/\s*(\d+)\)?$') {
        $d=[int]$Matches[1]; return "$displayName +1$suffix base + 1$suffix per $d refine levels"
    }
    # (10 * (.@r / N)) â€” e.g. 10*(.@r/2)
    if ($e -match '^(-?\d+)\s*\*\s*\(?\.@r\s*/\s*(\d+)\)?$') {
        $m=[int]$Matches[1]; $d=[int]$Matches[2]; return "$displayName $(s $m)$m$suffix per $d refine levels"
    }
    # (N + M * .@r) wrapped â€” e.g. (10+(5*.@r))
    if ($e -match '^\(?(-?\d+)\s*\+\s*(-?\d+)\s*\*\s*\.@r\)?$' -or $e -match '^\(?(-?\d+)\s*\+\s*\.@r\s*\*\s*(-?\d+)\)?$') {
        $b=[int]$Matches[1]; $p=[int]$Matches[2]
        return "$displayName $(s $b)$b$suffix, $(s $p)$p$suffix per refine level"
    }
    # pow(.@r, 2) * K / D
    if ($e -match '^pow\((?:min\()?\s*\.@r\s*(?:,\d+\))?,\s*2\)\s*\*\s*(\d+)(?:\s*/\s*(\d+))?$') {
        $k=[int]$Matches[1]; $d=if ($Matches[2]) {[int]$Matches[2]} else {1}
        if ($d -eq 100) { return "$displayName +refine^2 x $k%" }
        elseif ($d -eq 1) { return "$displayName +refine^2 x $k$suffix" }
        else { return "$displayName +refine^2 x $k/$d$suffix" }
    }
    # readparam(bX) / N
    if ($e -match '^readparam\(b?(\w+)\)\s*/\s*(\d+)$') {
        return "$displayName +1$suffix per $($Matches[2]) $($Matches[1])"
    }
    # N + readparam(bX) / M  â€” base + stat-scaled (with or without parens around readparam)
    if ($e -match '^(-?\d+)\s*\+\s*\(?readparam\(b?(\w+)\)\s*/\s*(\d+)\)?$') {
        $b=[int]$Matches[1]; $stat=$Matches[2]; $div=$Matches[3]
        return "$displayName $(s $b)$b$suffix + 1$suffix per $div $stat"
    }
    # readparam(bX) * N  or  N * readparam(bX)
    if ($e -match '^readparam\(b?(\w+)\)\s*\*\s*(\d+)$' -or $e -match '^(\d+)\s*\*\s*readparam\(b?(\w+)\)$') {
        return "$displayName +$($Matches[2])$suffix per $($Matches[1])"
    }
    # N * (.@r / M) + B
    if ($e -match '^(-?\d+)\s*\*\s*\(?\.@r\s*/\s*(\d+)\)?\s*\+\s*(-?\d+)$') {
        $m=[int]$Matches[1]; $d=[int]$Matches[2]; $b=[int]$Matches[3]
        return "$displayName $(s $b)$b$suffix, $(s $m)$m$suffix per $d refine levels"
    }
    if ($e -match '^(-?\d+)\s*\+\s*(-?\d+)\s*\*\s*\(?\.@r\s*/\s*(\d+)\)?$') {
        $b=[int]$Matches[1]; $m=[int]$Matches[2]; $d=[int]$Matches[3]
        return "$displayName $(s $b)$b$suffix, $(s $m)$m$suffix per $d refine levels"
    }
    # BaseLevel / N
    if ($e -match '^BaseLevel\s*/\s*(\d+)$') {
        return "$displayName +1$suffix per $($Matches[1]) Base Levels"
    }
    # BaseLevel * N  or  N * BaseLevel
    if ($e -match '^BaseLevel\s*\*\s*(-?\d+)$' -or $e -match '^(-?\d+)\s*\*\s*BaseLevel$') {
        $n=[int]$Matches[1]; return "$displayName $(s $n)$n$suffix per Base Level"
    }
    # N + BaseLevel / M
    if ($e -match '^(-?\d+)\s*\+\s*BaseLevel\s*/\s*(\d+)$') {
        $b=[int]$Matches[1]; $d=[int]$Matches[2]
        return "$displayName $(s $b)$b$suffix + 1$suffix per $d Base Levels"
    }
    # Ternary: any expression containing .@r and ? â€” extract numeric outcomes
    if ($e -match '\.@r' -and $e -match '\?') {
        $nums = [regex]::Matches($e, '(?<![.\d])-?\d+(?!\s*[,\d])') | ForEach-Object { [int]$_.Value } | Sort-Object -Unique
        if ($nums.Count -ge 2) {
            $minV = ($nums | Measure-Object -Minimum).Minimum
            $maxV = ($nums | Measure-Object -Maximum).Maximum
            if ($minV -eq 0) {
                $top = $maxV; return "$displayName up to $(s $top)$top$suffix (refine-dependent)"
            }
            if ($maxV -eq 0) {
                $bot = $minV; return "$displayName $bot$suffix to 0 (refine-dependent)"
            }
            return "$displayName $(s $minV)$minV$suffix to $(s $maxV)$maxV$suffix (refine-dependent)"
        }
    }
    # Simple ternary N + (cond ? A : 0)
    if ($e -match '^(-?\d+)\s*\+\s*\(?[^?]+\?\s*(-?\d+)\s*:\s*0\s*\)?$') {
        $b=[int]$Matches[1]; $a=[int]$Matches[2]
        return "$displayName $(s $b)$b$suffix ($(s $a)$a$suffix at higher refines)"
    }
    # N - (cond ? M : 0) â€” decreasing ternary
    if ($e -match '^(-?\d+)\s*-\s*\(?[^?]+\?\s*(\d+)\s*:\s*0\s*\)?$') {
        $b=[int]$Matches[1]; $pen=[int]$Matches[2]
        return "$displayName $(s $b)$b$suffix (-$pen$suffix at higher refines)"
    }
    # Fallback: starts with integer
    if ($e -match '^(-?\d+)') {
        $v=[int]$Matches[1]; return "$displayName $(s $v)$v$suffix (scales with refine)"
    }
    return $null
}

function Format-Bonus2($bonus, $param, $valExpr) {
    $bonus = $bonus -replace '^b',''
    $v = if ($valExpr -match '^(-?\d+)$') { [int]$valExpr } else { $null }
    switch -Wildcard ($bonus) {
        'SubEle'       { $e=$script:EleMap[$param];  if ($e -and $null -ne $v) { return "Reduces $e elemental damage by $v%" } }
        'AddEle'       { $e=$script:EleMap[$param];  if ($e -and $null -ne $v) { return "Increases damage against $e monsters by $v%" } }
        'MagicAddEle'  { $e=$script:EleMap[$param];  if ($e -and $null -ne $v) { return "Increases magic damage against $e monsters by $v%" } }
        'MagicAtkEle'  { $e=$script:EleMap[$param];  if ($e -and $null -ne $v) { return "Adds $e element to magic attacks, $v% power" } }
        'SubRace'      { $r=$script:RaceMap[$param]; if ($r -and $null -ne $v) { return "Reduces damage from $r by $v%" } }
        'AddRace'      { $r=$script:RaceMap[$param]; if ($r -and $null -ne $v) { return "Increases damage against $r by $v%" } }
        'MagicAddRace' { $r=$script:RaceMap[$param]; if ($r -and $null -ne $v) { return "Increases magic damage against $r by $v%" } }
        'SubSize'      { $s=$script:SizeMap[$param]; if ($s -and $null -ne $v) { return "Reduces damage from $s monsters by $v%" } }
        'AddSize'      { $s=$script:SizeMap[$param]; if ($s -and $null -ne $v) { return "Increases damage against $s monsters by $v%" } }
        'AddClass'     { if ($null -ne $v) { return "Increases physical damage by $v%" } }
        'AddEff'       { $ef=$script:EffMap[$param]; if ($ef -and $null -ne $v) { $p=[math]::Round($v/100,1); return "$p% chance to inflict $ef on target" } }
        'AddEff2'      { $ef=$script:EffMap[$param]; if ($ef -and $null -ne $v) { $p=[math]::Round($v/100,1); return "$p% chance to self-inflict $ef" } }
        'ResEff'       { $ef=$script:EffMap[$param]; if ($ef -and $null -ne $v) { $p=[math]::Round($v/100,1); return "$p% resistance against $ef" } }
        'SkillAtk'     { if ($null -ne $v) { return "$($param -replace '_',' ') damage +$v%" } }
        'SkillHeal'    { if ($null -ne $v) { return "$($param -replace '_',' ') heal +$v%" } }
        'ExpAddRace'   { $r=$script:RaceMap[$param]; if ($r -and $null -ne $v) { return "+$v% EXP from $r" } }
    }
    return $null
}

function ConvertTo-EffectText($stmt) {
    $s = ($stmt.Trim() -replace ';$','').Trim()
    if (-not $s) { return $null }
    if ($s -match '^bonus\s+(b\w+)\s*$')                                  { return $script:NoValueText[$Matches[1]] }
    if ($s -match '^bonus\s+(b\w+)\s*,\s*(.+)$')                         { return Format-Stat $Matches[1] $Matches[2].Trim() }
    if ($s -match '^bonus2\s+(b\w+)\s*,\s*(\w+)\s*,\s*(.+)$')           { return Format-Bonus2 $Matches[1] $Matches[2] $Matches[3].Trim() }
    if ($s -match '^bonus3\s+bAddEff2\s*,\s*(\w+)\s*,\s*(\d+)')         { $ef=$script:EffMap[$Matches[1]]; if($ef){$p=[math]::Round([int]$Matches[2]/100,1);return "$p% chance to self-inflict $ef when hit"} }
    if ($s -match '^itemheal\s+rand\((\d+),(\d+)\)\s*,\s*0')            { return "Heals $($Matches[1]) ~ $($Matches[2]) HP" }
    if ($s -match '^itemheal\s+0\s*,\s*rand\((\d+),(\d+)\)')            { return "Heals $($Matches[1]) ~ $($Matches[2]) SP" }
    if ($s -match '^itemheal\s+(\d+)\s*,\s*0')                          { return "Heals $($Matches[1]) HP" }
    if ($s -match '^itemheal\s+0\s*,\s*(\d+)')                          { return "Heals $($Matches[1]) SP" }
    if ($s -match '^percentheal\s+(\d+)\s*,\s*(\d+)') {
        $hp=[int]$Matches[1]; $sp=[int]$Matches[2]
        if ($hp-gt 0 -and $sp-gt 0) { return "Heals $hp% HP and $sp% SP" }
        if ($hp-gt 0) { return "Heals $hp% HP" }
        if ($sp-gt 0) { return "Heals $sp% SP" }
    }
    return $null
}

# ==============================================================
# PARSEUR DE SCRIPT rATHENA
# ==============================================================
function Parse-Script($scriptText) {
    if (-not $scriptText) { return @() }
    $unconditional = [System.Collections.Generic.List[string]]::new()
    $conditionals  = [System.Collections.Generic.SortedDictionary[int,object]]::new()
    $currentCond = $null; $inBlock = $false; $nextLineCond = $null

    # Normalize: strip line comments, strip block comments, then emit one statement per line
    $src = $scriptText -replace '//[^\n]*','' -replace '/\*[^*]*\*/',''
    # Tokenize character-by-character: split on ; outside {}, keep { } as own tokens
    $expandedLines = [System.Collections.Generic.List[string]]::new()
    $depth = 0; $cur = [System.Text.StringBuilder]::new()
    foreach ($ch in $src.ToCharArray()) {
        switch ($ch) {
            '{' {
                $p = $cur.ToString().Trim(); [void]$cur.Clear()
                if ($p) { $expandedLines.Add($p) }
                $expandedLines.Add("{")
                $depth++
            }
            '}' {
                $p = $cur.ToString().Trim(); [void]$cur.Clear()
                if ($p) { $expandedLines.Add($p) }
                $expandedLines.Add("}")
                $depth--
            }
            ';' {
                $p = $cur.ToString().Trim(); [void]$cur.Clear()
                if ($p) { $expandedLines.Add($p) }
            }
            default { [void]$cur.Append($ch) }
        }
    }
    $p = $cur.ToString().Trim(); if ($p) { $expandedLines.Add($p) }

    foreach ($rawLine in $expandedLines) {
        $line = $rawLine.Trim()
        if (-not $line -or $line -match '^\.@[a-z_]+\s*=\s*(getrefine|BaseLevel|JobLevel|0)') { continue }

        # "{" alone â€” enter block for pending condition
        if ($line -eq '{') { if ($null -ne $nextLineCond) { $currentCond=$nextLineCond; $nextLineCond=$null; $inBlock=$true }; continue }

        if ($line -match '^if\s*\(\.@r\s*>=\s*(\d+)\)\s*\{?$') {
            $nextLineCond = [int]$Matches[1]
            if ($line -match '\{$') { $currentCond=$nextLineCond; $nextLineCond=$null; $inBlock=$true }
            continue
        }
        if ($line -match '^if\s*\(\.@r\s*>=\s*(\d+)\)\s*([^{].+)') {
            $cond=[int]$Matches[1]; $fx=ConvertTo-EffectText $Matches[2]
            if ($fx) { if(-not $conditionals.ContainsKey($cond)){$conditionals[$cond]=[System.Collections.Generic.List[string]]::new()}; $conditionals[$cond].Add($fx) }
            continue
        }
        if ($line -eq '}') { $inBlock=$false; $currentCond=$null; continue }

        $fx = ConvertTo-EffectText $line
        if ($fx) {
            $cond = $null
            if ($null -ne $nextLineCond)               { $cond=$nextLineCond; $nextLineCond=$null }
            elseif ($inBlock -and $null -ne $currentCond) { $cond=$currentCond }
            if ($null -ne $cond) {
                if(-not $conditionals.ContainsKey($cond)){$conditionals[$cond]=[System.Collections.Generic.List[string]]::new()}
                $conditionals[$cond].Add($fx)
            } else { $unconditional.Add($fx) }
        } else { $nextLineCond=$null }
    }

    $result = [System.Collections.Generic.List[string]]::new()
    foreach ($fx in $unconditional) { $result.Add($fx) }
    foreach ($kv in $conditionals)  { foreach ($fx in $kv.Value) { $result.Add("^770000[Refine +$($kv.Key)]^000000 $fx") } }
    return $result
}

# ==============================================================
# TYPE / STATS / REQUIREMENTS
# ==============================================================
function Get-TypeDisplay($item) {
    switch ($item['Type']) {
        'Healing' { return 'Restorative' }
        'Usable'  { return 'Usable' }
        'Etc'     { return 'Etc' }
        'Cash'    { return 'Usable' }
        'Card'    { return 'Card' }
        'Ammo'    { return 'Ammo' }
        'Weapon'  { $sub=$item['SubType']; if($sub -and $script:WeaponTypeMap[$sub]){return $script:WeaponTypeMap[$sub]}; return 'Weapon' }
        'Armor'   { foreach($loc in $script:LocationMap.Keys){ if($item['Locations'] -contains $loc){return $script:LocationMap[$loc]} }; return 'Armor' }
    }
    return $item['Type']
}

function Get-CompoundOn($item) {
    foreach ($loc in $script:LocationMap.Keys) { if ($item['Locations'] -contains $loc) { return $script:LocationMap[$loc] } }
    return $null
}

function Get-Requirements($item) {
    $req = [System.Collections.Generic.List[string]]::new()
    if ($item['EquipLevelMin'] -gt 0) { $req.Add("Base level $($item['EquipLevelMin'])") }
    $jobs = $item['Jobs']
    if ($jobs -and $jobs.Count -gt 0) {
        if ($jobs -contains 'All') { $req.Add('All classes') }
        elseif ($jobs.Count -ge 12) { $req.Add($(if($jobs -contains 'Novice'){'All classes'}else{'All classes except Novice'})) }
        else {
            $names = $jobs | ForEach-Object { $n=$script:JobDisplayMap[$_]; if($n){$n}else{$_ -replace '_',' '} } | Select-Object -Unique
            $req.Add($names -join ', ')
        }
    } elseif ($item['Type'] -notin @('Healing','Usable','Etc','Cash','Card')) {
        $req.Add('All classes')
    }
    return $req
}

function Build-StatBlock($item) {
    $b = [System.Collections.Generic.List[string]]::new()
    if ($item['Type'] -eq 'Card') {
        $compound = Get-CompoundOn $item
        $b.Add('"^0000CCType:^000000 Card"')
        if ($compound) { $b.Add("`"^0000CCCompound on:^000000 $compound`"") }
    } else {
        $b.Add("`"^0000CCType:^000000 $(Get-TypeDisplay $item)`"")
        if ($item['Defense'])       { $b.Add("`"^0000CCDefense:^000000 $($item['Defense'])`"") }
        if ($item['Attack'])        { $b.Add("`"^0000CCAttack:^000000 $($item['Attack'])`"") }
        if ($item['WeaponLevel'])   { $b.Add("`"^0000CCWeapon Level:^000000 $($item['WeaponLevel'])`"") }
        if ($item['MagicAttack'])   { $b.Add("`"^0000CCMagic Attack:^000000 $($item['MagicAttack'])`"") }
        if ($item['Range'] -gt 1)   { $b.Add("`"^0000CCRange:^000000 $($item['Range'])`"") }
    }
    if ($item['Weight']) { $b.Add("`"^0000CCWeight:^000000 $($item['Weight'])`"") }
    if ($item['Refineable']) { $b.Add('"^0000CCRefinable:^000000 Yes#"') }
    $reqs = Get-Requirements $item
    if ($reqs.Count -gt 0) {
        $b.Add('"^0000CCRequirement:^000000"')
        foreach ($r in $reqs) { $b.Add("`"$r`"") }
    }
    return $b
}

# ==============================================================
# TEXTE DE SAVEUR
# ==============================================================
$FlavorBlacklist = '^\s*"?\s*(\^0000CC|\^770000|STR |AGI |VIT |INT |DEX |LUK |ATK |MATK |DEF |MDEF |HIT |FLEE |CRI |ASPD |MaxHP |MaxSP |All Stats |Reduces |Increases |Indestructible|Prevents|Heals |Adds |\d+%\s|Chance|Critical damage|Ranged ATK|Move Speed|Variable Cast|Fixed Cast|After-cast|HP Recovery|SP Recovery|Perfect |Melee |Magic |Weight|Type:|Defense:|Attack:|Weapon Level:|Magic Attack:|Range:|Refinable|Requirement|Compound on:|scales with refine|per refine level|per \d+ refine|refine-dependent|\^[0-9A-Fa-f]{6}|if \()'

function Get-FlavorLines($descLines) {
    $flavor = [System.Collections.Generic.List[string]]::new()
    foreach ($dl in $descLines) {
        if ($dl -imatch $script:FlavorBlacklist) { break }
        $clean = ($dl -replace '\^[0-9A-Fa-f]{6}','') -replace '^"\s*','' -replace '"[,]?$',''
        if ($clean.Trim().Length -gt 3) { $flavor.Add($dl) }
    }
    return $flavor
}

# ==============================================================
# PARSEUR YAML
# ==============================================================
function Parse-YamlItems($files) {
    $items = @{}
    foreach ($f in $files) {
        if (-not (Test-Path $f)) { Write-Warning "Introuvable : $f"; continue }
        $lines = [System.IO.File]::ReadAllLines($f, [System.Text.Encoding]::UTF8)
        $cid=$null; $item=$null; $section=$null; $inScript=$false; $sf='Script'
        foreach ($line in $lines) {
            if ($line -match '^\s{2}-\s+Id:\s+(\d+)') {
                if ($cid -ne $null -and -not $items.ContainsKey($cid)) { $items[$cid]=$item }
                $cid=[int]$Matches[1]
                $item=@{Jobs=[System.Collections.Generic.List[string]]::new();Locations=[System.Collections.Generic.List[string]]::new();Script='';EquipScript=''}
                $section=$null; $inScript=$false; continue
            }
            if ($cid -eq $null) { continue }
            if ($inScript) {
                if ($line -match '^\s{6}') { $item[$sf]+=$line.TrimStart()+"`n"; continue }
                else { $inScript=$false }
            }
            if ($line -match '^\s{4}(Equip)?Script:\s*\|$')    { $sf=if($Matches[1]){'EquipScript'}else{'Script'}; $item[$sf]=''; $inScript=$true; $section=$null; continue }
            if ($line -match '^\s{4}(Equip)?Script:\s+"(.+)"') { $sf=if($Matches[1]){'EquipScript'}else{'Script'}; $item[$sf]=($Matches[2]-replace'\\n',"`n"-replace'\\t',"`t"); $section=$null; continue }
            if ($line -match '^\s{4}Jobs:\s*$')      { $section='Jobs';      continue }
            if ($line -match '^\s{4}Locations:\s*$') { $section='Locations'; continue }
            if ($line -match '^\s{4}Classes:\s*$')   { $section='Classes';   continue }
            if ($line -match '^\s{6}(\w+):\s+true') {
                if ($section -eq 'Jobs')      { $item['Jobs'].Add($Matches[1]) }
                if ($section -eq 'Locations') { $item['Locations'].Add($Matches[1]) }
                continue
            }
            if ($line -match '^\s{4}\w') { $section=$null }
            if ($line -match '^\s{4}Name:\s+(.+)$')         { $item['Name']         =$Matches[1].Trim() }
            if ($line -match '^\s{4}Type:\s+(\w+)')          { $item['Type']         =$Matches[1] }
            if ($line -match '^\s{4}SubType:\s+(\w+)')       { $item['SubType']      =$Matches[1] }
            if ($line -match '^\s{4}Weight:\s+(\d+)')        { $item['Weight']       =[math]::Round([int]$Matches[1]/10) }
            if ($line -match '^\s{4}Defense:\s+(\d+)')       { $item['Defense']      =[int]$Matches[1] }
            if ($line -match '^\s{4}Attack:\s+(\d+)')        { $item['Attack']       =[int]$Matches[1] }
            if ($line -match '^\s{4}MagicAttack:\s+(\d+)')   { $item['MagicAttack']  =[int]$Matches[1] }
            if ($line -match '^\s{4}Slots:\s+(\d+)')         { $item['Slots']        =[int]$Matches[1] }
            if ($line -match '^\s{4}WeaponLevel:\s+(\d+)')   { $item['WeaponLevel']  =[int]$Matches[1] }
            if ($line -match '^\s{4}Refineable:\s+true')     { $item['Refineable']   =$true }
            if ($line -match '^\s{4}EquipLevelMin:\s+(\d+)') { $item['EquipLevelMin']=[int]$Matches[1] }
            if ($line -match '^\s{4}Range:\s+(\d+)')         { $item['Range']        =[int]$Matches[1] }
        }
        if ($cid -ne $null -and -not $items.ContainsKey($cid)) { $items[$cid]=$item }
    }
    return $items
}

# ==============================================================
# PARSEUR LUA
# ==============================================================
function Parse-LuaItems($luaText) {
    $items = @{}
    $lines = $luaText -split "`r?`n"
    $currentId=$null; $inDesc=$false; $afterDesc=$false
    $currentName=''; $currentResName=''; $currentDesc=@()
    $currentExtra=[System.Collections.Generic.List[string]]::new()

    foreach ($line in $lines) {
        if ($line -match "^\s*\[(\d+)\]\s*=\s*\{") {
            if ($currentId) { $items[$currentId]=@{Name=$currentName;ResName=$currentResName;Desc=$currentDesc;Extra=$currentExtra.ToArray()} }
            $currentId=[int]$Matches[1]; $currentName=''; $currentResName=''; $currentDesc=@()
            $currentExtra=[System.Collections.Generic.List[string]]::new(); $inDesc=$false; $afterDesc=$false; continue
        }
        if ($line -match 'identifiedDisplayName\s*=\s*"(.+)"')  { $currentName   =$Matches[1] }
        if ($line -match 'identifiedResourceName\s*=\s*"(.+)"') { $currentResName=$Matches[1] }
        if ($line -match 'identifiedDescriptionName\s*=\s*\{')  { $inDesc=$true; $afterDesc=$false; continue }
        if ($inDesc -and $line -match '^\s*\},?$') { $inDesc=$false; $afterDesc=$true; continue }
        if ($inDesc) { $currentDesc+=$line }
        if ($afterDesc -and $line -match '^\s*(slotCount|ClassNum|EffectID)\s*=') { $currentExtra.Add($line) }
    }
    if ($currentId) { $items[$currentId]=@{Name=$currentName;ResName=$currentResName;Desc=$currentDesc;Extra=$currentExtra.ToArray()} }
    return $items
}

# ==============================================================
# GENERATION D'UNE DESCRIPTION
# ==============================================================
function Generate-Description($yamlItem, $existingDescLines) {
    $result = [System.Collections.Generic.List[string]]::new()
    $ind = "`t`t`t"

    $flavor = Get-FlavorLines $existingDescLines
    foreach ($fl in $flavor) { $result.Add($fl) }

    $effects = [System.Collections.Generic.List[string]]::new()
    foreach ($sc in @('Script','EquipScript')) {
        if ($yamlItem[$sc]) {
            foreach ($fx in (Parse-Script $yamlItem[$sc])) {
                $fxClean = $fx.Trim()
                if ($fxClean) { $effects.Add($fxClean) }
            }
        }
    }

    for ($i=0; $i -lt $effects.Count; $i++) {
        $isLast = ($i -eq $effects.Count-1)
        $sfx = if ($isLast -and $flavor.Count-eq 0) { '#' } else { '' }
        $result.Add("$ind`"$($effects[$i])$sfx`",")
    }
    if ($effects.Count-gt 0 -and $flavor.Count-gt 0) {
        $last=$result[$result.Count-1]
        if ($last -notmatch '#"') { $result[$result.Count-1]=$last -replace '",$','#",' }
    }

    $statBlock = Build-StatBlock $yamlItem
    foreach ($sb in $statBlock) { $result.Add("$ind$sb,") }
    return $result
}

# ==============================================================
# MAIN
# ==============================================================
Write-Host "Chargement YAML..."
$yamlItems = Parse-YamlItems $ItemFiles
Write-Host "YAML : $($yamlItems.Count) items."

Write-Host "Lecture itemInfomoon.lua..."
$moonBytes=[System.IO.File]::ReadAllBytes($MoonPath); $moonText=$Enc.GetString($moonBytes)
$moonItems=Parse-LuaItems $moonText

Write-Host "Lecture itemInfokro.lua (texte de saveur)..."
$kroBytes=[System.IO.File]::ReadAllBytes($KroPath); $kroText=$Enc.GetString($kroBytes)
$kroItems=Parse-LuaItems $kroText

Write-Host "Generation pour $($moonItems.Count) items..."
$newLines=[System.Collections.Generic.List[string]]::new()
$newLines.Add("tbl = {")
$generated=0; $skipped=0

foreach ($id in ($moonItems.Keys | Sort-Object)) {
    $mi=$moonItems[$id]; $yi=$yamlItems[$id]
    $newLines.Add("`t[$id] = {")
    $newLines.Add("`t`tidentifiedDisplayName = `"$($mi['Name'])`",")
    $newLines.Add("`t`tidentifiedResourceName = `"$($mi['ResName'])`",")
    $newLines.Add("`t`tidentifiedDescriptionName = {")

    if ($yi -and ($yi['Script'] -or $yi['EquipScript'] -or $yi['Type'])) {
        $srcDesc = if ($mi['Desc'].Count-gt 0){$mi['Desc']} elseif($kroItems[$id]){$kroItems[$id]['Desc']} else {@()}
        foreach ($dl in (Generate-Description $yi $srcDesc)) { $newLines.Add($dl) }
        $generated++
    } else {
        foreach ($dl in $mi['Desc']) { $newLines.Add($dl) }
        $skipped++
    }

    $newLines.Add("`t`t},")
    foreach ($ex in $mi['Extra']) { $newLines.Add($ex) }
    $newLines.Add("`t},")
}
$newLines.Add("}")

Write-Host "Generes : $generated | Conserves : $skipped"
$newText=$newLines -join "`r`n"
$newBytes=$Enc.GetBytes($newText)
[System.IO.File]::WriteAllBytes($MoonPath,$newBytes)
Write-Host "Ecrit : $MoonPath ($($newLines.Count) lignes)"

$chk=$Enc.GetString([System.IO.File]::ReadAllBytes($MoonPath))-split"`r?`n"
Write-Host "Verification identifiedResourceName :"
$chk|Select-String "identifiedResourceName"|Select-Object -First 3|ForEach-Object{Write-Host "  $_"}


