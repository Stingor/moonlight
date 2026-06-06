#!/usr/bin/env python3
"""
Refactor MVP event : supprime la requete mob_spawn SQL, utilise getmvprespawn() C++.

Changements :
  1. mvps.npc   : remplace query_sql mob_spawn par sentinelle respawn_at=0
                  + exclut lhz_dun03 / lhz_dun04 / niflheim (spawn script, hors cache)
  2. groq.npc   : ajoute le bloc MVP event dans OnTimer3000
                  (resolution cache -> depart -> retour / timeout)
  3. groq.npc   : corrige S_GmReset (killmonster NPC fantome -> enablenpc)

Encodage : latin-1.
"""
import os, sys

MVPS = r"D:\Mes documents\GitHub\moonlight\moon\mobs\mvps.npc"
GROQ = r"D:\Mes documents\GitHub\moonlight\moon\groq.npc"

# =============================================================================
# 1. mvps.npc — remplacer le bloc query_sql mob_spawn
# =============================================================================
with open(MVPS, encoding="latin-1") as f:
    mv = f.read()

OLD1 = (
    "getmapxy(.@mv_map$, .@mx, .@my, BL_PC);\n"
    "\t\t\t\t\tquery_sql \"SELECT delay1, x, y FROM mob_spawn WHERE mob_id=\"+killedrid+\" AND map='\"+.@mv_map$+\"' LIMIT 1\","
    " .@mv_delay, .@mv_x, .@mv_y;\n"
    "\t\t\t\t\tif (.@mv_delay > 0) {\n"
    "\t\t\t\t\t\t$sting_mvp_ev_mob_id     = killedrid;\n"
    "\t\t\t\t\t\t$sting_mvp_ev_name$       = getmonsterinfo(killedrid, MOB_NAME);\n"
    "\t\t\t\t\t\t$sting_mvp_ev_map$        = .@mv_map$;\n"
    "\t\t\t\t\t\t$sting_mvp_ev_x           = .@mv_x;\t// coordonnees du spawn (pas du kill)\n"
    "\t\t\t\t\t\t$sting_mvp_ev_y           = .@mv_y;\n"
    "\t\t\t\t\t\t$sting_mvp_ev_respawn_at  = gettimetick(2) + .@mv_delay / 1000;\n"
    "\t\t\t\t\t\t$sting_mvp_ev_active      = 1;\n"
    "\t\t\t\t\t\t$sting_mvp_ev_mvp_dead    = 0;\n"
    "\t\t\t\t\t}\n"
    "\t\t\t\t}\n"
)
NEW1 = (
    "getmapxy(.@mv_map$, .@mx, .@my, BL_PC);\n"
    "\t\t\t\t\t// Exclure maps a spawn script : pas de mvp_respawn_cache C++ pour ces mobs\n"
    "\t\t\t\t\t// (biolab lhz_dun03/04 spawnes par timer NPC, LoD niflheim par invasion script)\n"
    "\t\t\t\t\tif (.@mv_map$ != \"lhz_dun03\" && .@mv_map$ != \"lhz_dun04\" && .@mv_map$ != \"niflheim\") {\n"
    "\t\t\t\t\t\t$sting_mvp_ev_mob_id     = killedrid;\n"
    "\t\t\t\t\t\t$sting_mvp_ev_name$      = getmonsterinfo(killedrid, MOB_NAME);\n"
    "\t\t\t\t\t\t$sting_mvp_ev_map$       = .@mv_map$;\n"
    "\t\t\t\t\t\t$sting_mvp_ev_x          = 0;  // resolu via getmvprespawn() dans OnTimer3000\n"
    "\t\t\t\t\t\t$sting_mvp_ev_y          = 0;\n"
    "\t\t\t\t\t\t$sting_mvp_ev_killer$    = $sting_mvp_killer$;\n"
    "\t\t\t\t\t\t$sting_mvp_ev_respawn_at = 0;  // sentinelle : cache C++ pas encore peuple\n"
    "\t\t\t\t\t\t$sting_mvp_ev_active     = 1;\n"
    "\t\t\t\t\t\t$sting_mvp_ev_mvp_dead   = 0;\n"
    "\t\t\t\t\t}\n"
    "\t\t\t\t}\n"
)
if OLD1 in mv:
    mv = mv.replace(OLD1, NEW1, 1)
    print("OK 1: bloc mob_spawn remplace dans mvps.npc")
else:
    print("ERREUR 1: ancre mvps.npc non trouvee", file=sys.stderr); sys.exit(1)

