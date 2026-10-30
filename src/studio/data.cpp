/* studio/data.cpp — the Data workspace and the Inspector.
 *
 * Void Hormiga's Data section, carried over (the author: "the data tab might be very
 * useful to copy over"): mantles on the left, the mantle's runes as a table in the
 * middle, the selected rune's form on the right. The form is Void Maiz's widget
 * registry (widget_form), so a glyph's hints pick each field's editor and every commit
 * is exactly one command. The filter is Maiz's one tag grammar (node_matches).
 */
#include "studio/studio.hpp"

#include "IconsFontAwesome6.h"
#include "imgui.h"
#include "voidmaiz/project.hpp"

#include <algorithm>
#include <cctype>

namespace squid::studio {

namespace {
std::string lower(std::string s) {
    for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}
}  // namespace

void Studio::draw_data() {
    if (!doc_.contains("mantles") || doc_["mantles"].empty()) {
        ImGui::TextWrapped("%s  This project has no mantles yet. Open a library, or type a command in the Console "
                           "(`mantle new notes`, then `rune new ...`).", ICON_FA_DATABASE);
        return;
    }
    // ── the mantles ──
    ImGui::BeginChild("data-mantles", ImVec2(190, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
    ImGui::TextDisabled("mantles");
    for (const auto& m : doc_["mantles"]) {
        const std::string name = m.value("name", "");
        char lbl[256];
        std::snprintf(lbl, sizeof lbl, "%s  (%zu)", name.c_str(), m["runes"].size());
        if (ImGui::Selectable(lbl, name == data_mantle_)) { data_mantle_ = name; data_rune_.clear(); }
    }
    ImGui::EndChild();
    ImGui::SameLine();

    const json* mm = mantle(data_mantle_);
    if (!mm) { ImGui::TextDisabled("pick a mantle"); return; }
    const maiz::Scene& sc = scene_of(data_mantle_);

    // ── the runes, as a table ──
    const float detail_w = std::max(320.0f, ImGui::GetContentRegionAvail().x * 0.40f);
    ImGui::BeginChild("data-list", ImVec2(ImGui::GetContentRegionAvail().x - detail_w - 8, 0), ImGuiChildFlags_Borders);
    ImGui::SetNextItemWidth(-90);
    ImGui::InputTextWithHint("##data-filter", ICON_FA_MAGNIFYING_GLASS "  filter: words, or a tag expression", data_filter_, sizeof data_filter_);
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_PLUS " New")) ImGui::OpenPopup("new-rune");
    if (ImGui::BeginPopup("new-rune")) {
        static char nm[128] = {};
        static int gi = 0;
        std::vector<std::string> glyphs;
        for (const char* g : {"turkey", "game", "kind", "track", "table", "feast"}) glyphs.push_back(g);
        ImGui::SetNextItemWidth(160);
        if (ImGui::BeginCombo("glyph", glyphs[static_cast<size_t>(gi)].c_str())) {
            for (size_t i = 0; i < glyphs.size(); ++i) if (ImGui::Selectable(glyphs[i].c_str(), static_cast<int>(i) == gi)) gi = static_cast<int>(i);
            ImGui::EndCombo();
        }
        ImGui::SetNextItemWidth(220);
        ImGui::InputText("name", nm, sizeof nm);
        if (ImGui::Button("Create") && nm[0]) {
            edit(data_mantle_, {"rune new " + glyphs[static_cast<size_t>(gi)] + " " + maiz::arg(nm), "set " + maiz::arg(nm) + " made_by person"});
            data_rune_ = nm;
            nm[0] = 0;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    // columns: name, glyph, the first three declared fields, tags
    std::vector<std::string> cols;
    if (!sc.nodes.empty()) for (const auto& f : sc.nodes[0].fields) if (cols.size() < 3) cols.push_back(f.key);
    const std::string f = data_filter_;
    const bool expr = f.find(" AND ") != std::string::npos || f.find(" OR ") != std::string::npos || f.rfind("NOT ", 0) == 0;
    std::vector<const maiz::SceneNode*> rows;
    for (const auto& n : sc.nodes) {
        if (!f.empty()) {
            bool hit = expr ? maiz::node_matches(f, n) : lower(n.name).find(lower(f)) != std::string::npos;
            if (!hit && !expr) for (const auto& fl : n.fields) if (lower(maiz::field_of(n, fl.key)).find(lower(f)) != std::string::npos) { hit = true; break; }
            if (!hit && !expr) for (const auto& t : n.tags) if (t == lower(f)) hit = true;
            if (!hit) continue;
        }
        rows.push_back(&n);
    }
    ImGui::TextDisabled("%zu of %zu", rows.size(), sc.nodes.size());
    const int ncol = 3 + static_cast<int>(cols.size());
    if (ImGui::BeginTable("runes", ncol, ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV)) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("name");
        ImGui::TableSetupColumn("glyph", ImGuiTableColumnFlags_WidthFixed, 70);
        for (const auto& c : cols) {
            std::string lbl = c;
            for (const auto& fl : sc.nodes[0].fields) if (fl.key == c && !fl.label.empty()) lbl = fl.label;
            ImGui::TableSetupColumn(lbl.c_str());
        }
        ImGui::TableSetupColumn("tags");
        ImGui::TableHeadersRow();
        ImGuiListClipper clip;
        clip.Begin(static_cast<int>(rows.size()));
        while (clip.Step()) {
            for (int i = clip.DisplayStart; i < clip.DisplayEnd; ++i) {
                const maiz::SceneNode& n = *rows[static_cast<size_t>(i)];
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::PushID(n.name.c_str());
                if (ImGui::Selectable(n.name.c_str(), n.name == data_rune_, ImGuiSelectableFlags_SpanAllColumns)) {
                    data_rune_ = n.name;
                    inspect_mantle_ = data_mantle_;
                    inspect_rune_ = n.name;
                    if (data_mantle_ == "turkeys") sel_turkey_ = n.name;
                    if (data_mantle_ == "tables") sel_table_ = n.name;
                }
                ImGui::PopID();
                ImGui::TableNextColumn();
                ImGui::TextDisabled("%s", n.glyph.c_str());
                for (const auto& c : cols) {
                    ImGui::TableNextColumn();
                    const std::string v = maiz::field_of(n, c);
                    ImGui::TextUnformatted(v.size() > 80 ? (v.substr(0, 80) + "...").c_str() : v.c_str());
                }
                ImGui::TableNextColumn();
                std::string tags;
                for (const auto& t : n.tags) { if (tags.size() > 70) { tags += " ..."; break; } tags += (tags.empty() ? "" : " ") + t; }
                ImGui::TextDisabled("%s", tags.c_str());
            }
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();
    ImGui::SameLine();

    // ── the rune ──
    ImGui::BeginChild("data-detail", ImVec2(0, 0), ImGuiChildFlags_Borders);
    const maiz::SceneNode* node = nullptr;
    for (const auto& n : sc.nodes) if (n.name == data_rune_) node = &n;
    if (!node) { ImGui::TextDisabled("pick a rune"); ImGui::EndChild(); return; }
    ImGui::Text("%s", node->name.c_str());
    ImGui::TextDisabled("%s in %s", node->label.c_str(), data_mantle_.c_str());
    ImGui::Separator();
    std::vector<std::string> cmds;
    maiz::WidgetContext ctx{sc, cmds, "", -1.0f};
    maiz::widget_form(ctx, widgets_, *node);
    if (!cmds.empty()) edit(data_mantle_, cmds);
    ImGui::SeparatorText("Tags");
    if (const json* r = rune(data_mantle_, node->name)) {
        tag_chips(data_mantle_, *r, true);
        static char t[64] = {};
        ImGui::SetNextItemWidth(160);
        if (ImGui::InputTextWithHint("##tag", "add a tag", t, sizeof t, ImGuiInputTextFlags_EnterReturnsTrue) && t[0]) {
            add_tag(data_mantle_, node->name, t);
            t[0] = 0;
        }
    }
    ImGui::SeparatorText("Links");
    int nl = 0;
    if (mm->contains("layout") && (*mm)["layout"].contains("edges"))
        for (const auto& e : (*mm)["layout"]["edges"]) {
            const std::string from = e.value("from", ""), to = e.value("to", "");
            if (from != node->name && to != node->name) continue;
            ImGui::BulletText("%s  -%s->  %s", from.c_str(), e.value("relation", "").c_str(), to.c_str());
            ++nl;
        }
    if (!nl) ImGui::TextDisabled("none");
    ImGui::Spacing();
    if (ImGui::Button(ICON_FA_XMARK "  Remove this rune")) ImGui::OpenPopup("confirm-rm");
    if (ImGui::BeginPopup("confirm-rm")) {
        ImGui::Text("Remove %s? Undo (Ctrl+Z) brings it back.", node->name.c_str());
        if (ImGui::Button("Remove")) {
            edit(data_mantle_, {"rune rm " + maiz::arg(node->name)});
            data_rune_.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Keep it")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    ImGui::EndChild();
}

void Studio::draw_inspector() {
    if (inspect_mantle_.empty() || inspect_rune_.empty() || !mantle(inspect_mantle_)) {
        ImGui::TextWrapped("%s  Whatever you select, anywhere, shows here with every field it declares. "
                           "Each edit is one command, so it can be undone and an agent can see it.", ICON_FA_CIRCLE_INFO);
        return;
    }
    const maiz::Scene& sc = scene_of(inspect_mantle_);
    const maiz::SceneNode* node = nullptr;
    for (const auto& n : sc.nodes) if (n.name == inspect_rune_) node = &n;
    if (!node) { ImGui::TextDisabled("%s is not in %s any more", inspect_rune_.c_str(), inspect_mantle_.c_str()); return; }
    ImGui::Text("%s", node->name.c_str());
    ImGui::TextDisabled("%s in %s", node->label.c_str(), inspect_mantle_.c_str());
    if (const json* r = rune(inspect_mantle_, inspect_rune_)) {
        if (inspect_mantle_ == "turkeys") thumbnail(*r, std::min(160.0f, ImGui::GetContentRegionAvail().x));
        tag_chips(inspect_mantle_, *r, false);
    }
    ImGui::Separator();
    std::vector<std::string> cmds;
    maiz::WidgetContext ctx{sc, cmds, "", -1.0f};
    maiz::widget_form(ctx, widgets_, *node);
    if (!cmds.empty()) edit(inspect_mantle_, cmds);
}

}  // namespace squid::studio
