// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// [Stingor] Les règles du classement survivors : voir l'en-tête.

#include "survivors_ranking_rules.hpp"

#include <algorithm>
#include <cctype>
#include <functional>
#include <iterator>
#include <numeric>
#include <string>
#include <vector>

namespace {

// ── LE CALENDRIER GRÉGORIEN, en entiers ─────────────────────────────────────
// 365 jours par an, un de plus les années bissextiles (divisibles par 4, sauf
// les siècles que 400 ne divise pas). Quatre cents ans font donc toujours le
// même nombre de jours : un jour se ramène près de son année sans compter les
// siècles un à un.
constexpr int64 EPOCH_YEAR = 1970;
constexpr int64 YEARS_PER_CYCLE = 400;
constexpr int64 LEAP_YEARS_PER_CYCLE = 97;
constexpr int64 DAYS_PER_YEAR = 365;
constexpr int64 DAYS_PER_LEAP_YEAR = DAYS_PER_YEAR + 1;
constexpr int64 DAYS_PER_CYCLE = YEARS_PER_CYCLE * DAYS_PER_YEAR + LEAP_YEARS_PER_CYCLE;
constexpr int64 LEAP_EVERY_YEARS = 4;
constexpr int64 NO_LEAP_EVERY_YEARS = 100;
constexpr int64 MONTHS_PER_YEAR = 12;
constexpr int64 SECONDS_PER_DAY = 86400;

constexpr int64 DAYS_PER_WEEK = 7;
// Le 1970-01-01 est un jeudi : jour 4 de la semaine ISO (lundi = 1).
constexpr int64 EPOCH_ISO_WEEKDAY = 4;
constexpr int64 THURSDAY_ISO_WEEKDAY = 4;

constexpr int64 WEEKS_PER_SEED_YEAR = 100;
// Le sel de la graine : « SURVWEEK » en ASCII.
constexpr uint64 CHALLENGE_SEED_SALT = 0x535552565745454BULL;
// Les constantes publiées de SplitMix64 : l'incrément (partie fractionnaire du
// nombre d'or) et les deux multiplicateurs du finaliseur.
constexpr uint64 SPLITMIX_GAMMA = 0x9E3779B97F4A7C15ULL;
constexpr uint64 SPLITMIX_FIRST_MULTIPLIER = 0xBF58476D1CE4E5B9ULL;
constexpr uint64 SPLITMIX_SECOND_MULTIPLIER = 0x94D049BB133111EBULL;
constexpr uint32 SPLITMIX_FIRST_SHIFT = 30;
constexpr uint32 SPLITMIX_SECOND_SHIFT = 27;
constexpr uint32 SPLITMIX_LAST_SHIFT = 31;

struct s_civil_date {
	int32 year = 1970;
	uint8 month = 1; ///< 1 à 12
	uint8 day = 1;   ///< 1 à 31
};

constexpr int64 floor_divide(int64 value, int64 by) {
	const int64 quotient = value / by;
	return (value % by != 0 && (value < 0) != (by < 0)) ? quotient - 1 : quotient;
}

constexpr bool is_leap_year(int64 year) {
	return year % LEAP_EVERY_YEARS == 0 && (year % NO_LEAP_EVERY_YEARS != 0 || year % YEARS_PER_CYCLE == 0);
}

constexpr int64 days_in_month(int64 year, int64 month) {
	constexpr int64 FEBRUARY = 2;
	constexpr int64 LONG_MONTH = 31;
	constexpr int64 SHORT_MONTH = 30;
	constexpr int64 LEAP_FEBRUARY = 29;
	constexpr int64 FEBRUARY_DAYS = 28;
	if (month == FEBRUARY)
		return is_leap_year(year) ? LEAP_FEBRUARY : FEBRUARY_DAYS;
	// Avril, juin, septembre et novembre ont trente jours.
	constexpr int64 APRIL = 4;
	constexpr int64 JUNE = 6;
	constexpr int64 SEPTEMBER = 9;
	constexpr int64 NOVEMBER = 11;
	if (month == APRIL || month == JUNE || month == SEPTEMBER || month == NOVEMBER)
		return SHORT_MONTH;
	return LONG_MONTH;
}

// Un compte cumulé des années bissextiles : `leap_years_through(y) −
// leap_years_through(y − 1)` vaut 1 exactement quand `y` est bissextile, y
// compris avant l'an 1 (l'an 0 l'est).
constexpr int64 leap_years_through(int64 year) {
	return floor_divide(year, LEAP_EVERY_YEARS) - floor_divide(year, NO_LEAP_EVERY_YEARS) + floor_divide(year, YEARS_PER_CYCLE);
}

// Les jours du 1970-01-01 au 1er janvier de `year`, négatifs avant 1970.
constexpr int64 days_before_year(int64 year) {
	return (year - EPOCH_YEAR) * DAYS_PER_YEAR + leap_years_through(year - 1) - leap_years_through(EPOCH_YEAR - 1);
}

s_civil_date civil_date_of_day(int64 days_since_epoch) {
	const int64 cycles = floor_divide(days_since_epoch, DAYS_PER_CYCLE);
	const int64 in_cycle = days_since_epoch - cycles * DAYS_PER_CYCLE;
	// Aucune année n'a plus de 366 jours : l'estimation tombe sur l'année ou
	// juste avant, et ne monte que d'une ou deux années.
	int64 year = EPOCH_YEAR + cycles * YEARS_PER_CYCLE + in_cycle / DAYS_PER_LEAP_YEAR;
	while (days_before_year(year + 1) <= days_since_epoch)
		++year;
	int64 day_of_year = days_since_epoch - days_before_year(year);
	int64 month = 1;
	while (month < MONTHS_PER_YEAR && day_of_year >= days_in_month(year, month)) {
		day_of_year -= days_in_month(year, month);
		++month;
	}
	s_civil_date date;
	date.year = static_cast<int32>(year);
	date.month = static_cast<uint8>(month);
	date.day = static_cast<uint8>(day_of_year + 1);
	return date;
}

int64 day_of_civil_date(const s_civil_date& date) {
	int64 days = days_before_year(date.year);
	for (int64 month = 1; month < date.month && month <= MONTHS_PER_YEAR; ++month)
		days += days_in_month(date.year, month);
	return days + date.day - 1;
}

s_survivors_iso_week iso_week_of_day(int64 days_since_epoch) {
	// Le jeudi de la semaine décide de son année : la semaine 1 est celle du
	// premier jeudi.
	const int64 from_monday = days_since_epoch + EPOCH_ISO_WEEKDAY - 1;
	const int64 weekday = from_monday - floor_divide(from_monday, DAYS_PER_WEEK) * DAYS_PER_WEEK + 1;
	const int64 thursday = days_since_epoch - weekday + THURSDAY_ISO_WEEKDAY;
	const int32 iso_year = civil_date_of_day(thursday).year;
	s_civil_date first;
	first.year = iso_year;
	const int64 first_day = day_of_civil_date(first);
	s_survivors_iso_week week;
	week.year = iso_year;
	week.week = static_cast<uint8>((thursday - first_day) / DAYS_PER_WEEK + 1);
	return week;
}

// ── LE RULESET 3 ─────────────────────────────────────────────────────────────
// La première arène complète (prt_fild08), le Magicien, quinze minutes et
// Baphomet, puis cinq minutes de dépassement tant qu'il vit.
constexpr uint16 RULESET_3 = 3;
constexpr uint16 RULESET_3_ARENAS[] = { 1 };
constexpr uint16 RULESET_3_CLASSES[] = { 1 };
constexpr uint32 MINUTE_MS = 60000;
constexpr uint32 RULESET_3_RUN_MS = 15 * MINUTE_MS;
constexpr uint32 RULESET_3_OVERTIME_MS = 5 * MINUTE_MS;
// Les points d'un tué : niveau RO × 20 pour un mini-boss, × 100 pour un boss.
// Mastering (2), Eclipse (6), Dragon Fly (8), Vagabond Wolf (24), Baphomet (81).
constexpr uint32 MINI_BOSS_KILL_FACTOR = 20;
constexpr uint32 BOSS_KILL_FACTOR = 100;
constexpr s_survivors_boss_bound RULESET_3_BOSSES[] = {
	{  3 * MINUTE_MS,  2 * MINI_BOSS_KILL_FACTOR, 20, false },
	{  5 * MINUTE_MS,  6 * MINI_BOSS_KILL_FACTOR, 30, false },
	{  7 * MINUTE_MS,  8 * MINI_BOSS_KILL_FACTOR, 40, false },
	{  9 * MINUTE_MS, 24 * MINI_BOSS_KILL_FACTOR, 60, false },
	{ 15 * MINUTE_MS, 81 * BOSS_KILL_FACTOR,       0, true  },
};
constexpr s_survivors_exp_segment RULESET_3_EXP_CURVE[] = {
	{  1,    5,  10 },
	{ 20,  220,  40 },
	{ 40, 3500, 200 },
};
// Le tué ordinaire le plus fort : Evil Druid, niveau RO 58 et 22 d'XP, en élite
// (points × 5, XP à 500 %).
constexpr uint32 ELITE_FACTOR = 5;
constexpr uint32 RULESET_3_COMMON_KILL_POINTS = 58 * ELITE_FACTOR;
constexpr uint32 RULESET_3_COMMON_KILL_EXP = 22 * ELITE_FACTOR;
// Deux fois le plafond de naissances par pas (10), à cinquante pas par seconde :
// les lots des vagues, les nuées et les invocations s'ajoutent au remplissage.
constexpr uint32 RULESET_3_KILLS_PER_SECOND = 2 * 10 * SURVIVORS_STEPS_PER_SECOND;
// Une partie classée part neutre : aucune majoration d'XP que le Battle Manual
// (+100 %).
constexpr uint32 RULESET_3_EXP_GAIN_PCT = 200;

constexpr s_survivors_ruleset RULESETS[] = {
	{ RULESET_3,
	  RULESET_3_ARENAS, std::size(RULESET_3_ARENAS),
	  RULESET_3_CLASSES, std::size(RULESET_3_CLASSES),
	  RULESET_3_RUN_MS, RULESET_3_OVERTIME_MS,
	  RULESET_3_BOSSES, std::size(RULESET_3_BOSSES),
	  RULESET_3_EXP_CURVE, std::size(RULESET_3_EXP_CURVE),
	  RULESET_3_KILLS_PER_SECOND, RULESET_3_COMMON_KILL_POINTS, RULESET_3_COMMON_KILL_EXP, RULESET_3_EXP_GAIN_PCT },
};

constexpr uint64 SCORE_PER_SECOND = 10;
constexpr uint64 SCORE_PER_LEVEL = 100;
constexpr uint64 SURVIVED_SCORE = 5000;
constexpr uint64 VICTORY_SCORE = 25000;
constexpr uint64 WHOLE_PCT = 100;

bool contains(const uint16* ids, size_t count, uint16 id) {
	for (size_t i = 0; i < count; ++i) {
		if (ids[i] == id)
			return true;
	}
	return false;
}

// L'XP qui mène du niveau `level` au suivant ; 0 : aucun niveau de plus.
uint64 exp_to_next(const s_survivors_ruleset& rules, uint16 level) {
	const s_survivors_exp_segment* segment = nullptr;
	for (size_t i = 0; i < rules.exp_segment_count; ++i) {
		if (rules.exp_curve[i].from_level <= level)
			segment = &rules.exp_curve[i];
	}
	if (segment == nullptr || level == 0)
		return 0;
	const uint64 needed = uint64{ segment->base } + uint64{ segment->per_level } * static_cast<uint64>(level - segment->from_level);
	return needed == 0 ? 1 : needed;
}

// Le niveau que `exp` donne au héros parti du niveau 1.
uint32 level_reachable(const s_survivors_ruleset& rules, uint64 exp) {
	constexpr uint32 MOST_LEVEL = UINT16_MAX;
	uint32 level = 1;
	while (level < MOST_LEVEL) {
		const uint64 needed = exp_to_next(rules, static_cast<uint16>(level));
		if (needed == 0 || exp < needed)
			break;
		exp -= needed;
		++level;
	}
	return level;
}

} // namespace

