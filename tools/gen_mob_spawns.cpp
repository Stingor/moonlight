// gen_mob_spawns.cpp
// Parse scripts_moon.conf (recursively) and generate mob_spawn.sql
//
// Build:  cl /std:c++17 /EHsc /O2 /Fe:gen_mob_spawns.exe gen_mob_spawns.cpp
// Usage:  gen_mob_spawns <repo_root> <output.sql>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

struct SpawnEntry {
    std::string map;
    int x = 0, y = 0, xs = 0, ys = 0;
    int mob_id = 0;
    int amount  = 1;
    int delay1  = 0, delay2 = 0;
    bool is_boss = false;
};

static std::vector<SpawnEntry>              g_spawns;
static std::set<std::string>                g_visited;
static std::unordered_map<std::string, int> g_aegis_to_id; // AegisName -> numeric mob_id

static std::string trim(const std::string& s) {
    auto a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return {};
    auto b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

// ---------------------------------------------------------------------------
// mob_db YAML loader — minimal line-by-line parser (no full YAML library)
// Handles:
//   - Id: 1001
//     AegisName: SCORPION
// and Footer imports:
//   Footer:
//     - Path: db/import/mobs/mvps.yml
// ---------------------------------------------------------------------------

static void load_mob_yml(const fs::path& root, const fs::path& path);

static void load_mob_yml(const fs::path& root, const fs::path& path) {
    std::string key = fs::weakly_canonical(path).string();
    if (!g_visited.insert(key).second) return;

    std::ifstream f(path);
    if (!f) {
        std::cerr << "warning: cannot open mob yml " << path << '\n';
        return;
    }

    int  cur_id     = 0;
    bool in_footer  = false;

    std::string line;
    while (std::getline(f, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::string t = trim(line);
        if (t.empty() || t[0] == '#') continue;

        // Detect Footer section
        if (t == "Footer:") { in_footer = true; continue; }

        if (in_footer) {
            // "  - Path: db/import/mobs/mvps.yml"
            auto pos = t.find("Path:");
            if (pos != std::string::npos) {
                std::string rel = trim(t.substr(pos + 5));
                load_mob_yml(root, root / rel);
            }
            continue;
        }

        // "- Id: 1001"  or  "  - Id: 1001"
        {
            auto pos = t.find("- Id:");
            if (pos != std::string::npos) {
                cur_id = std::stoi(trim(t.substr(pos + 5)));
                continue;
            }
        }

        // "  AegisName: SCORPION"
        {
            auto pos = t.find("AegisName:");
            if (pos != std::string::npos && cur_id > 0) {
                std::string aegis = trim(t.substr(pos + 10));
                g_aegis_to_id[aegis] = cur_id;
                continue;
            }
        }
    }
}

static void load_mob_db(const fs::path& root) {
    fs::path main_yml = root / "db" / "import" / "mob_db.yml";
    if (!fs::exists(main_yml)) {
        std::cerr << "warning: " << main_yml << " not found, AegisNames won't be resolved\n";
        return;
    }
    load_mob_yml(root, main_yml);
    std::cout << "Loaded " << g_aegis_to_id.size() << " AegisName->ID mappings\n";
}

// ---------------------------------------------------------------------------
// NPC file parser
// ---------------------------------------------------------------------------

static void parse_npc_file(const fs::path& path) {
    std::string key = fs::weakly_canonical(path).string();
    if (!g_visited.insert(key).second) return;

    std::ifstream f(path, std::ios::binary);
    if (!f) {
        std::cerr << "warning: cannot open " << path << '\n';
        return;
    }

    // Header: map[,x,y[,xs,ys]]  — x,y optional (0,0 = random spawn)
    // Keyword: monster | boss_monster
    // Params:  mob_id_or_aegis,qty[,delay1[,delay2[,...]]]
    static const std::regex re(
        R"(^(\S+?)(?:,(\d+),(\d+)(?:,(\d+),(\d+))?)?\t+(boss_monster|monster)\t+[^\t]+\t+([A-Za-z_][A-Za-z0-9_]*|\d+),(\d+)(?:,(\d+))?(?:,(\d+))?)",
        std::regex::optimize
    );

    std::string line;
    while (std::getline(f, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::string t = trim(line);
        if (t.empty() || t.rfind("//", 0) == 0) continue;

        std::smatch m;
        if (!std::regex_search(line, m, re)) continue;

        SpawnEntry e;
        e.map     = m[1].str();
        e.x       = m[2].matched ? std::stoi(m[2].str()) : 0;
        e.y       = m[3].matched ? std::stoi(m[3].str()) : 0;
        e.xs      = m[4].matched ? std::stoi(m[4].str()) : 0;
        e.ys      = m[5].matched ? std::stoi(m[5].str()) : 0;
        e.is_boss = (m[6].str() == "boss_monster");
        e.amount  = std::stoi(m[8].str());
        e.delay1  = m[9].matched  ? std::stoi(m[9].str())  : 0;
        e.delay2  = m[10].matched ? std::stoi(m[10].str()) : 0;

        const std::string& mob_token = m[7].str();
        if (!mob_token.empty() && std::isdigit((unsigned char)mob_token[0])) {
            e.mob_id = std::stoi(mob_token);
        } else {
            auto it = g_aegis_to_id.find(mob_token);
            if (it != g_aegis_to_id.end()) {
                e.mob_id = it->second;
            } else {
                std::cerr << "warning: unresolved AegisName '" << mob_token
                          << "' in " << path << '\n';
                continue; // skip unresolvable entries
            }
        }

        g_spawns.push_back(e);
    }
}

// ---------------------------------------------------------------------------
// conf parser
// ---------------------------------------------------------------------------

static void parse_conf(const fs::path& root, const fs::path& conf_path) {
    std::ifstream f(conf_path);
    if (!f) {
        std::cerr << "error: cannot open conf " << conf_path << '\n';
        return;
    }

    std::string line;
    while (std::getline(f, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::string t = trim(line);
        if (t.empty() || t.rfind("//", 0) == 0) continue;

        if (t.rfind("npc:", 0) == 0) {
            parse_npc_file(root / trim(t.substr(4)));
        } else if (t.rfind("import:", 0) == 0) {
            parse_conf(root, root / trim(t.substr(7)));
        }
    }
}

// ---------------------------------------------------------------------------
// SQL output
// ---------------------------------------------------------------------------

static void write_sql(const fs::path& out_path) {
    std::ofstream out(out_path);
    if (!out) {
        std::cerr << "error: cannot write " << out_path << '\n';
        std::exit(1);
    }

    out <<
        "-- Generated by tools/gen_mob_spawns\n"
        "-- " << g_spawns.size() << " spawn entries\n\n"
        "DROP TABLE IF EXISTS `mob_spawn`;\n"
        "CREATE TABLE `mob_spawn` (\n"
        "  `id`      INT UNSIGNED      NOT NULL AUTO_INCREMENT,\n"
        "  `mob_id`  SMALLINT UNSIGNED NOT NULL,\n"
        "  `map`     VARCHAR(16)       NOT NULL,\n"
        "  `x`       SMALLINT UNSIGNED NOT NULL DEFAULT 0,\n"
        "  `y`       SMALLINT UNSIGNED NOT NULL DEFAULT 0,\n"
        "  `xs`      SMALLINT UNSIGNED NOT NULL DEFAULT 0,\n"
        "  `ys`      SMALLINT UNSIGNED NOT NULL DEFAULT 0,\n"
        "  `amount`  SMALLINT UNSIGNED NOT NULL DEFAULT 1,\n"
        "  `delay1`  INT UNSIGNED      NOT NULL DEFAULT 0,\n"
        "  `delay2`  INT UNSIGNED      NOT NULL DEFAULT 0,\n"
        "  `is_boss` TINYINT(1)        NOT NULL DEFAULT 0,\n"
        "  PRIMARY KEY (`id`),\n"
        "  KEY `idx_mob_id` (`mob_id`),\n"
        "  KEY `idx_map`    (`map`)\n"
        ") ENGINE=MyISAM DEFAULT CHARSET=utf8;\n\n";

    if (g_spawns.empty()) return;

    out << "INSERT INTO `mob_spawn`"
           " (`mob_id`,`map`,`x`,`y`,`xs`,`ys`,`amount`,`delay1`,`delay2`,`is_boss`)\nVALUES\n";

    for (size_t i = 0; i < g_spawns.size(); ++i) {
        const auto& e = g_spawns[i];
        out << "  (" << e.mob_id
            << ",'" << e.map << "'"
            << ',' << e.x << ',' << e.y
            << ',' << e.xs << ',' << e.ys
            << ',' << e.amount
            << ',' << e.delay1 << ',' << e.delay2
            << ',' << (e.is_boss ? 1 : 0)
            << ')' << (i + 1 < g_spawns.size() ? ",\n" : ";\n");
    }
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "usage: gen_mob_spawns <repo_root> <output.sql>\n";
        return 1;
    }

    fs::path root   = argv[1];
    fs::path output = argv[2];
    fs::path conf   = root / "moon" / "scripts_moon.conf";

    if (!fs::exists(conf)) {
        std::cerr << "error: " << conf << " not found\n";
        return 1;
    }

    load_mob_db(root);
    parse_conf(root, conf);
    write_sql(output);

    std::cout << "Generated " << g_spawns.size()
              << " spawn entries -> " << output << '\n';
    return 0;
}
