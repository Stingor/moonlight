#!/usr/bin/env python3
"""
Rend persistants les cooldowns de Sting-Bot (survivent aux reloads/restarts).
Variables mirroir globales : $sting_persist_pvpcdtick, _tripcd, _buffcd, _mockcd

Changements :
  1. OnTimer3000 : sauvegarde des 4 cooldowns au debut de chaque tick (toutes les 3s)
  2. OnInit      : restauration depuis les globaux apres les defaults
  3. S_GmReset   : efface les globaux de persistance lors d'un reset complet
Encodage : latin-1.
"""
import os, sys

PATH = r"D:\Mes documents\GitHub\moonlight\moon\groq.npc"

with open(PATH, encoding="latin-1") as f:
    src = f.read()

changes = 0

# -----------------------------------------------------------------------------
# 1. OnTimer3000 : sauvegarde au debut du timer
# -----------------------------------------------------------------------------
OLD1 = (
    "OnTimer3000:\n"
    "\t// --- Relai shoutbox web"
)
NEW1 = (
    "OnTimer3000:\n"
    "\t// [Persist] Sauvegarde des cooldowns (survit aux reloads et redemarrages serveur)\n"
    "\t$sting_persist_pvpcdtick = .pvpcdtick;\n"
    "\t$sting_persist_tripcd    = .tripcd;\n"
    "\t$sting_persist_buffcd    = .buffcd;\n"
    "\t$sting_persist_mockcd    = .mockcd;\n"
    "\t// --- Relai shoutbox web"
)
if OLD1 in src:
    src = src.replace(OLD1, NEW1, 1); changes += 1
    print("OK 1: sauvegarde ajoutee dans OnTimer3000")
else:
    print("ERREUR 1: ancre OnTimer3000 non trouvee", file=sys.stderr); sys.exit(1)

# -----------------------------------------------------------------------------
# 2. OnInit : restauration apres les defaults, juste avant activatepset
# -----------------------------------------------------------------------------
OLD2 = (
    "\t$sting_dbg_forcepvp  = 0;  // flag debug GM : force declenchement event PvP\n"
    "\tactivatepset 2;\n"
)
NEW2 = (
    "\t$sting_dbg_forcepvp  = 0;  // flag debug GM : force declenchement event PvP\n"
    "\t// [Persist] Restauration des cooldowns apres reload ou redemarrage\n"
    "\t// Les timestamps > 0 indiquent une valeur sauvegardee (gettimetick(2) ~ 1.7e9)\n"
    "\tif ($sting_persist_pvpcdtick > 0) { .pvpcdtick = $sting_persist_pvpcdtick; }\n"
    "\tif ($sting_persist_tripcd > 0)    { .tripcd    = $sting_persist_tripcd; }\n"
    "\tif ($sting_persist_buffcd > 0)    { .buffcd    = $sting_persist_buffcd; }\n"
    "\tif ($sting_persist_mockcd > 0)    { .mockcd    = $sting_persist_mockcd; }\n"
    "\tactivatepset 2;\n"
)
if OLD2 in src:
    src = src.replace(OLD2, NEW2, 1); changes += 1
    print("OK 2: restauration ajoutee dans OnInit")
else:
    print("ERREUR 2: ancre forcepvp/activatepset non trouvee", file=sys.stderr); sys.exit(1)

# -----------------------------------------------------------------------------
# 3. S_GmReset : effacer les globaux de persistance lors du reset complet
# -----------------------------------------------------------------------------
OLD3 = (
    "\t$sting_dbg_forcepvp      = 0;\n"
    "\t// Reset \xe9tat NPC\n"
)
NEW3 = (
    "\t$sting_dbg_forcepvp      = 0;\n"
    "\t// [Persist] Efface les cooldowns sauvegardes (reset complet = on repart de zero)\n"
    "\t$sting_persist_pvpcdtick = 0;\n"
    "\t$sting_persist_tripcd    = 0;\n"
    "\t$sting_persist_buffcd    = 0;\n"
    "\t$sting_persist_mockcd    = 0;\n"
    "\t// Reset \xe9tat NPC\n"
)
if OLD3 in src:
    src = src.replace(OLD3, NEW3, 1); changes += 1
    print("OK 3: clear persist ajoutee dans S_GmReset")
else:
    print("ERREUR 3: ancre S_GmReset/Reset etat NPC non trouvee", file=sys.stderr); sys.exit(1)

# -----------------------------------------------------------------------------
# Verification finale + ecriture atomique
# -----------------------------------------------------------------------------
bad_utf = src.count('\xef\xbf\xbd')
print("\n%d/3 modifications appliquees - bytes 0xEFBFBD : %d" % (changes, bad_utf))

if bad_utf > 0:
    print("ERREUR: bytes UTF-8 corrompus detectes !", file=sys.stderr); sys.exit(1)

try:
    encoded = src.encode('latin-1')
except UnicodeEncodeError as e:
    print("ERREUR encodage latin-1: %s" % e, file=sys.stderr)
    for i, c in enumerate(src):
        if ord(c) > 255:
            print("  pos %d: U+%04X ctx=%r" % (i, ord(c), src[max(0,i-30):i+30]), file=sys.stderr)
    sys.exit(1)

TMP = PATH + ".tmp"
with open(TMP, "wb") as f:
    f.write(encoded)
os.replace(TMP, PATH)
print("Fichier ecrit en latin-1 :", PATH)
