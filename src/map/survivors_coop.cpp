// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// [Stingor] LA COOP DU MODE SURVIVORS de moonclient (0x0F58 à 0x0F5D, voir
// packets_struct.hpp ; le cahier du lot 14 de moonclient).
//
// Le groupe rAthena joue ensemble : chaque client simule toute la partie, et le
// serveur ne calcule rien. Il tient le salon, tire la graine, puis relaie les
// entrées dans l'ordre où il les reçoit : c'est cet ordre qui fait foi.
//
// Un groupe a au plus un état, le salon ou la partie, EN MÉMOIRE, par party_id.
//   - Le salon s'ouvre quand un membre y entre et s'efface quand le dernier en
//     sort. Il part quand TOUS les membres du groupe y sont prêts et qu'aucune
//     raison de refus n'est levée ; il se réévalue à chaque changement du groupe,
//     que le membre concerné soit dans le salon ou non.
//   - La partie garde ses joueurs, fixés au départ. Un joueur qui en sort ne
//     peut plus y revenir ; un membre arrivé ensuite n'y entre pas. Elle s'efface
//     quand il n'y reste plus personne.
//
// Une déconnexion, un changement de carte ou une sortie de groupe font quitter
// la partie sans rien attendre du client : un client qui plante n'enverrait
// jamais sa sortie, et l'équipe attendrait ses entrées à jamais. Le salon, lui,
// survit au changement de carte : le lieu n'y est qu'une raison de refus.
//
// 🔴 RIEN NE PART À UNE SESSION QUI N'A PAS ANNONCÉ MOONLIGHT_UI_SURVIVORS_COOP,
// et rien n'est entendu d'une session moonclient non vérifiée ou non chargée.

#include <algorithm>
#include <cstring>
#include <unordered_map>
#include <vector>

#include <common/cbasetypes.hpp>
#include <common/mmo.hpp>
#include <common/nullpo.hpp>
#include <common/random.hpp>
#include <common/socket.hpp>
#include <common/strlib.hpp>
#include <common/timer.hpp>

#include "clif.hpp"
#include "map.hpp" // map_charid2sd
#include "packets.hpp"
#include "party.hpp"
#include "pc.hpp"

