"""Patch invasion.npc : leaderboard top-3 + récompenses 3 places."""
import sys

PATH = "moon/event/invasion.npc"

with open(PATH, "r", encoding="latin-1") as f:
    c = f.read()

errors = []

# ── R1 : init leaderboard dans OnLaunchInvasion ───────────────────────────────
old1 = '\t$@NbrKillMax = 0;\n'
new1 = '\t$@Score1 = 0; $@Score2 = 0; $@Score3 = 0;\n\t$@Winn1$ = ""; $@Winn2$ = ""; $@Winn3$ = "";\n'
if old1 not in c: errors.append('R1')

# ── R2 : announce prix ────────────────────────────────────────────────────────
old2 = '\tannounce "Bloody branch, Field manual, Lucky Candy(disguise) et 1 hat au plus gros tueur!",bc_all|bc_npc,0xFFFF00;\n'
new2 = '\tannounce "Top 3 r\xe9compens\xe9s : 1er: 5xBB+BubbleGum+30Candy+Costume | 2\xe8me: 3xBB+15Candy+Costume | 3\xe8me: 1xBB+5Candy",bc_all|bc_npc,0xFFFF00;\n'
if old2 not in c: errors.append('R2')

# ── R3 : OnMobUndeadKill – leaderboard + milestones ──────────────────────────
old3 = (
    '\t\t@NbrKill += 1; // +1 kill au score du joueur\n'
    '\n'
    '\t\tif( rand(0,8) == 8 )\n'
    '\t\t\t#maluswininvasion -= 1;\n'
    '\n'
    '\t\tif( #maluswininvasion < 0 )\n'
    '\t\t\t#maluswininvasion = 0;\n'
    '\n'
    '\t\tdispbottom @NbrKill + " monstres tu\xe9s" + (#maluswininvasion > 0?" - Malus: " + #maluswininvasion:".");\n'
    '\n'
    "\t\tif( @NbrKill > ($@NbrKillMax + #maluswininvasion) ) { // On augmente le score de kill le plus haut chaque fois qu'un joueur le d\xe9passe\n"
    '\t\t\t$@NbrKillMax += 1;\n'
    '\t\t\t$@Winn$ = strcharinfo(0);\n'
    '\t\t}\n'
    '\n'
    '\t\tswitch( $@NbrKillMax ) {\n'
    '\t\t\tcase 5:\t\tmapannounce $mapinvasion$,$@Winn$+" en est \xe0 son 5\xe9me mob tu\xe9!",bc_all,0xFFFF00; break;\n'
    '\t\t\tcase 20:\tmapannounce $mapinvasion$,"WouaW "+$@Winn$+" prend la t\xeate avec un total de 20 mobs tu\xe9s!",bc_all,0xFFFF00; break;\n'
    '\t\t\tcase 50:\tmapannounce $mapinvasion$,$@Winn$+" est bien \xe9chauff\xe9 avec 50 mobs d\xe9gomm\xe9s!",bc_all,0xFFFF00; break;\n'
    '\t\t\tcase 100:\tmapannounce $mapinvasion$,$@Winn$+" est en forme et prend la t\xeate avec un total de 100 mobs tu\xe9s!",bc_all,0xFFFF00; break;\n'
    '\t\t\tcase 200:\tmapannounce $mapinvasion$,"WOW "+$@Winn$+" se d\xe9chaine avec un total de 200 mobs tu\xe9s!",bc_all,0xFFFF00; break;\n'
    '\t\t\tcase 400:\tmapannounce $mapinvasion$,"WOW "+$@Winn$+" est en folie meurtri\xe8re avec un total de 400 mobs tu\xe9s!",bc_all,0xFFFF00; break;\n'
    '\t\t\tcase 800:\tmapannounce $mapinvasion$,"OMG "+$@Winn$+" est une machine de guerre avec un total de 800 mobs tu\xe9s!",bc_all,0xFFFF00; break;\n'
    '\t\t\tcase 1000:\tmapannounce $mapinvasion$,"MOMOMONMONMONSTERKILL "+$@Winn$+" a abattu 1000 mobs!",bc_all,0xFFFF00; break;\n'
    '\t\t\tcase 1500:\tmapannounce $mapinvasion$,$@Winn$+" a abattu 1500 mobs, rien \xe0 dire \xb0\xb0!",bc_all,0xFFFF00; break;\n'
    '\t\t}\n'
)
new3 = (
    '\t\t@NbrKill += 1; // +1 kill au score du joueur\n'
    '\n'
    '\t\tif( rand(0,8) == 8 )\n'
    '\t\t\t#maluswininvasion -= 1;\n'
    '\n'
    '\t\tif( #maluswininvasion < 0 )\n'
    '\t\t\t#maluswininvasion = 0;\n'
    '\n'
    '\t\tcallsub L_UpdateLeaderboard;\n'
    '\n'
    '\t\t.@pos = ($@Winn1$ == strcharinfo(0)) ? 1 : ($@Winn2$ == strcharinfo(0)) ? 2 : ($@Winn3$ == strcharinfo(0)) ? 3 : 0;\n'
    '\t\tdispbottom @NbrKill + " kills" + (#maluswininvasion > 0 ? " (malus -" + #maluswininvasion + ")" : "") + (.@pos > 0 ? " | #" + .@pos + " du classement" : "");\n'
    '\n'
    '\t\tswitch( @NbrKill ) {\n'
    '\t\t\tcase 5:\t\tif( $@Winn1$ == strcharinfo(0) ) mapannounce $mapinvasion$,$@Winn1$+" en est \xe0 son 5\xe8me mob tu\xe9!",bc_all,0xFFFF00; break;\n'
    '\t\t\tcase 20:\tif( $@Winn1$ == strcharinfo(0) ) mapannounce $mapinvasion$,"WouaW "+$@Winn1$+" prend la t\xeate avec 20 mobs tu\xe9s!",bc_all,0xFFFF00; break;\n'
    '\t\t\tcase 50:\tif( $@Winn1$ == strcharinfo(0) ) mapannounce $mapinvasion$,$@Winn1$+" est bien \xe9chauff\xe9 avec 50 mobs d\xe9gomm\xe9s!",bc_all,0xFFFF00; break;\n'
    '\t\t\tcase 100:\tif( $@Winn1$ == strcharinfo(0) ) mapannounce $mapinvasion$,$@Winn1$+" est en forme avec 100 mobs tu\xe9s!",bc_all,0xFFFF00; break;\n'
    '\t\t\tcase 200:\tif( $@Winn1$ == strcharinfo(0) ) mapannounce $mapinvasion$,"WOW "+$@Winn1$+" se d\xe9chaine avec 200 mobs tu\xe9s!",bc_all,0xFFFF00; break;\n'
    '\t\t\tcase 400:\tif( $@Winn1$ == strcharinfo(0) ) mapannounce $mapinvasion$,"WOW "+$@Winn1$+" est en folie avec 400 mobs tu\xe9s!",bc_all,0xFFFF00; break;\n'
    '\t\t\tcase 800:\tif( $@Winn1$ == strcharinfo(0) ) mapannounce $mapinvasion$,"OMG "+$@Winn1$+" est une machine de guerre avec 800 mobs tu\xe9s!",bc_all,0xFFFF00; break;\n'
    '\t\t\tcase 1000:\tif( $@Winn1$ == strcharinfo(0) ) mapannounce $mapinvasion$,"MOMOMONMONMONSTERKILL "+$@Winn1$+" a abattu 1000 mobs!",bc_all,0xFFFF00; break;\n'
    '\t\t\tcase 1500:\tif( $@Winn1$ == strcharinfo(0) ) mapannounce $mapinvasion$,$@Winn1$+" a abattu 1500 mobs, rien \xe0 dire \xb0\xb0!",bc_all,0xFFFF00; break;\n'
    '\t\t}\n'
)
if old3 not in c: errors.append('R3')

