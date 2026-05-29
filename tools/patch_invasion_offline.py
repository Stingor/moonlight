"""Patch invasion.npc : offline fix — mail direct + $@CharId1/2/3."""
import sys

PATH = "moon/event/invasion.npc"

with open(PATH, "r", encoding="latin-1") as f:
    c = f.read()

errors = []

# ── P1 : init $@CharId dans OnLaunchInvasion ────────────────────────────────
old1 = '\t$@Score1 = 0; $@Score2 = 0; $@Score3 = 0;\n\t$@Winn1$ = ""; $@Winn2$ = ""; $@Winn3$ = "";\n'
new1 = '\t$@Score1 = 0; $@Score2 = 0; $@Score3 = 0;\n\t$@Winn1$ = ""; $@Winn2$ = ""; $@Winn3$ = "";\n\t$@CharId1 = 0; $@CharId2 = 0; $@CharId3 = 0;\n'
if old1 not in c: errors.append('P1')

# ── P2 : L_UpdateLeaderboard — .@cid + swaps CharId ─────────────────────────
old2 = (
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
)
new2 = (
    'L_UpdateLeaderboard:\n'
    '\t.@name$ = strcharinfo(0);\n'
    '\t.@score = @NbrKill;\n'
    '\t.@cid   = getcharid(0);\n'
    '\tif( $@Winn1$ == .@name$ )      { $@Score1 = .@score; $@CharId1 = .@cid; }\n'
    '\telse if( $@Winn2$ == .@name$ ) { $@Score2 = .@score; $@CharId2 = .@cid; }\n'
    '\telse if( $@Winn3$ == .@name$ ) { $@Score3 = .@score; $@CharId3 = .@cid; }\n'
    '\telse if( $@Winn3$ == "" || .@score > $@Score3 ) {\n'
    '\t\t$@Winn3$ = .@name$; $@Score3 = .@score; $@CharId3 = .@cid;\n'
    '\t} else return;\n'
    '\t// Tri \xe0 bulles d\xe9croissant (3 \xe9l\xe9ments)\n'
    '\tif( $@Score2 > $@Score1 ) {\n'
    '\t\t.@t$ = $@Winn1$; $@Winn1$ = $@Winn2$; $@Winn2$ = .@t$;\n'
    '\t\t.@t  = $@Score1; $@Score1 = $@Score2; $@Score2 = .@t;\n'
    '\t\t.@t  = $@CharId1; $@CharId1 = $@CharId2; $@CharId2 = .@t;\n'
    '\t}\n'
    '\tif( $@Score3 > $@Score2 ) {\n'
    '\t\t.@t$ = $@Winn2$; $@Winn2$ = $@Winn3$; $@Winn3$ = .@t$;\n'
    '\t\t.@t  = $@Score2; $@Score2 = $@Score3; $@Score3 = .@t;\n'
    '\t\t.@t  = $@CharId2; $@CharId2 = $@CharId3; $@CharId3 = .@t;\n'
    '\t}\n'
    '\tif( $@Score2 > $@Score1 ) {\n'
    '\t\t.@t$ = $@Winn1$; $@Winn1$ = $@Winn2$; $@Winn2$ = .@t$;\n'
    '\t\t.@t  = $@Score1; $@Score1 = $@Score2; $@Score2 = .@t;\n'
    '\t\t.@t  = $@CharId1; $@CharId1 = $@CharId2; $@CharId2 = .@t;\n'
    '\t}\n'
    '\treturn;\n'
)
if old2 not in c: errors.append('P2')

