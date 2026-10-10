// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef AUTH_HPP
#define AUTH_HPP

#include <common/cbasetypes.hpp>

#include "http.hpp"

bool isAuthorized(const Request &request, bool checkGuildLeader=false);

// [Stingor] Lit un champ du formulaire en int32, sans exception : un champ
// absent, vide, suivi de caracteres ou hors de int32 est refuse.
bool parseInt32Field(const Request &request, const char *field, int32 &value);

// [Stingor] Lit un identifiant (strictement positif) : AID pour le compte, GID
// pour le personnage.
bool parseAccountId(const Request &request, int32 &account_id);
bool parseCharId(const Request &request, int32 &char_id);

// [Stingor] Vrai si le jeton est valide pour le compte ET si le personnage
// appartient a ce compte : un jeton valide ne donne acces qu'aux personnages
// de son propre compte.
bool isAuthorizedForCharacter(const Request &request, int32 account_id, int32 char_id);

// [Stingor] Vrai si le compte a une session de jeu active (jeton web active par
// le login-server, qui le desactive peu apres la deconnexion) ET si la requete
// vient de l'adresse IP de cette session (login.last_ip). Les adresses sont
// comparees sous forme binaire : une IPv4 et sa forme mappee ::ffff:a.b.c.d sont
// la meme adresse.
bool isFromActiveSession(const Request &request, int32 account_id);

#endif