try:
    enc_mv = mv.encode("latin-1")
except UnicodeEncodeError as e:
    print(f"ERREUR encodage mvps.npc: {e}", file=sys.stderr); sys.exit(1)

TMP = MVPS + ".tmp"
with open(TMP, "wb") as f: f.write(enc_mv)
os.replace(TMP, MVPS)
print("   mvps.npc ecrit en latin-1")

# =============================================================================
# 2 & 3. groq.npc — bloc MVP dans OnTimer3000 + correction S_GmReset
# =============================================================================
with open(GROQ, encoding="latin-1") as f:
    gr = f.read()

# --- 2. Bloc MVP event dans OnTimer3000 (avant // --- Scan HP) ---------------
MVP_BLOCK = (
    "\t// --- Event MVP : Sting file sur la map d'un MVP qui va respawn ---\n"
    "\t// active=1 : en attente resolution respawn_at | active=2 : Sting parti\n"
    "\tif ($sting_mvp_ev_active == 1) {\n"
    "\t\tif (.pvpevent == 0) {\n"
    "\t\t\tif (.trip == 0) {\n"
    "\t\t\t\t// Resolution via cache C++ (peuple quelques secondes apres mob_setdelayspawn)\n"
    "\t\t\t\tif ($sting_mvp_ev_respawn_at == 0) {\n"
    "\t\t\t\t\t.@_s = getmvprespawn($sting_mvp_ev_mob_id, .@_x, .@_y, .@_m$, .@_k$, .@_t);\n"
    "\t\t\t\t\tif (.@_s >= 0) {\n"
    "\t\t\t\t\t\t$sting_mvp_ev_respawn_at = gettimetick(2) + .@_s;\n"
    "\t\t\t\t\t\t$sting_mvp_ev_x          = .@_x;\n"
    "\t\t\t\t\t\t$sting_mvp_ev_y          = .@_y;\n"
    "\t\t\t\t\t\tif (.@_k$ != \"\") { $sting_mvp_ev_killer$ = .@_k$; }\n"
    "\t\t\t\t\t}\n"
    "\t\t\t\t\t// Sinon : retry au prochain tick (3s)\n"
    "\t\t\t\t}\n"
    "\t\t\t\t// Decider du depart selon la fenetre de temps\n"
    "\t\t\t\tif ($sting_mvp_ev_respawn_at > 0) {\n"
    "\t\t\t\t\t.@_delta = $sting_mvp_ev_respawn_at - gettimetick(2);\n"
    "\t\t\t\t\t// Trop tard (> 5 min apres respawn prevu) : annuler l'event\n"
    "\t\t\t\t\tif (.@_delta < -300) {\n"
    "\t\t\t\t\t\t$sting_mvp_ev_cd     = gettimetick(2);\n"
    "\t\t\t\t\t\t$sting_mvp_ev_active = 0;\n"
    "\t\t\t\t\t}\n"
    "\t\t\t\t\t// Dans la fenetre (<= 2 min avant ou < 5 min apres) : Sting part\n"
    "\t\t\t\t\tif (.@_delta >= -300 && .@_delta <= 120) {\n"
    "\t\t\t\t\t\t$sting_mvp_ev_active = 2;\n"
    "\t\t\t\t\t\t.trip = 1;\n"
    "\t\t\t\t\t\t.@_t2$ = strnpcinfo(0)+\" : Je file sur \"+$sting_mvp_ev_map$+\" surveiller \"+$sting_mvp_ev_name$+\". Il va pas respawn sans moi.\";\n"
    "\t\t\t\t\t\tnpctalk .@_t2$; logchat .@_t2$;\n"
    "\t\t\t\t\t\tannounce strnpcinfo(0)+\" part surveiller \"+$sting_mvp_ev_name$+\" sur \"+$sting_mvp_ev_map$+\" ! Rejoignez-le pour 5 points d'event !\", bc_blue|bc_all;\n"
    "\t\t\t\t\t\tsleep 1000;\n"
    "\t\t\t\t\t\tnpcwalkto 165, 127;\n"
    "\t\t\t\t\t\tsleep 6000;\n"
    "\t\t\t\t\t\tdisablenpc strnpcinfo(3);\n"
    "\t\t\t\t\t\tinitnpctimer;\n"
    "\t\t\t\t\t\tend;\n"
    "\t\t\t\t\t}\n"
    "\t\t\t\t}\n"
    "\t\t\t}\n"
    "\t\t}\n"
    "\t}\n"
    "\t// Phase active : Sting est sur la map, en attente du kill MVP\n"
    "\tif ($sting_mvp_ev_active == 2) {\n"
    "\t\t// MVP tue pendant l'event -> Sting rentre et pose le cooldown\n"
    "\t\tif ($sting_mvp_ev_mvp_dead == 1) {\n"
    "\t\t\tmovenpc strnpcinfo(3), .ox, .oy;\n"
    "\t\t\tnpcstop;\n"
    "\t\t\tenablenpc strnpcinfo(3);\n"
    "\t\t\t.@_t3$ = strnpcinfo(0)+\" : Bien jou\xe9. Maintenant repartez farmer.\";\n"
    "\t\t\tnpctalk .@_t3$; logchat .@_t3$;\n"
    "\t\t\t$sting_mvp_ev_cd     = gettimetick(2);\n"
    "\t\t\t$sting_mvp_ev_active = 0;\n"
    "\t\t\t.trip = 0;\n"
    "\t\t}\n"
    "\t\t// Timeout : MVP pas tue dans les 3h apres le respawn prevu\n"
    "\t\tif ($sting_mvp_ev_mvp_dead == 0) {\n"
    "\t\t\tif (gettimetick(2) - $sting_mvp_ev_respawn_at > 10800) {\n"
    "\t\t\t\tmovenpc strnpcinfo(3), .ox, .oy;\n"
    "\t\t\t\tnpcstop;\n"
    "\t\t\t\tenablenpc strnpcinfo(3);\n"
    "\t\t\t\t$sting_mvp_ev_cd     = gettimetick(2);\n"
    "\t\t\t\t$sting_mvp_ev_active = 0;\n"
    "\t\t\t\t.trip = 0;\n"
    "\t\t\t}\n"
    "\t\t}\n"
    "\t}\n"
    "\n"
)