# ── P3 : bloc récompenses end_invasion ──────────────────────────────────────
old3 = (
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
new3 = (
    '\t// Pool de costumes (commun aux 3 places)\n'
    '\tsetarray .@reward[0], 20219, 20297, 31251, 20026, 19857, 19959, 20103,\n'
    '\t                       31106, 31107, 19817, 20075, 20433, 19521, 19514,\n'
    '\t                       20259, 31518, 20183;\n'
    '\n'
    '\t// === 1\xe8re place : mail direct (fonctionne en ligne et hors-ligne) ===\n'
    '\tif( $@CharId1 > 0 ) {\n'
    '\t\t.@item1 = .@reward[rand(getarraysize(.@reward))];\n'
    '\t\tsetarray .@mi[0], 12103, 12210, 570, .@item1;\n'
    '\t\tsetarray .@ma[0], 5, 1, 30, 1;\n'
    '\t\tmail $@CharId1, "Invasion", "1\xe8re place !", "F\xe9licitations ! Vos r\xe9compenses sont dans ce mail.", 0, .@mi, .@ma;\n'
    '\t\tattachrid($@CharId1);\n'
    '\t\tif( playerattached() ) {\n'
    '\t\t\tif( #maluswininvasion <= 0 ) #maluswininvasion += 100;\n'
    '\t\t\telse #maluswininvasion += 100 + (#maluswininvasion / 2);\n'
    '\t\t\tlogmes "1er invasion (" + $@Score1 + " kills) malus " + #maluswininvasion;\n'
    '\t\t}\n'
    '\t\tannounce $@Winn1$ + " termine 1er avec " + $@Score1 + " kills !",bc_all|bc_npc,0xFFFF00;\n'
    '\t\tannounce $@Winn1$ + " gagne 5 Bloody Branch, 1 Bubble Gum X2, 30 Lucky Candy et " + getitemname(.@item1) + " !",bc_all|bc_npc,0xFFFF00;\n'
    '\t}\n'
    '\n'
    '\t// === 2\xe8me place ===\n'
    '\tif( $@CharId2 > 0 ) {\n'
    '\t\t.@item2 = .@reward[rand(getarraysize(.@reward))];\n'
    '\t\tsetarray .@mi[0], 12103, 570, .@item2;\n'
    '\t\tsetarray .@ma[0], 3, 15, 1;\n'
    '\t\tmail $@CharId2, "Invasion", "2\xe8me place !", "F\xe9licitations ! Vos r\xe9compenses sont dans ce mail.", 0, .@mi, .@ma;\n'
    '\t\tattachrid($@CharId2);\n'
    '\t\tif( playerattached() ) {\n'
    '\t\t\t#maluswininvasion += 50;\n'
    '\t\t\tlogmes "2\xe8me invasion (" + $@Score2 + " kills) malus " + #maluswininvasion;\n'
    '\t\t}\n'
    '\t\tannounce $@Winn2$ + " termine 2\xe8me avec " + $@Score2 + " kills !",bc_all|bc_npc,0xFFFF00;\n'
    '\t\tannounce $@Winn2$ + " gagne 3 Bloody Branch, 15 Lucky Candy et " + getitemname(.@item2) + " !",bc_all|bc_npc,0xFFFF00;\n'
    '\t}\n'
    '\n'
    '\t// === 3\xe8me place ===\n'
    '\tif( $@CharId3 > 0 ) {\n'
    '\t\tsetarray .@mi[0], 12103, 570;\n'
    '\t\tsetarray .@ma[0], 1, 5;\n'
    '\t\tmail $@CharId3, "Invasion", "3\xe8me place !", "F\xe9licitations ! Vos r\xe9compenses sont dans ce mail.", 0, .@mi, .@ma;\n'
    '\t\tattachrid($@CharId3);\n'
    '\t\tif( playerattached() ) {\n'
    '\t\t\t#maluswininvasion += 25;\n'
    '\t\t\tlogmes "3\xe8me invasion (" + $@Score3 + " kills) malus " + #maluswininvasion;\n'
    '\t\t}\n'
    '\t\tannounce $@Winn3$ + " termine 3\xe8me avec " + $@Score3 + " kills !",bc_all|bc_npc,0xFFFF00;\n'
    '\t\tannounce $@Winn3$ + " gagne 1 Bloody Branch et 5 Lucky Candy !",bc_all|bc_npc,0xFFFF00;\n'
    '\t}\n'
    '\n'
    '\t$@Score1 = 0; $@Score2 = 0; $@Score3 = 0;\n'
    '\t$@Winn1$ = ""; $@Winn2$ = ""; $@Winn3$ = "";\n'
    '\t$@CharId1 = 0; $@CharId2 = 0; $@CharId3 = 0;\n'
)
if old3 not in c: errors.append('P3')

# ── application ──────────────────────────────────────────────────────────────
if errors:
    print("ERREURS patterns non trouves:", errors)
    sys.exit(1)

c = c.replace(old1, new1, 1)
c = c.replace(old2, new2, 1)
c = c.replace(old3, new3, 1)

# Vérification anti-corruption
raw = c.encode("latin-1")
if b'\xef\xbf\xbd' in raw:
    print("ERREUR: bytes UTF-8 detectes !")
    sys.exit(1)

with open(PATH, "w", encoding="latin-1") as f:
    f.write(c)

print("OK - 3 remplacements appliques")