# ── R4 : OnMvpUndeadKill – leaderboard ───────────────────────────────────────
old4 = (
    '\t\t@NbrKill += 1; // +1 kill au score du joueur\n'
    '\n'
    '\t\t#maluswininvasion -= 1;\n'
    '\t\tif( #maluswininvasion < 0 )\n'
    '\t\t\t#maluswininvasion = 0;\n'
    '\n'
    "\t\tif( @NbrKill > ($@NbrKillMax + #maluswininvasion) ) { // On augmente le score de kill le plus haut chaque fois qu'un joueur le d\xe9passe\n"
    '\t\t\t$@NbrKillMax += 1;\n'
    '\t\t\t$@Winn$ = strcharinfo(0);\n'
    '\t\t}\n'
    '\n'
    '\t\tswitch( $@MvpUndeadM ) {\n'
)
new4 = (
    '\t\t@NbrKill += 1; // +1 kill au score du joueur\n'
    '\n'
    '\t\t#maluswininvasion -= 1;\n'
    '\t\tif( #maluswininvasion < 0 )\n'
    '\t\t\t#maluswininvasion = 0;\n'
    '\n'
    '\t\tcallsub L_UpdateLeaderboard;\n'
    '\n'
    '\t\t.@pos = ($@Winn1$ == strcharinfo(0)) ? 1 : ($@Winn2$ == strcharinfo(0)) ? 2 : ($@Winn3$ == strcharinfo(0)) ? 3 : 0;\n'
    '\t\tdispbottom @NbrKill + " kills" + (#maluswininvasion > 0 ? " (malus -" + #maluswininvasion + ")" : "") + (.@pos > 0 ? " | #" + .@pos + " du classement" : "");\n'
    '\n'
    '\t\tswitch( $@MvpUndeadM ) {\n'
)
if old4 not in c: errors.append('R4')

