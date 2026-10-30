/* studio/table.cpp — the Table workspace and the Feast.
 *
 * Table: the worlds you place turkeys on, which one is played in what order, what
 * music it plays, and a map of what is on it, seen from above. Placing turkeys in 3D
 * is still the exercise's 3D table (`effect table3d`), until Void Squid's own viewport
 * can draw a world: Void Jelly reads no glTF yet, and Squid computes no geometry.
 *
 * Feast: who the player is and how they move. The feast rune is the truth; Play and
 * "Write for the runtime" compile it to the runtime's feast.json and worlds.json
 * (pillar 2's project/compile pair, written out by an effect).
 */
#include "studio/studio.hpp"

#include "library/library.hpp"
#include "studio/platform.hpp"

#include "IconsFontAwesome6.h"
#include "imgui.h"
#include "voidmaiz/project.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>

namespace squid::studio {

namespace fs = std::filesystem;

namespace {
double num(const json& v, double d) {
    if (v.is_number()) return v.get<double>();
    if (v.is_string()) { try { return std::stod(v.get<std::string>()); } catch (...) {} }
    return d;
}

/* The tables in feast order. */
std::vector<const json*> ordered(const json* tm) {
    std::vector<const json*> v;
    if (!tm) return v;
    for (const auto& r : (*tm)["runes"]) v.push_back(&r);
    std::stable_sort(v.begin(), v.end(), [](const json* a, const json* b) {
        return num((*a)["content"].value("order", json(999)), 999) < num((*b)["content"].value("order", json(999)), 999);
    });
    return v;
}

ImU32 kind_colour(const std::string& k) {
    if (k == "planet") return IM_COL32(110, 150, 230, 200);
    if (k == "enemy") return IM_COL32(240, 100, 90, 255);
    if (k == "coin" || k == "starbit" || k == "powerstar" || k == "oneup") return IM_COL32(250, 210, 80, 255);
    if (k == "npc") return IM_COL32(120, 220, 150, 255);
    if (k == "launch" || k == "sling") return IM_COL32(200, 140, 250, 255);
    if (k == "water") return IM_COL32(80, 190, 240, 160);
    return IM_COL32(170, 180, 200, 160);
}

bool position(const json& o, float& x, float& y, float& z) {
    if (o.contains("matrix") && o["matrix"].is_array() && o["matrix"].size() >= 3) {
        const auto& m = o["matrix"];
        x = static_cast<float>(num(m[0][3], 0)); y = static_cast<float>(num(m[1][3], 0)); z = static_cast<float>(num(m[2][3], 0));
        return true;
    }
    if (o.contains("pos") && o["pos"].is_array() && o["pos"].size() >= 3) {
        x = static_cast<float>(num(o["pos"][0], 0)); y = static_cast<float>(num(o["pos"][1], 0)); z = static_cast<float>(num(o["pos"][2], 0));
        return true;
    }
    return false;
}
}  // namespace

void Studio::draw_table() {
    const json* tm = mantle("tables");
    if (!tm) {
        ImGui::TextWrapped("%s  No tables yet. A table is a world you place turkeys on. Open a library to bring in the worlds it holds.", ICON_FA_MAP);
        return;
    }
    const auto tables = ordered(tm);
    if (sel_table_.empty() && !tables.empty()) sel_table_ = (*tables[0])["spirit"].value("name", "");

    ImGui::BeginChild("tables-list", ImVec2(300, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
    ImGui::TextDisabled("the tables, in the order they are played");
    for (size_t i = 0; i < tables.size(); ++i) {
        const json& r = *tables[i];
        const std::string name = r["spirit"].value("name", "");
        ImGui::PushID(name.c_str());
        const bool missing = std::find(r["tags"].begin(), r["tags"].end(), "missing") != r["tags"].end();
        char lbl[512];
        std::snprintf(lbl, sizeof lbl, "%zu.  %s%s", i + 1, r["content"].value("title", name).c_str(), missing ? "  (file missing)" : "");
        if (ImGui::Selectable(lbl, name == sel_table_)) { sel_table_ = name; inspect_mantle_ = "tables"; inspect_rune_ = name; }
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("SQUID_TURKEY")) {
                toast(std::string("placing a turkey on a table happens in the 3D table for now (") + static_cast<const char*>(p->Data) + ")");
            }
            ImGui::EndDragDropTarget();
        }
        ImGui::PopID();
    }
    ImGui::EndChild();
    ImGui::SameLine();

    ImGui::BeginChild("table-detail");
    const json* r = rune("tables", sel_table_);
    if (!r) { ImGui::TextDisabled("pick a table"); ImGui::EndChild(); return; }
    const json& c = (*r)["content"];
    const std::string file = c.value("file", "");
    const std::string lib = library_dir();
    if (big) ImGui::PushFont(big);
    ImGui::TextUnformatted(c.value("title", sel_table_).c_str());
    if (big) ImGui::PopFont();
    ImGui::TextDisabled("%s", file.c_str());

    // order
    {
        size_t at = 0;
        for (size_t i = 0; i < tables.size(); ++i) if ((*tables[i])["spirit"].value("name", "") == sel_table_) at = i;
        auto swap_with = [&](size_t j) {
            std::vector<std::string> cmds;
            for (size_t i = 0; i < tables.size(); ++i) {
                size_t k = i == at ? j : (i == j ? at : i);
                cmds.push_back("setjson " + maiz::arg((*tables[i])["spirit"].value("name", "")) + " order " + std::to_string(k + 1));
            }
            edit("tables", cmds);
        };
        ImGui::BeginDisabled(at == 0);
        if (ImGui::Button(ICON_FA_ARROW_UP " earlier")) swap_with(at - 1);
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::BeginDisabled(at + 1 >= tables.size());
        if (ImGui::Button(ICON_FA_ARROW_DOWN " later")) swap_with(at + 1);
        ImGui::EndDisabled();
        ImGui::SameLine(0, 24);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.52f, 0.38f, 1));
        if (ImGui::Button(ICON_FA_PLAY "  Play this table")) {
            const json* f = rune("feast", "feast");
            const json feast = f ? library::feast_file((*f)["content"]) : json();
            const maiz::Result res = run("effect play " + maiz::arg(lib) + " " + maiz::arg(file) + " " + maiz::arg(feast.is_null() ? "" : feast.dump()));
            json j;
            try { j = json::parse(res.data); } catch (...) {}
            if (j.is_object() && j.contains("error")) toast(j["error"].get<std::string>(), true); else toast("playing " + file);
        }
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_FOLDER_OPEN)) run("effect reveal " + maiz::arg((fs::path(lib) / file).string()));
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show the world file");
    }

    // music
    {
        const std::string music = c.value("music", "");
        std::string shown = music.empty() ? "(none: the runtime's default)" : music.substr(music.find_last_of("/:") + 1);
        ImGui::SetNextItemWidth(380);
        if (ImGui::BeginCombo(ICON_FA_MUSIC "  Music", shown.c_str())) {
            if (ImGui::Selectable("(none)", music.empty())) edit("tables", {"set " + maiz::arg(sel_table_) + " music \"\""});
            if (const json* tr = mantle("tracks"))
                for (const auto& t : (*tr)["runes"]) {
                    const std::string tn = t["spirit"].value("name", "");
                    const std::string title = t["content"].value("title", tn);
                    if (ImGui::Selectable(title.c_str(), tn == music)) edit("tables", {"set " + maiz::arg(sel_table_) + " music " + maiz::arg(tn)});
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s\n%s", tn.c_str(), t["content"].value("game", "").c_str());
                }
            ImGui::EndCombo();
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(plays in the runtime)");
    }

    // the map: read the world file once per selection
    if (map_file_ != file) {
        map_file_ = file;
        map_ = json();
        map_zoom_ = 1; map_pan_x_ = map_pan_y_ = 0;
        std::ifstream in(fs::path(lib) / file);
        if (in) { try { map_ = json::parse(in); } catch (...) { map_ = json(); } }
    }
    if (!map_.is_object() || !map_.contains("objects")) {
        ImGui::TextDisabled("the world file could not be read: %s", (fs::path(lib) / file).string().c_str());
        ImGui::EndChild();
        return;
    }
    const auto& objs = map_["objects"];
    const size_t grav = map_.contains("gravity") && map_["gravity"].is_array() ? map_["gravity"].size() : 0;
    ImGui::Text("%zu objects   %zu gravity fields   %s", objs.size(), grav, map_.value("units", "").c_str());

    // the views of one table: the same world, three ways to look at it
    {
        const char* views[] = {ICON_FA_MAP "  From above", ICON_FA_LAYER_GROUP "  Objects", ICON_FA_CUBE "  In 3D"};
        for (int v = 0; v < 3; ++v) {
            if (v) ImGui::SameLine(0, 4);
            const bool on = table_view_ == v;
            if (on) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
            if (ImGui::Button(views[v])) table_view_ = v;
            if (on) ImGui::PopStyleColor();
        }
    }
    if (table_view_ == 1) {   // every object in the world file, and which turkey it is
        std::map<std::string, std::string> by_model;
        if (const json* tm = mantle("turkeys"))
            for (const auto& t : (*tm)["runes"]) {
                const std::string mdl = t["content"].value("model", "");
                if (!mdl.empty()) by_model[mdl] = t["spirit"].value("name", "");
            }
        if (ImGui::BeginTable("objects", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV)) {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("name");
            ImGui::TableSetupColumn("kind", ImGuiTableColumnFlags_WidthFixed, 80);
            ImGui::TableSetupColumn("the game's class");
            ImGui::TableSetupColumn("position", ImGuiTableColumnFlags_WidthFixed, 190);
            ImGui::TableSetupColumn("turkey");
            ImGui::TableHeadersRow();
            ImGuiListClipper clip;
            clip.Begin(static_cast<int>(objs.size()));
            while (clip.Step())
                for (int i = clip.DisplayStart; i < clip.DisplayEnd; ++i) {
                    const json& o = objs[static_cast<size_t>(i)];
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::PushID(i);
                    if (ImGui::Selectable(o.value("name", "?").c_str(), map_pick_ == i, ImGuiSelectableFlags_SpanAllColumns)) {
                        map_pick_ = i;
                        const std::string mdl = o.contains("model") && o["model"].is_string() ? o["model"].get<std::string>() : "";
                        auto it = by_model.find(mdl);
                        if (it != by_model.end()) { sel_turkey_ = it->second; inspect_mantle_ = "turkeys"; inspect_rune_ = it->second; }
                    }
                    ImGui::PopID();
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(o.value("kind", "").c_str());
                    ImGui::TableNextColumn();
                    ImGui::TextDisabled("%s", o.contains("class") && o["class"].is_string() ? o["class"].get<std::string>().c_str() : "");
                    ImGui::TableNextColumn();
                    float x, y, z;
                    if (position(o, x, y, z)) ImGui::TextDisabled("%.0f  %.0f  %.0f", x, y, z);
                    ImGui::TableNextColumn();
                    const std::string mdl = o.contains("model") && o["model"].is_string() ? o["model"].get<std::string>() : "";
                    auto it = by_model.find(mdl);
                    if (it != by_model.end()) ImGui::TextUnformatted(it->second.c_str());
                    else ImGui::TextDisabled("%s", mdl.empty() ? "(no model)" : "not on the shelf");
                }
            ImGui::EndTable();
        }
        ImGui::EndChild();
        return;
    }
    if (table_view_ == 2) {
        draw_table_3d(file);
        ImGui::EndChild();
        return;
    }
    ImGui::TextDisabled("seen from above; drag to pan, wheel to zoom, hover for a name, click to pick");

    float minx = 1e30f, maxx = -1e30f, minz = 1e30f, maxz = -1e30f;
    std::vector<std::array<float, 3>> pts(objs.size());
    std::vector<bool> has(objs.size(), false);
    std::vector<float> xs, zs;
    for (size_t i = 0; i < objs.size(); ++i) {
        float x, y, z;
        if (!position(objs[i], x, y, z)) continue;
        pts[i] = {x, y, z};
        has[i] = true;
        xs.push_back(x);
        zs.push_back(z);
    }
    // fit the middle 90%: a sky dome or a far backdrop must not shrink the level to a speck
    if (!xs.empty()) {
        std::sort(xs.begin(), xs.end());
        std::sort(zs.begin(), zs.end());
        const size_t lo = xs.size() / 20, hi = xs.size() - 1 - xs.size() / 20;
        minx = xs[lo]; maxx = xs[hi]; minz = zs[lo]; maxz = zs[hi];
    }
    const ImVec2 p0 = ImGui::GetCursorScreenPos();
    ImVec2 sz = ImGui::GetContentRegionAvail();
    sz.y = std::max(sz.y, 200.0f);
    ImGui::InvisibleButton("map", sz);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(p0, ImVec2(p0.x + sz.x, p0.y + sz.y), IM_COL32(6, 9, 20, 230), 8);
    dl->AddRect(p0, ImVec2(p0.x + sz.x, p0.y + sz.y), IM_COL32(60, 74, 120, 160), 8);
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) { map_pan_x_ += ImGui::GetIO().MouseDelta.x; map_pan_y_ += ImGui::GetIO().MouseDelta.y; }
    if (ImGui::IsItemHovered() && ImGui::GetIO().MouseWheel != 0) map_zoom_ = std::clamp(map_zoom_ * (ImGui::GetIO().MouseWheel > 0 ? 1.15f : 0.87f), 0.1f, 40.0f);
    if (minx > maxx) { ImGui::EndChild(); return; }
    const float span = std::max(maxx - minx, maxz - minz) + 1.0f;
    const float k = std::min(sz.x, sz.y) * 0.9f / span * map_zoom_;
    const float cx = (minx + maxx) * 0.5f, cz = (minz + maxz) * 0.5f;
    auto to_screen = [&](float x, float z) {
        return ImVec2(p0.x + sz.x * 0.5f + (x - cx) * k + map_pan_x_, p0.y + sz.y * 0.5f + (z - cz) * k + map_pan_y_);
    };
    dl->PushClipRect(p0, ImVec2(p0.x + sz.x, p0.y + sz.y), true);
    // a grid every power of ten that gives ~60 px
    {
        float step = std::pow(10.0f, std::floor(std::log10(std::max(1e-3f, 60.0f / k))));
        if (step * k < 30) step *= 5;
        const ImU32 gc = IM_COL32(40, 52, 90, 90);
        const float gx0 = std::floor((cx - (sz.x * 0.5f + map_pan_x_) / k) / step) * step;
        for (float gx = gx0; to_screen(gx, 0).x < p0.x + sz.x; gx += step) dl->AddLine(ImVec2(to_screen(gx, 0).x, p0.y), ImVec2(to_screen(gx, 0).x, p0.y + sz.y), gc);
        const float gz0 = std::floor((cz - (sz.y * 0.5f + map_pan_y_) / k) / step) * step;
        for (float gz = gz0; to_screen(0, gz).y < p0.y + sz.y; gz += step) dl->AddLine(ImVec2(p0.x, to_screen(0, gz).y), ImVec2(p0.x + sz.x, to_screen(0, gz).y), gc);
    }
    const ImVec2 mouse = ImGui::GetIO().MousePos;
    int hover = -1;
    float best = 64;
    for (size_t i = 0; i < objs.size(); ++i) {
        if (!has[i]) continue;
        const std::string kind = objs[i].value("kind", "");
        const ImVec2 s = to_screen(pts[i][0], pts[i][2]);
        const float rad = kind == "planet" ? 5.5f : 3.0f;
        if (kind == "planet") dl->AddCircle(s, rad + 2, kind_colour(kind), 16, 1.5f);
        dl->AddCircleFilled(s, rad, kind_colour(kind), 12);
        const float d = (s.x - mouse.x) * (s.x - mouse.x) + (s.y - mouse.y) * (s.y - mouse.y);
        if (d < best) { best = d; hover = static_cast<int>(i); }
    }
    if (map_.contains("start")) {
        float x, y, z;
        if (position(map_["start"], x, y, z)) {
            const ImVec2 s = to_screen(x, z);
            dl->AddText(ImVec2(s.x - 6, s.y - 8), IM_COL32(120, 255, 170, 255), ICON_FA_STAR);
        }
    }
    dl->PopClipRect();
    if (hover >= 0 && ImGui::IsItemHovered() && ImGui::IsMouseReleased(0) && ImGui::GetMouseDragDelta(0).x == 0 && ImGui::GetMouseDragDelta(0).y == 0)
        map_pick_ = hover;
    if (map_pick_ >= 0 && static_cast<size_t>(map_pick_) < objs.size() && has[static_cast<size_t>(map_pick_)]) {
        const ImVec2 sp = to_screen(pts[static_cast<size_t>(map_pick_)][0], pts[static_cast<size_t>(map_pick_)][2]);
        dl->AddCircle(sp, 11, IM_COL32(86, 214, 222, 255), 24, 2.0f);
    }
    if (hover >= 0 && ImGui::IsItemHovered()) {
        const json& o = objs[static_cast<size_t>(hover)];
        ImGui::SetTooltip("%s\n%s   %s\n(%.0f, %.0f, %.0f)", o.value("name", "").c_str(), o.value("kind", "").c_str(),
                          o.value("class", "").c_str(), pts[static_cast<size_t>(hover)][0], pts[static_cast<size_t>(hover)][1], pts[static_cast<size_t>(hover)][2]);
    }
    ImGui::EndChild();
}