namespace {

// Les tailles du contrat, que le client épingle à l'octet.
static_assert(sizeof(PACKET_CZ_MOONLIGHT_SURVIVORS_COOP_LOBBY) == 5);
static_assert(sizeof(PACKET_ZC_MOONLIGHT_SURVIVORS_COOP_LOBBY) == 10);
static_assert(sizeof(PACKET_ZC_MOONLIGHT_SURVIVORS_COOP_LOBBY_entry) == 33);
static_assert(sizeof(PACKET_ZC_MOONLIGHT_SURVIVORS_COOP_START) == 14);
static_assert(sizeof(PACKET_ZC_MOONLIGHT_SURVIVORS_COOP_START_entry) == 6);
static_assert(sizeof(PACKET_CZ_MOONLIGHT_SURVIVORS_COOP_FRAME) == 4);
static_assert(sizeof(PACKET_ZC_MOONLIGHT_SURVIVORS_COOP_FRAME) == 8);
static_assert(sizeof(PACKET_ZC_MOONLIGHT_SURVIVORS_COOP_LEFT) == 12);

// ⚠ Valeurs sur le fil : on AJOUTE en fin.
enum e_survivors_coop_action : uint8 {
	SURVIVORS_COOP_ENTER = 0,
	SURVIVORS_COOP_READY = 1,
	SURVIVORS_COOP_LEAVE = 2,
	SURVIVORS_COOP_CHOICE = 3,
};
enum e_survivors_coop_state : uint8 {
	SURVIVORS_COOP_CLOSED  = 0,
	SURVIVORS_COOP_LOBBY   = 1,
	SURVIVORS_COOP_PLAYING = 2,
};
enum e_survivors_coop_refusal : uint32 {
	SURVIVORS_COOP_REFUSE_TOO_MANY  = 0x01, ///< plus de membres que de places
	SURVIVORS_COOP_REFUSE_TOO_FEW   = 0x02, ///< moins de 2 membres en ligne
	SURVIVORS_COOP_REFUSE_OFFLINE   = 0x04, ///< un membre hors ligne
	SURVIVORS_COOP_REFUSE_PLACE     = 0x08, ///< un membre ni en ville ni en intérieur
	SURVIVORS_COOP_REFUSE_NO_CLIENT = 0x10, ///< un membre sans le bit de capacité
	SURVIVORS_COOP_REFUSE_NOT_IN_LOBBY = 0x20, ///< un membre n'est pas au salon
};

constexpr int32 SURVIVORS_COOP_MOST_MEMBERS = 4;
constexpr int32 SURVIVORS_COOP_FEWEST_ONLINE = 2;
// La part d'un joueur, jamais interprétée ici.
constexpr size_t SURVIVORS_COOP_BLOB_MAX_BYTES = 512;
// Une trame commence par son through_tick.
constexpr size_t SURVIVORS_COOP_FRAME_MIN_BYTES = sizeof(uint32);
constexpr size_t SURVIVORS_COOP_FRAME_MAX_BYTES = 256;
// Une entrée tapée au pas t vaut pour le pas t + ce délai.
constexpr uint8 SURVIVORS_COOP_INPUT_DELAY_TICKS = 4;
// Un client envoie ses trames sans arrêt, pause comprise, sauf quand toute l'équipe
// attend un coéquipier gelé : le serveur coupe celui-ci après `stall_time` (60 s).
// Au-delà de cette durée sans trame, le bouclier d'arène du joueur tombe.
constexpr t_tick SURVIVORS_COOP_SHIELD_SILENCE_MS = 2 * 60 * 1000;
// UN JOUEUR FIGÉ MAIS CONNECTÉ retient toute l'équipe : chacun s'arrête à son avance
// maximale, faute de ses entrées. Tout le monde se tait alors, et seul le retard sur
// les autres désigne le fautif. Il sort de la partie, et les autres reprennent avec le
// robot. Un joueur qui n'a encore rien envoyé peut monter son arène plus longtemps.
constexpr t_tick SURVIVORS_COOP_STALLED_MS = 15 * 1000;
constexpr t_tick SURVIVORS_COOP_FIRST_FRAME_MS = 60 * 1000;
constexpr uint32 SURVIVORS_COOP_STALLED_BEHIND_TICKS = 10;
constexpr t_tick SURVIVORS_COOP_WATCH_EVERY_MS = 1000;
// CE QUE LE SERVEUR LIT D'UNE TRAME, et rien de plus : [through_tick:4][count:1], puis
// `count` entrées de 3 octets, puis éventuellement une empreinte de 12 octets. Le retard
// qui désigne un joueur figé se lit de through_tick : une trame compte si elle apporte
// des tics neufs qu'elle porte vraiment. Seule la première peut sauter, de la longueur
// du délai d'entrée et de ce qui a pu être bridé.
constexpr size_t SURVIVORS_COOP_FRAME_HEAD_BYTES = sizeof(uint32) + sizeof(uint8);
constexpr size_t SURVIVORS_COOP_INPUT_BYTES = 3;
constexpr size_t SURVIVORS_COOP_CHOICE_BYTES = 2 * sizeof(uint16);
constexpr size_t SURVIVORS_COOP_DIGEST_BYTES = 12;
constexpr uint32 SURVIVORS_COOP_MOST_FIRST_THROUGH_TICK = 128;

struct s_survivors_coop_member {
	uint32 char_id = 0;
	bool ready = false;
	uint16 class_id = 0;
	uint16 first_skill_id = 0;
	std::vector<uint8> blob;
	uint32 through_tick = 0;
	// La dernière trame entendue, ou le départ de la partie.
	t_tick heard_at = 0;
	// Il a demandé à marcher pendant la partie : plus de bouclier d'arène.
	bool walked = false;
};

struct s_survivors_coop_group {
	uint8 state = SURVIVORS_COOP_LOBBY;
	// Les raisons de refus que le salon a dites pour la dernière fois.
	uint32 told_refusals = 0;
	// Dans l'ordre des places du groupe dès qu'un paquet les a énumérés.
	std::vector<s_survivors_coop_member> members;
};

// Par party_id.
std::unordered_map<int32, s_survivors_coop_group> survivors_coop_groups;
// Le groupe de chaque joueur en salon ou en partie, par char_id.
std::unordered_map<uint32, int32> survivors_coop_party_of_char;

// Une session moonclient vérifiée, chargée, qui a annoncé le bit.
bool coop_heard(const map_session_data& sd) {
	return sd.state.moonclient && sd.state.pc_loaded && (sd.bourgeon_ui_caps & MOONLIGHT_UI_SURVIVORS_COOP) != 0;
}

// Une session qui sait lire ces ZC, et dont le client est encore là : un
// vendeur en @autotrade garde ses bits mais n'a plus de connexion.
bool coop_listens(const map_session_data& sd) {
	return sd.state.moonclient && (sd.bourgeon_ui_caps & MOONLIGHT_UI_SURVIVORS_COOP) != 0 && session_isActive(sd.fd);
}

bool coop_place_accepted(const map_session_data& sd) {
	return survivors_arena_place(sd.m);
}

void coop_write(const map_session_data& sd, const void* bytes, size_t length) {
	if (!coop_listens(sd))
		return;
	const int32 fd = sd.fd;
	WFIFOHEAD(fd, length);
	memcpy(WFIFOP(fd, 0), bytes, length);
	WFIFOSET(fd, length);
}

// Le même paquet à chaque joueur de la liste.
void coop_write_members(const s_survivors_coop_group& group, const void* bytes, size_t length) {
	for (const s_survivors_coop_member& member : group.members) {
		if (const map_session_data* tsd = map_charid2sd(static_cast<int32>(member.char_id)); tsd != nullptr)
			coop_write(*tsd, bytes, length);
	}
}

// La place de `char_id` dans le groupe ; MAX_PARTY s'il n'y est pas.
int32 coop_seat_of(const party_data& party, uint32 char_id) {
	int32 seat;
	ARR_FIND(0, MAX_PARTY, seat, party.party.member[seat].char_id == char_id);
	return seat;
}

void coop_seat_members(const party_data& party, s_survivors_coop_group& group) {
	std::stable_sort(group.members.begin(), group.members.end(),
		[&party](const s_survivors_coop_member& left, const s_survivors_coop_member& right) {
			return coop_seat_of(party, left.char_id) < coop_seat_of(party, right.char_id);
		});
}

s_survivors_coop_member* coop_member_of(s_survivors_coop_group& group, uint32 char_id) {
	const auto found = std::find_if(group.members.begin(), group.members.end(),
		[char_id](const s_survivors_coop_member& member) { return member.char_id == char_id; });
	return found == group.members.end() ? nullptr : &*found;
}

// Ce qui empêche le groupe de partir. `departing` se déconnecte : il compte déjà
// pour hors ligne, alors que son groupe le tient encore en ligne.
uint32 coop_refusals(const party_data& party, const map_session_data* departing) {
	uint32 refusals = 0;
	int32 members = 0;
	int32 online = 0;
	for (int32 seat = 0; seat < MAX_PARTY; ++seat) {
		if (party.party.member[seat].char_id == 0)
			continue;
		++members;
		const map_session_data* tsd = party.data[seat].sd;
		if (tsd == nullptr || tsd == departing) {
			refusals |= SURVIVORS_COOP_REFUSE_OFFLINE;
			continue;
		}
		++online;
		if (!coop_place_accepted(*tsd))
			refusals |= SURVIVORS_COOP_REFUSE_PLACE;
		if (!coop_listens(*tsd))
			refusals |= SURVIVORS_COOP_REFUSE_NO_CLIENT;
	}
	if (members > SURVIVORS_COOP_MOST_MEMBERS)
		refusals |= SURVIVORS_COOP_REFUSE_TOO_MANY;
	if (online < SURVIVORS_COOP_FEWEST_ONLINE)
		refusals |= SURVIVORS_COOP_REFUSE_TOO_FEW;
	return refusals;
}

bool coop_all_ready(const party_data& party, s_survivors_coop_group& group) {
	for (int32 seat = 0; seat < MAX_PARTY; ++seat) {
		const uint32 char_id = party.party.member[seat].char_id;
		if (char_id == 0)
			continue;
		const s_survivors_coop_member* member = coop_member_of(group, char_id);
		if (member == nullptr || !member->ready)
			return false;
	}
	return true;
}

std::vector<uint8> coop_lobby_packet(uint8 state, uint32 refusals, const s_survivors_coop_group* group) {
	using t_entry = PACKET_ZC_MOONLIGHT_SURVIVORS_COOP_LOBBY_entry;
	std::vector<t_entry> entries;
	if (group != nullptr) {
		for (const s_survivors_coop_member& member : group->members) {
			const map_session_data* tsd = map_charid2sd(static_cast<int32>(member.char_id));
			if (tsd == nullptr)
				continue;
			t_entry entry{};
			entry.char_id = member.char_id;
			entry.ready   = member.ready ? 1 : 0;
			safestrncpy(entry.name, tsd->status.name, sizeof(entry.name));
			entry.class_id       = member.class_id;
			entry.first_skill_id = member.first_skill_id;
			entries.push_back(entry);
		}
	}
	const size_t head = sizeof(PACKET_ZC_MOONLIGHT_SURVIVORS_COOP_LOBBY);
	std::vector<uint8> bytes(head + entries.size() * sizeof(t_entry));
	PACKET_ZC_MOONLIGHT_SURVIVORS_COOP_LOBBY* packet = reinterpret_cast<PACKET_ZC_MOONLIGHT_SURVIVORS_COOP_LOBBY*>(bytes.data());
	packet->packetType   = HEADER_ZC_MOONLIGHT_SURVIVORS_COOP_LOBBY;
	packet->packetLength = static_cast<int16>(bytes.size());
	packet->state        = state;
	packet->refusals     = refusals;
	packet->count        = static_cast<uint8>(entries.size());
	if (!entries.empty())
		memcpy(bytes.data() + head, entries.data(), entries.size() * sizeof(t_entry));
	return bytes;
}

// « Ni salon ni partie pour toi. »
void coop_tell_closed(const map_session_data& sd, uint32 refusals) {
	const std::vector<uint8> bytes = coop_lobby_packet(SURVIVORS_COOP_CLOSED, refusals, nullptr);
	coop_write(sd, bytes.data(), bytes.size());
}

// L'état du groupe, à un seul.
void coop_tell_state(const map_session_data& sd, const s_survivors_coop_group& group) {
	const std::vector<uint8> bytes = coop_lobby_packet(group.state,
		group.state == SURVIVORS_COOP_LOBBY ? group.told_refusals : 0, &group);
	coop_write(sd, bytes.data(), bytes.size());
}

// L'état du groupe, à tous ses joueurs.
void coop_tell_group(const s_survivors_coop_group& group) {
	const std::vector<uint8> bytes = coop_lobby_packet(group.state,
		group.state == SURVIVORS_COOP_LOBBY ? group.told_refusals : 0, &group);
	coop_write_members(group, bytes.data(), bytes.size());
}

// Le salon devient une partie. Ses joueurs apprennent l'état, puis les données
// du départ.
void coop_start(const party_data& party, s_survivors_coop_group& group) {
	coop_seat_members(party, group);
	group.state = SURVIVORS_COOP_PLAYING;
	coop_tell_group(group);

	const size_t head = sizeof(PACKET_ZC_MOONLIGHT_SURVIVORS_COOP_START);
	size_t length = head;
	for (const s_survivors_coop_member& member : group.members)
		length += sizeof(PACKET_ZC_MOONLIGHT_SURVIVORS_COOP_START_entry) + member.blob.size();
	std::vector<uint8> bytes(length);
	PACKET_ZC_MOONLIGHT_SURVIVORS_COOP_START* packet = reinterpret_cast<PACKET_ZC_MOONLIGHT_SURVIVORS_COOP_START*>(bytes.data());
	packet->packetType        = HEADER_ZC_MOONLIGHT_SURVIVORS_COOP_START;
	packet->packetLength      = static_cast<int16>(length);
	packet->seed              = rnd_value<uint64>(1, UINT64_MAX);
	packet->input_delay_ticks = SURVIVORS_COOP_INPUT_DELAY_TICKS;
	packet->count             = static_cast<uint8>(group.members.size());
	size_t offset = head;
	for (s_survivors_coop_member& member : group.members) {
		PACKET_ZC_MOONLIGHT_SURVIVORS_COOP_START_entry entry{};
		entry.char_id  = member.char_id;
		entry.blob_len = static_cast<uint16>(member.blob.size());
		memcpy(bytes.data() + offset, &entry, sizeof(entry));
		offset += sizeof(entry);
		if (!member.blob.empty())
			memcpy(bytes.data() + offset, member.blob.data(), member.blob.size());
		offset += member.blob.size();
		// La part a fait son office : une partie longue n'a pas à la garder.
		member.blob.clear();
		member.blob.shrink_to_fit();
		member.heard_at = gettick();
	}
	coop_write_members(group, bytes.data(), bytes.size());
}

void coop_dissolve(int32 party_id);

// Quelque chose a changé pour le salon du groupe : il s'efface s'il est vide,
// part si plus rien ne le retient, ou redit son état. `lobby_changed` faux, le
// changement ne touche que le groupe autour du salon : rien ne se redit tant que
// les raisons de refus restent les mêmes.
void coop_lobby_changed(int32 party_id, const map_session_data* departing, bool lobby_changed) {
	const auto found = survivors_coop_groups.find(party_id);
	if (found == survivors_coop_groups.end() || found->second.state != SURVIVORS_COOP_LOBBY)
		return;
	s_survivors_coop_group& group = found->second;
	if (group.members.empty()) {
		survivors_coop_groups.erase(found);
		return;
	}
	const party_data* party = party_search(party_id);
	if (party == nullptr) {
		coop_dissolve(party_id);
		return;
	}
	uint32 refusals = coop_refusals(*party, departing);
	for (int32 seat = 0; seat < MAX_PARTY; ++seat) {
		const uint32 seat_char_id = party->party.member[seat].char_id;
		if (seat_char_id != 0 && coop_member_of(group, seat_char_id) == nullptr)
			refusals |= SURVIVORS_COOP_REFUSE_NOT_IN_LOBBY;
	}
	if (refusals == 0 && coop_all_ready(*party, group)) {
		group.told_refusals = 0;
		coop_start(*party, group);
		return;
	}
	if (!lobby_changed && refusals == group.told_refusals)
		return;
	group.told_refusals = refusals;
	coop_seat_members(*party, group);
	coop_tell_group(group);
}

// `char_id` quitte le salon ou la partie. Il l'apprend si `tell_leaver` : celui
// qui se déconnecte n'écoute plus, et peut être déjà à la sélection de personnage.
void coop_drop(int32 party_id, uint32 char_id, bool tell_leaver, const map_session_data* departing) {
	const auto found = survivors_coop_groups.find(party_id);
	if (found == survivors_coop_groups.end())
		return;
	s_survivors_coop_group& group = found->second;
	const s_survivors_coop_member* member = coop_member_of(group, char_id);
	if (member == nullptr)
		return;
	const uint32 through_tick = member->through_tick;
	group.members.erase(group.members.begin() + (member - group.members.data()));
	survivors_coop_party_of_char.erase(char_id);
	const map_session_data* leaver = tell_leaver ? map_charid2sd(static_cast<int32>(char_id)) : nullptr;
	if (leaver != nullptr)
		coop_tell_closed(*leaver, 0);
	if (group.state == SURVIVORS_COOP_PLAYING) {
		PACKET_ZC_MOONLIGHT_SURVIVORS_COOP_LEFT packet{};
		packet.packetType   = HEADER_ZC_MOONLIGHT_SURVIVORS_COOP_LEFT;
		packet.packetLength = static_cast<int16>(sizeof(packet));
		packet.char_id      = char_id;
		packet.through_tick = through_tick;
		coop_write_members(group, &packet, sizeof(packet));
		// Lui aussi : l'état fermé ne le lui dit pas, puisqu'il suit aussi chaque annonce
		// du service. Sans cela, sorti par un changement de carte, il attendrait les autres.
		if (leaver != nullptr)
			coop_write(*leaver, &packet, sizeof(packet));
		if (group.members.empty())
			survivors_coop_groups.erase(found);
		return;
	}
	coop_lobby_changed(party_id, departing, true);
}

// Le groupe n'existe plus. Un salon se ferme d'un coup : retirer ses membres un à
// un pourrait le faire partir avec ceux qui restent. Une partie perd ses joueurs
// un à un, et chacun l'apprend des autres.
void coop_dissolve(int32 party_id) {
	const auto found = survivors_coop_groups.find(party_id);
	if (found == survivors_coop_groups.end())
		return;
	std::vector<uint32> char_ids;
	for (const s_survivors_coop_member& member : found->second.members)
		char_ids.push_back(member.char_id);
	if (found->second.state == SURVIVORS_COOP_PLAYING) {
		for (const uint32 char_id : char_ids)
			coop_drop(party_id, char_id, true, nullptr);
		return;
	}
	survivors_coop_groups.erase(found);
	for (const uint32 char_id : char_ids) {
		survivors_coop_party_of_char.erase(char_id);
		if (const map_session_data* tsd = map_charid2sd(static_cast<int32>(char_id)); tsd != nullptr)
			coop_tell_closed(*tsd, 0);
	}
}

// Le groupe de `char_id` en salon ou en partie ; 0 sans.
int32 coop_party_of(uint32 char_id) {
	const auto found = survivors_coop_party_of_char.find(char_id);
	return found == survivors_coop_party_of_char.end() ? 0 : found->second;
}

void coop_enter(map_session_data& sd) {
	const uint32 char_id = sd.status.char_id;
	const int32 party_id = sd.status.party_id;
	const party_data* party = party_id != 0 ? party_search(party_id) : nullptr;
	if (party == nullptr) {
		coop_tell_closed(sd, SURVIVORS_COOP_REFUSE_TOO_FEW);
		return;
	}
	s_survivors_coop_group& group = survivors_coop_groups.try_emplace(party_id).first->second;
	// Déjà là, ou une partie déjà lancée sans lui : il apprend où en est le groupe.
	if (group.state == SURVIVORS_COOP_PLAYING || coop_member_of(group, char_id) != nullptr) {
		coop_tell_state(sd, group);
		return;
	}
	s_survivors_coop_member member;
	member.char_id = char_id;
	group.members.push_back(std::move(member));
	survivors_coop_party_of_char[char_id] = party_id;
	coop_lobby_changed(party_id, nullptr, true);
}

void coop_ready(map_session_data& sd, const uint8* blob, size_t blob_bytes) {
	const uint32 char_id = sd.status.char_id;
	const int32 party_id = coop_party_of(char_id);
	if (party_id == 0) {
		coop_tell_closed(sd, 0);
		return;
	}
	s_survivors_coop_group& group = survivors_coop_groups[party_id];
	s_survivors_coop_member* member = coop_member_of(group, char_id);
	if (member == nullptr || group.state != SURVIVORS_COOP_LOBBY)
		return;
	member->ready = true;
	member->blob.assign(blob, blob + blob_bytes);
	coop_lobby_changed(party_id, nullptr, true);
}

// Le choix change : la part déposée ne le dit plus, le joueur n'est plus prêt.
void coop_choose(const map_session_data& sd, uint16 class_id, uint16 first_skill_id) {
	const int32 party_id = coop_party_of(sd.status.char_id);
	if (party_id == 0)
		return;
	s_survivors_coop_group& group = survivors_coop_groups[party_id];
	s_survivors_coop_member* member = coop_member_of(group, sd.status.char_id);
	if (member == nullptr || group.state != SURVIVORS_COOP_LOBBY)
		return;
	if (member->class_id == class_id && member->first_skill_id == first_skill_id)
		return;
	member->class_id       = class_id;
	member->first_skill_id = first_skill_id;
	member->ready          = false;
	member->blob.clear();
	coop_lobby_changed(party_id, nullptr, true);
}

} // namespace

