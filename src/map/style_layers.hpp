// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// [Stingor] Les TEINTURES DE CALQUES de moonclient (CZ 0x0F5E, ZC 0x0F5F et
// 0x0F60, voir packets_struct.hpp ; l'ADR 0146 de moonclient et
// docs/couleurs-du-personnage/conception.md).
//
// À côté de la recette v7 du corps, qui ne change pas, un joueur teint sa tête,
// sa peau, ses coiffes et sa cape. Le serveur ne lit jamais une couleur : il
// juge la structure, range, et diffuse ce que chaque joueur AFFICHE.
//
// Deux rangements (sql-files/moonclient_style_layers.sql) :
// - la tête, les deux peaux et la suite du corps appartiennent au PERSONNAGE,
//   une ligne par char_id ;
// - une coiffe ou une cape teinte appartient à l'EXEMPLAIRE de l'objet, une
//   ligne par `unique_id`, avec l'identifiant d'objet qu'elle teint.
//
// 🔴 PERSONNE NE TEINT L'OBJET D'UN AUTRE. Le CZ désigne un EMPLACEMENT
// d'équipement de l'émetteur, jamais un `unique_id` (il se devine) : l'objet se
// trouve par `sd->equip_index`, doit y être porté au moment du traitement, de
// l'identifiant attendu. Avec `moonclient_style_atelier`, le styliste doit
// en plus avoir ouvert un atelier à quelques cases. Une teinture d'objet ne s'applique à l'affichage qu'à
// l'objet qui porte à la fois son `unique_id` ET son identifiant.

#ifndef MAP_STYLE_LAYERS_HPP
#define MAP_STYLE_LAYERS_HPP

#include <string>
#include <vector>

#include <common/cbasetypes.hpp>

class map_session_data;
struct block_list;
struct npc_data;

// L'écart minimal entre deux CZ 0x0F5E d'une session, en millisecondes. Il ne se
// partage pas avec celui du corps (0x0F26) : « Valider » envoie les deux.
#define MOONCLIENT_STYLE_LAYERS_CADENCE_MS 1000
// Les octets de CZ 0x0F5E qu'une session envoie par minute, au plus. Toute trame
// reçue compte, refusée ou non : c'est le coût de la lire qu'elle borne.
// Seize trames pleines du profil mixte (4 Kio chacune) : de quoi régler à la main,
// pas de quoi faire lire au serveur un flux sans fin.
#define MOONCLIENT_STYLE_LAYERS_BYTES_PER_MINUTE 65536
// Les teintures qu'un personnage range, au plus : têtes, peau du visage, suite
// et peau de chaque corps. Une teinture de cheveux se range sous sa coupe et sa
// couleur, et dort tant qu'on ne les porte pas. Les suites et les peaux des
// corps en prennent huit au plus : il en reste au moins onze aux têtes.
#define MOONCLIENT_STYLE_LAYERS_MAX 20
// Le plus d'effets de chapeau qu'un CZ teint d'un coup.
#define MOONCLIENT_STYLE_HAT_EFFECTS_MAX 8
// La distance au styliste, en cases, pour qu'une teinture d'objet soit acceptée.
#define MOONCLIENT_STYLE_ATELIER_CELLS 5
// La durée d'un atelier que le script ouvre sans la dire, en secondes.
#define MOONCLIENT_STYLE_ATELIER_DEFAULT_S 600
// La plus longue qu'un script puisse demander, en secondes.
#define MOONCLIENT_STYLE_ATELIER_MAX_S 3600

// La session a-t-elle annoncé MOONLIGHT_UI_STYLE_LAYERS ?
bool style_layers_receiver(const map_session_data& sd);

// ── Diffusion ──────────────────────────────────────────────────────────────

