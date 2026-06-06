#!/usr/bin/env python3
"""
Ajoute un menu debug GM au NPC Sting-Bot (moon/groq.npc).
Changements :
  1. Corps click : check GM -> callsub S_GmMenu avant close
  2. OnTimer3000 : restructure trigger PvP pour supporter $sting_dbg_forcepvp
  3. Avant OnInit : subroutines GM (S_GmMenu, S_GmForceMvp, S_GmForcePvp,
                    S_GmReset, S_GmShowVars)
  4. OnInit : initialise $sting_dbg_forcepvp = 0
Encodage : latin-1.
"""
import sys

PATH = r"D:\Mes documents\GitHub\moonlight\moon\groq.npc"

with open(PATH, encoding="latin-1") as f:
    src = f.read()

changes = 0

# -----------------------------------------------------------------------------
# 1. Corps click : ajouter le menu GM avant close
# -----------------------------------------------------------------------------
OLD1 = "\tclose;\n\nL_chat:\n"
NEW1 = (
    "\tif (getgmlevel() >= 99) {\n"
    "\t\tnext;\n"
    "\t\tcallsub S_GmMenu;\n"
    "\t\tend;\n"
    "\t}\n"
    "\tclose;\n"
    "\n"
    "L_chat:\n"
)
if OLD1 in src:
    src = src.replace(OLD1, NEW1, 1); changes += 1
    print("OK 1: check GM ajouté dans le corps click")
else:
    print("ERREUR 1: ancre close/L_chat non trouvée", file=sys.stderr); sys.exit(1)

# -----------------------------------------------------------------------------
# 2. Restructure le trigger PvP pour supporter le flag debug
#    OLD: if (.pvpevent == 0 && gettimetick(2) - .pvpcdtick > .pvpcd && rand(1000) < 10) {
#    NEW: .@do_pvp = 0; if/else propre avec force flag
# -----------------------------------------------------------------------------
OLD2 = (
    "\t// --- Event PvP : Sting defie les joueurs et part affronter qui ose au PvP ---\n"
    "\tif (.pvpevent == 0 && gettimetick(2) - .pvpcdtick > .pvpcd && rand(1000) < 10) {\n"
)
NEW2 = (
    "\t// --- Event PvP : Sting defie les joueurs et part affronter qui ose au PvP ---\n"
    "\t// $sting_dbg_forcepvp=1 : flag debug GM pour forcer l'event sans cooldown ni rand\n"
    "\t.@do_pvp = 0;\n"
    "\tif (.pvpevent == 0) {\n"
    "\t\tif ($sting_dbg_forcepvp == 1) {\n"
    "\t\t\t$sting_dbg_forcepvp = 0;\n"
    "\t\t\t.@do_pvp = 1;\n"
    "\t\t}\n"
    "\t\tif (.@do_pvp == 0) {\n"
    "\t\t\tif (gettimetick(2) - .pvpcdtick > .pvpcd) {\n"
    "\t\t\t\tif (rand(1000) < 10) { .@do_pvp = 1; }\n"
    "\t\t\t}\n"
    "\t\t}\n"
    "\t}\n"
    "\tif (.@do_pvp) {\n"
)
if OLD2 in src:
    src = src.replace(OLD2, NEW2, 1); changes += 1
    print("OK 2: trigger PvP restructuré avec force flag")
else:
    print("ERREUR 2: ancre trigger PvP non trouvée", file=sys.stderr); sys.exit(1)