void clif_moonlight_survivors_coop_announce(map_session_data& sd) {
	if (!coop_listens(sd))
		return;
	// L'annonce dit « fermé » même à qui tient un salon : le client redit ses
	// capacités à chaque entrée en carte. Son véritable état la suit donc.
	coop_tell_closed(sd, 0);
	if (const int32 party_id = coop_party_of(sd.status.char_id); party_id != 0)
		coop_tell_state(sd, survivors_coop_groups[party_id]);
	if (sd.status.party_id != 0)
		coop_lobby_changed(sd.status.party_id, nullptr, false);
}

void survivors_coop_caps_changed(map_session_data& sd) {
	if (!(sd.bourgeon_ui_caps & MOONLIGHT_UI_SURVIVORS_COOP)) {
		if (const int32 party_id = coop_party_of(sd.status.char_id); party_id != 0) {
			coop_drop(party_id, sd.status.char_id, false, nullptr);
			return;
		}
	}
	if (sd.status.party_id != 0)
		coop_lobby_changed(sd.status.party_id, nullptr, false);
}

void survivors_coop_quit(map_session_data& sd) {
	if (const int32 party_id = coop_party_of(sd.status.char_id); party_id != 0)
		coop_drop(party_id, sd.status.char_id, false, &sd);
	else if (sd.status.party_id != 0)
		coop_lobby_changed(sd.status.party_id, &sd, false);
}