s_survivors_iso_week survivors_iso_week_of_utc(int64 utc_s) {
	return iso_week_of_day(floor_divide(utc_s, SECONDS_PER_DAY));
}

uint32 survivors_week_key(const s_survivors_iso_week& week) {
	return static_cast<uint32>(int64{ week.year } * WEEKS_PER_SEED_YEAR + week.week);
}

uint64 survivors_seed_of_week(const s_survivors_iso_week& week) {
	const uint64 week_number = static_cast<uint64>(int64{ week.year } * WEEKS_PER_SEED_YEAR + int64{ week.week });
	uint64 mixed = week_number + CHALLENGE_SEED_SALT + SPLITMIX_GAMMA;
	mixed = (mixed ^ (mixed >> SPLITMIX_FIRST_SHIFT)) * SPLITMIX_FIRST_MULTIPLIER;
	mixed = (mixed ^ (mixed >> SPLITMIX_SECOND_SHIFT)) * SPLITMIX_SECOND_MULTIPLIER;
	return mixed ^ (mixed >> SPLITMIX_LAST_SHIFT);
}

uint64 survivors_score(const s_survivors_components& run) {
	uint64 outcome_score = 0;
	if (run.outcome == SURVIVORS_OUTCOME_SURVIVED)
		outcome_score = SURVIVED_SCORE;
	else if (run.outcome == SURVIVORS_OUTCOME_VICTORY)
		outcome_score = VICTORY_SCORE;
	const uint64 survived_seconds = run.survived_steps / SURVIVORS_STEPS_PER_SECOND;
	return uint64{ run.kill_points } + SCORE_PER_SECOND * survived_seconds + SCORE_PER_LEVEL * uint64{ run.level } + outcome_score;
}

