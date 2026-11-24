"""Dorance (mid_camp) redonne le Ring_Of_Wise_King (2782) perdu.

Une fois ep13_2_rhea a 100, aucune source du jeu ne redonnait l'anneau :
un joueur qui l'avait vendu a un PNJ restait bloque hors des dialogues
Sapha. Le remplacement coute 1 000 z (revente PNJ 250 z, surcharge
comprise ~310 z : pas de boucle de zeny).
"""
import os
import sys

PATH = os.path.join(os.path.dirname(__file__), '..', '..',
                    'moon', 'rathena', 'quests', 'quests_13_2.txt')

OLD = (
    '\t\telse {\n'
    '\t\t\tmes "[Mechanic Engineer Dorance]";\n'
    '\t\t\tmes "Thank you for your help.";\n'
    '\t\t\tnext;\n'
    '\t\t\tmes "[Mechanic Engineer Dorance]";\n'
    '\t\t\tmes "I hope this ring would be a big help for your journey through the other world.";\n'
    '\t\t\tclose;\n'
    '\t\t}\n'
)

NEW = (
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

with open(PATH, 'rb') as f:
    raw = f.read()
crlf = b'\r\n' in raw
src = raw.decode('latin-1').replace('\r\n', '\n')

if src.count(OLD) != 1:
    sys.exit('ancre trouvee %d fois, attendu 1' % src.count(OLD))
src = src.replace(OLD, NEW)
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