void survivors_coop_map_left(map_session_data& sd) {
	const int32 party_id = coop_party_of(sd.status.char_id);
	if (party_id != 0 && survivors_coop_groups[party_id].state == SURVIVORS_COOP_PLAYING)
		coop_drop(party_id, sd.status.char_id, true, nullptr);
}

void survivors_coop_member_moved(map_session_data& sd) {
	if (sd.status.party_id != 0)
		coop_lobby_changed(sd.status.party_id, nullptr, false);
}

void survivors_coop_party_left(int32 party_id, uint32 char_id) {
	if (coop_party_of(char_id) == party_id)
		coop_drop(party_id, char_id, true, nullptr);
	else
		coop_lobby_changed(party_id, nullptr, false);
}

void survivors_coop_party_changed(int32 party_id) {
	coop_lobby_changed(party_id, nullptr, false);
}

void survivors_coop_party_broken(int32 party_id) {
	coop_dissolve(party_id);
}

void survivors_coop_drop_stalled(t_tick now) {
	std::vector<std::pair<int32, uint32>> stalled;
	for (const auto& [party_id, group] : survivors_coop_groups) {
		if (group.state != SURVIVORS_COOP_PLAYING)
			continue;
		uint32 furthest = 0;
		for (const s_survivors_coop_member& member : group.members)
			furthest = std::max(furthest, member.through_tick);
		for (const s_survivors_coop_member& member : group.members) {
			const t_tick silent = DIFF_TICK(now, member.heard_at);
			const bool holds_back = member.through_tick == 0
				? furthest != 0 && silent >= SURVIVORS_COOP_FIRST_FRAME_MS
				: silent >= SURVIVORS_COOP_STALLED_MS && member.through_tick + SURVIVORS_COOP_STALLED_BEHIND_TICKS < furthest;
			if (holds_back)
				stalled.emplace_back(party_id, member.char_id);
		}
	}
	for (const auto& [party_id, char_id] : stalled)
		coop_drop(party_id, char_id, true, nullptr);
}

