/* library_test.cpp — the library's pure halves, without a file, a network or a window.
 *
 * What it pins: titles fold the same way on both sides; the community list's Markdown
 * parses (headings, AI marks, work-in-progress lines); a ROM header title finds its
 * decompilation and a different platform weakens the match; a re-import keeps a
 * person's tags, sounds, chosen kind and removals; the feast and the tables compile to
 * the runtime's files in order.
 */
#include "library/catalogue.hpp"
#include "library/library.hpp"
#include "library/loadgame.hpp"

#include <cstdio>
#include <string>

using nlohmann::json;
using namespace squid::library;

static int failures = 0;
#define CHECK(cond)                                                              \
    do {                                                                         \
        if (!(cond)) { std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); ++failures; } \
    } while (0)

static void titles() {
    CHECK(fold_title("Donkey Kong 64 (Europe) (En,Fr,De,Es)") == "donkeykong64");
    CHECK(fold_title("DONKEY KONG 64") == "donkeykong64");
    CHECK(fold_title("Banjo & Kazooie") == "banjoandkazooie");
    CHECK(platform_of_heading("Nintendo 64") == "n64");
    CHECK(platform_of_heading("Nintendo DS") == "ds");
    CHECK(platform_of_heading("Nintendo 3DS") == "3ds");
    CHECK(platform_of_heading("Nintendo Wii U") == "wiiu");
    CHECK(platform_of_heading("Nintendo Gamecube") == "gc");
    CHECK(platform_of_heading("GameBoy Advance") == "gba");
    CHECK(platform_of_heading("Gameboy Colour") == "gbc");
    CHECK(platform_of_heading("NES / Famicom") == "nes");
    CHECK(platform_of_heading("SNES / Super Famicom") == "snes");
}

static void community_list() {
    const std::string md =
        "# awesome\nsome prose\n- [Not a game](https://example.com)\n"
        "## Nintendo 64\n"
        "- [Donkey Kong 64](https://gitlab.com/dk64_decomp/dk64)\n"
        "- [Super Mario 64](https://github.com/n64decomp/sm64)\n"
        "- //[Snowboard Kids](https://github.com/sonicdcer/sk)\n"
        "## NES / Famicom\n"
        "- [\xE2\x9A\xA0\xEF\xB8\x8F Battle City (Browser reimplementation)](https://github.com/vgrichina/battlecity)\n"
        "- [Donkey Kong](https://github.com/RussianManSMWC/Donkey-Kong-NES-Disassembly)\n";
    const auto g = parse_community_list(md);
    CHECK(g.size() == 5);   // the line before any platform heading is not a game
    if (g.size() == 5) {
        CHECK(g[0].title == "Donkey Kong 64" && g[0].platform == "n64");
        CHECK(g[2].work_in_progress);
        CHECK(g[3].ai_flagged && g[3].kind == "reimplementation" && g[3].title.rfind("Battle City", 0) == 0);
        CHECK(g[4].kind == "disassembly");
    }
    // a ROM header title finds DK64 on N64; the NES Donkey Kong is weaker
    const auto m = match({"DONKEY KONG 64", "Donkey Kong 64 (Europe) (En,Fr,De,Es)"}, "n64", g);
    CHECK(!m.empty() && m[0].game.title == "Donkey Kong 64" && m[0].score >= 90);
    // the same title on the wrong platform is penalised below the right one
    const auto wrong = match({"Super Mario 64"}, "ds", g);
    CHECK(wrong.empty() || wrong[0].score < 60);
}

static void the_plan() {
    std::vector<KnownGame> seed = parse_seed(json::parse(R"({"games":[{"title":"Donkey Kong 64","aliases":["DONKEY KONG 64"],
        "platform":"n64","url":"u","kind":"toolkit","route":"the toolbox"}]})"));
    const json recon = {{"kind", "rom"}, {"platform", "n64"}, {"title", "DONKEY KONG 64"}};
    const json inv = {{"setups", json::array({{{"id", "minimum"}, {"ready", true}}, {{"id", "agent"}, {"ready", false}}})}};
    const json p = plan("C:/games/Donkey Kong 64 (Europe).z64", recon, seed, inv);
    CHECK(p["matches"].size() == 1);
    CHECK(p["summary"].get<std::string>().find("route") != std::string::npos);
    bool extract_ready = false;
    for (const auto& s : p["steps"]) if (s["step"] == "Extract its assets" && s["state"] == "ready") extract_ready = true;
    CHECK(extract_ready);
    const json unknown = plan("x.bin", json{{"kind", "unknown"}}, seed, inv);
    CHECK(unknown["matches"].empty());
}