// Le bloc de `sd`, à `sd` et aux joueurs en vue qui savent le lire, puis ceux de
// tous les joueurs en vue, à `sd` : à la poignée de main et au gain du bit.
void style_layers_handshake(map_session_data& sd);
// Les blocs des joueurs en vue, au SPECTATEUR qui vient d'annoncer le bit des
// calques. Le sien ne se compose jamais : un spectateur ne se montre pas.
void style_layers_spectator_join(map_session_data& sd);
// Le bloc de `owner`, aux joueurs en vue qui savent le lire, `owner` exclu : à
// son apparition (connexion, warp).
void style_layers_spawn(map_session_data& owner);
// Le bloc de `owner`, à `sd` seul : `owner` entre dans la vue de `sd`.
void style_layers_single(map_session_data& sd, map_session_data& owner);
// LES COMPAGNONS (calque 7) : la teinture d'un homoncule ou d'un familier sur la
// carte, sous le gid de l'unité, à qui le voit. Elle appartient au compagnon
// (`hom_id`, `pet_id`) et ne sort que sous la classe pour laquelle elle a été
// faite. Rien d'un homoncule au repos ou d'un familier dans son œuf.
//
// Le compagnon `bl` apparaît : sa teinture, aux joueurs en vue qui savent la lire.
void style_layers_companion_spawn(const block_list& bl);
// Le compagnon `bl` entre dans la vue de `sd`.
void style_layers_companion_single(map_session_data& sd, block_list& bl);
// Les genres de compagnons, tels que `moonclient_companion_dyes` les range.
#define MOONCLIENT_COMPANION_KIND_HOMUNCULUS 1
#define MOONCLIENT_COMPANION_KIND_PET 2
// Le compagnon est supprimé : sa teinture part avec lui.
void style_layers_forget_companion(uint8 kind, int32 companion_id);

// Ce que `owner` affiche a peut-être changé (équipement, coupe, couleur) : son
// bloc repart à la zone, lui compris, s'il diffère du dernier diffusé. Appelé
// APRÈS le paquet natif d'apparence.
void style_layers_look_changed(map_session_data& owner);

// ── Verdict et atelier ─────────────────────────────────────────────────────

// Le verdict (ZC 0x0F60), à `sd` seul, s'il a le bit.
void style_layers_verdict(map_session_data& sd, uint8 subject, uint8 result, uint16 wait_ms, uint32 detail);
// Le styliste `nd` ouvre l'atelier de `sd` pour `duration_s` secondes, lié à sa
// carte et à sa position ; zéro le ferme. Le client l'apprend sous le bit.
void style_layers_atelier(map_session_data& sd, const npc_data& nd, uint32 duration_s);

// ── Cycle de vie ───────────────────────────────────────────────────────────

// Jette l'état de la session qui part : teintures en mémoire, atelier, cadence.
void style_layers_on_logout(const map_session_data& sd);
// Oublie le cache des teintures d'objets, à l'arrêt.
void style_layers_final();

// ── Staff ──────────────────────────────────────────────────────────────────

// Ce que le joueur `sd` a rangé et affiche, une ligne par teinture.
std::vector<std::string> style_layers_describe(map_session_data& sd);
// Retire les teintures de tête et de peau de `sd`. Faux si la base refuse.
bool style_layers_staff_clear_character(map_session_data& sd, const map_session_data& staff);
// La teinture de l'exemplaire `unique_id`, en une ligne ; vide s'il n'en a pas.
std::string style_layers_describe_item(uint64 unique_id);
// Retire la teinture de l'exemplaire `unique_id`. Faux s'il n'en avait pas, ou
// si la base refuse.
bool style_layers_staff_clear_item(uint64 unique_id, const map_session_data& staff);

void clif_parse_moonclient_style_layers(int32 fd, map_session_data* sd);

// Une entrée de calque à la forme du ZC 0x0F5F (calques 1 à 5, clé affichée,
// aucun drapeau), telle qu'une tenue la range : sa taille, ou zéro si elle est
// mal formée.
size_t style_layers_check_shown_entry(const uint8* p, size_t available);

// Ce que `owner` affiche, tel qu'un bloc du ZC 0x0F5F le porte sans son gid :
// [nombre:1] puis ses entrées. Pour les styles hors de vue.
std::vector<uint8> style_layers_shown_entries(map_session_data& owner);
// Les teintures du personnage de `owner` se lisent-elles en base ? Faux si la
// lecture a échoué : ce qu'il affiche serait alors appauvri.
bool style_layers_readable(map_session_data& owner);

#endif /* MAP_STYLE_LAYERS_HPP */