uint16 survivors_ruleset_low() {
	return RULESETS[0].ruleset;
}

uint16 survivors_ruleset_high() {
	return RULESETS[std::size(RULESETS) - 1].ruleset;
}

const s_survivors_ruleset* survivors_find_ruleset(uint16 ruleset) {
	for (const s_survivors_ruleset& rules : RULESETS) {
		if (rules.ruleset == ruleset)
			return &rules;
	}
	return nullptr;
}

bool survivors_board_known(const s_survivors_ruleset& rules, uint16 arena_id, uint16 class_id, uint8 board) {
	return (board == SURVIVORS_BOARD_RANKED || board == SURVIVORS_BOARD_CHALLENGE)
		&& contains(rules.arena_ids, rules.arena_count, arena_id)
		&& contains(rules.class_ids, rules.class_count, class_id);
}

bool survivors_plausible(const s_survivors_ruleset& rules, const s_survivors_components& run, int64 elapsed_s) {
	if (run.outcome < SURVIVORS_OUTCOME_FELL || run.outcome > SURVIVORS_OUTCOME_VICTORY)
		return false;
	const uint64 longest_steps = (uint64{ rules.run_ms } + rules.overtime_ms) / SURVIVORS_MS_PER_STEP;
	if (run.survived_steps > longest_steps + SURVIVORS_STEPS_SLACK)
		return false;
	const uint64 survived_seconds = run.survived_steps / SURVIVORS_STEPS_PER_SECOND;
	if (static_cast<int64>(survived_seconds) > elapsed_s + SURVIVORS_DURATION_SLACK_S)
		return false;
	if (run.boss_kills > run.kills || run.kills > uint64{ rules.most_kills_per_second } * (survived_seconds + 1))
		return false;

	const uint64 survived_ms = uint64{ run.survived_steps } * SURVIVORS_MS_PER_STEP;
	// Les boss parus avant ce pas ; des boss tués, on retient au plus fort les
	// points et l'XP des `boss_kills` plus riches.
	std::vector<uint32> met_points;
	std::vector<uint32> met_exp;
	bool final_met = false;
	for (size_t i = 0; i < rules.boss_count; ++i) {
		const s_survivors_boss_bound& boss = rules.bosses[i];
		if (boss.at_ms > survived_ms)
			break;
		met_points.push_back(boss.kill_points);
		met_exp.push_back(boss.exp);
		final_met = final_met || boss.final;
	}
	if (run.boss_kills > met_points.size())
		return false;
	std::sort(met_points.begin(), met_points.end(), std::greater<uint32>());
	std::sort(met_exp.begin(), met_exp.end(), std::greater<uint32>());
	const uint64 boss_points = std::accumulate(met_points.begin(), met_points.begin() + run.boss_kills, uint64{ 0 });
	const uint64 boss_exp = std::accumulate(met_exp.begin(), met_exp.begin() + run.boss_kills, uint64{ 0 });
	const uint64 common_kills = run.kills - run.boss_kills;
	if (run.kill_points > common_kills * rules.most_common_kill_points + boss_points)
		return false;
	const uint64 most_exp = (common_kills * rules.most_common_kill_exp + boss_exp) * rules.most_exp_gain_pct / WHOLE_PCT;
	if (run.level > level_reachable(rules, most_exp))
		return false;

	if (run.outcome == SURVIVORS_OUTCOME_SURVIVED && survived_ms < rules.run_ms)
		return false;
	if (run.outcome == SURVIVORS_OUTCOME_VICTORY && (!final_met || run.boss_kills == 0))
		return false;
	return true;
}

