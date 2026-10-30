/* studio/settings.cpp — the Profile and Settings windows.
 *
 * Profile is the person on this machine: the name that signs every command they make (the
 * journal's actor, "human:<name>"), and the reef a Coral will one day say they are. Settings
 * is this machine: how Void Squid looks, and where it should look for the things it cannot
 * find by itself (the runtime, Void Bicho, the ingest routes, a local model).
 *
 * Neither is the project. They live in the person's settings folder (host/paths.hpp) and
 * travel with nobody's .squid file. Nothing here dispatches a command: they are not the
 * document, so they are not in its journal.
 */
#include "studio/studio.hpp"

#include "effects.hpp"
#include "host/paths.hpp"
#include "library/setup.hpp"
#include "studio/platform.hpp"

#include "IconsFontAwesome6.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <cstring>
#include <filesystem>

namespace squid::studio {

namespace fs = std::filesystem;

namespace {
/* A text field bound to a std::string, committing on edit. */
bool text_row(const char* label, std::string& value, const char* hint, float width = -120) {
    char buf[1024];
    std::snprintf(buf, sizeof buf, "%s", value.c_str());
    ImGui::SetNextItemWidth(width);
    const bool changed = ImGui::InputTextWithHint(label, hint, buf, sizeof buf);
    if (changed) value = buf;
    return ImGui::IsItemDeactivatedAfterEdit();
}
bool folder_row(const char* id, const char* label, std::string& value, const char* hint) {
    ImGui::PushID(id);
    ImGui::TextUnformatted(label);
    bool commit = text_row("##v", value, hint, -110);
    ImGui::SameLine();
    if (ImGui::Button("Choose...")) {
        const std::string p = pick_folder(label);
        if (!p.empty()) { value = p; commit = true; }
    }
    ImGui::PopID();
    return commit;
}
}  // namespace

void Studio::draw_profile() {
    ImGui::SetNextWindowSize(ImVec2(520, 420), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Profile", &win_profile_)) { ImGui::End(); return; }
    static host::Profile p;
    static bool loaded = false;
    if (!loaded) { p = host::load_profile(); loaded = true; }
    ImGui::TextWrapped("You, on this machine. Your name signs every change you make to a project, in its journal, "
                       "so you can tell your work from an agent's. Later, when you share a game with other players "
                       "(Coral), this is who they will see.");
    ImGui::Spacing();
    // a coloured disc with the first letter: the reef's face until there is a picture
    {
        const ImVec2 c = ImGui::GetCursorScreenPos();
        const float r = 34;
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddCircleFilled(ImVec2(c.x + r, c.y + r), r, ImGui::ColorConvertFloat4ToU32(ImVec4(p.color[0], p.color[1], p.color[2], 1)), 48);
        const std::string letter = p.name.empty() ? "?" : std::string(1, static_cast<char>(std::toupper(static_cast<unsigned char>(p.name[0]))));
        if (big) ImGui::PushFont(big);
        const ImVec2 ts = ImGui::CalcTextSize(letter.c_str());
        dl->AddText(ImVec2(c.x + r - ts.x / 2, c.y + r - ts.y / 2), IM_COL32(10, 14, 30, 255), letter.c_str());
        if (big) ImGui::PopFont();
        ImGui::Dummy(ImVec2(r * 2, r * 2));
    }
    ImGui::SameLine(0, 16);
    ImGui::BeginGroup();
    bool save = text_row("Name", p.name, "how you sign your work", 260);
    ImGui::SetNextItemWidth(260);
    if (ImGui::ColorEdit3("Colour", p.color.data(), ImGuiColorEditFlags_NoInputs)) save = true;
    ImGui::EndGroup();
    ImGui::Spacing();
    ImGui::TextUnformatted("About you");
    char about[1024];
    std::snprintf(about, sizeof about, "%s", p.about.c_str());
    if (ImGui::InputTextMultiline("##about", about, sizeof about, ImVec2(-1, 90))) p.about = about;
    if (ImGui::IsItemDeactivatedAfterEdit()) save = true;
    if (save) {
        host::save_profile(p);
        toast("profile saved; new commands are signed human:" + p.name);
    }
    ImGui::Spacing();
    ImGui::TextDisabled("signing as: human:%s (takes effect when a project is opened)", p.name.c_str());
    ImGui::TextDisabled("kept in %s", (host::config_dir() / "profile.json").string().c_str());
    ImGui::End();
}

void Studio::draw_settings() {
    ImGui::SetNextWindowSize(ImVec2(640, 560), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Settings", &win_settings_)) { ImGui::End(); return; }
    host::Settings s = host::load_settings();
    bool save = false;
    if (ImGui::BeginTabBar("settings-tabs")) {
        if (ImGui::BeginTabItem(ICON_FA_STAR "  Look")) {
            if (ImGui::Checkbox("Light mode", &light_)) { s.light_mode = light_; apply_theme(); save = true; }
            if (ImGui::Checkbox("Stars in the background", &stars_)) { s.stars = stars_; save = true; }
            float scale = ImGui::GetIO().FontGlobalScale;
            ImGui::SetNextItemWidth(220);
            if (ImGui::SliderFloat("Text size", &scale, 0.8f, 1.6f, "%.2fx")) ImGui::GetIO().FontGlobalScale = scale;
            if (ImGui::IsItemDeactivatedAfterEdit()) { s.ui_scale = scale; save = true; }
            if (ImGui::Button("Reset the window layout")) { dock_seeded_ = false; ImGui::DockBuilderRemoveNode(ImGui::GetID("squid-dock")); }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(ICON_FA_FOLDER_OPEN "  Places")) {
            ImGui::TextWrapped("Where Void Squid should look. Leave a field empty and it looks for itself (beside this "
                               "program, beside its checkout, and in the usual install folders).");
            ImGui::Spacing();
            save |= folder_row("proj", "New projects go in", s.projects_dir, host::projects_dir().string().c_str());
            save |= folder_row("rt", "The runtime (the program Play starts)", s.runtime_dir, runtime_dir().empty() ? "not found" : runtime_dir().c_str());
            save |= folder_row("tb", "The ingest routes (the toolbox)", s.toolbox_dir, toolbox_dir().empty() ? "not found" : toolbox_dir().c_str());
            save |= folder_row("bi", "Void Bicho (vision)", s.bicho_root, "found by Setup, or VOIDBICHO_ROOT");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(ICON_FA_MICROCHIP "  Models")) {
            ImGui::TextWrapped("The local model the local-model setup uses (Ollama). Pick one you pulled; smaller is faster, "
                               "larger names and tags better. Every answer a model gives is still checked.");
            std::vector<std::string> models;
            if (inventory_.is_object())
                for (const auto& t : inventory_.value("tools", json::array()))
                    if (t.value("id", "") == "ollama" && t.contains("models"))
                        for (const auto& m : t["models"]) models.push_back(m.value("name", ""));
            ImGui::SetNextItemWidth(260);
            if (ImGui::BeginCombo("Local model", s.ollama_model.empty() ? "(the smallest pulled)" : s.ollama_model.c_str())) {
                if (ImGui::Selectable("(the smallest pulled)", s.ollama_model.empty())) { s.ollama_model.clear(); save = true; }
                for (const auto& m : models)
                    if (ImGui::Selectable(m.c_str(), m == s.ollama_model)) { s.ollama_model = m; save = true; }
                ImGui::EndCombo();
            }
            if (models.empty()) {
                ImGui::TextDisabled("no models listed yet:");
                ImGui::SameLine();
                if (ImGui::SmallButton("check this machine")) busy("Looking at this machine...", [this] { inventory_ = json::parse(run("effect setup").data, nullptr, false); });
            }
            ImGui::Spacing();
            ImGui::SeparatorText("Load a game uses");
            const char* tiers[] = {"minimum", "local", "vision"};
            const char* tier_text[] = {"Minimum: no model at all; every step is a rule (the default)",
                                       "Local model: as minimum, then the local model proposes names, under guards",
                                       "Vision: as minimum, and Void Bicho tags what it sees (when installed)"};
            for (int i = 0; i < 3; ++i)
                if (ImGui::RadioButton(tier_text[i], s.load_tier == tiers[i])) { s.load_tier = tiers[i]; save = true; }
            ImGui::Spacing();
            ImGui::TextDisabled("Load a game downloads its tools (apicula, nodtool, dk64_lib, Python packages) into %s", host::tools_dir().string().c_str());
            ImGui::TextDisabled("Needle (through Void Bicho) is planned as the smallest model; its telemetry is switched off before it starts.");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(ICON_FA_CIRCLE_INFO "  Data")) {
            ImGui::TextWrapped("What Void Squid keeps on this machine, and where. Nothing is sent anywhere unless a button says it will.");
            ImGui::BulletText("settings, layout, profile: %s", host::config_dir().string().c_str());
            ImGui::BulletText("projects: %s", host::projects_dir().string().c_str());
            if (ImGui::Button("Show the settings folder")) run("effect reveal " + maiz::arg(host::config_dir().string()));
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    if (save) host::save_settings(s);
    ImGui::End();
}

}  // namespace squid::studio