void Studio::draw_feast() {
    const json* f = rune("feast", "feast");
    if (!f) {
        ImGui::TextWrapped("%s  The feast is who the player is and how they move. It appears when a library is open.", ICON_FA_UTENSILS);
        return;
    }
    inspect_mantle_ = "feast";
    inspect_rune_ = "feast";
    const json& c = (*f)["content"];
    const std::string player = c.value("player", "");
    const json* pr = rune("turkeys", player);

    if (big) ImGui::PushFont(big);
    ImGui::TextUnformatted("The feast");
    if (big) ImGui::PopFont();
    ImGui::TextDisabled("who you play as, how they move, and the tables in order");
    ImGui::Separator();

    ImGui::BeginGroup();
    if (pr) thumbnail(*pr, 180); else { json none = {{"content", json::object()}}; thumbnail(none, 180); }
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("SQUID_TURKEY")) {
            edit("feast", {"set feast player " + maiz::arg(static_cast<const char*>(p->Data))});
        }
        ImGui::EndDragDropTarget();
    }
    ImGui::TextDisabled("drop a turkey here");
    ImGui::EndGroup();
    ImGui::SameLine(0, 18);
    ImGui::BeginGroup();
    ImGui::Text("Player:  %s", pr ? turkey_label(*pr).c_str() : (player.empty() ? "(nobody)" : player.c_str()));
    if (pr) ImGui::TextDisabled("%s, %s", (*pr)["content"].value("game", "").c_str(), (*pr)["content"].value("kind", "").c_str());
    if (!sel_turkey_.empty() && sel_turkey_ != player) {
        if (ImGui::Button(("Play as the selected turkey (" + sel_turkey_ + ")").c_str())) edit("feast", {"set feast player " + maiz::arg(sel_turkey_)});
    }
    if (player != "smg.Kuribo" && ImGui::SmallButton("back to the Goomba")) edit("feast", {"set feast player smg.Kuribo"});
    ImGui::TextDisabled("the characters below are animated; any other turkey plays as its model, unanimated");
    ImGui::Spacing();
    const maiz::Scene& sc = scene_of("feast");
    for (const auto& n : sc.nodes) {
        if (n.name != "feast") continue;
        std::vector<std::string> cmds;
        maiz::WidgetContext ctx{sc, cmds, "", 220.0f};
        for (const auto& fld : n.fields) {
            if (fld.key == "player") continue;
            maiz::widget_field(ctx, widgets_, n, fld);
        }
        if (!cmds.empty()) edit("feast", cmds);
    }
    ImGui::EndGroup();

    // every playable character the library has, by game: one click makes it the player
    if (const json* tm = mantle("turkeys")) {
        std::map<std::string, std::vector<const json*>> by_game;
        for (const auto& t : (*tm)["runes"])
            if (t["content"].value("kind", "") == "player" || t["content"].contains("rig")) by_game[t["content"].value("game", "")].push_back(&t);
        if (!by_game.empty()) {
            ImGui::SeparatorText("Play as");
            for (const auto& [game, list] : by_game) {
                ImGui::TextDisabled("%s", game.c_str());
                const float cell = 92;
                const int per_row = std::max(1, static_cast<int>(ImGui::GetContentRegionAvail().x / (cell + 8)));
                int k = 0;
                for (const json* t : list) {
                    const std::string id = (*t)["spirit"].value("name", "");
                    if (k++ % per_row) ImGui::SameLine(0, 8);
                    ImGui::PushID(id.c_str());
                    ImGui::BeginGroup();
                    const ImVec2 p = ImGui::GetCursorScreenPos();
                    const bool on = id == player;
                    if (ImGui::InvisibleButton("pick", ImVec2(cell, cell + 18))) edit("feast", {"set feast player " + maiz::arg(id)});
                    const bool hov = ImGui::IsItemHovered();
                    thumb_at(*t, p.x + 4, p.y + 2, cell - 8);
                    ImDrawList* dl = ImGui::GetWindowDrawList();
                    if (on || hov)
                        dl->AddRect(p, ImVec2(p.x + cell, p.y + cell + 18), on ? IM_COL32(86, 214, 222, 255) : IM_COL32(158, 122, 240, 180), 6, 0, on ? 2.5f : 1.5f);
                    std::string label = turkey_label(*t);
                    const auto paren = label.find(" (");
                    if (paren != std::string::npos) label = label.substr(0, paren);
                    dl->AddText(ImVec2(p.x + 4, p.y + cell - 2), ImGui::GetColorU32(on ? ImGuiCol_Text : ImGuiCol_TextDisabled), label.c_str());
                    ImGui::EndGroup();
                    if (hov) ImGui::SetTooltip("%s\n%s", turkey_label(*t).c_str(), (*t)["content"].value("game", "").c_str());
                    ImGui::PopID();
                }
            }
            ImGui::TextDisabled("in the game, Q (or the left stick press) switches to the next character");
        }
    }

    ImGui::SeparatorText("The tables, in order");
    const auto tables = ordered(mantle("tables"));
    for (size_t i = 0; i < tables.size(); ++i)
        ImGui::BulletText("%zu.  %s", i + 1, (*tables[i])["content"].value("title", "").c_str());
    ImGui::TextDisabled("reorder them in the Table workspace");

    ImGui::Spacing();
    const std::string lib = library_dir();
    if (ImGui::Button(ICON_FA_FLOPPY_DISK "  Write for the runtime")) {
        const json feast = library::feast_file(c);
        const json* tm = mantle("tables");
        const json worlds = tm ? library::worlds_file(*tm) : json();
        const maiz::Result r = run("effect library export " + maiz::arg(lib) + " " + maiz::arg(feast.dump()) + " " + maiz::arg(worlds.dump()));
        toast(r.ok ? "wrote feast.json and worlds.json into the library" : r.text(), !r.ok);
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("The runtime reads feast.json and worlds.json from the library folder.\nPlay writes the feast for you.");
}

}  // namespace squid::studio