// ── LES ABRIS : où une partie classée peut partir ───────────────────────────
// La garde d'entrée du client accepte une carte qui a une fiche de ville, ou
// qu'il tient pour un intérieur. Ces deux listes en sont la copie.
namespace {

// Les intérieurs : la table des intérieurs de moonclient
// (data/yaml_files/indoor_maps.yml), tirée de data/indoorrswtable.txt de
// l'installation. Clés en minuscules, le numéro d'instance déjà retiré.
constexpr const char* SURVIVORS_INDOOR_MAPS[] = {
	"1@4mag", "1@ecl", "1@gef_in", "1@sthc", "1@thts", "1@vrcas", "1@vrgen", "2@vrclo", "air_evt",
	"airplane", "airplane_01", "airport", "alberta_in", "alde_alche", "aldeba_in", "ama_in01",
	"ama_in02", "arena_room", "auction_01", "auction_02", "ayo_in01", "ayo_in02", "ba_in01",
	"bat_room", "bra_in01", "ch1_gef_in", "ch1_sf01", "ch1_sf02", "ch1_sf03", "ch1_vrgef1",
	"cmd_in01", "cmd_in02", "conch_in", "d_mond01", "dew_in01", "dic_in01", "ecl_in01", "ecl_in02",
	"ecl_in03", "ecl_in04", "ecl_tdun04", "ein_in01", "gef_tower", "geffen_in", "gon_in", "har_in01",
	"hu_in01", "icas_in", "icas_in2", "in_orcs01", "itemmall", "iz_in_a", "iz_in_b", "iz_in_c",
	"iz_in_d", "iz_ng01", "izlude_in", "jalbe_in", "jawaii_in", "job3_guil01", "job3_guil02",
	"job4_bio", "job4_mag", "kh_mansion", "kh_rossi", "kh_school", "kh_vila", "landguild01",
	"lasa_in01", "lhz_airport", "lhz_cu_evt", "lhz_cube", "lhz_in01", "lhz_in02", "lhz_in03",
	"lhz_que01", "lou_in01", "lou_in02", "ma_in01", "ma_zif01", "ma_zif02", "ma_zif03", "ma_zif04",
	"ma_zif05", "ma_zif06", "ma_zif07", "ma_zif08", "ma_zif09", "mal_in01", "mal_in02", "man_in01",
	"mbase_in", "memohall", "mid_campin", "moc_castle", "moc_para01", "moc_para0a", "moc_para0b",
	"moc_para0c", "monk_in", "morocc_in", "mosk_in", "nakhyang", "nameless_in", "new_1-2_evt",
	"nif_in", "paramk", "payon_in01", "payon_in02", "payon_in03", "prt_are_in", "prt_cas",
	"prt_cas_evt", "prt_cas_q", "prt_castle", "prt_ch_evt", "prt_church", "prt_elib", "prt_evt_in",
	"prt_in", "prt_in_evt", "prt_pri00", "pub_cat", "que_avan01", "que_ba", "que_bingo", "que_dan02",
	"que_god01", "que_god02", "que_house_s", "que_job01", "que_ng", "que_rachel", "que_sign01",
	"que_thor", "ra_in01", "ra_temin", "rebel_in", "s_atelier", "sec_in01", "spl_in01", "spl_in02",
	"sword_1-1", "sword_2-1", "sword_3-1", "thana_step", "um_in", "un_bk_q", "un_bunker", "un_myst",
	"ve_in", "ve_in02", "vr_cas", "vr_clo2", "vr_gen", "x_prt_in", "xmas_in", "y_airport",
	"ygg_fruit", "yuno_in01", "yuno_in03", "yuno_in04", "yuno_in05", "yuno_pre",
};

// Les cartes qui ont une fiche de ville chez moonclient
// (data/yaml_files/town_info.yml), y compris celles sans MF_TOWN.
constexpr const char* SURVIVORS_TOWN_MAPS[] = {
	"alb2trea", "alberta", "aldebaran", "amatsu", "ayothaya", "brasilis", "cmd_fild07", "comodo",
	"dewata", "dicastes01", "eclage", "einbech", "einbroch", "geffen", "gonryun", "harboro1", "hugel",
	"izlude", "izlude_a", "izlude_b", "izlude_c", "izlude_d", "lasagna", "lighthalzen", "louyang",
	"malangdo", "malaya", "moc_ruins", "mora", "morocc", "moscovia", "niflheim", "pay_arche", "payon",
	"prontera", "prt_fild05", "prt_monk", "rachel", "umbala", "veins", "xmas", "yuno",
};

// Une copie d'instance préfixe le nom de sa carte de trois chiffres
// (« 0011@infi » est une copie de « 1@infi ») : ils commencent quatre
// caractères avant le dernier « @ », et se retirent comme le client les retire.
constexpr size_t INSTANCE_AT_LEAST = 4;
constexpr size_t INSTANCE_BACK = 4;
constexpr size_t INSTANCE_CUT = 3;

bool listed_in(const std::string& key, const char* const* first, const char* const* last) {
	return std::find_if(first, last, [&key](const char* map) { return key == map; }) != last;
}

} // namespace

bool survivors_sheltered_place(const char* map_name) {
	std::string key(map_name);
	std::transform(key.begin(), key.end(), key.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	if (listed_in(key, std::begin(SURVIVORS_TOWN_MAPS), std::end(SURVIVORS_TOWN_MAPS)))
		return true;
	if (const size_t at = key.rfind('@'); at != std::string::npos && at >= INSTANCE_AT_LEAST)
		key.erase(at - INSTANCE_BACK, INSTANCE_CUT);
	return listed_in(key, std::begin(SURVIVORS_INDOOR_MAPS), std::end(SURVIVORS_INDOOR_MAPS));
}
