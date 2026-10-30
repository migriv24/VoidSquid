/* studio/turkey.cpp — the Turkey workspace and the Shelf.
 *
 * Turkey is careful inspection (the author, 2026-10-12): "making sure they were set up
 * correctly, have accurate tags". So every tag shows where it came from, a person's
 * tag is marked as theirs and survives a re-import, a removed tag stays removed, and
 * the checks say what is still wrong. Every change is a command on the turkeys mantle.
 */
#include "studio/studio.hpp"

#include "studio/platform.hpp"

#include "IconsFontAwesome6.h"
#include "imgui.h"
#include "voidmaiz/project.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <map>

namespace squid::studio {

namespace fs = std::filesystem;

namespace {
ImVec4 source_colour(const std::string& src) {
    if (src.rfind("person", 0) == 0) return ImVec4(0.92f, 0.55f, 0.31f, 1);
    if (src.rfind("game", 0) == 0) return ImVec4(0.42f, 0.56f, 0.84f, 1);
    if (src.rfind("wiki", 0) == 0) return ImVec4(0.47f, 0.75f, 0.47f, 1);
    if (src.rfind("vision", 0) == 0) return ImVec4(0.67f, 0.51f, 0.90f, 1);
    if (src.rfind("name", 0) == 0) return ImVec4(0.86f, 0.75f, 0.35f, 1);
    return ImVec4(0.5f, 0.55f, 0.65f, 1);
}
std::string lower(std::string s) {
    for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}
std::string clean_tag(const std::string& in) {
    std::string t;
    for (char c : lower(in)) t += (c == ' ') ? '-' : c;
    while (!t.empty() && t.back() == '-') t.pop_back();
    return t;
}
const char* const kKinds[] = {"player", "planet", "deco", "enemy", "npc", "coin", "starbit", "oneup", "powerstar", "launch", "sling", "itembox", "water"};
}  // namespace

// ── shared helpers ──────────────────────────────────────────────────────────

bool Studio::thumb_at(const json& turkey, float x, float y, float size) {
    const std::string th = turkey["content"].value("thumb", "");
    const std::string lib = library_dir();
    Texture t;
    if (!th.empty() && !lib.empty()) t = texture((fs::path(lib) / th).string());
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImVec2 p(x, y);
    if (t.ok()) {
        const float k = std::min(size / t.w, size / t.h);
        const float w = t.w * k, h = t.h * k;
        dl->AddRectFilled(p, ImVec2(p.x + size, p.y + size), IM_COL32(8, 10, 22, 200), size * 0.06f);
        const ImVec2 a(p.x + (size - w) * 0.5f, p.y + (size - h) * 0.5f);
        dl->AddImage(static_cast<ImTextureID>(t.id), a, ImVec2(a.x + w, a.y + h));
        return true;
    }
    dl->AddRectFilled(p, ImVec2(p.x + size, p.y + size), IM_COL32(26, 32, 58, 255), size * 0.08f);
    const char* icon = ICON_FA_CUBE;
    const ImVec2 isz = ImGui::CalcTextSize(icon);
    dl->AddText(ImVec2(p.x + (size - isz.x) * 0.5f, p.y + (size - isz.y) * 0.5f), IM_COL32(110, 124, 170, 255), icon);
    return false;
}

bool Studio::thumbnail(const json& turkey, float size) {
    const ImVec2 p = ImGui::GetCursorScreenPos();
    const bool ok = thumb_at(turkey, p.x, p.y, size);
    ImGui::Dummy(ImVec2(size, size));
    return ok;
}

void Studio::add_tag(const std::string& m, const std::string& r, const std::string& raw) {
    const std::string tag = clean_tag(raw);
    const json* ru = rune(m, r);
    if (!ru || tag.empty()) return;
    json sources = (*ru)["content"].value("tag_sources", json::object());
    sources[tag] = "person";
    std::vector<std::string> cmds = {"tag " + maiz::arg(r) + " +" + maiz::arg(tag),
                                     "setjson " + maiz::arg(r) + " tag_sources " + maiz::arg(sources.dump())};
    json removed = (*ru)["content"].value("tags_removed", json::array());
    json kept = json::array();
    for (const auto& x : removed) if (x != tag) kept.push_back(x);
    if (kept.size() != removed.size()) cmds.push_back("setjson " + maiz::arg(r) + " tags_removed " + maiz::arg(kept.dump()));
    edit(m, cmds);
}

void Studio::remove_tag(const std::string& m, const std::string& r, const std::string& tag) {
    const json* ru = rune(m, r);
    if (!ru) return;
    json sources = (*ru)["content"].value("tag_sources", json::object());
    const bool persons = sources.contains(tag) && sources[tag].is_string() && sources[tag].get<std::string>().rfind("person", 0) == 0;
    sources.erase(tag);
    std::vector<std::string> cmds = {"tag " + maiz::arg(r) + " -" + maiz::arg(tag),
                                     "setjson " + maiz::arg(r) + " tag_sources " + maiz::arg(sources.dump())};
    if (!persons) {   // a tag the games or the wiki gave stays removed across a re-import
        json removed = (*ru)["content"].value("tags_removed", json::array());
        removed.push_back(tag);
        cmds.push_back("setjson " + maiz::arg(r) + " tags_removed " + maiz::arg(removed.dump()));
    }
    edit(m, cmds);
}

void Studio::tag_chips(const std::string& m, const json& r, bool editable) {
    const json sources = r["content"].value("tag_sources", json::object());
    const std::string name = r["spirit"].value("name", "");
    std::string to_remove;
    const float right = ImGui::GetContentRegionAvail().x;
    float used = 0;
    for (const auto& t : r["tags"]) {
        if (!t.is_string()) continue;
        const std::string tag = t.get<std::string>();
        const std::string src = sources.contains(tag) && sources[tag].is_string() ? sources[tag].get<std::string>() : "";
        const ImVec4 col = source_colour(src);
        const float w = ImGui::CalcTextSize(tag.c_str()).x + 24;
        if (used > 0 && used + w < right) ImGui::SameLine(0, 5); else used = 0;
        used += w + 5;
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(col.x, col.y, col.z, 0.30f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(col.x, col.y, col.z, 0.55f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(col.x, col.y, col.z, 0.8f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
        ImGui::PushID(tag.c_str());
        if (ImGui::SmallButton(tag.c_str()) && editable) to_remove = tag;
        ImGui::PopID();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(3);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s\nfrom: %s%s", tag.c_str(), src.empty() ? "(not recorded)" : src.c_str(), editable ? "\nclick to remove" : "");
    }
    if (!to_remove.empty()) remove_tag(m, name, to_remove);
}

// ── the Shelf ───────────────────────────────────────────────────────────────

void Studio::draw_shelf() {
    const json* tm = mantle("turkeys");
    if (!tm || (*tm)["runes"].empty()) {
        ImGui::TextWrapped("The shelf is empty. Open a library (File > Open library...) to fill it with what you recovered from your games.");
        if (ImGui::Button(ICON_FA_DATABASE "  Open library...")) {
            std::string p = pick_folder("Choose a library folder (the one holding assets.db)");
            if (!p.empty()) open_library(p);
        }
        return;
    }
    ImGui::SetNextItemWidth(-1);
    ImGui::InputTextWithHint("##shelf-filter", ICON_FA_MAGNIFYING_GLASS "  search, or a tag expression (enemy AND smg)", shelf_filter_, sizeof shelf_filter_);
    // kinds present, as chips
    std::map<std::string, int> kinds, games;
    for (const auto& r : (*tm)["runes"]) {
        kinds[r["content"].value("kind", "")]++;
        games[r["content"].value("game", "")]++;
    }
    auto chip = [&](const std::string& label, std::string& slot, const std::string& value) {
        const bool on = slot == value;
        if (on) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
        if (ImGui::SmallButton(label.c_str())) slot = on ? "" : value;
        if (on) ImGui::PopStyleColor();
    };
    chip("all", shelf_kind_, "");
    for (const auto& [k, n] : kinds) {
        if (k.empty()) continue;
        ImGui::SameLine(0, 4);
        if (ImGui::GetCursorPosX() + ImGui::CalcTextSize(k.c_str()).x + 16 > ImGui::GetContentRegionMax().x) ImGui::NewLine();
        chip(k, shelf_kind_, k);
    }
    ImGui::SetNextItemWidth(-1);
    if (ImGui::BeginCombo("##game", shelf_game_.empty() ? "every game" : shelf_game_.c_str())) {
        if (ImGui::Selectable("every game", shelf_game_.empty())) shelf_game_.clear();
        for (const auto& [g, n] : games) {
            if (g.empty()) continue;
            char lbl[256];
            std::snprintf(lbl, sizeof lbl, "%s  (%d)", g.c_str(), n);
            if (ImGui::Selectable(lbl, shelf_game_ == g)) shelf_game_ = g;
        }
        ImGui::EndCombo();
    }

    // the filter: Void Maiz's one tag grammar (node_matches), and plain text over names
    const maiz::Scene& sc = scene_of("turkeys");
    std::map<std::string, const maiz::SceneNode*> nodes;
    for (const auto& n : sc.nodes) nodes[n.name] = &n;
    const std::string f = shelf_filter_;
    const std::string fl = lower(f);
    const bool expr = f.find(" AND ") != std::string::npos || f.find(" OR ") != std::string::npos || f.rfind("NOT ", 0) == 0;
    std::vector<const json*> shown;
    for (const auto& r : (*tm)["runes"]) {
        const json& c = r["content"];
        if (!shelf_kind_.empty() && c.value("kind", "") != shelf_kind_) continue;
        if (!shelf_game_.empty() && c.value("game", "") != shelf_game_) continue;
        if (!f.empty()) {
            const std::string name = r["spirit"].value("name", "");
            bool hit = false;
            auto it = nodes.find(name);
            if (expr) hit = it != nodes.end() && maiz::node_matches(f, *it->second);
            else {
                hit = lower(name).find(fl) != std::string::npos || lower(c.value("name", "")).find(fl) != std::string::npos ||
                      lower(c.value("wiki", "")).find(fl) != std::string::npos;
                for (const auto& t : r["tags"]) if (!hit && t.is_string() && t.get<std::string>() == fl) hit = true;
            }
            if (!hit) continue;
        }
        shown.push_back(&r);
    }
    ImGui::TextDisabled("%zu of %zu turkeys", shown.size(), (*tm)["runes"].size());
    ImGui::Separator();
    ImGui::BeginChild("shelf-list");
    const float row = 40;
    ImGuiListClipper clip;
    clip.Begin(static_cast<int>(shown.size()), row + ImGui::GetStyle().ItemSpacing.y);
    while (clip.Step()) {
        for (int i = clip.DisplayStart; i < clip.DisplayEnd; ++i) {
            const json& r = *shown[static_cast<size_t>(i)];
            const std::string name = r["spirit"].value("name", "");
            ImGui::PushID(name.c_str());
            const ImVec2 p = ImGui::GetCursorScreenPos();
            const bool sel = name == sel_turkey_;
            if (ImGui::Selectable("##row", sel, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0, row))) {
                sel_turkey_ = name;
                inspect_mantle_ = "turkeys";
                inspect_rune_ = name;
                if (ImGui::IsMouseDoubleClicked(0) || ws_ == Workspace::Start) focus(Workspace::Turkey);
            }
            if (ImGui::BeginDragDropSource()) {
                ImGui::SetDragDropPayload("SQUID_TURKEY", name.c_str(), name.size() + 1);
                ImGui::Text("%s  %s", ICON_FA_DRUMSTICK_BITE, turkey_label(r).c_str());
                ImGui::EndDragDropSource();
            }
            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Inspect in Turkey")) { sel_turkey_ = name; focus(Workspace::Turkey); }
                if (ImGui::MenuItem("Play as this (Feast)")) {
                    edit("feast", {"set feast player " + maiz::arg(name)});
                    toast("the player is now " + turkey_label(r));
                }
                ImGui::EndPopup();
            }
            thumb_at(r, p.x + 2, p.y + 2, row - 4);
            ImDrawList* dl = ImGui::GetWindowDrawList();
            dl->AddText(ImVec2(p.x + row + 8, p.y + 2), ImGui::GetColorU32(ImGuiCol_Text), turkey_label(r).c_str());
            const std::string sub = r["content"].value("kind", "") + "  " + r["content"].value("game", "");
            dl->AddText(ImVec2(p.x + row + 8, p.y + 21), ImGui::GetColorU32(ImGuiCol_TextDisabled), sub.c_str());
            ImGui::PopID();
        }
    }
    ImGui::EndChild();
}

// ── the Turkey workspace ────────────────────────────────────────────────────

void Studio::draw_turkey() {
    const json* r = rune("turkeys", sel_turkey_);
    if (!r) {
        ImGui::Dummy(ImVec2(0, 20));
        ImGui::TextWrapped("%s  Pick a turkey on the Shelf to inspect it: what it is, which game it came from, every tag and "
                           "where that tag came from, and what still needs checking.", ICON_FA_DRUMSTICK_BITE);
        return;
    }
    const json& c = (*r)["content"];
    const std::string name = (*r)["spirit"].value("name", "");
    inspect_mantle_ = "turkeys";
    inspect_rune_ = name;

    if (big) ImGui::PushFont(big);
    ImGui::TextUnformatted(turkey_label(*r).c_str());
    if (big) ImGui::PopFont();
    ImGui::TextDisabled("%s   %s   %s", c.value("game", "").c_str(), c.value("kind", "").c_str(), name.c_str());
    {   // the views of one turkey: the card (what it is, its tags) and the thing itself, in 3D
        const char* views[] = {ICON_FA_CIRCLE_INFO "  Card", ICON_FA_CUBE "  In 3D"};
        for (int v = 0; v < 2; ++v) {
            if (v) ImGui::SameLine(0, 4);
            const bool on = turkey_view_ == v;
            if (on) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
            if (ImGui::Button(views[v])) turkey_view_ = v;
            if (on) ImGui::PopStyleColor();
        }
    }
    ImGui::Separator();
    if (turkey_view_ == 1) {
        const std::string model = c.value("model", "");
        const std::string lib = library_dir();
        if (model.empty()) {
            ImGui::TextWrapped("This turkey has no model file%s.", c.contains("rig") ? " (it is a character: its skeleton and animations are played by the runtime; Void Jelly does not model armatures yet)" : "");
            return;
        }
        const std::string path = (fs::path(lib) / model).string();
        view3d("turkey:" + path, [this, path] { return run("effect open " + maiz::arg(path)).ok && app_.scene->loaded; });
        return;
    }

    const float thumb = std::min(300.0f, ImGui::GetContentRegionAvail().x * 0.38f);
    ImGui::BeginGroup();
    thumbnail(*r, thumb);
    ImGui::TextDisabled("a thumbnail the toolbox rendered");
    ImGui::EndGroup();
    ImGui::SameLine(0, 18);
    ImGui::BeginGroup();
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
    if (!c.value("description", "").empty()) {
        ImGui::TextWrapped("%s", c.value("description", "").c_str());
        if (!c.value("wiki", "").empty()) ImGui::TextDisabled("from the wiki entry \"%s\"", c.value("wiki", "").c_str());
    } else {
        ImGui::TextDisabled("No description: no wiki entry matched this one.");
    }
    // a small local model's proposal for its wiki page (library/pilot.cpp): shown, never applied, until a person says
    if (c.value("wiki", "").empty()) {
        const std::string libp = library_dir();
        if (!libp.empty() && proposals_lib_ != libp) {
            proposals_lib_ = libp;
            std::ifstream f((fs::path(libp) / "pilot_proposals.json").string(), std::ios::binary);
            std::stringstream ss;
            ss << f.rdbuf();
            proposals_ = json::parse(ss.str(), nullptr, false);
        }
        const json rejected = c.value("wiki_rejected", json::array());
        const json* pr = proposals_.is_object() && proposals_.contains(name) ? &proposals_[name] : nullptr;
        const bool has = pr && (*pr)["proposal"].is_string() &&
                         std::find(rejected.begin(), rejected.end(), (*pr)["proposal"]) == rejected.end();
        ImGui::Spacing();
        if (has) {
            const std::string page = (*pr)["proposal"].get<std::string>(), model = pr->value("model", std::string("a local model"));
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.30f, 0.24f, 0.45f, 0.35f));
            ImGui::BeginChild("proposal", ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 4.6f), ImGuiChildFlags_Borders);
            ImGui::TextColored(ImVec4(0.75f, 0.62f, 0.98f, 1), "%s  Proposed by %s: the wiki page \"%s\"", ICON_FA_WAND_MAGIC_SPARKLES, model.c_str(), page.c_str());
            std::string guards;
            const json gj = pr->value("guards", json::object());   // held: items() of a temporary would dangle
            for (const auto& [k, v] : gj.items()) guards += (v.is_boolean() && v.get<bool>() ? "  ok " : "  NO ") + k;
            ImGui::TextDisabled("passed:%s", guards.c_str());
            if (ImGui::Button(ICON_FA_CHECK "  Accept")) {
                edit("turkeys", {"set " + maiz::arg(name) + " wiki " + maiz::arg(page), "set " + maiz::arg(name) + " wiki_source " + maiz::arg("local:" + model)});
                toast("matched to \"" + page + "\" (source local:" + model + ")");
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_XMARK "  Not this")) {
                json rj = rejected;
                rj.push_back(page);
                edit("turkeys", {"setjson " + maiz::arg(name) + " wiki_rejected " + maiz::arg(rj.dump())});
            }
            ImGui::SameLine();
            ImGui::TextDisabled("the shortlist it chose from: %zu pages", pr->value("shortlist", json::array()).size());
            if (ImGui::IsItemHovered()) {
                std::string sl;
                for (const auto& s : pr->value("shortlist", json::array())) sl += s.value("name", "") + "\n";
                ImGui::SetTooltip("%s", sl.c_str());
            }
            ImGui::EndChild();
            ImGui::PopStyleColor();
        } else if (!libp.empty()) {
            if (ImGui::SmallButton(ICON_FA_WAND_MAGIC_SPARKLES "  Ask the local model for its wiki page")) {
                busy("Asking the local model...", [this, libp, name] {
                    json r = json::parse(run("effect pilot propose " + maiz::arg(libp) + " \"\" 1 " + maiz::arg(name)).data, nullptr, false);
                    proposals_lib_.clear();   // read the file again
                    if (r.contains("error")) toast(r["error"].get<std::string>(), true);
                    else {
                        const json& p = r["proposals"];
                        toast(p.empty() ? "nothing to ask: this one has no shortlist in its game's wiki"
                                        : p[0]["proposal"].is_string() ? "proposed: " + p[0]["proposal"].get<std::string>()
                                                                       : "the model abstained (the guards did not all pass)");
                    }
                });
            }
            if (pr && !(*pr)["proposal"].is_string()) ImGui::TextDisabled("asked before: the model abstained");
        }
    }
    ImGui::Spacing();
    ImGui::SeparatorText("Checks");
    const std::string lib = library_dir();
    std::error_code ec;
    auto check = [&](bool ok, bool warn_only, const char* what) {
        ImGui::TextColored(ok ? ImVec4(0.45f, 0.86f, 0.6f, 1) : (warn_only ? ImVec4(0.95f, 0.78f, 0.35f, 1) : ImVec4(1, 0.45f, 0.4f, 1)), "%s",
                           ok ? ICON_FA_CIRCLE_CHECK : ICON_FA_TRIANGLE_EXCLAMATION);
        ImGui::SameLine();
        ImGui::TextUnformatted(what);
    };
    const std::string model = c.value("model", "");
    check(!model.empty() && !lib.empty() && fs::exists(fs::path(lib) / model, ec), false, "its model is in the library");
    check(c.contains("collision") && c["collision"].is_string() && !c["collision"].get<std::string>().empty(), true,
          "has collision of its own (else its shape is used)");
    check(!c.value("wiki", "").empty(), true, "matched to a wiki entry");
    check((*r)["tags"].size() >= 3, true, "has at least three tags");
    check(c.value("named_by", "").find("eye") == std::string::npos, true, "named by the game, not by eye");
    ImGui::PopTextWrapPos();
    ImGui::EndGroup();

    ImGui::SeparatorText("Kind  (what the game does with it)");
    const std::string kind = c.value("kind", "");
    int col = 0;
    for (const char* k : kKinds) {
        if (col++ % 6) ImGui::SameLine(0, 5);
        const bool on = kind == k;
        if (on) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
        if (ImGui::Button(k, ImVec2(92, 0)) && !on) {
            edit("turkeys", {"set " + maiz::arg(name) + " kind " + k, "set " + maiz::arg(name) + " kind_source person"});
        }
        if (on) ImGui::PopStyleColor();
    }
    if (c.value("kind_source", "") == "person") ImGui::TextDisabled("you chose this kind; a re-import keeps it");

    ImGui::SeparatorText("Tags  (colour: where it came from; click one to remove it)");
    ImGui::TextColored(ImVec4(0.42f, 0.56f, 0.84f, 1), "game");
    ImGui::SameLine(); ImGui::TextColored(ImVec4(0.47f, 0.75f, 0.47f, 1), "wiki");
    ImGui::SameLine(); ImGui::TextColored(ImVec4(0.86f, 0.75f, 0.35f, 1), "name");
    ImGui::SameLine(); ImGui::TextColored(ImVec4(0.67f, 0.51f, 0.90f, 1), "vision");
    ImGui::SameLine(); ImGui::TextColored(ImVec4(0.92f, 0.55f, 0.31f, 1), "you");
    tag_chips("turkeys", *r, true);
    // add a tag, with the vocabulary the shelf already uses as suggestions
    ImGui::SetNextItemWidth(220);
    const bool enter = ImGui::InputTextWithHint("##newtag", "add a tag", new_tag_, sizeof new_tag_, ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SameLine();
    if ((ImGui::Button(ICON_FA_PLUS "  Add") || enter) && new_tag_[0]) {
        add_tag("turkeys", name, new_tag_);
        new_tag_[0] = 0;
    }
    if (new_tag_[0]) {
        std::map<std::string, int> vocab;
        if (const json* tm = mantle("turkeys"))
            for (const auto& x : (*tm)["runes"]) for (const auto& t : x["tags"]) if (t.is_string()) vocab[t.get<std::string>()]++;
        const std::string pre = lower(new_tag_);
        int shown = 0;
        for (const auto& [w, n] : vocab) {
            if (w.rfind(pre, 0) != 0 || shown >= 10) continue;
            if (shown++) ImGui::SameLine(0, 4);
            char lbl[96];
            std::snprintf(lbl, sizeof lbl, "%s (%d)", w.c_str(), n);
            if (ImGui::SmallButton(lbl)) { add_tag("turkeys", name, w); new_tag_[0] = 0; }
        }
    }
    if (c.contains("tags_removed") && c["tags_removed"].is_array() && !c["tags_removed"].empty()) {
        std::string gone;
        for (const auto& t : c["tags_removed"]) if (t.is_string()) gone += (gone.empty() ? "" : ", ") + t.get<std::string>();
        ImGui::TextDisabled("removed by you, and kept removed on a re-import: %s", gone.c_str());
    }

    ImGui::SeparatorText("Sounds  (this turkey's own, by event)");
    for (const char* ev : {"collect", "defeat", "touch", "talk"}) {
        const std::string key = std::string("sound_") + ev;
        const std::string v = c.value(key, "");
        ImGui::PushID(ev);
        ImGui::AlignTextToFramePadding();
        ImGui::Text("%-8s", ev);
        ImGui::SameLine(90);
        ImGui::TextDisabled("%s", v.empty() ? "the game's default" : fs::path(v).filename().string().c_str());
        ImGui::SameLine(ImGui::GetContentRegionAvail().x > 420 ? 420 : 0);
        if (ImGui::SmallButton(ICON_FA_MUSIC " choose")) {
            std::string p = pick_file(FileKind::Audio, "A sound for this turkey");
            if (!p.empty()) edit("turkeys", {"set " + maiz::arg(name) + " " + key + " " + maiz::arg(p)});
        }
        if (!v.empty()) {
            ImGui::SameLine();
            if (ImGui::SmallButton(ICON_FA_XMARK)) edit("turkeys", {"set " + maiz::arg(name) + " " + key + " \"\""});
        }
        ImGui::PopID();
    }
    ImGui::TextDisabled("sounds play in the runtime; Void Squid itself has no audio yet");

    if (ImGui::CollapsingHeader("Every field")) {
        const maiz::Scene& sc = scene_of("turkeys");
        for (const auto& n : sc.nodes) {
            if (n.name != name) continue;
            std::vector<std::string> cmds;
            maiz::WidgetContext ctx{sc, cmds, "", 320.0f};
            maiz::widget_form(ctx, widgets_, n);
            if (!cmds.empty()) edit("turkeys", cmds);
            break;
        }
    }
}

}  // namespace squid::studio
