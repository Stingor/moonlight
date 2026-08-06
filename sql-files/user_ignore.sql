--
-- @ignore / @unignore / @ignorelist [Stingor]
--
-- Liste des personnes dont TOUT le chat est masque : zone, chatroom,
-- chuchotements, party, guilde, clan, BG, channels et texte au-dessus de la
-- tete. A ne pas confondre avec la liste d'ignore native (/ex, /in) qui ne
-- bloque que les chuchotements et qui, elle, n'est pas persistee.
--
-- La relation est etablie entre COMPTES MOONLIGHT (login.user_id) et non entre
-- personnages ou comptes de jeu : ni un renommage, ni un changement de perso,
-- ni un second compte de jeu ne permet de contourner un ignore. Symetriquement,
-- la liste suit son proprietaire sur tous ses personnages.
--
-- Aucun nom n'est stocke ici, volontairement : ce serait figer un libelle que la
-- personne visee peut renommer ou supprimer, alors que l'entree porte sur son
-- compte entier. Le nom affiche par @ignorelist est recalcule a chaque connexion
-- (pc_ignorechat_load), en prenant le personnage de plus haut niveau du compte.
--
-- Ecrite au fil de l'eau par le map-server (une requete par ajout/retrait).
--
-- `user_id`         : le compte qui ignore
-- `ignored_user_id` : le compte ignore
--

CREATE TABLE IF NOT EXISTS `user_ignore` (
  `user_id`         int(11) unsigned NOT NULL DEFAULT 0,
  `ignored_user_id` int(11) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`user_id`,`ignored_user_id`),
  KEY `ignored_user_id` (`ignored_user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
