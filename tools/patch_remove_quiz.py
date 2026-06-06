#!/usr/bin/env python3
"""
Supprime entièrement le système de quiz spontané de moon/groq.npc :
  1. Label L_quiz (et tout son corps)
  2. Bloc quiz dans OnTimer3000 (timeout + trigger)
  3. .quiz_active = 0 dans le départ PvP
  4. .quiz_active = 0 dans le départ MVP
  5. Variables et Q&A quiz dans OnInit
  6. defpattern L_quiz dans OnInit
Encodage : latin-1.
"""
import sys

PATH = r"D:\Mes documents\GitHub\moonlight\moon\groq.npc"

with open(PATH, encoding="latin-1") as f:
    src = f.read()

changes = 0

# ── 1. Label L_quiz (avant L_chat) ───────────────────────────────────────────
OLD1 = (
    "L_quiz:\n"
    "\t// Catch-all : v\xe9rifie si la r\xe9ponse correspond au quiz actif.\n"
    "\t// Fir\xe9e pour TOUT message en chat (pattern catch-all). Quitte imm\xe9diatement si pas de quiz.\n"
    "\tif (.quiz_active == 0) end;\n"
    "\t.@resp_pl$  = $@p1$;\n"
    "\t.@resp_msg$ = $@p2$;\n"
    "\texplode(.@qav$, .quiz_a$[.quiz_idx], \"|\");\n"
    "\t.@qk = 0; .@qmatch = 0;\n"
    "\twhile (.@qav$[.@qk] != \"\") {\n"
    "\t\tif (.@resp_msg$ == .@qav$[.@qk]) { .@qmatch = 1; }\n"
    "\t\t.@qk++;\n"
    "\t}\n"
    "\tif (.@qmatch == 0) end;\n"
    "\t// Bonne r\xe9ponse !\n"
    "\t.quiz_active = 0;\n"
    "\t.quiz_cdtick = gettimetick(2);\n"
    "\t#KAFRAPOINTS += 3;\n"
    "\tdispbottom \"[Quiz Sting] Bonne r\xe9ponse ! +3 points d'event.\";\n"
    "\tannounce .@resp_pl$+\" a r\xe9pondu correctement au quiz de Sting ! +3 points d'event !\", bc_blue|bc_all;\n"
    "\t.@qwin$ = strnpcinfo(0)+\" : \"+.@resp_pl$+\"... pas mal. Comme quoi m\xeame les tocards ont leur moment.\";\n"
    "\tnpctalk .@qwin$; logchat .@qwin$;\n"
    "\tend;\n"
    "\n"
    "L_chat:\n"
)
NEW1 = "L_chat:\n"
if OLD1 in src:
    src = src.replace(OLD1, NEW1, 1); changes += 1
    print("OK 1: label L_quiz supprimé")
else:
    print("ERREUR 1: L_quiz non trouvé", file=sys.stderr); sys.exit(1)

# ── 2. Bloc quiz dans OnTimer3000 ─────────────────────────────────────────────
OLD2 = (
    "\t// --- Quiz spontan\xe9 : question RO, premier \xe0 r\xe9pondre gagne des kafra ---\n"
    "\tif (.quiz_active == 1) {\n"
    "\t\tif (gettimetick(2) - .quiztick >= 60) {\n"
    "\t\t\t.quiz_active = 0;\n"
    "\t\t\t.quiz_cdtick = gettimetick(2);\n"
    "\t\t\t.@tout$ = strnpcinfo(0)+\" : Personne en 60s... La r\xe9ponse : '\"+.quiz_expected$+\"'. Path\xe9tique.\";\n"
    "\t\t\tnpctalk .@tout$; logchat .@tout$;\n"
    "\t\t}\n"
    "\t}\n"
    "\tif (.quiz_active == 0 && .online == 1 && .pvpevent == 0 && .trip == 0) {\n"
    "\t\tif (.quiz_cdtick == 0 || gettimetick(2) - .quiz_cdtick >= .quiz_cd) {\n"
    "\t\t\tif (rand(1000) < 5) {\n"
    "\t\t\t\tif (.quiz_n > 1)\n"
    "\t\t\t\t\t.quiz_idx = rand(.quiz_n);\n"
    "\t\t\t\telse\n"
    "\t\t\t\t\t.quiz_idx = 0;\n"
    "\t\t\t\t.quiz_active = 1;\n"
    "\t\t\t\t.quiztick    = gettimetick(2);\n"
    "\t\t\t\texplode(.@av$, .quiz_a$[.quiz_idx], \"|\");\n"
    "\t\t\t\t.quiz_expected$ = .@av$[0];\n"
    "\t\t\t\t.@qmsg$ = strnpcinfo(0)+\" : \"+.quiz_q$[.quiz_idx];\n"
    "\t\t\t\tnpctalk .@qmsg$; logchat .@qmsg$;\n"
    "\t\t\t}\n"
    "\t\t}\n"
    "\t}\n"
    "\n"
)
if OLD2 in src:
    src = src.replace(OLD2, "", 1); changes += 1
    print("OK 2: bloc quiz OnTimer3000 supprimé")
else:
    print("ERREUR 2: bloc quiz OnTimer3000 non trouvé", file=sys.stderr); sys.exit(1)

# ── 3. .quiz_active = 0 dans le départ PvP ───────────────────────────────────
OLD3 = (
    "\t\t.trip = 1;                 // bloque chat/trip pendant l'event\n"
    "\t\t.quiz_active = 0;          // annule le quiz si Sting part\n"
    "\t\tcallsub S_SayEvent, \"[EVENT_PVP_TAUNT]\""
)
NEW3 = (
    "\t\t.trip = 1;                 // bloque chat/trip pendant l'event\n"
    "\t\tcallsub S_SayEvent, \"[EVENT_PVP_TAUNT]\""
)
if OLD3 in src:
    src = src.replace(OLD3, NEW3, 1); changes += 1
    print("OK 3: .quiz_active PvP supprimé")
