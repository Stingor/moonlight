"""Messages explicites pour moon/episode_clear.npc (Episode Clear Tickets).

Remplace le menu, l'achat, le choix de partie, la synchronisation et
S_Need, et ajoute S_Why (ce qui arrete un personnage) et S_Disp (dispbottom
decoupe). Fichier latin-1, ecriture atomique.
"""
import os
import sys

PATH = os.path.join(os.path.dirname(__file__), '..', '..', 'moon', 'episode_clear.npc')
B = '\x95'  # puce des menus (0x95 en Windows-1252)

MAIN = '''	.@npc$ = getarg(0, "[ Warp Agent ]");
	// Un personnage monte de niveau sans se reconnecter : on rattrape ici.
	callsub S_Sync;
L_Menu:
	clear;
	mes .@npc$;
	mes "Terminez une partie d'épisode une fois, sur n'importe quel personnage : un ^0055FFEpisode Clear Ticket^000000 la débloque ensuite pour ^0055FFtout votre compte Moonlight^000000.";
	mes "Vos autres personnages la reçoivent comme terminée, sans récompense ni EXP, dès qu'ils remplissent ses conditions.";
	switch (select("@ Utiliser un ticket:@ Acheter un ticket:@ Quels épisodes ?:@ Comment ça marche ?:@ ^e64c00Menu Précédent^000000")) {
	case 1:
		.@part = callsub(S_PickPart);
		if (!.@part)
			goto L_Menu;
		.@ep = .@part / 10;
		.@name$ = callsub(S_Name, .@part);
		clear;
		mes .@npc$;
		if (getd("##epc_tkt_" + .@part)) {
			mes "L'épisode " + .@name$ + " est ^0055FFdéjà débloqué pour tout votre compte Moonlight^000000 : un ticket a déjà été utilisé pour cette partie. Gardez votre ticket, il ne débloquerait rien de plus.";
			if (callsub(S_Done, .@part))
				mes "Ce personnage l'a déjà : il est terminé pour lui.";
			else
				mes "^e64c00Ce personnage ne l'a pas encore reçu.^000000 " + callsub(S_Why, .@part);
			next;
			goto L_Menu;
		}
		if (!getd("##epc_done_" + .@part)) {
			mes "^e64c00Impossible pour l'instant^000000 : un ticket ne sert qu'une fois l'épisode " + .@name$ + " terminé ^0055FFune fois^000000, sur n'importe quel personnage de votre compte Moonlight. Ce n'est pas encore le cas.";
			mes "Si un de vos personnages l'a déjà fini, connectez-le une fois (ou ouvrez ce menu avec lui) pour que je le note au compte, puis revenez utiliser le ticket.";
			next;
			goto L_Menu;
		}
		if (countitem(.ticket[.@ep]) < 1) {
			mes "Ce personnage n'a pas d'^0055FF" + getitemname(.ticket[.@ep]) + "^000000 dans son inventaire.";
			mes "Il en faut un par partie. Achetez-le avec « Acheter un ticket » (" + callfunc("F_InsertComma", .price[.@ep]) + " zenys), ou passez-le à ce personnage par le stockage.";
			next;
			goto L_Menu;
		}
		mes "Utiliser un ^0055FF" + getitemname(.ticket[.@ep]) + "^000000 pour débloquer l'épisode " + .@name$ + " sur tout votre compte Moonlight ?";
		mes "Le ticket est consommé. Chaque personnage qui n'a pas fini cet épisode le recevra comme terminé, à sa connexion ou en ouvrant ce menu, dès qu'il remplit les conditions (niveau, épisode précédent). Ses quêtes encore ouvertes de cette chaîne seront fermées.";
		next;
		if (select("Oui, l'utiliser:Annuler") == 2)
			goto L_Menu;
		if (countitem(.ticket[.@ep]) < 1 || getd("##epc_tkt_" + .@part))
			close;
		.@had = callsub(S_Done, .@part);
		delitem .ticket[.@ep], 1;
		setd "##epc_tkt_" + .@part, 1;
		logmes "EpisodeClear: ticket utilise partie " + callsub(S_Name, .@part) + " | " + callsub(S_LogState, .@ep);
		callsub S_Sync;
		specialeffect2 154;
		clear;
		mes .@npc$;
		mes "C'est fait. L'épisode " + .@name$ + " est débloqué pour tout votre compte Moonlight.";
		if (.@had)
			mes "Ce personnage l'avait déjà terminé.";
		else if (callsub(S_Done, .@part))
			mes "Ce personnage vient de le recevoir.";
		else
			mes "^e64c00Ce personnage ne le reçoit pas encore.^000000 " + callsub(S_Why, .@part) + " Il le recevra dès que ce sera réglé, à sa connexion ou en rouvrant ce menu.";
		mes "Vos autres personnages le recevront à leur connexion, chacun dès qu'il remplit les mêmes conditions. « Quels épisodes ? » dit, pour chacun, ce qui l'arrête.";
		close;
	case 2:
		goto L_Buy;
	case 3:
		clear;
		mes .@npc$;
		mes "État des épisodes pour ^0055FF" + strcharinfo(0) + "^000000 et votre compte Moonlight :";
		.@n = getarraysize(.ready);
		for (.@i = 0; .@i < .@n; .@i++) {
			.@p = .ready[.@i];
			if (callsub(S_Done, .@p)) {
				if (getd("##epc_tkt_" + .@p))
					.@s$ = "^20a020terminé sur ce personnage^000000, débloqué au compte.";
				else
					.@s$ = "^20a020terminé sur ce personnage^000000. Un ticket peut le débloquer pour vos autres personnages.";
			} else if (getd("##epc_tkt_" + .@p))
				.@s$ = "^0055FFdébloqué au compte^000000, ^e64c00pas encore reçu par ce personnage^000000. " + callsub(S_Why, .@p);
			else if (getd("##epc_done_" + .@p))
				.@s$ = "à faire. Fini une fois sur le compte : un ticket peut le débloquer pour tous vos personnages.";
			else
				.@s$ = "à faire. Jamais fini sur le compte : un ticket ne peut pas encore servir.";
			mes "Épisode " + callsub(S_Name, .@p) + " : " + .@s$;
			if (.@i == 7) {
				next;
				mes .@npc$;
			}
		}
		mes "Un ticket débloque une seule partie, pour tout le compte, une fois qu'elle a été finie sur un personnage. Les tickets des épisodes 13 à 17 servent pour chacune de leurs parties : un ticket par partie.";
		next;
		goto L_Menu;
	case 4:
		clear;
		mes .@npc$;
		mes "^0055FF1.^000000 Finissez une partie d'épisode une fois, sur n'importe quel personnage de votre compte Moonlight. Il doit ensuite se connecter (ou ouvrir ce menu) pour que je le note au compte.";
		mes "^0055FF2.^000000 Utilisez alors ^0055FFun seul ticket^000000 pour cette partie : il vaut pour tout le compte Moonlight, tous vos comptes de jeu rattachés compris. Inutile d'en racheter un par personnage.";
		mes "^0055FF3.^000000 Un ticket passe une seule partie : 13.1, 13.2 et 13.3 demandent trois tickets 13, 14.1 à 14.3 trois tickets 14, 15.1 et 15.2 deux tickets 15, 16.1 et 16.2 deux tickets 16, 17.1 et 17.2 deux tickets 17.";
		next;
		mes .@npc$;
		mes "^0055FF4.^000000 Chaque autre personnage reçoit la partie à sa connexion ou en ouvrant ce menu, seulement s'il remplit ses conditions : le niveau de base demandé, et la partie précédente terminée (jouée ou reçue par ticket).";
		mes "^0055FF5.^000000 Les parties s'enchaînent : si l'une attend, toutes celles qui la suivent attendent aussi. « Quels épisodes ? » dit, pour le personnage connecté, ce qui l'arrête.";
		mes "^0055FF6.^000000 Pas d'EXP ni de récompense de quête ; les quêtes encore ouvertes de la chaîne sont fermées.";
		next;
		mes .@npc$;
		mes "Les chaînes et leurs conditions :";
		mes "13.1 (niveau 70), puis 13.2, puis 13.3.";
		mes "14.1 (niveau 100).";
		mes "14.2 (niveau 99), puis 14.3 (niveau 140).";
		mes "15.1 (niveau 140, Eden Group Mark dans l'inventaire), puis 15.2.";
		mes "16.1 (niveau 100, route du Nouveau Monde ouverte).";
		mes "16.2 (Room of Consciousness atteinte dans 16.1), puis 17.1, 17.2, 18 (niveau 170), 19 (niveau 200) et 20 (niveau 215).";
		next;
		goto L_Menu;
	}
	return;

// Achat : on revient a la liste apres chaque ticket, sans fermer le Warp Agent.
L_Buy:
	.@n = getarraysize(.sell);
	.@menu$ = "";
	for (.@i = 0; .@i < .@n; .@i++) {
		.@e = .sell[.@i];
		.@open = 0;
		.@usable = 0;
		.@m = getarraysize(.ready);
		for (.@j = 0; .@j < .@m; .@j++) {
			if (.ready[.@j] / 10 != .@e || getd("##epc_tkt_" + .ready[.@j]))
				continue;
			.@open++;
			if (getd("##epc_done_" + .ready[.@j]))
				.@usable++;
		}
		if (!.@open)
			.@st$ = "déjà tout débloqué";
		else if (.@usable)
			.@st$ = "utilisable";
		else
			.@st$ = "pas encore utilisable";
		.@menu$ += "Ticket épisode " + .@e + " (" + callfunc("F_InsertComma", .price[.@e]) + " z, " + .@st$ + "):";
	}
	clear;
	mes .@npc$;
	mes "Quel ticket voulez-vous acheter ?";
	mes "« utilisable » : une partie au moins a été finie une fois sur le compte et n'est pas encore débloquée.";
	.@c = select(.@menu$ + "^e64c00Menu Précédent^000000") - 1;
	if (.@c >= .@n)
		goto L_Menu;
	.@ep = .sell[.@c];
	clear;
	mes .@npc$;
	// Parties que couvre ce ticket.
	deletearray .@parts;
	.@m = getarraysize(.ready);
	for (.@j = 0; .@j < .@m; .@j++) {
		if (.ready[.@j] / 10 == .@ep)
			.@parts[getarraysize(.@parts)] = .ready[.@j];
	}
	.@np = getarraysize(.@parts);
	.@open = 0;
	.@usable = 0;
	for (.@j = 0; .@j < .@np; .@j++) {
		if (getd("##epc_tkt_" + .@parts[.@j]))
			continue;
		.@open++;
		if (getd("##epc_done_" + .@parts[.@j]))
			.@usable++;
	}
	// Refuse seulement quand toutes les parties de l'episode sont deja
	// debloquees pour le compte : un ticket de plus ne servirait a rien.
	if (!.@open) {
		mes "^e64c00Achat refusé^000000 : chaque partie que couvre un ticket de l'épisode " + .@ep + " a ^0055FFdéjà été débloquée par ticket pour votre compte Moonlight^000000. Un ticket vaut pour tout le compte, un autre ne débloquerait rien de plus.";
		for (.@j = 0; .@j < .@np; .@j++) {
			if (callsub(S_Done, .@parts[.@j]))
				mes "Épisode " + callsub(S_Name, .@parts[.@j]) + " : ^20a020déjà terminé sur ce personnage^000000.";
			else
				mes "Épisode " + callsub(S_Name, .@parts[.@j]) + " : ^e64c00pas encore reçu par ce personnage^000000. " + callsub(S_Why, .@parts[.@j]);
		}
		mes "Ce personnage recevra ces parties tout seul, à sa connexion ou en rouvrant ce menu, dès que ce qui l'arrête est réglé.";
		next;
		goto L_Buy;
	}
	mes "Un ^0055FF" + getitemname(.ticket[.@ep]) + "^000000 coûte " + callfunc("F_InsertComma", .price[.@ep]) + " zenys. Il débloque ^0055FFune seule partie^000000, pour tout votre compte Moonlight :";
	for (.@j = 0; .@j < .@np; .@j++) {
		.@p = .@parts[.@j];
		if (getd("##epc_tkt_" + .@p))
			mes "Épisode " + callsub(S_Name, .@p) + " : déjà débloqué au compte, pas besoin de ticket.";
		else if (getd("##epc_done_" + .@p))
			mes "Épisode " + callsub(S_Name, .@p) + " : ^20a020ticket utilisable^000000, il a été fini une fois sur le compte.";
		else
			mes "Épisode " + callsub(S_Name, .@p) + " : ^e64c00ticket pas encore utilisable^000000, à finir d'abord une fois sur un personnage du compte.";
	}
	if (!.@usable)
		mes "^e64c00Attention^000000 : ce ticket ne peut servir à rien pour l'instant. Il ne deviendra utile qu'une fois une de ces parties finie sur un de vos personnages.";
	else if (countitem(.ticket[.@ep]) >= .@usable)
		mes "Ce personnage a déjà " + countitem(.ticket[.@ep]) + " ticket(s) de cet épisode : assez pour les parties utilisables aujourd'hui.";
	next;
	if (.@usable)
		.@buy$ = "L'acheter";
	else
		.@buy$ = "L'acheter quand même";
	if (select(.@buy$ + ":Annuler") == 2)
		goto L_Buy;
	if (Zeny < .price[.@ep]) {
		mes .@npc$;
		mes "Vous n'avez pas assez de zenys : il en faut " + callfunc("F_InsertComma", .price[.@ep]) + ", vous en avez " + callfunc("F_InsertComma", Zeny) + ".";
		next;
		goto L_Buy;
	}
	if (!checkweight(.ticket[.@ep], 1)) {
		mes .@npc$;
		mes "Vous portez trop de choses : libérez du poids ou une place dans l'inventaire, puis revenez.";
		next;
		goto L_Buy;
	}
	Zeny -= .price[.@ep];
	getitem .ticket[.@ep], 1;
	logmes "EpisodeClear: achat ticket ep" + .@ep + " " + .price[.@ep] + "z | " + callsub(S_LogState, .@ep);
	mes .@npc$;
	mes "Voici votre ticket. Utilisez-le avec « Utiliser un ticket ».";
	next;
	goto L_Buy;

// Demande une partie de .ready. La renvoie, ou 0 pour revenir.
// Pas de ':' dans un libelle : select() coupe le menu a chaque ':'.
S_PickPart:
	.@n = getarraysize(.ready);
	.@menu$ = "";
	for (.@i = 0; .@i < .@n; .@i++) {
		.@p = .ready[.@i];
		if (getd("##epc_tkt_" + .@p))
			.@st$ = "déjà débloqué au compte";
		else if (getd("##epc_done_" + .@p))
			.@st$ = "ticket utilisable";
		else
			.@st$ = "jamais fini sur le compte";
		.@menu$ += "Épisode " + callsub(S_Name, .@p) + " (" + .@st$ + ", vous en avez " + countitem(.ticket[.@p / 10]) + "):";
	}
	.@c = select(.@menu$ + "^e64c00Menu Précédent^000000") - 1;
	if (.@c >= .@n)
		return 0;
	return .ready[.@c];

// Synchronise le personnage avec son compte Moonlight :
// 1. note au compte les parties que ce personnage a finies ;
// 2. applique, dans l'ordre de l'histoire, les parties debloquees par
//    ticket que ce personnage n'a pas finies, si leurs conditions sont
//    remplies (une partie appliquee peut ouvrir la suivante, d'ou .order).
// Un drapeau ## par partie : les ## sont lus a la connexion et le dernier
// ecrit gagne, un seul entier en masque de bits perdrait des parties.
S_Sync:
	.@n = getarraysize(.ready);
	for (.@i = 0; .@i < .@n; .@i++) {
		.@p = .ready[.@i];
		if (!getd("##epc_done_" + .@p) && callsub(S_Done, .@p))
			setd "##epc_done_" + .@p, 1;
	}
	.@wait = 0;
	.@n = getarraysize(.order);
	for (.@i = 0; .@i < .@n; .@i++) {
		.@p = .order[.@i];
		if (!.@p || !getd("##epc_tkt_" + .@p))
			continue;
		if (callsub(S_Done, .@p))
			continue;
		.@why$ = callsub(S_Why, .@p);
		if (.@why$ == "") {
			callsub S_Skip, .@p;
			dispbottom "Épisode " + callsub(S_Name, .@p) + " : reçu par ce personnage, débloqué par votre compte Moonlight (Episode Clear Ticket).";
			continue;
		}
		// Une partie qui attend une partie elle-meme debloquee par ticket se
		// tait : celle-ci parle deja et nomme celles qui l'attendent.
		.@prev = callsub(S_Prev, .@p);
		if (.@prev) {
			if (!callsub(S_Done, .@prev) && getd("##epc_tkt_" + .@prev))
				continue;
		}
		.@f$ = "";
		for (.@j = .@i + 1; .@j < .@n; .@j++) {
			.@q = .order[.@j];
			if (!.@q)
				break;
			if (!getd("##epc_tkt_" + .@q))
				break;
			if (.@f$ != "")
				.@f$ += ", ";
			.@f$ += callsub(S_Name, .@q);
		}
		callsub S_Disp, "Épisode " + callsub(S_Name, .@p) + " (débloqué au compte, Episode Clear Ticket) : pas encore reçu par ce personnage. " + .@why$;
		if (.@f$ != "")
			dispbottom "Débloqués au compte eux aussi, ils attendent celui-ci : épisodes " + .@f$ + ".";
		.@wait = 1;
	}
	if (.@wait)
		dispbottom "Détails au Warp Agent : Épisodes (Clear Tickets), Quels épisodes ?";
	return;
'''