static void the_splice() {
    // a project where the person tagged, removed a tag, chose a kind and a sound
    json old_turkey = make_rune("turkeys", "smg.Kuribo", "turkey");
    old_turkey["tags"] = json::array({"enemy", "smg", "cute"});
    old_turkey["content"] = {{"name", "Kuribo"}, {"kind", "npc"}, {"kind_source", "person"}, {"sound_defeat", "C:/sounds/boop.wav"},
                             {"tag_sources", {{"enemy", "game:kind"}, {"smg", "game:title"}, {"cute", "person"}}},
                             {"tags_removed", json::array({"shopkeeper"})}};
    json old_mantle = make_mantle("turkeys");
    old_mantle["runes"].push_back(old_turkey);
    json mine = make_rune("turkeys", "my.thing", "turkey");
    mine["content"] = {{"name", "Mine"}, {"made_by", "person"}};
    old_mantle["runes"].push_back(mine);
    json notes = make_mantle("notes");
    json state = {{"version", 1}, {"mantles", json::array({old_mantle, notes})}, {"config", json::object()}, {"active", {{"mantle", nullptr}}}};

    // the library, rebuilt: different tags from the games and the wiki, no person tags
    json fresh_turkey = make_rune("turkeys", "smg.Kuribo", "turkey");
    fresh_turkey["tags"] = json::array({"enemy", "smg", "goomba", "shopkeeper"});
    fresh_turkey["content"] = {{"name", "Kuribo"}, {"kind", "enemy"},
                               {"tag_sources", {{"enemy", "game:kind"}, {"smg", "game:title"}, {"goomba", "wiki:Goomba"}, {"shopkeeper", "wiki:x"}}}};
    json fresh = make_mantle("turkeys");
    fresh["runes"].push_back(fresh_turkey);
    const json imported = {{"library", "C:/lib"}, {"mantles", json::array({fresh})}, {"feast", {{"player", "smg.Kuribo"}}}};

    const std::string out = splice(state.dump(), imported);
    CHECK(!out.empty());
    const json s = json::parse(out);
    const json* t = nullptr;
    const json* m = nullptr;
    bool has_notes = false, has_feast = false;
    for (const auto& mm : s["mantles"]) {
        if (mm["name"] == "turkeys") m = &mm;
        if (mm["name"] == "notes") has_notes = true;
        if (mm["name"] == "feast") has_feast = true;
    }
    CHECK(has_notes);   // a mantle that is not the library's is untouched
    CHECK(has_feast);   // the feast appears when the project had none
    CHECK(m != nullptr);
    if (m) {
        CHECK((*m)["id"] == old_mantle["id"]);
        for (const auto& r : (*m)["runes"]) if (r["spirit"]["name"] == "smg.Kuribo") t = &r;
        bool mine_kept = false;
        for (const auto& r : (*m)["runes"]) if (r["spirit"]["name"] == "my.thing") mine_kept = true;
        CHECK(mine_kept);
    }
    CHECK(t != nullptr);
    if (t) {
        const json& c = (*t)["content"];
        auto has = [&](const char* tag) { for (const auto& x : (*t)["tags"]) if (x == tag) return true; return false; };
        CHECK(has("goomba"));       // the wiki's new tag arrives
        CHECK(has("cute"));         // the person's tag survives
        CHECK(!has("shopkeeper"));  // the person's removal survives
        CHECK(c["kind"] == "npc");  // the person's kind survives
        CHECK(c["sound_defeat"] == "C:/sounds/boop.wav");
        CHECK(c["tag_sources"]["cute"] == "person");
        CHECK((*t)["spirit"]["id"] == old_turkey["spirit"]["id"]);
    }
    CHECK(s["config"]["squid.library"] == "C:/lib");
    CHECK(s["active"]["mantle"] == "turkeys");
    CHECK(splice("not json", imported).empty());
}

static void the_runtime_files() {
    const json f = feast_file({{"player", "dk64.orange"}, {"walk", "7.5"}, {"jump", 12}});
    CHECK(f["player"] == "dk64.orange");
    CHECK(f["walk"] == 7.5);
    CHECK(f["jump"] == 12.0);
    CHECK(f["gravity"] == 22.0);   // defaults fill the rest
    json tables = make_mantle("tables");
    for (auto [file, order] : std::vector<std::pair<std::string, int>>{{"b.json", 2}, {"a.json", 1}, {"c.json", 3}}) {
        json r = make_rune("tables", file, "table");
        r["content"] = {{"title", file}, {"file", file}, {"order", order}};
        tables["runes"].push_back(r);
    }
    const json w = worlds_file(tables);
    CHECK(w["worlds"].size() == 3 && w["worlds"][0]["file"] == "a.json" && w["worlds"][2]["file"] == "c.json");
}

static void identities() {
    // the same mantle and name give the same id; a different one does not
    CHECK(make_rune("turkeys", "a", "turkey")["spirit"]["id"] == make_rune("turkeys", "a", "turkey")["spirit"]["id"]);
    CHECK(make_rune("turkeys", "a", "turkey")["spirit"]["id"] != make_rune("tables", "a", "table")["spirit"]["id"]);
    CHECK(glyphs().size() == 6);
    for (const auto& g : glyphs()) CHECK(json::parse(g).contains("glyph"));
}

int main() {
    titles();
    community_list();
    the_plan();
    the_splice();
    the_runtime_files();
    identities();
    if (failures) { std::printf("%d failure(s)\n", failures); return 1; }
    std::printf("library: all checks passed\n");
    return 0;
}