else:
    print("ERREUR 3: ancre départ PvP non trouvée", file=sys.stderr); sys.exit(1)

# ── 4. .quiz_active = 0 dans le départ MVP ───────────────────────────────────
OLD4 = (
    "\t\t\t.trip = 1;\n"
    "\t\t\t.quiz_active = 0;\t\t// annule le quiz si Sting part\n"
    "\t\t\tannounce strnpcinfo(0)"
)
NEW4 = (
    "\t\t\t.trip = 1;\n"
    "\t\t\tannounce strnpcinfo(0)"
)
if OLD4 in src:
    src = src.replace(OLD4, NEW4, 1); changes += 1
    print("OK 4: .quiz_active MVP supprimé")
else:
    print("ERREUR 4: ancre départ MVP non trouvée", file=sys.stderr); sys.exit(1)

# ── 5. Variables et Q&A quiz dans OnInit ─────────────────────────────────────
OLD5 = (
    "\t// --- Variables quiz ---\n"
    "\t.quiz_active  = 0;\t\t// 0=aucun, 1=question pos\xe9e\n"
    "\t.quiz_idx     = 0;\t\t// index de la question en cours\n"
    "\t.quiz_cdtick  = 0;\t\t// timestamp de fin du dernier quiz\n"
    "\t.quiz_cd      = 1800;\t// 30min entre deux quiz\n"
    "\t.quiztick     = 0;\t\t// timestamp du d\xe9but du quiz (timeout 60s)\n"
    "\t.quiz_expected$ = \"\";\t// r\xe9ponse canonique (affich\xe9e au timeout)\n"
    "\t.quiz_n       = 10;\t\t// nombre de questions\n"
    "\t// Questions et r\xe9ponses (variants s\xe9par\xe9s par |, comparison exacte)\n"
    "\t.quiz_q$[0]  = \"QUIZ : Quel est le max level sur Moonlight-Destiny ?\";\n"
    "\t.quiz_a$[0]  = \"999\";\n"
    "\t.quiz_q$[1]  = \"QUIZ : Quel element contrecarre les monstres Fire/Feu ?\";\n"
    "\t.quiz_a$[1]  = \"Water|water|Eau|eau\";\n"
    "\t.quiz_q$[2]  = \"QUIZ : Quel skill du High Priest ressuscite les joueurs morts ?\";\n"
    "\t.quiz_a$[2]  = \"Resurrection|resurrection|Resurection|resurection\";\n"
    "\t.quiz_q$[3]  = \"QUIZ : Quelle classe peut utiliser le skill Cart Revolution ?\";\n"
    "\t.quiz_a$[3]  = \"Merchant|merchant|Blacksmith|blacksmith|Whitesmith|whitesmith|Mastersmith|mastersmith\";\n"
    "\t.quiz_q$[4]  = \"QUIZ : Quel mob drope la fameuse Poring Card ?\";\n"
    "\t.quiz_a$[4]  = \"Poring|poring\";\n"
    "\t.quiz_q$[5]  = \"QUIZ : Quel skill Priest protege avec une bulle bleue contre un one-shot fatal ?\";\n"
    "\t.quiz_a$[5]  = \"Kyrie Eleison|Kyrie|kyrie|kyrie eleison|Kyrie eleison\";\n"
    "\t.quiz_q$[6]  = \"QUIZ : Quel est le taux d'EXP sur ce serveur ? (format x1000)\";\n"
    "\t.quiz_a$[6]  = \"x1000|1000x|1000\";\n"
    "\t.quiz_q$[7]  = \"QUIZ : Quel element contrecarre les monstres Water/Eau ?\";\n"
    "\t.quiz_a$[7]  = \"Thunder|thunder|Foudre|foudre|Wind|wind|Vent|vent\";\n"
    "\t.quiz_q$[8]  = \"QUIZ : Quelle stat augmente la vitesse d'attaque ? (abreviation 3 lettres)\";\n"
    "\t.quiz_a$[8]  = \"AGI|agi|Agi\";\n"
    "\t.quiz_q$[9]  = \"QUIZ : Quel element contrecarre les monstres Earth/Terre ?\";\n"
    "\t.quiz_a$[9]  = \"Wind|wind|Vent|vent|Thunder|thunder|Foudre|foudre\";\n"
    "\t.mvpev_cd = 10800;"
)
NEW5 = "\t.mvpev_cd = 10800;"
if OLD5 in src:
    src = src.replace(OLD5, NEW5, 1); changes += 1
    print("OK 5: variables quiz OnInit supprimées")
else:
    print("ERREUR 5: variables quiz OnInit non trouvées", file=sys.stderr); sys.exit(1)

# ── 6. defpattern L_quiz ─────────────────────────────────────────────────────
OLD6 = "\tdefpattern 2, \"([^:]+) *: *(.*)\", \"L_quiz\";\t\t// catch-all quiz (prioritaire)\n"
if OLD6 in src:
    src = src.replace(OLD6, "", 1); changes += 1
    print("OK 6: defpattern L_quiz supprimé")
else:
    print("ERREUR 6: defpattern L_quiz non trouvé", file=sys.stderr); sys.exit(1)

print(f"\n{changes}/6 suppressions appliquées.")

with open(PATH, "w", encoding="latin-1") as f:
    f.write(src)
print("Fichier écrit en latin-1 :", PATH)