NEED = '''S_Need:
	switch (getarg(0)) {
	case 131:
		// Le Promotional Staff d'Izlude (quests_13_1.txt).
		if (BaseLevel < 70)
			return "L'épisode 13.1 demande le niveau de base 70 : ce personnage est niveau " + BaseLevel + ".";
		break;
	case 132:
		if (countitem(2782) < 1 && !checkweight(2782, 1))
			return "L'épisode 13.2 donne l'anneau de l'ancien roi sage, et ce personnage porte trop de choses pour le recevoir : libérez du poids ou une place dans l'inventaire.";
		break;
	case 141:
		// Theore (quests_mora.txt).
		if (BaseLevel < 100)
			return "L'épisode 14.1 demande le niveau de base 100 : ce personnage est niveau " + BaseLevel + ".";
		break;
	case 142:
		// Les voyageurs d'Eclage (quests_eclage.txt).
		if (BaseLevel < 99)
			return "L'épisode 14.2 demande le niveau de base 99 : ce personnage est niveau " + BaseLevel + ".";
		break;
	case 143:
		// Les chiffons de Morroc (quests_14_3_bis.txt).
		if (BaseLevel < 140)
			return "L'épisode 14.3 demande le niveau de base 140 : ce personnage est niveau " + BaseLevel + ".";
		break;
	case 151:
		// La secretaire Lime Evenor (quests_15_1.txt).
		if (BaseLevel < 140)
			return "L'épisode 15.1 demande le niveau de base 140 : ce personnage est niveau " + BaseLevel + ".";
		if (countitem(6219) < 1 && countitem(22508) < 1)
			return "L'épisode 15.1 demande que ce personnage ait une Eden Group Mark dans son inventaire : l'équipe de fouilles ne prend que des membres d'Eden.";
		break;
	case 161:
		// Le messager royal (quests_16_1.txt).
		if (BaseLevel < 100)
			return "L'épisode 16.1 demande le niveau de base 100 : ce personnage est niveau " + BaseLevel + ".";
		if (ep13_ryu < 100 && ep13_start < 100)
			return "L'épisode 16.1 demande que ce personnage ait ouvert la route du Nouveau Monde : la quête Onward to the New World (épisode 13.1), ou celle du Cat Paw Agent.";
		break;
	case 162:
		// Le premier passage dans la Room of Consciousness (quests_16_2.txt
		// teste ep16_royal < 17).
		if (ep16_royal < 17) {
			.@s$ = "L'épisode 16.2 demande que ce personnage ait atteint la Room of Consciousness dans l'épisode 16.1";
			if (getd("##epc_tkt_161"))
				return .@s$ + ". L'épisode 16.1 est débloqué au compte mais ce personnage ne l'a pas encore reçu. " + callsub(S_Need, 161);
			if (getd("##epc_done_161"))
				return .@s$ + " : jouez 16.1 jusque-là, ou débloquez 16.1 avec un ticket épisode 16 (il a déjà été fini une fois sur votre compte).";
			return .@s$ + " : jouez 16.1 jusque-là. L'épisode 16.1 n'a jamais été fini sur votre compte, aucun ticket ne peut encore le passer.";
		}
		break;
	case 180:
		// Smart Elly (quests_17_2.npc).
		if (BaseLevel < 170)
			return "L'épisode 18 demande le niveau de base 170 : ce personnage est niveau " + BaseLevel + ".";
		break;
	case 190:
		// Le villageois loup gris de Wolfvill (quests_19.npc).
		if (BaseLevel < 200)
			return "L'épisode 19 demande le niveau de base 200 : ce personnage est niveau " + BaseLevel + ".";
		break;
	case 200:
		// Horuru a l'Ice Castle (quests_20_main.npc).
		if (BaseLevel < 215)
			return "L'épisode 20 demande le niveau de base 215 : ce personnage est niveau " + BaseLevel + ".";
		break;
	}
	return "";

// Pourquoi ce personnage n'a pas (encore) recu une partie : "" s'il l'a
// finie ou si rien ne l'arrete (S_Sync la lui donne), sinon le texte.
// Remonte les parties precedentes debloquees elles aussi jusqu'a celle
// qui bloque vraiment, pour nommer la vraie cause.
S_Why:
	.@p = getarg(0);
	if (callsub(S_Done, .@p))
		return "";
	.@prev = callsub(S_Prev, .@p);
	if (!.@prev)
		return callsub(S_Need, .@p);
	if (callsub(S_Done, .@prev))
		return callsub(S_Need, .@p);
	.@root = .@prev;
	while (getd("##epc_tkt_" + .@root)) {
		.@q = callsub(S_Prev, .@root);
		if (!.@q)
			break;
		if (callsub(S_Done, .@q))
			break;
		.@root = .@q;
	}
	.@r$ = callsub(S_Name, .@root);
	if (getd("##epc_tkt_" + .@root)) {
		if (.@root == .@prev)
			.@s$ = "Il doit d'abord recevoir l'épisode " + .@r$ + ", débloqué au compte lui aussi.";
		else
			.@s$ = "Il doit d'abord recevoir les épisodes " + .@r$ + " à " + callsub(S_Name, .@prev) + ", débloqués au compte eux aussi, en commençant par " + .@r$ + ".";
		.@need$ = callsub(S_Need, .@root);
		if (.@need$ == "")
			.@need$ = "Reconnectez-vous ou rouvrez ce menu pour le recevoir.";
		return .@s$ + " " + .@need$;
	}
	.@s$ = "Il doit d'abord finir l'épisode " + .@r$ + " sur ce personnage";
	if (.@root != .@prev)
		.@s$ += " (les épisodes débloqués qui le suivent viendront ensuite)";
	if (getd("##epc_done_" + .@root))
		return .@s$ + ", ou vous pouvez débloquer " + .@r$ + " avec un ticket épisode " + (.@root / 10) + " : il a déjà été fini une fois sur votre compte.";
	return .@s$ + ". L'épisode " + .@r$ + " n'a jamais été fini sur votre compte : aucun ticket ne peut encore le passer.";

// dispbottom coupe a CHAT_SIZE_MAX : un texte long part en plusieurs
// lignes, coupees sur une espace.
S_Disp:
	.@s$ = getarg(0);
	while (getstrlen(.@s$) > 200) {
		.@c = 200;
		while (.@c > 100 && charat(.@s$, .@c) != " ")
			.@c--;
		dispbottom substr(.@s$, 0, .@c - 1);
		.@s$ = substr(.@s$, .@c + 1, getstrlen(.@s$) - 1);
	}
	dispbottom .@s$;
	return;
'''