# ── R5 : L_UpdateLeaderboard avant fermeture de F_EventMobUndead ─────────────
old5 = '\tend;\n}\n\nfunction\tscript\tend_invasion\t{\n'
new5 = (
    '\tend;\n'
    '\n'
    'L_UpdateLeaderboard:\n'
    '\t.@name$ = strcharinfo(0);\n'
    '\t.@score = @NbrKill;\n'
    '\tif( $@Winn1$ == .@name$ )      { $@Score1 = .@score; }\n'
    '\telse if( $@Winn2$ == .@name$ ) { $@Score2 = .@score; }\n'
    '\telse if( $@Winn3$ == .@name$ ) { $@Score3 = .@score; }\n'
    '\telse if( $@Winn3$ == "" || .@score > $@Score3 ) {\n'
    '\t\t$@Winn3$ = .@name$; $@Score3 = .@score;\n'
    '\t} else return;\n'
    '\t// Tri \xe0 bulles d\xe9croissant (3 \xe9l\xe9ments)\n'
    '\tif( $@Score2 > $@Score1 ) {\n'
    '\t\t.@t$ = $@Winn1$; $@Winn1$ = $@Winn2$; $@Winn2$ = .@t$;\n'
    '\t\t.@t  = $@Score1; $@Score1 = $@Score2; $@Score2 = .@t;\n'
    '\t}\n'
    '\tif( $@Score3 > $@Score2 ) {\n'
    '\t\t.@t$ = $@Winn2$; $@Winn2$ = $@Winn3$; $@Winn3$ = .@t$;\n'
    '\t\t.@t  = $@Score2; $@Score2 = $@Score3; $@Score3 = .@t;\n'
    '\t}\n'
    '\tif( $@Score2 > $@Score1 ) {\n'
    '\t\t.@t$ = $@Winn1$; $@Winn1$ = $@Winn2$; $@Winn2$ = .@t$;\n'
    '\t\t.@t  = $@Score1; $@Score1 = $@Score2; $@Score2 = .@t;\n'
    '\t}\n'
    '\treturn;\n'
    '}\n'
    '\n'
    'function\tscript\tend_invasion\t{\n'
)
if old5 not in c: errors.append('R5')