# -----------------------------------------------------------------------------
# 3. Subroutines GM insérées juste avant OnInit
# -----------------------------------------------------------------------------
GM_SUBS = (
    # -- S_GmMenu --------------------------------------------------------------
    "// ===========================================================\n"
    "// SUBROUTINES DEBUG GM (getgmlevel() >= 99 requis)\n"
    "// ===========================================================\n"
    "S_GmMenu:\n"
    "\tmes \"^FF0000[DEBUG GM]^000000 \" + strnpcinfo(0);\n"
    "\tmes \"\";\n"
    "\tmes \"^888888.online=^000000\" + .online + \"^888888  .trip=^000000\" + .trip + \"^888888  .pvpevent=^000000\" + .pvpevent;\n"
    "\tmes \"^888888$mvp_ev_active=^000000\" + $sting_mvp_ev_active + \"^888888  mob=\"+$sting_mvp_ev_mob_id+\"  map=\"+$sting_mvp_ev_map$+\"^000000\";\n"
    "\tmes \"\";\n"
    "\t.@ch = select(\n"
    "\t\t\"^00AA00Force Event MVP^000000 (mob_id + map GM)\",\n"
    "\t\t\"^0066FF Skip d\xe9lai MVP^000000 (active=1 requis)\",\n"
    "\t\t\"^FF6600Force Event PvP^000000 (bypass rand+cd)\",\n"
    "\t\t\"^888888Mettre en PAUSE^000000 (offline+timer stop)\",\n"
    "\t\t\"^00AA00Reprendre^000000 (online+timer restart)\",\n"
    "\t\t\"^FF0000RESET COMPLET^000000 (variables+mobs+flags)\",\n"
    "\t\t\"^0000FFAfficher \xe9tat^000000 (toutes les variables)\",\n"
    "\t\t\"Fermer\"\n"
    "\t);\n"
    "\tnext;\n"
    "\tif (.@ch == 1) { callsub S_GmForceMvp; }\n"
    "\telse if (.@ch == 2) { callsub S_GmSkipMvpDelay; }\n"
    "\telse if (.@ch == 3) { callsub S_GmForcePvp; }\n"
    "\telse if (.@ch == 4) {\n"
    "\t\t.online = 0;\n"
    "\t\tstopnpctimer;\n"
    "\t\tmes \"^FF8800Sting mis en PAUSE (offline).^000000\";\n"
    "\t\tmes \"Timer arr\xeat\xe9. Utilise 'Reprendre' pour relancer.\";\n"
    "\t}\n"
    "\telse if (.@ch == 5) {\n"
    "\t\tif (.online == 0) {\n"
    "\t\t\t.online = 1;\n"
    "\t\t\tinitnpctimer;\n"
    "\t\t\tmes \"^00AA00Sting remis EN LIGNE.^000000\";\n"
    "\t\t} else {\n"
    "\t\t\tmes \"D\xe9j\xe0 en ligne (online=\" + .online + \").\"\x3b\n"
    "\t\t}\n"
    "\t}\n"
    "\telse if (.@ch == 6) { callsub S_GmReset; }\n"
    "\telse if (.@ch == 7) { callsub S_GmShowVars; }\n"
    "\treturn;\n"
    "\n"
    # -- S_GmForceMvp ----------------------------------------------------------
    "S_GmForceMvp:\n"
    "\tmes \"^FF0000[Force Event MVP]^000000\";\n"
    "\tmes \"Entrez le mob_id du MVP \xe0 simuler.\";\n"
    "\tmes \"^888888Exemples : 1113 Baphomet | 1327 Eddga | 1389 Dracula^000000\";\n"
    "\tmes \"^888888(laisser 0 pour annuler)^000000\";\n"
    "\tinput .@mob_id;\n"
    "\tif (.@mob_id <= 0) { mes \"^888888Annul\xe9.^000000\"; return; }\n"
    "\t.@mn$ = getmonsterinfo(.@mob_id, MOB_NAME);\n"
    "\tif (.@mn$ == \"NULL\" || .@mn$ == \"\") {\n"
    "\t\tmes \"^FF0000Mob ID \" + .@mob_id + \" inconnu.^000000\";\n"
    "\t\treturn;\n"
    "\t}\n"
    "\tif ($sting_mvp_ev_active == 2) {\n"
    "\t\tmes \"^FF0000Un event MVP est d\xe9j\xe0 en cours (active=2).^000000\";\n"
    "\t\tmes \"Utilisez RESET COMPLET d'abord.\";\n"
    "\t\treturn;\n"
    "\t}\n"
    "\t$sting_mvp_ev_cd         = 0;                      // bypass cooldown\n"
    "\t$sting_mvp_ev_mob_id     = .@mob_id;\n"
    "\t$sting_mvp_ev_name$      = .@mn$;\n"
    "\t$sting_mvp_ev_map$       = strcharinfo(3);         // map du GM\n"
    "\t$sting_mvp_ev_x          = 0;                      // spawn al\xe9atoire\n"
    "\t$sting_mvp_ev_y          = 0;\n"
    "\t$sting_mvp_ev_respawn_at = gettimetick(2) + 3;    // d\xe9clenche dans ~3s\n"
    "\t$sting_mvp_ev_active     = 1;\n"
    "\t$sting_mvp_ev_mvp_dead   = 0;\n"
    "\t$sting_mvp_ev_killer$    = \"\";\n"
    "\tmes \"^00AA00Event MVP configur\xe9 pour \" + .@mn$ + \" (id=\" + .@mob_id + \").^000000\";\n"
    "\tmes \"Map : ^0000FF\" + $sting_mvp_ev_map$ + \"^000000 (spawn al\xe9atoire)\";\n"
    "\tmes \"OnTimer3000 d\xe9clenchera le d\xe9part de Sting dans ~3s.\";\n"
    "\treturn;\n"
    "\n"
    # -- S_GmSkipMvpDelay ------------------------------------------------------
    "S_GmSkipMvpDelay:\n"
    "\tif ($sting_mvp_ev_active != 1) {\n"
    "\t\tmes \"^FF0000Aucun event MVP en attente (active=\" + $sting_mvp_ev_active + \").^000000\";\n"
    "\t\tmes \"Utilisez 'Force Event MVP' d'abord.\";\n"
    "\t\treturn;\n"
    "\t}\n"
    "\t$sting_mvp_ev_respawn_at = gettimetick(2) - 200;  // passe le seuil -120s\n"
    "\tmes \"^00AA00D\xe9lai MVP supprim\xe9.^000000\";\n"
    "\tmes \"Le prochain tick OnTimer3000 d\xe9clenchera imm\xe9diatement le d\xe9part de Sting.\";\n"
    "\treturn;\n"
    "\n"
    # -- S_GmForcePvp ----------------------------------------------------------
    "S_GmForcePvp:\n"
    "\tif (.pvpevent != 0 || .trip != 0) {\n"
    "\t\tmes \"^FF0000Impossible : un event est d\xe9j\xe0 actif.^000000\";\n"
    "\t\tmes \".pvpevent=\" + .pvpevent + \"  .trip=\" + .trip;\n"
    "\t\treturn;\n"
    "\t}\n"
    "\tif (.online == 0) {\n"
    "\t\tmes \"^FF0000Sting est offline. Remettez-le en ligne d'abord.^000000\";\n"
    "\t\treturn;\n"
    "\t}\n"
    "\tif ($sting_mvp_ev_active != 0) {\n"
    "\t\tmes \"^FF0000Un event MVP est en cours (active=\" + $sting_mvp_ev_active + \").^000000\";\n"
    "\t\tmes \"Utilisez RESET COMPLET d'abord.\";\n"
    "\t\treturn;\n"
    "\t}\n"
    "\t$sting_dbg_forcepvp = 1;\n"
    "\tmes \"^00AA00Event PvP forc\xe9.^000000\";\n"
    "\tmes \"OnTimer3000 d\xe9clenchera le d\xe9part de Sting vers pvp_n_3-5 dans ~3s.\";\n"
    "\tmes \"^888888(cooldown et rand() bypass\xe9s)^000000\";\n"
    "\treturn;\n"
    "\n"
    # -- S_GmReset -------------------------------------------------------------
    "S_GmReset:\n"
    "\tmes \"^FF0000[RESET COMPLET]^000000\";\n"
    "\t// Tuer Stingor si event MVP en phase active (active=2)\n"
    "\tif ($sting_mvp_ev_active == 2) {\n"
    "\t\tkillmonster $sting_mvp_ev_map$, \"#sting_mvp::OnStingMvpDead\";\n"
    "\t}\n"
    "\t// Retirer le flag PvP de la map MVP si on l'avait pos\xe9\n"
    "\tif ($sting_pvpevent == 1 && $sting_mvp_ev_map_was_pvp == 0 && $sting_mvp_ev_map$ != \"\") {\n"
    "\t\tremovemapflag $sting_mvp_ev_map$, mf_pvp;\n"
    "\t}\n"
    "\t// Tuer Stingor si event PvP en cours\n"
    "\tif (.pvpevent == 1) {\n"
    "\t\tkillmonster \"pvp_n_3-5\", \"#sting_pvp::OnStingDead\";\n"
    "\t\tremovemapflag \"pvp_n_3-5\", mf_pvp;\n"
    "\t}\n"
    "\t// Reset variables globales MVP\n"
    "\t$sting_mvp_ev_active     = 0;\n"
    "\t$sting_mvp_ev_cd         = 0;\n"
    "\t$sting_mvp_ev_mob_id     = 0;\n"
    "\t$sting_mvp_ev_name$      = \"\";\n"
    "\t$sting_mvp_ev_map$       = \"\";\n"
    "\t$sting_mvp_ev_x          = 0;\n"
    "\t$sting_mvp_ev_y          = 0;\n"
    "\t$sting_mvp_ev_respawn_at = 0;\n"
    "\t$sting_mvp_ev_mvp_dead   = 0;\n"
    "\t$sting_mvp_ev_killer$    = \"\";\n"
    "\t$sting_pvpevent          = 0;\n"
    "\t$sting_dbg_forcepvp      = 0;\n"
    "\t// Reset \xe9tat NPC\n"
    "\t.pvpevent = 0;\n"
    "\t.trip     = 0;\n"
    "\t.online   = 1;\n"
    "\t// Re-enable et repositionner le NPC\n"
    "\tenablenpc strnpcinfo(3);\n"
    "\tnpcstop;\n"
    "\tmovenpc strnpcinfo(3), .ox, .oy;\n"
    "\t// Relancer le timer\n"
    "\tstopnpctimer;\n"
    "\tinitnpctimer;\n"
    "\tmes \"^00AA00Reset complet effectu\xe9.^000000\";\n"
    "\tmes \"Timer relanc\xe9, NPC repositionn\xe9 en (\"+.ox+\",\"+.oy+\").\";\n"
    "\treturn;\n"
    "\n"
    # -- S_GmShowVars ----------------------------------------------------------
    "S_GmShowVars:\n"
    "\tmes \"^FF0000[\xc9tat des variables]^000000\";\n"
    "\tmes \"\";\n"
    "\tmes \"^0000FFNPC local :^000000\";\n"
    "\tmes \"  .online=\" + .online + \"  .trip=\" + .trip + \"  .pvpevent=\" + .pvpevent;\n"
    "\tmes \"  .pvpcdtick=\" + .pvpcdtick + \"  .pvpcd=\" + .pvpcd;\n"
    "\tmes \"  .afkcd=\" + .afkcd + \"  .buffcd=\" + .buffcd;\n"
    "\tmes \"\";\n"
    "\tmes \"^0000FFEvent MVP :^000000\";\n"
    "\tmes \"  $active=\" + $sting_mvp_ev_active + \"  $mob=\" + $sting_mvp_ev_mob_id;\n"
    "\tmes \"  $name=\" + $sting_mvp_ev_name$ + \"  $map=\" + $sting_mvp_ev_map$;\n"
    "\tmes \"  $x=\" + $sting_mvp_ev_x + \"  $y=\" + $sting_mvp_ev_y;\n"
    "\t.@rt = $sting_mvp_ev_respawn_at - gettimetick(2);\n"
    "\tmes \"  respawn dans \" + .@rt + \"s  (mvp_dead=\" + $sting_mvp_ev_mvp_dead + \")\";\n"
    "\t.@cdd = gettimetick(2) - $sting_mvp_ev_cd;\n"
    "\tmes \"  cooldown \xe9coul\xe9 : \" + .@cdd + \"s / 10800s  killer=\" + $sting_mvp_ev_killer$;\n"
    "\tmes \"\";\n"
    "\tmes \"^0000FFEvent PvP :^000000\";\n"
    "\tmes \"  $sting_pvpevent=\" + $sting_pvpevent;\n"
    "\tmes \"  $sting_dbg_forcepvp=\" + $sting_dbg_forcepvp;\n"
    "\treturn;\n"
    "\n"
)