MAIN = MAIN.replace('"@ ', '"' + B + ' ').replace(':@ ', ':' + B + ' ')


def main():
    raw = open(PATH, 'rb').read()
    src = raw.decode('latin-1')
    lines = src.split('\n')
    # Bornes verifiees : 45 = .@npc$, 222 = fin de S_Sync, 285 = S_Need:, 347 = return "".
    assert lines[44].startswith('\t.@npc$ = getarg(0'), lines[44]
    assert lines[191] == 'S_Sync:' and lines[221] == '\treturn;', (lines[191], lines[221])
    assert lines[284] == 'S_Need:' and lines[346] == '\treturn "";', (lines[284], lines[346])
    new = lines[:44] + MAIN.rstrip('\n').split('\n') + lines[222:284] \
        + NEED.rstrip('\n').split('\n') + lines[347:]
    out = '\n'.join(new)
    if '\r\n' in src:
        print('CRLF detecte, abandon', file=sys.stderr)
        sys.exit(1)
    encoded = out.encode('latin-1')
    tmp = PATH + '.tmp'
    with open(tmp, 'wb') as f:
        f.write(encoded)
    os.replace(tmp, PATH)
    assert b'\xef\xbf\xbd' not in encoded
    print('ok', len(lines), '->', len(new))


if __name__ == '__main__':
    main()