static TIMER_FUNC(survivors_coop_watch) {
	survivors_coop_drop_stalled(tick);
	return 0;
}

void do_init_survivors_coop() {
	add_timer_func_list(survivors_coop_watch, "survivors_coop_watch");
	add_timer_interval(gettick() + SURVIVORS_COOP_WATCH_EVERY_MS, survivors_coop_watch, 0, 0, SURVIVORS_COOP_WATCH_EVERY_MS);
}

bool survivors_coop_shields(const map_session_data& sd) {
	const int32 party_id = coop_party_of(sd.status.char_id);
	if (party_id == 0 || !coop_heard(sd))
		return false;
	const auto found = survivors_coop_groups.find(party_id);
	if (found == survivors_coop_groups.end() || found->second.state != SURVIVORS_COOP_PLAYING)
		return false;
	const s_survivors_coop_member* member = coop_member_of(found->second, sd.status.char_id);
	return member != nullptr && !member->walked && DIFF_TICK(gettick(), member->heard_at) < SURVIVORS_COOP_SHIELD_SILENCE_MS;
}

void survivors_coop_walked(const map_session_data& sd) {
	const int32 party_id = coop_party_of(sd.status.char_id);
	if (party_id == 0)
		return;
	const auto found = survivors_coop_groups.find(party_id);
	if (found == survivors_coop_groups.end() || found->second.state != SURVIVORS_COOP_PLAYING)
		return;
	// Sa première trame prouve qu'il a reçu le départ : une marche d'avant s'est
	// croisée avec lui sur le fil.
	if (s_survivors_coop_member* member = coop_member_of(found->second, sd.status.char_id); member != nullptr && member->through_tick != 0)
		member->walked = true;
}