OLD2 = "\t\t\tinitnpctimer;\n\t\t}\n\t\tend;\n\t}\n\n\t// --- Scan HP"
NEW2 = "\t\t\tinitnpctimer;\n\t\t}\n\t\tend;\n\t}\n\n" + MVP_BLOCK + "\t// --- Scan HP"

if OLD2 in gr:
    gr = gr.replace(OLD2, NEW2, 1)
    print("OK 2: bloc MVP event ajoute dans OnTimer3000")
else:
    print("ERREUR 2: ancre fin-PvP/Scan-HP non trouvee", file=sys.stderr); sys.exit(1)

# --- 3. S_GmReset : remplacer killmonster fantome par enablenpc ----------
OLD3 = (
    "\t// Tuer Stingor si event MVP en phase active (active=2)\n"
    "\tif ($sting_mvp_ev_active == 2) {\n"
    "\t\tkillmonster $sting_mvp_ev_map$, \"#sting_mvp::OnStingMvpDead\";\n"
    "\t}\n"
)
NEW3 = (
    "\t// Restaurer Sting si event MVP en phase active (active=2 = disablenpc)\n"
    "\tif ($sting_mvp_ev_active == 2) {\n"
    "\t\tenablenpc strnpcinfo(3);\n"
    "\t\tnpcstop;\n"
    "\t\tmovenpc strnpcinfo(3), .ox, .oy;\n"
    "\t}\n"
)
if OLD3 in gr:
    gr = gr.replace(OLD3, NEW3, 1)
    print("OK 3: S_GmReset corrige (killmonster -> enablenpc)")
else:
    print("ERREUR 3: ancre S_GmReset/killmonster non trouvee", file=sys.stderr); sys.exit(1)

# --- Verification et ecriture -----------------------------------------------
bad = gr.count('\xef\xbf\xbd')
print("\n3/3 modifications appliquees - bytes 0xEFBFBD : %d" % bad)
if bad > 0:
    print("ERREUR: bytes UTF-8 corrompus!", file=sys.stderr); sys.exit(1)

try:
    enc_gr = gr.encode("latin-1")
except UnicodeEncodeError as e:
    print(f"ERREUR encodage groq.npc: {e}", file=sys.stderr)
    for i, c in enumerate(gr):
        if ord(c) > 255:
            print("  pos %d: U+%04X ctx=%r" % (i, ord(c), gr[max(0,i-30):i+30]), file=sys.stderr)
    sys.exit(1)

TMP = GROQ + ".tmp"
with open(TMP, "wb") as f: f.write(enc_gr)
os.replace(TMP, GROQ)
print("groq.npc ecrit en latin-1 :", GROQ)
