// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef AUTH_HPP
#define AUTH_HPP

#include <common/cbasetypes.hpp>

#include "http.hpp"

bool isAuthorized(const Request &request, bool checkGuildLeader=false);

// [Stingor] Lit le champ AID en entier strictement positif, sans exception :
// un champ absent, vide, signe, suivi de caracteres ou hors de int32 est refuse.
bool parseAccountId(const Request &request, int32 &account_id);

// [Stingor] Vrai si le compte a une session de jeu active (jeton web active par
// le login-server, qui le desactive peu apres la deconnexion) ET si la requete
// vient de l'adresse IP de cette session (login.last_ip). Les adresses sont
// comparees sous forme binaire : une IPv4 et sa forme mappee ::ffff:a.b.c.d sont
// la meme adresse.
bool isFromActiveSession(const Request &request, int32 account_id);

#endif