// Dans l'ordre : l'écoute, la cadence, l'action.
void clif_parse_moonlight_survivors_coop_lobby(int32 fd, map_session_data* sd) {
	nullpo_retv(sd);
	if (!coop_heard(*sd))
		return;
	if (!clif_moonlight_throttle_pass(*sd, MOONLIGHT_THROTTLE_SURVIVORS_COOP_LOBBY))
		return;
	const PACKET_CZ_MOONLIGHT_SURVIVORS_COOP_LOBBY* p =
		reinterpret_cast<const PACKET_CZ_MOONLIGHT_SURVIVORS_COOP_LOBBY*>(RFIFOP(fd, 0));
	const size_t length = static_cast<uint16>(p->packetLength);
	if (length < sizeof(*p))
		return;
	switch (p->action) {
		case SURVIVORS_COOP_ENTER:
			coop_enter(*sd);
			break;
		case SURVIVORS_COOP_READY: {
			const size_t blob_bytes = length - sizeof(*p);
			if (blob_bytes <= SURVIVORS_COOP_BLOB_MAX_BYTES)
				coop_ready(*sd, RFIFOP(fd, sizeof(*p)), blob_bytes);
			break;
		}
		case SURVIVORS_COOP_LEAVE:
			if (const int32 party_id = coop_party_of(sd->status.char_id); party_id != 0)
				coop_drop(party_id, sd->status.char_id, true, nullptr);
			else
				coop_tell_closed(*sd, 0);
			break;
		case SURVIVORS_COOP_CHOICE:
			if (length == sizeof(*p) + SURVIVORS_COOP_CHOICE_BYTES)
				coop_choose(*sd, RFIFOW(fd, sizeof(*p)), RFIFOW(fd, sizeof(*p) + sizeof(uint16)));
			break;
		default:
			break;
	}
}

