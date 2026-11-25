"""Dorance (mid_camp) redonne le Ring_Of_Wise_King (2782) perdu.

Une fois ep13_2_rhea a 100, aucune source du jeu ne redonnait l'anneau :
un joueur qui l'avait vendu a un PNJ restait bloque hors des dialogues
Sapha. Le remplacement coute 1 000 z (revente PNJ 250 z, surcharge
comprise ~310 z : pas de boucle de zeny).

Etape 2 : avant de vendre, Dorance cherche l'anneau ailleurs que dans
l'inventaire (chariot, stockage, stockages annexes, stockage de guilde),
sinon un joueur qui l'a range en rachete un pour rien.

Usage : python tools/util/patch_dorance_ring.py   (idempotent par etape)
"""
import os
import sys

PATH = os.path.join(os.path.dirname(__file__), '..', '..',
                    'moon', 'rathena', 'quests', 'quests_13_2.npc')

# --- Etape 1 : la branche de remplacement -------------------------------
OLD1 = (
    '\t\telse {\n'
    '\t\t\tmes "[Mechanic Engineer Dorance]";\n'
    '\t\t\tmes "Thank you for your help.";\n'
    '\t\t\tnext;\n'
    '\t\t\tmes "[Mechanic Engineer Dorance]";\n'
    '\t\t\tmes "I hope this ring would be a big help for your journey through the other world.";\n'
    '\t\t\tclose;\n'
    '\t\t}\n'
)

NEW1 = (
    '\t\telse {\n'
    '\t\t\t// Moonlight : l\'anneau perdu (vendu, jete) n\'avait aucune autre\n'
    '\t\t\t// source une fois la quete finie. 1 000 z : au-dessus de la\n'
    '\t\t\t// revente PNJ (250 z), pas de boucle de zeny.\n'
    '\t\t\tif (countitem(2782) < 1) {\n'
    '\t\t\t\tmes "[Mechanic Engineer Dorance]";\n'
    '\t\t\t\tmes "Vous avez perdu l\'anneau que je vous avais donné ?";\n'
    '\t\t\t\tmes "Il m\'en reste un exemplaire. Je vous le cède pour ^0055FF1 000 zenys^000000, le prix des pièces.";\n'
    '\t\t\t\tnext;\n'
    '\t\t\t\tif (select("Payer 1 000 zenys:Plus tard") == 2) {\n'
    '\t\t\t\t\tmes "[Mechanic Engineer Dorance]";\n'
    '\t\t\t\t\tmes "Revenez quand vous voulez.";\n'
    '\t\t\t\t\tclose;\n'
    '\t\t\t\t}\n'
    '\t\t\t\tif (Zeny < 1000) {\n'
    '\t\t\t\t\tmes "[Mechanic Engineer Dorance]";\n'
    '\t\t\t\t\tmes "Il vous manque des zenys : il en faut 1 000.";\n'
    '\t\t\t\t\tclose;\n'
    '\t\t\t\t}\n'
    '\t\t\t\tZeny -= 1000;\n'
    '\t\t\t\tgetitem 2782,1; //Ring_Of_Wise_King\n'
    '\t\t\t\tmes "[Mechanic Engineer Dorance]";\n'
    '\t\t\t\tmes "Le voici. Cette fois, ne le vendez pas : sans lui, les Saphas ne vous comprendront pas.";\n'
    '\t\t\t\tclose;\n'
    '\t\t\t}\n'
    '\t\t\tmes "[Mechanic Engineer Dorance]";\n'
    '\t\t\tmes "Thank you for your help.";\n'
    '\t\t\tnext;\n'
    '\t\t\tmes "[Mechanic Engineer Dorance]";\n'
    '\t\t\tmes "I hope this ring would be a big help for your journey through the other world.";\n'
    '\t\t\tclose;\n'
    '\t\t}\n'
)

# --- Etape 2 : chercher l'anneau range ailleurs --------------------------
OLD2 = (
    '\t\t\tif (countitem(2782) < 1) {\n'
    '\t\t\t\tmes "[Mechanic Engineer Dorance]";\n'
    '\t\t\t\tmes "Vous avez perdu l\'anneau que je vous avais donné ?";\n'
)