OLD3 = "\tinitnpctimer;   // en dernier, comme le retour trip (relance OnTimer3000)\n\tend;\n\nOnInit:\n"
NEW3 = "\tinitnpctimer;   // en dernier, comme le retour trip (relance OnTimer3000)\n\tend;\n\n" + GM_SUBS + "OnInit:\n"
if OLD3 in src:
    src = src.replace(OLD3, NEW3, 1); changes += 1
    print("OK 3: subroutines GM ajoutées avant OnInit")
else:
    print("ERREUR 3: ancre initnpctimer/OnInit non trouvée", file=sys.stderr); sys.exit(1)

# -----------------------------------------------------------------------------
# 4. Initialiser $sting_dbg_forcepvp dans OnInit (avant activatepset)
# -----------------------------------------------------------------------------
OLD4 = "\tactivatepset 2;\n\tinitnpctimer;\n"
NEW4 = (
    "\t$sting_dbg_forcepvp  = 0;  // flag debug GM : force declenchement event PvP\n"
    "\tactivatepset 2;\n"
    "\tinitnpctimer;\n"
)
if OLD4 in src:
    src = src.replace(OLD4, NEW4, 1); changes += 1
    print("OK 4: $sting_dbg_forcepvp initialise dans OnInit")
else:
    print("ERREUR 4: ancre activatepset non trouvee", file=sys.stderr); sys.exit(1)

# -----------------------------------------------------------------------------
# Verification finale + ecriture atomique
# -----------------------------------------------------------------------------
bad_utf = src.count('\xef\xbf\xbd')
print("\n%d/4 modifications appliquees - bytes 0xEFBFBD : %d" % (changes, bad_utf))

if bad_utf > 0:
    print("ERREUR: bytes UTF-8 corrompus detectes !", file=sys.stderr); sys.exit(1)

# Verifier l'encodage latin-1 AVANT d'ouvrir le fichier cible
try:
    encoded = src.encode('latin-1')
except UnicodeEncodeError as e:
    print("ERREUR encodage latin-1: %s" % e, file=sys.stderr)
    for i, c in enumerate(src):
        if ord(c) > 255:
            print("  pos %d: U+%04X ctx=%r" % (i, ord(c), src[max(0,i-30):i+30]), file=sys.stderr)
    sys.exit(1)

# Ecriture atomique : tmp puis rename (protege contre truncation en cas d'erreur)
import os
TMP = PATH + ".tmp"
with open(TMP, "wb") as f:
    f.write(encoded)
os.replace(TMP, PATH)
print("Fichier ecrit en latin-1 :", PATH)