void clif_parse_moonlight_survivors_coop_frame(int32 fd, map_session_data* sd) {
	nullpo_retv(sd);
	if (!coop_heard(*sd))
		return;
	if (!clif_moonlight_throttle_pass(*sd, MOONLIGHT_THROTTLE_SURVIVORS_COOP_FRAME))
		return;
	const PACKET_CZ_MOONLIGHT_SURVIVORS_COOP_FRAME* p =
		reinterpret_cast<const PACKET_CZ_MOONLIGHT_SURVIVORS_COOP_FRAME*>(RFIFOP(fd, 0));
	const size_t length = static_cast<uint16>(p->packetLength);
	if (length < sizeof(*p) + SURVIVORS_COOP_FRAME_MIN_BYTES || length > sizeof(*p) + SURVIVORS_COOP_FRAME_MAX_BYTES)
		return;
	const uint32 char_id = sd->status.char_id;
	const int32 party_id = coop_party_of(char_id);
	if (party_id == 0)
		return;
	s_survivors_coop_group& group = survivors_coop_groups[party_id];
	s_survivors_coop_member* sender = coop_member_of(group, char_id);
	if (sender == nullptr || group.state != SURVIVORS_COOP_PLAYING)
		return;
	const size_t payload_bytes = length - sizeof(*p);
	if (payload_bytes < SURVIVORS_COOP_FRAME_HEAD_BYTES)
		return;
	const uint32 through_tick = RFIFOL(fd, sizeof(*p));
	const uint32 input_count = RFIFOB(fd, sizeof(*p) + sizeof(uint32));
	const size_t inputs_end = SURVIVORS_COOP_FRAME_HEAD_BYTES + input_count * SURVIVORS_COOP_INPUT_BYTES;
	if (payload_bytes != inputs_end && payload_bytes != inputs_end + SURVIVORS_COOP_DIGEST_BYTES)
		return;
	const bool first = sender->through_tick == 0;
	if (input_count == 0 || through_tick <= sender->through_tick ||
		through_tick - sender->through_tick > (first ? SURVIVORS_COOP_MOST_FIRST_THROUGH_TICK : input_count))
		return;
	sender->through_tick = through_tick;
	sender->heard_at = gettick();

	PACKET_ZC_MOONLIGHT_SURVIVORS_COOP_FRAME head{};
	head.packetType   = HEADER_ZC_MOONLIGHT_SURVIVORS_COOP_FRAME;
	head.packetLength = static_cast<int16>(sizeof(head) + payload_bytes);
	head.char_id      = char_id;
	for (const s_survivors_coop_member& member : group.members) {
		if (member.char_id == char_id)
			continue;
		const map_session_data* tsd = map_charid2sd(static_cast<int32>(member.char_id));
		if (tsd == nullptr || !coop_listens(*tsd))
			continue;
		const int32 out = tsd->fd;
		WFIFOHEAD(out, sizeof(head) + payload_bytes);
		memcpy(WFIFOP(out, 0), &head, sizeof(head));
		memcpy(WFIFOP(out, sizeof(head)), RFIFOP(fd, sizeof(*p)), payload_bytes);
		WFIFOSET(out, sizeof(head) + payload_bytes);
	}
}