NEW2 = (
    '\t\t\tif (countitem(2782) < 1) {\n'
    '\t\t\t\t// Range ailleurs : chariot, stockage (en memoire depuis la\n'
    '\t\t\t\t// connexion), stockages annexes 1 a 5 (lus en base : ils ne\n'
    '\t\t\t\t// sont charges qu\'a l\'ouverture et sauves a la fermeture),\n'
    '\t\t\t\t// stockage de guilde. -1 = comptage impossible, compte pour 0.\n'
    '\t\t\t\t.@where$ = "";\n'
    '\t\t\t\tif (cartcountitem(2782) > 0)\n'
    '\t\t\t\t\t.@where$ = "dans votre chariot";\n'
    '\t\t\t\telse if (storagecountitem(2782) > 0)\n'
    '\t\t\t\t\t.@where$ = "dans votre stockage";\n'
    '\t\t\t\telse {\n'
    '\t\t\t\t\tsetarray .@tbl$[1], "storagealt1", "storagealt2", "storagealt3", "storagealt4", "storagecard";\n'
    '\t\t\t\t\tfor (.@i = 1; .@i <= 5; .@i++) {\n'
    '\t\t\t\t\t\t.@n = 0;\n'
    '\t\t\t\t\t\tquery_sql "SELECT COUNT(*) FROM `" + .@tbl$[.@i] + "` WHERE `account_id` = " + getcharid(3) + " AND `nameid` = 2782", .@n;\n'
    '\t\t\t\t\t\tif (.@n > 0) {\n'
    '\t\t\t\t\t\t\t.@where$ = "dans votre stockage annexe " + .@i;\n'
    '\t\t\t\t\t\t\tbreak;\n'
    '\t\t\t\t\t\t}\n'
    '\t\t\t\t\t}\n'
    '\t\t\t\t\t// Pas de && : rAthena evalue toujours les deux operandes.\n'
    '\t\t\t\t\tif (.@where$ == "" && getcharid(2)) {\n'
    '\t\t\t\t\t\tif (guildstoragecountitem(2782) > 0)\n'
    '\t\t\t\t\t\t\t.@where$ = "dans le stockage de votre guilde";\n'
    '\t\t\t\t\t}\n'
    '\t\t\t\t}\n'
    '\t\t\t\tif (.@where$ != "") {\n'
    '\t\t\t\t\tmes "[Mechanic Engineer Dorance]";\n'
    '\t\t\t\t\tmes "Vous cherchez l\'anneau ? Vous en avez déjà un ^0055FF" + .@where$ + "^000000.";\n'
    '\t\t\t\t\tmes "Reprenez-le et portez-le : les Saphas ne parlent qu\'à qui le porte.";\n'
    '\t\t\t\t\tclose;\n'
    '\t\t\t\t}\n'
    '\t\t\t\tmes "[Mechanic Engineer Dorance]";\n'
    '\t\t\t\tmes "Vous avez perdu l\'anneau que je vous avais donné ?";\n'
)

with open(PATH, 'rb') as f:
    raw = f.read()
crlf = b'\r\n' in raw
src = raw.decode('latin-1').replace('\r\n', '\n')

# Marqueur de chaque etape : l'etape 2 modifie le texte de l'etape 1.
MARK = {'etape 1': "Moonlight : l'anneau perdu", 'etape 2': 'Range ailleurs : chariot'}

for name, old, new in (('etape 1', OLD1, NEW1), ('etape 2', OLD2, NEW2)):
    if MARK[name] in src:
        print('%s deja appliquee' % name)
        continue
    if src.count(old) != 1:
        sys.exit('%s : ancre trouvee %d fois, attendu 1' % (name, src.count(old)))
    src = src.replace(old, new)
    print('%s appliquee' % name)

if crlf:
    src = src.replace('\n', '\r\n')

encoded = src.encode('latin-1')
if b'\xef\xbf\xbd' in encoded:
    sys.exit('U+FFFD present, abandon')
tmp = PATH + '.tmp'
with open(tmp, 'wb') as f:
    f.write(encoded)
os.replace(tmp, PATH)
print('ok, crlf=%s' % crlf)