# ── R6 : bloc récompenses end_invasion ───────────────────────────────────────
old6 = (
    '\tif( $@NbrKillMax > 0 && $@Winn$ != "" ) {\n'
    '\t\tattachrid(getcharid(3,$@Winn$));\n'
    '\n'
    '\t\tif( #maluswininvasion <= 0)\n'
    '\t\t\t#maluswininvasion += 100;\n'
    '\t\telse\n'
    '\t\t\t#maluswininvasion += 100 + (#maluswininvasion / 2);\t//\t100 + 50% du malus restant\n'
    '\n'
    '\t\tsetarray .@reward[0],20219,  // Costume Angel Marcher Hat\n'
    '                             20297,  // Costume Cactus Hat\n'
    "                             31251,  // Costume Cat's Mouth\n"
    '                             20026,  // Costume Cow Hat\n'
    '                             19857,  // Costume Crayfish Hat\n'
    '                             19959,  // Costume Drooping Argiope\n'
    '                             20103,  // Costume Drooping Panda\n'
    '                             31106,  // Costume Eyepatch of Peace\n'
    '                             31107,  // Costume Eyepatch of Prosperity\n'
    "                             19817,  // Costume Ifrit's Breath\n"
    '                             20075,  // Costume Little Feather Hat\n'
    '                             20433,  // Costume Louise Red Hat\n'
    '                             19521,  // Costume Old Timey Derby\n'
    '                             19514,  // Costume Old Timey Mustache\n'
    '                             20259,  // Costume Pink Clover\n'
    '                             31518,  // Costume Pop Popcorn Hat\n'
    '                             20183;  // Costume There is...Something\n'
    '\t\t.@winitem = .@reward[rand(getarraysize(.@reward))];\n'
    '\n'
    '\t\tlogmes "Gagne " + getitemname(.@winitem) + " \xe0 l\'invasion avec " + $@NbrKillMax + " monstres, malus de " + #maluswininvasion;\n'
    '\t\tif( playerattached() ) {\n'
    '\t\t\tif( checkweight(12103,5,12210,1,570,30,.@winitem,1) ) {\n'
    '\t\t\t\tgetitem 12103,5;\n'
    '\t\t\t\tgetitem 12210,1;\n'
    '\t\t\t\tgetitem 570,30;\n'
    '\t\t\t\tgetitem .@winitem,1;\n'
    '\t\t\t\tlogmes "R\xe9compenses re\xe7ues.";\n'
    '\t\t\t}\n'
    '\t\t\telse {\n'
    '\t\t\t\tsetarray .@mailitem[0], 12103, 12210, 570, .@winitem;\n'
    '\t\t\t\tsetarray .@mailamount[0], 5, 1, 30, 1;\n'
    '\t\t\t\t.@title$ = "Invasion";\n'
    '\t\t\t\t.@body$ = "F\xe9licitation vous avez gagn\xe9 l\'invasion !";\n'
    '\t\t\t\tmail getcharid(0), "Invasion", .@title$, .@body$, 0, .@mailitem, .@mailamount;\n'
    '\t\t\t\tdispbottom "Des r\xe9compenses vous sont envoy\xe9es par mail ! (Pas de place)";\n'
    '\t\t\t\tlogmes "R\xe9compenses re\xe7ues par mail.";\n'
    '\t\t\t}\n'
    '\t\t}\n'
    '\t\telse\n'
    '\t\t\tlogmes "R\xe9compenses non re\xe7ues.";\n'
    '\n'
    '\t\tannounce $@Winn$ + " a gagn\xe9 en tuant un total de " + $@NbrKillMax + " monstres !",bc_all|bc_npc,0xFFFF00;\n'
    '\t\tannounce ((Sex == SEX_FEMALE) ? "Elle" : "Il") + " gagne 5 bloody branch, 30 lucky candy, 1 Bubble Gum X2 et " + getitemname(.@winitem) + "!",bc_all|bc_npc,0xFFFF00;\n'
    '\t}\n'
    '\t$@NbrKillMax = 0;\n'
    '\t$@Winn$ = "";\n'
)
new6 = (
    '\t// Pool de costumes (commun aux 3 places)\n'
    '\tsetarray .@reward[0], 20219, 20297, 31251, 20026, 19857, 19959, 20103,\n'
    '\t                       31106, 31107, 19817, 20075, 20433, 19521, 19514,\n'
    '\t                       20259, 31518, 20183;\n'
    '\n'
    '\t// === 1\xe8re place ===\n'
    '\tif( $@Winn1$ != "" ) {\n'
    '\t\tattachrid(getcharid(3, $@Winn1$));\n'
    '\t\tif( playerattached() ) {\n'
    '\t\t\tif( #maluswininvasion <= 0 )\n'
    '\t\t\t\t#maluswininvasion += 100;\n'
    '\t\t\telse\n'
    '\t\t\t\t#maluswininvasion += 100 + (#maluswininvasion / 2);\n'
    '\t\t\t.@item1 = .@reward[rand(getarraysize(.@reward))];\n'
    '\t\t\tlogmes "1er invasion (" + $@Score1 + " kills) - malus " + #maluswininvasion;\n'
    '\t\t\tif( checkweight(12103,5,12210,1,570,30,.@item1,1) ) {\n'
    '\t\t\t\tgetitem 12103,5; getitem 12210,1; getitem 570,30; getitem .@item1,1;\n'
    '\t\t\t} else {\n'
    '\t\t\t\tsetarray .@mi[0], 12103, 12210, 570, .@item1;\n'
    '\t\t\t\tsetarray .@ma[0], 5, 1, 30, 1;\n'
    '\t\t\t\tmail getcharid(0), "Invasion #1", "F\xe9licitations !", "Tu as fini 1er de l\'invasion !", 0, .@mi, .@ma;\n'
    '\t\t\t\tdispbottom "R\xe9compenses envoy\xe9es par mail.";\n'
    '\t\t\t}\n'
    '\t\t\tannounce $@Winn1$ + " termine 1er avec " + $@Score1 + " kills !",bc_all|bc_npc,0xFFFF00;\n'
    '\t\t\tannounce ((Sex == SEX_FEMALE) ? "Elle" : "Il") + " gagne 5 Bloody Branch, 1 Bubble Gum X2, 30 Lucky Candy et " + getitemname(.@item1) + "!",bc_all|bc_npc,0xFFFF00;\n'
    '\t\t} else\n'
    '\t\t\tlogmes $@Winn1$ + " (1er) hors ligne, pas de r\xe9compense.";\n'
    '\t}\n'
    '\n'
    '\t// === 2\xe8me place ===\n'
    '\tif( $@Winn2$ != "" ) {\n'
    '\t\tattachrid(getcharid(3, $@Winn2$));\n'
    '\t\tif( playerattached() ) {\n'
    '\t\t\t#maluswininvasion += 50;\n'
    '\t\t\t.@item2 = .@reward[rand(getarraysize(.@reward))];\n'
    '\t\t\tlogmes "2\xe8me invasion (" + $@Score2 + " kills) - malus " + #maluswininvasion;\n'
    '\t\t\tif( checkweight(12103,3,570,15,.@item2,1) ) {\n'
    '\t\t\t\tgetitem 12103,3; getitem 570,15; getitem .@item2,1;\n'
    '\t\t\t} else {\n'
    '\t\t\t\tsetarray .@mi[0], 12103, 570, .@item2;\n'
    '\t\t\t\tsetarray .@ma[0], 3, 15, 1;\n'
    '\t\t\t\tmail getcharid(0), "Invasion #2", "F\xe9licitations !", "Tu as fini 2\xe8me de l\'invasion !", 0, .@mi, .@ma;\n'
    '\t\t\t\tdispbottom "R\xe9compenses envoy\xe9es par mail.";\n'
    '\t\t\t}\n'
    '\t\t\tannounce $@Winn2$ + " termine 2\xe8me avec " + $@Score2 + " kills !",bc_all|bc_npc,0xFFFF00;\n'
    '\t\t\tannounce ((Sex == SEX_FEMALE) ? "Elle" : "Il") + " gagne 3 Bloody Branch, 15 Lucky Candy et " + getitemname(.@item2) + "!",bc_all|bc_npc,0xFFFF00;\n'
    '\t\t} else\n'
    '\t\t\tlogmes $@Winn2$ + " (2\xe8me) hors ligne, pas de r\xe9compense.";\n'
    '\t}\n'
    '\n'
    '\t// === 3\xe8me place ===\n'
    '\tif( $@Winn3$ != "" ) {\n'
    '\t\tattachrid(getcharid(3, $@Winn3$));\n'
    '\t\tif( playerattached() ) {\n'
    '\t\t\t#maluswininvasion += 25;\n'
    '\t\t\tlogmes "3\xe8me invasion (" + $@Score3 + " kills) - malus " + #maluswininvasion;\n'
    '\t\t\tif( checkweight(12103,1,570,5) ) {\n'
    '\t\t\t\tgetitem 12103,1; getitem 570,5;\n'
    '\t\t\t} else {\n'
    '\t\t\t\tsetarray .@mi[0], 12103, 570;\n'
    '\t\t\t\tsetarray .@ma[0], 1, 5;\n'
    '\t\t\t\tmail getcharid(0), "Invasion #3", "F\xe9licitations !", "Tu as fini 3\xe8me de l\'invasion !", 0, .@mi, .@ma;\n'
    '\t\t\t\tdispbottom "R\xe9compenses envoy\xe9es par mail.";\n'
    '\t\t\t}\n'
    '\t\t\tannounce $@Winn3$ + " termine 3\xe8me avec " + $@Score3 + " kills !",bc_all|bc_npc,0xFFFF00;\n'
    '\t\t\tannounce ((Sex == SEX_FEMALE) ? "Elle" : "Il") + " gagne 1 Bloody Branch et 5 Lucky Candy!",bc_all|bc_npc,0xFFFF00;\n'
    '\t\t} else\n'
    '\t\t\tlogmes $@Winn3$ + " (3\xe8me) hors ligne, pas de r\xe9compense.";\n'
    '\t}\n'
    '\n'
    '\t$@Score1 = 0; $@Score2 = 0; $@Score3 = 0;\n'
    '\t$@Winn1$ = ""; $@Winn2$ = ""; $@Winn3$ = "";\n'
)
if old6 not in c: errors.append('R6')

# ── application ──────────────────────────────────────────────────────────────
if errors:
    print("ERREURS patterns non trouves:", errors)
    sys.exit(1)

c = c.replace(old1, new1, 1)
c = c.replace(old2, new2, 1)
c = c.replace(old3, new3, 1)
c = c.replace(old4, new4, 1)
c = c.replace(old5, new5, 1)
c = c.replace(old6, new6, 1)

# Vérification anti-corruption UTF-8
raw = c.encode("latin-1")
if b'\xef\xbf\xbd' in raw:
    print("ERREUR: bytes UTF-8 détectés !")
    sys.exit(1)

with open(PATH, "w", encoding="latin-1") as f:
    f.write(c)

print("OK - 6 remplacements appliqués")
