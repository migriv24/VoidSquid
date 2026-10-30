/* studio/studio.cpp — the shell: the project, dispatching, the theme, the menus, the
 * docked layout. The workspaces are in their own files (turkey.cpp, table.cpp, ...). */
#include "studio/studio.hpp"

#include "effects.hpp"
#include "host/paths.hpp"
#include "host/process.hpp"
#include "library/library.hpp"
#include "studio/platform.hpp"
#include "viewport.hpp"

#include "IconsFontAwesome6.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "voidmaiz/project.hpp"

#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>

namespace squid::studio {

namespace fs = std::filesystem;

namespace {
/* Who signs the journal: the profile's name (Profile window), else the account name. */
std::string person() { return host::load_profile().name; }

const char* ws_window(Workspace w) {
    switch (w) {
        case Workspace::Start: return "Start";
        case Workspace::Turkey: return "Turkey";
        case Workspace::Table: return "Table";
        case Workspace::Feast: return "Feast";
        case Workspace::Data: return "Data";
        case Workspace::Scene: return "Scene";
    }
    return "Start";
}

ImVec4 rgb(int r, int g, int b, float a = 1.0f) { return ImVec4(r / 255.f, g / 255.f, b / 255.f, a); }
}  // namespace

Studio::Studio(App& app) : app_(app) {
    widgets_ = maiz::WidgetRegistry::defaults();
    widgets_.add_path([](const maiz::SceneNode&, const char*, std::string_view) { return pick_file(FileKind::Any); });
}

Studio::~Studio() { shutdown(); }

// ── the project ─────────────────────────────────────────────────────────────

std::string Studio::project_path() const { return session_ ? session_->state_path() : std::string(); }

std::string Studio::library_dir() const {
    if (doc_.is_object() && doc_.contains("config") && doc_["config"].is_object()) {
        const auto& c = doc_["config"];
        if (c.contains("squid.library") && c["squid.library"].is_string()) return c["squid.library"].get<std::string>();
    }
    return {};
}

bool Studio::open_project(const std::string& path) {
    if (session_) session_->close();
    session_.reset();
    std::error_code ec;
    fs::create_directories(fs::path(path).parent_path(), ec);
    maiz::SessionOptions o;
    o.state_path = path;
    o.kind = maiz::SessionKind::Human;          // the person outranks any agent on this project
    o.actor = "human:" + person();
    o.effects = maiz::EffectPolicy::Allow;      // the window IS the grant; every effect is still journalled
    o.on_command = [this](std::string_view c) {
        transcript_.push_back("> " + std::string(c));
        if (transcript_.size() > 4000) transcript_.erase(transcript_.begin(), transcript_.begin() + 1000);
    };
    auto s = std::make_unique<maiz::Session>(app_.host, o);
    if (!s->start()) {
        session_error_ = s->error();
        if (session_error_.find("held by") != std::string::npos) locked_path_ = path;   // the person decides
        else toast("could not open " + path + ": " + session_error_, true);
        return false;
    }
    session_ = std::move(s);
    session_error_.clear();
    dirty_ = true;
    scenes_.clear();
    drop_textures();
    sel_turkey_.clear();
    sel_table_.clear();
    data_rune_.clear();
    inspect_rune_.clear();
    map_file_.clear();
    host::remember_recent(path);
    refresh();
    if (on_title) on_title("Void Squid  -  " + fs::path(path).filename().string());
    // what the project remembers of the scene harness
    if (doc_["config"].is_object()) {
        const auto& c = doc_["config"];
        if (c.contains(kInkKey) && c[kInkKey].is_string()) {
            std::snprintf(ink_path_, sizeof ink_path_, "%s", c[kInkKey].get<std::string>().c_str());
        }
        if (c.contains(kCameraKey) && c[kCameraKey].is_string()) Camera3D::parse(c[kCameraKey].get<std::string>(), cam_);
    }
    focus(library_dir().empty() ? Workspace::Start : Workspace::Turkey);
    return true;
}

void Studio::new_project() {
    const fs::path suggest = host::projects_dir() / "my-game.squid";
    std::string p = save_file(suggest.string(), FileKind::Project);
    if (p.empty()) { if (auto why = dialog_problem(); !why.empty()) toast(why, true); return; }
    std::error_code ec;
    if (fs::exists(p, ec)) fs::remove(p, ec);   // the dialog asked about overwriting
    if (open_project(p)) toast("new project: " + fs::path(p).filename().string());
}

void Studio::save_project() {
    if (!session_) return;
    run("save");
    if (!session_->save()) toast("save failed: " + session_->error(), true);
    else toast("saved " + fs::path(project_path()).filename().string());
}

void Studio::save_project_as() {
    if (!session_) return;
    const std::string cur = project_path();
    std::string p = save_file(cur.empty() ? (host::projects_dir() / "my-game.squid").string() : cur, FileKind::Project);
    if (p.empty()) { if (auto why = dialog_problem(); !why.empty()) toast(why, true); return; }
    const std::string state = session_->core().export_state();
    {
        std::ofstream out(p, std::ios::binary);
        if (!out) { toast("cannot write " + p, true); return; }
        out << state;
    }
    if (open_project(p)) toast("saved as " + fs::path(p).filename().string());
}

void Studio::open_library(const std::string& path) {
    if (!session_ || path.empty()) return;
    busy("Reading the library...", [this, path] {
        const maiz::Result r = run("effect library import " + maiz::arg(path));
        json reply;
        try { reply = json::parse(r.data); } catch (...) {}
        if (!r.ok || !reply.is_object() || reply.contains("error")) {
            toast(reply.is_object() && reply.contains("error") ? reply["error"].get<std::string>() : r.text(), true);
            return;
        }
        const std::string next = library::splice(session_->core().export_state(), reply);
        if (next.empty() || !session_->core().replace_state(next)) { toast("the library could not be spliced into the project", true); return; }
        dirty_ = true;
        scenes_.clear();
        drop_textures();
        transcript_.push_back("  library: " + reply.value("library", std::string()) + "  " + reply["counts"].dump());
        const auto& c = reply["counts"];
        toast("library opened: " + std::to_string(c.value("turkeys", 0)) + " turkeys, " + std::to_string(c.value("tables", 0)) +
              " tables, " + std::to_string(c.value("tracks", 0)) + " tracks");
        focus(Workspace::Turkey);
    });
}

// ── dispatching ─────────────────────────────────────────────────────────────

maiz::Result Studio::run(const std::string& command) {
    if (!session_) { maiz::Result r; r.ok = false; return r; }
    const maiz::Result r = session_->dispatch(command);
    const std::string t = r.text();
    if (!t.empty()) {
        std::string shown = t.size() > 600 ? t.substr(0, 600) + " ..." : t;
        transcript_.push_back((r.ok ? "  " : "  ! ") + shown);
    }
    dirty_ = true;
    return r;
}

void Studio::edit(const std::string& mantle, const std::vector<std::string>& commands) {
    if (!session_ || commands.empty()) return;
    std::string active;
    if (doc_.contains("active") && doc_["active"].is_object() && doc_["active"]["mantle"].is_string())
        active = doc_["active"]["mantle"].get<std::string>();
    if (active != mantle) run("use " + maiz::arg(mantle));
    if (commands.size() == 1) {
        const maiz::Result r = run(commands[0]);
        if (!r.ok) toast(r.text(), true);
    } else {
        const maiz::Result r = session_->batch(commands);
        for (const auto& c : commands) transcript_.push_back("> " + c);
        if (!r.ok) toast(r.text(), true);
        dirty_ = true;
    }
}

void Studio::refresh() {
    if (!dirty_ || !session_) return;
    try { doc_ = json::parse(session_->core().export_state()); } catch (...) { doc_ = json::object(); }
    for (auto& [m, d] : scene_dirty_) d = true;
    dirty_ = false;
}

const maiz::Scene& Studio::scene_of(const std::string& mantle) {
    auto it = scenes_.find(mantle);
    if (it == scenes_.end() || scene_dirty_[mantle]) {
        maiz::ProjectOptions o;
        o.mantle = mantle;
        scenes_[mantle] = maiz::project_scene(session_->core(), o);
        scene_dirty_[mantle] = false;
    }
    return scenes_[mantle];
}

const json* Studio::mantle(const std::string& name) const {
    if (!doc_.contains("mantles") || !doc_["mantles"].is_array()) return nullptr;
    for (const auto& m : doc_["mantles"]) if (m.value("name", "") == name) return &m;
    return nullptr;
}

const json* Studio::rune(const std::string& m, const std::string& name) const {
    const json* mm = mantle(m);
    if (!mm || name.empty()) return nullptr;
    for (const auto& r : (*mm)["runes"]) if (r["spirit"].value("name", "") == name) return &r;
    return nullptr;
}

void Studio::busy(const std::string& label, std::function<void()> fn) {
    busy_label_ = label;
    busy_fn_ = std::move(fn);
    busy_frames_ = 2;   // the overlay paints first, then the work blocks
}

void Studio::toast(const std::string& text, bool bad) {
    toasts_.push_back({text, bad, ImGui::GetTime() + (bad ? 9.0 : 4.5)});
    if (toasts_.size() > 5) toasts_.erase(toasts_.begin());
}

void Studio::focus(Workspace w) {
    ws_ = w;
    focus_request_ = w;
    focus_pending_ = true;
}

std::string Studio::turkey_label(const json& r) const {
    const json& c = r["content"];
    std::string n = c.value("name", r["spirit"].value("name", ""));
    return n;
}

// ── start ───────────────────────────────────────────────────────────────────

bool Studio::start(const std::string& project, const std::string& ink) {
    const host::Settings s = host::load_settings();
    light_ = s.light_mode;
    stars_ = s.stars;
    ImGui::GetIO().FontGlobalScale = s.ui_scale;
    apply_theme();
    std::string path = project;
    std::error_code ec;
    if (path.empty())
        for (const auto& r : s.recent) if (fs::exists(r, ec)) { path = r; break; }
    if (path.empty()) path = (host::projects_dir() / "untitled.squid").string();
    if (!open_project(path)) {
        // a locked or broken project must not leave the window with nothing at all
        const std::string keep = locked_path_;
        const std::string fallback = (host::projects_dir() / "untitled.squid").string();
        if (path == fallback || !open_project(fallback)) { if (keep.empty()) return false; }
        locked_path_ = keep;   // still ask about the one the person wanted
    }
    if (!ink.empty()) {
        std::snprintf(ink_path_, sizeof ink_path_, "%s", ink.c_str());
        ink_reply_ = run("effect open " + maiz::arg(ink)).text();
        if (app_.scene->loaded) run(std::string("config set ") + kInkKey + " " + maiz::arg(ink));
        focus(Workspace::Scene);
    }
    return true;
}

void Studio::shutdown() {
    if (session_) session_->close();
    session_.reset();
}

// ── the look ────────────────────────────────────────────────────────────────

void Studio::apply_theme() {
    ImGuiStyle& st = ImGui::GetStyle();
    st = ImGuiStyle();
    if (light_) ImGui::StyleColorsLight(); else ImGui::StyleColorsDark();
    st.FramePadding = ImVec2(8, 5);
    st.ItemSpacing = ImVec2(9, 7);
    st.ItemInnerSpacing = ImVec2(7, 5);
    st.CellPadding = ImVec2(7, 4);
    st.WindowPadding = ImVec2(12, 10);
    st.FrameRounding = 5.0f;
    st.GrabRounding = 5.0f;
    st.ChildRounding = 7.0f;
    st.PopupRounding = 7.0f;
    st.WindowRounding = 8.0f;
    st.TabRounding = 6.0f;
    st.ScrollbarSize = 13.0f;
    st.ScrollbarRounding = 8.0f;
    st.WindowBorderSize = 1.0f;
    st.TabBarBorderSize = 1.0f;
    st.DockingSeparatorSize = 3.0f;
    ImVec4* c = st.Colors;
    const ImVec4 accent = rgb(86, 214, 222), violet = rgb(158, 122, 240);
    auto tint = [&](ImGuiCol k, ImVec4 v, float a) { c[k] = ImVec4(v.x, v.y, v.z, a); };
    if (!light_) {
        // the deep: navy panels you can see the stars through, a cyan and a violet for life
        c[ImGuiCol_Text] = rgb(226, 232, 246);
        c[ImGuiCol_TextDisabled] = rgb(128, 140, 170);
        c[ImGuiCol_WindowBg] = rgb(13, 17, 34, 0.78f);
        c[ImGuiCol_ChildBg] = rgb(0, 0, 0, 0.0f);
        c[ImGuiCol_PopupBg] = rgb(16, 20, 38, 0.98f);
        c[ImGuiCol_Border] = rgb(48, 58, 96, 0.65f);
        c[ImGuiCol_FrameBg] = rgb(24, 30, 56, 0.92f);
        c[ImGuiCol_TitleBg] = rgb(10, 13, 26);
        c[ImGuiCol_TitleBgActive] = rgb(18, 24, 46);
        c[ImGuiCol_TitleBgCollapsed] = rgb(10, 13, 26, 0.8f);
        c[ImGuiCol_MenuBarBg] = rgb(9, 12, 24);
        c[ImGuiCol_Button] = rgb(34, 42, 78, 0.92f);
        c[ImGuiCol_Tab] = rgb(20, 26, 48);
        c[ImGuiCol_TabSelected] = rgb(40, 52, 96);
        c[ImGuiCol_TabDimmed] = rgb(16, 20, 38);
        c[ImGuiCol_TabDimmedSelected] = rgb(30, 38, 70);
        c[ImGuiCol_DockingEmptyBg] = rgb(0, 0, 0, 0.0f);
        c[ImGuiCol_TableHeaderBg] = rgb(22, 28, 52);
        c[ImGuiCol_TableRowBgAlt] = rgb(255, 255, 255, 0.025f);
        c[ImGuiCol_ScrollbarBg] = rgb(0, 0, 0, 0.0f);
        c[ImGuiCol_Separator] = rgb(48, 58, 96, 0.8f);
    } else {
        c[ImGuiCol_WindowBg] = rgb(245, 247, 252, 0.97f);
        c[ImGuiCol_DockingEmptyBg] = rgb(228, 232, 242);
    }
    tint(ImGuiCol_FrameBgHovered, accent, 0.20f);
    tint(ImGuiCol_FrameBgActive, accent, 0.32f);
    tint(ImGuiCol_ButtonHovered, accent, 0.45f);
    tint(ImGuiCol_ButtonActive, accent, 0.65f);
    tint(ImGuiCol_Header, violet, 0.30f);
    tint(ImGuiCol_HeaderHovered, violet, 0.45f);
    tint(ImGuiCol_HeaderActive, violet, 0.60f);
    tint(ImGuiCol_TabHovered, accent, 0.45f);
    tint(ImGuiCol_CheckMark, accent, 1.0f);
    tint(ImGuiCol_SliderGrab, accent, 0.85f);
    tint(ImGuiCol_SliderGrabActive, accent, 1.0f);
    tint(ImGuiCol_SeparatorHovered, accent, 0.6f);
    tint(ImGuiCol_SeparatorActive, accent, 0.9f);
    tint(ImGuiCol_ResizeGrip, accent, 0.2f);
    tint(ImGuiCol_ResizeGripHovered, accent, 0.5f);
    tint(ImGuiCol_DockingPreview, accent, 0.5f);
    tint(ImGuiCol_TextSelectedBg, accent, 0.35f);
    tint(ImGuiCol_NavHighlight, accent, 0.8f);
    tint(ImGuiCol_TabSelectedOverline, accent, 1.0f);
    tint(ImGuiCol_PlotHistogram, violet, 0.9f);
}

/* The stars the author liked in the turkey table, kept: a deterministic field that
 * twinkles, two faint nebulae, a gradient from the deep to the dusk. Drawn on ImGui's
 * background list, so it costs one draw call and no texture. */
void Studio::draw_stars() {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImDrawList* dl = ImGui::GetBackgroundDrawList(vp);
    const ImVec2 p0 = vp->Pos, sz = vp->Size;
    if (light_) {
        dl->AddRectFilledMultiColor(p0, ImVec2(p0.x + sz.x, p0.y + sz.y), IM_COL32(236, 240, 250, 255), IM_COL32(236, 240, 250, 255),
                                    IM_COL32(222, 228, 246, 255), IM_COL32(222, 228, 246, 255));
        return;
    }
    dl->AddRectFilledMultiColor(p0, ImVec2(p0.x + sz.x, p0.y + sz.y), IM_COL32(6, 9, 22, 255), IM_COL32(8, 10, 26, 255),
                                IM_COL32(20, 12, 38, 255), IM_COL32(10, 14, 34, 255));
    if (!stars_) return;
    const float t = static_cast<float>(ImGui::GetTime());
    auto nebula = [&](float fx, float fy, float r, ImU32 rgb_) {
        const ImVec2 c(p0.x + fx * sz.x, p0.y + fy * sz.y);
        for (int i = 6; i >= 1; --i) dl->AddCircleFilled(c, r * i / 6.0f, (rgb_ & 0x00FFFFFF) | (static_cast<ImU32>(5) << 24), 48);
    };
    nebula(0.78f, 0.28f, sz.y * 0.55f, IM_COL32(120, 80, 220, 0));
    nebula(0.18f, 0.82f, sz.y * 0.45f, IM_COL32(40, 160, 200, 0));
    uint32_t h = 2166136261u;
    auto rnd = [&]() { h ^= h << 13; h ^= h >> 17; h ^= h << 5; return (h & 0xFFFFFF) / 16777216.0f; };
    const int n = static_cast<int>(sz.x * sz.y / 2600.0f);
    for (int i = 0; i < n; ++i) {
        const float x = p0.x + rnd() * sz.x, y = p0.y + rnd() * sz.y;
        const float big = rnd(), phase = rnd() * 6.283f, speed = 0.6f + rnd() * 2.2f, hue = rnd();
        const float tw = 0.55f + 0.45f * std::sin(t * speed + phase);
        const float r = big > 0.97f ? 1.9f : (big > 0.85f ? 1.3f : 0.8f);
        const int a = static_cast<int>(255 * tw * (big > 0.85f ? 0.95f : 0.6f));
        ImU32 col = hue > 0.85f ? IM_COL32(170, 200, 255, a) : (hue > 0.72f ? IM_COL32(255, 220, 190, a) : IM_COL32(235, 240, 255, a));
        dl->AddCircleFilled(ImVec2(x, y), r, col, 8);
        if (big > 0.985f) {   // a few bright ones get a cross
            dl->AddLine(ImVec2(x - 5 * tw, y), ImVec2(x + 5 * tw, y), (col & 0x00FFFFFF) | (static_cast<ImU32>(a / 3) << 24));
            dl->AddLine(ImVec2(x, y - 5 * tw), ImVec2(x, y + 5 * tw), (col & 0x00FFFFFF) | (static_cast<ImU32>(a / 3) << 24));
        }
    }
}

// ── menus ───────────────────────────────────────────────────────────────────

void Studio::draw_menu() {
    if (!ImGui::BeginMainMenuBar()) return;
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem(ICON_FA_FILE "  New project...", "Ctrl+N")) new_project();
        if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN "  Open project...", "Ctrl+O")) {
            std::string p = pick_file(FileKind::Project, "Open a Void Squid project");
            if (!p.empty()) open_project(p);
            else if (auto why = dialog_problem(); !why.empty()) toast(why, true);
        }
        if (ImGui::BeginMenu("Recent projects")) {
            const auto s = host::load_settings();
            if (s.recent.empty()) ImGui::MenuItem("(none yet)", nullptr, false, false);
            for (const auto& r : s.recent) {
                const bool here = r == project_path();
                if (ImGui::MenuItem(fs::path(r).filename().string().c_str(), here ? "open" : nullptr, false, !here)) open_project(r);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", r.c_str());
            }
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem(ICON_FA_FLOPPY_DISK "  Save", "Ctrl+S")) save_project();
        if (ImGui::MenuItem("Save project as...")) save_project_as();
        ImGui::SeparatorText("Library");
        if (ImGui::MenuItem(ICON_FA_DATABASE "  Open library...")) {
            std::string p = pick_folder("Choose a library folder (the one holding assets.db)");
            if (!p.empty()) open_library(p);
            else if (auto why = dialog_problem(); !why.empty()) toast(why, true);
        }
        const std::string lib = library_dir();
        if (ImGui::MenuItem("Reload library", nullptr, false, !lib.empty())) open_library(lib);
        if (ImGui::MenuItem(ICON_FA_COMPACT_DISC "  Load a game...")) win_loadgame_ = true;
        ImGui::Separator();
        if (ImGui::MenuItem("Show project folder", nullptr, false, session_ != nullptr))
            run("effect reveal " + maiz::arg(fs::path(project_path()).parent_path().string()));
        if (ImGui::MenuItem("Show library folder", nullptr, false, !lib.empty())) run("effect reveal " + maiz::arg(lib));
        ImGui::Separator();
        if (ImGui::MenuItem("Quit", "Alt+F4") && on_quit) on_quit();
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem(ICON_FA_ROTATE_LEFT "  Undo", "Ctrl+Z")) run("undo");
        if (ImGui::MenuItem(ICON_FA_ROTATE_RIGHT "  Redo", "Ctrl+Y")) run("redo");
        ImGui::Separator();
        if (ImGui::MenuItem("Show unsaved changes")) { run("status"); win_console_ = true; }
        if (ImGui::MenuItem("Revert to the last save")) run("revert");
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View")) {
        struct W { Workspace w; const char* label; const char* key; };
        const W ws[] = {{Workspace::Start, ICON_FA_HOUSE "  Start", nullptr},
                        {Workspace::Turkey, ICON_FA_DRUMSTICK_BITE "  Turkey", "F5"},
                        {Workspace::Table, ICON_FA_MAP "  Table", "F6"},
                        {Workspace::Feast, ICON_FA_UTENSILS "  Feast", "F7"},
                        {Workspace::Data, ICON_FA_DATABASE "  Data", "F8"},
                        {Workspace::Scene, ICON_FA_CUBE "  Scene (.ink)", "F9"}};
        for (const auto& w : ws) if (ImGui::MenuItem(w.label, w.key, ws_ == w.w)) focus(w.w);
        ImGui::Separator();
        if (ImGui::MenuItem("Light mode", nullptr, light_)) {
            light_ = !light_;
            apply_theme();
            auto s = host::load_settings(); s.light_mode = light_; host::save_settings(s);
        }
        if (ImGui::MenuItem("Stars", nullptr, stars_)) {
            stars_ = !stars_;
            auto s = host::load_settings(); s.stars = stars_; host::save_settings(s);
        }
        float scale = ImGui::GetIO().FontGlobalScale;
        ImGui::SetNextItemWidth(160);
        if (ImGui::SliderFloat("Text size", &scale, 0.8f, 1.6f, "%.2fx")) ImGui::GetIO().FontGlobalScale = scale;
        if (ImGui::IsItemDeactivatedAfterEdit()) { auto s = host::load_settings(); s.ui_scale = scale; host::save_settings(s); }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Windows")) {
        ImGui::MenuItem(ICON_FA_LAYER_GROUP "  Shelf", nullptr, &win_shelf_);
        ImGui::MenuItem(ICON_FA_CIRCLE_INFO "  Inspector", nullptr, &win_inspector_);
        ImGui::MenuItem(ICON_FA_TERMINAL "  Console", nullptr, &win_console_);
        ImGui::MenuItem(ICON_FA_COMPACT_DISC "  Load a game", nullptr, &win_loadgame_);
        ImGui::MenuItem(ICON_FA_MICROCHIP "  Setup", nullptr, &win_setup_);
        ImGui::MenuItem(ICON_FA_LIST_CHECK "  Games (what each one holds, how complete)", nullptr, &win_progress_);
        ImGui::MenuItem(ICON_FA_CIRCLE_INFO "  Profile", nullptr, &win_profile_);
        ImGui::MenuItem(ICON_FA_GEAR "  Settings", nullptr, &win_settings_);
        ImGui::Separator();
        if (ImGui::MenuItem("Reset the layout")) { dock_seeded_ = false; ImGui::DockBuilderRemoveNode(ImGui::GetID("squid-dock")); }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Help")) {
        if (ImGui::MenuItem(ICON_FA_HOUSE "  Getting started")) focus(Workspace::Start);
        if (ImGui::MenuItem(ICON_FA_BOOK "  Setup guide (SETUP.md)")) {
            const std::string g = host::first_existing({host::ship_dir() / "SETUP.md", fs::path(SQUID_SOURCE_DIR) / "SETUP.md"});
            if (!g.empty()) run("effect reveal " + maiz::arg(g)); else toast("SETUP.md is not beside this copy of Void Squid", true);
        }
        if (ImGui::MenuItem(ICON_FA_MICROCHIP "  What can this machine do?")) win_setup_ = true;
        ImGui::Separator();
        if (ImGui::MenuItem(ICON_FA_CIRCLE_QUESTION "  About Void Squid")) win_about_ = true;
        ImGui::EndMenu();
    }
    // the right side: where you are
    {
        const std::string proj = session_ ? fs::path(project_path()).stem().string() : "(no project)";
        const std::string lib = library_dir();
        std::string where = proj + (lib.empty() ? "   no library" : "   " + fs::path(lib).filename().string());
        const float w = ImGui::CalcTextSize(where.c_str()).x + 140;
        ImGui::SameLine(ImGui::GetWindowWidth() - w);
        ImGui::TextDisabled("%s", where.c_str());
        ImGui::SameLine();
        if (ImGui::SmallButton(ICON_FA_ROTATE_LEFT)) run("undo");
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Undo (Ctrl+Z)");
        ImGui::SameLine();
        if (ImGui::SmallButton(ICON_FA_ROTATE_RIGHT)) run("redo");
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Redo (Ctrl+Y)");
        ImGui::SameLine();
        if (ImGui::SmallButton(ICON_FA_FLOPPY_DISK)) save_project();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Save (Ctrl+S)");
    }
    ImGui::EndMainMenuBar();
}

void Studio::draw_workspace_bar() {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    const float h = ImGui::GetFrameHeight() + 18;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 9));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, light_ ? rgb(232, 236, 246) : rgb(10, 13, 27, 0.96f));
    if (ImGui::BeginViewportSideBar("##workspaces", vp, ImGuiDir_Up, h, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar * 0)) {
        struct W { Workspace w; const char* label; const char* tip; };
        const W ws[] = {
            {Workspace::Start, ICON_FA_HOUSE "  Start", "What to do first"},
            {Workspace::Turkey, ICON_FA_DRUMSTICK_BITE "  Turkey", "Inspect one thing carefully: its tags, kind, checks and sounds (F5)"},
            {Workspace::Table, ICON_FA_MAP "  Table", "The worlds: which, in what order, what plays there (F6)"},
            {Workspace::Feast, ICON_FA_UTENSILS "  Feast", "The player: who, how fast, how many lives (F7)"},
            {Workspace::Data, ICON_FA_DATABASE "  Data", "Every mantle of the project, as tables (F8)"},
            {Workspace::Scene, ICON_FA_CUBE "  Scene", "The .ink harness: Void Jelly's draw list and residual (F9)"},
        };
        for (const auto& w : ws) {
            const bool on = ws_ == w.w;
            if (on) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
            if (ImGui::Button(w.label)) focus(w.w);
            if (on) ImGui::PopStyleColor();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", w.tip);
            ImGui::SameLine(0, 6);
        }
        // the right: Play, Load a game, Setup
        const float right = 3 * 130.0f + 2 * 44.0f;
        ImGui::SameLine(ImGui::GetWindowWidth() - right);
        ImGui::PushStyleColor(ImGuiCol_Button, rgb(46, 132, 98));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, rgb(62, 170, 124));
        const bool can_play = !library_dir().empty();
        ImGui::BeginDisabled(!can_play);
        if (ImGui::Button(ICON_FA_PLAY "  Play", ImVec2(120, 0))) {
            const json* t = rune("tables", sel_table_);
            std::string file;
            if (t) file = (*t)["content"].value("file", "");
            const json* f = rune("feast", "feast");
            const json feast = f ? library::feast_file((*f)["content"]) : json();
            const maiz::Result r = run("effect play " + maiz::arg(library_dir()) + " " + maiz::arg(file) + " " + maiz::arg(feast.is_null() ? "" : feast.dump()));
            json j;
            try { j = json::parse(r.data); } catch (...) {}
            if (j.is_object() && j.contains("error")) toast(j["error"].get<std::string>(), true);
            else toast(file.empty() ? "playing the first table" : "playing " + file);
        }
        ImGui::EndDisabled();
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip(can_play ? "Play the selected table (or the first) with the feast's player" : "Open a library first");
        ImGui::PopStyleColor(2);
        ImGui::SameLine(0, 6);
        if (ImGui::Button(ICON_FA_COMPACT_DISC "  Load a game", ImVec2(124, 0))) win_loadgame_ = true;
        ImGui::SameLine(0, 6);
        if (ImGui::Button(ICON_FA_MICROCHIP "  Setup", ImVec2(110, 0))) win_setup_ = true;
        ImGui::SameLine(0, 6);
        if (ImGui::Button(ICON_FA_GEAR, ImVec2(36, 0))) win_settings_ = !win_settings_;
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Settings");
        ImGui::SameLine(0, 6);
        {
            const host::Profile pr = host::load_profile();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(pr.color[0], pr.color[1], pr.color[2], 0.85f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.04f, 0.06f, 0.12f, 1));
            const std::string letter(1, pr.name.empty() ? '?' : static_cast<char>(std::toupper(static_cast<unsigned char>(pr.name[0]))));
            if (ImGui::Button((letter + "##profile").c_str(), ImVec2(36, 0))) win_profile_ = !win_profile_;
            ImGui::PopStyleColor(2);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Profile: %s", pr.name.c_str());
        }
    }
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

void Studio::dock_layout() {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    const ImGuiID dock = ImGui::GetID("squid-dock");
    if (!dock_seeded_ && ImGui::DockBuilderGetNode(dock) == nullptr) {
        dock_seeded_ = true;
        ImGui::DockBuilderRemoveNode(dock);
        ImGui::DockBuilderAddNode(dock, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dock, vp->WorkSize);
        ImGuiID center = dock, left, right, bottom;
        left = ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.22f, nullptr, &center);
        right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.30f, nullptr, &center);
        bottom = ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.24f, nullptr, &center);
        for (const char* w : {"Start", "Turkey", "Table", "Feast", "Data", "Scene", "Load a game", "Setup", "Profile", "Settings", "Games"})
            ImGui::DockBuilderDockWindow(w, center);
        ImGui::DockBuilderDockWindow("Shelf", left);
        ImGui::DockBuilderDockWindow("Inspector", right);
        ImGui::DockBuilderDockWindow("Console", bottom);
        ImGui::DockBuilderFinish(dock);
    }
    ImGui::DockSpaceOverViewport(dock, vp, ImGuiDockNodeFlags_PassthruCentralNode);
}

void Studio::shortcuts() {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantTextInput) return;
    if (ImGui::IsKeyPressed(ImGuiKey_F5, false)) focus(Workspace::Turkey);
    if (ImGui::IsKeyPressed(ImGuiKey_F6, false)) focus(Workspace::Table);
    if (ImGui::IsKeyPressed(ImGuiKey_F7, false)) focus(Workspace::Feast);
    if (ImGui::IsKeyPressed(ImGuiKey_F8, false)) focus(Workspace::Data);
    if (ImGui::IsKeyPressed(ImGuiKey_F9, false)) focus(Workspace::Scene);
    if (!io.KeyCtrl) return;
    if (ImGui::IsKeyPressed(ImGuiKey_S, false)) { if (io.KeyShift) save_project_as(); else save_project(); }
    if (ImGui::IsKeyPressed(ImGuiKey_Z, false)) run(io.KeyShift ? "redo" : "undo");
    if (ImGui::IsKeyPressed(ImGuiKey_Y, false)) run("redo");
    if (ImGui::IsKeyPressed(ImGuiKey_N, false)) new_project();
    if (ImGui::IsKeyPressed(ImGuiKey_O, false)) {
        std::string p = pick_file(FileKind::Project, "Open a Void Squid project");
        if (!p.empty()) open_project(p);
    }
}

void Studio::draw_toasts() {
    const double now = ImGui::GetTime();
    toasts_.erase(std::remove_if(toasts_.begin(), toasts_.end(), [&](const Toast& t) { return t.until < now; }), toasts_.end());
    if (toasts_.empty()) return;
    ImGuiViewport* vp = ImGui::GetMainViewport();
    float y = vp->WorkPos.y + vp->WorkSize.y - 16;
    for (size_t i = toasts_.size(); i-- > 0;) {
        const Toast& t = toasts_[i];
        const ImVec2 sz = ImGui::CalcTextSize(t.text.c_str(), nullptr, false, 520);
        y -= sz.y + 26;
        ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x + vp->WorkSize.x - 16, y), ImGuiCond_Always, ImVec2(1, 0));
        ImGui::SetNextWindowBgAlpha(0.95f);
        ImGui::PushStyleColor(ImGuiCol_Border, t.bad ? rgb(230, 90, 90) : rgb(86, 214, 222));
        char id[32];
        std::snprintf(id, sizeof id, "##toast%zu", i);
        ImGui::Begin(id, nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing |
                                      ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings);
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + 520);
        ImGui::TextColored(t.bad ? rgb(255, 150, 140) : rgb(150, 236, 240), "%s", t.bad ? ICON_FA_TRIANGLE_EXCLAMATION : ICON_FA_CIRCLE_CHECK);
        ImGui::SameLine();
        ImGui::TextUnformatted(t.text.c_str());
        ImGui::PopTextWrapPos();
        ImGui::End();
        ImGui::PopStyleColor();
    }
}

void Studio::draw_busy() {
    if (busy_label_.empty()) return;
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::GetForegroundDrawList()->AddRectFilled(vp->Pos, ImVec2(vp->Pos.x + vp->Size.x, vp->Pos.y + vp->Size.y), IM_COL32(4, 6, 14, 150));
    ImGui::SetNextWindowPos(ImVec2(vp->Pos.x + vp->Size.x * 0.5f, vp->Pos.y + vp->Size.y * 0.45f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::Begin("##busy", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings);
    ImGui::Text("%s  %s", ICON_FA_COMPACT_DISC, busy_label_.c_str());
    ImGui::End();
}

// ── the frame ───────────────────────────────────────────────────────────────

void Studio::frame() {
    // deferred heavy work: the overlay was painted last frame
    if (busy_fn_ && --busy_frames_ <= 0) {
        auto f = std::move(busy_fn_);
        busy_fn_ = nullptr;
        f();
        busy_label_.clear();
    }
    refresh();
    draw_stars();
    shortcuts();
    draw_menu();
    draw_workspace_bar();
    dock_layout();

    // the workspaces: always present, docked together; the focused one is the workspace
    auto workspace = [&](Workspace w, void (Studio::*body)()) {
        const char* name = ws_window(w);
        if (focus_pending_ && focus_request_ == w) { ImGui::SetNextWindowFocus(); focus_pending_ = false; }
        ImGuiWindowFlags fl = 0;
        if (!light_) ImGui::SetNextWindowBgAlpha(0.62f);   // the workspace is where the stars show through
        if (ImGui::Begin(name, nullptr, fl)) {
            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) ws_ = w;
            (this->*body)();
        }
        ImGui::End();
    };
    workspace(Workspace::Start, &Studio::draw_start);
    workspace(Workspace::Turkey, &Studio::draw_turkey);
    workspace(Workspace::Table, &Studio::draw_table);
    workspace(Workspace::Feast, &Studio::draw_feast);
    workspace(Workspace::Data, &Studio::draw_data);
    workspace(Workspace::Scene, &Studio::draw_scene_window);

    if (win_shelf_) { if (ImGui::Begin("Shelf", &win_shelf_)) draw_shelf(); ImGui::End(); }
    if (win_inspector_) { if (ImGui::Begin("Inspector", &win_inspector_)) draw_inspector(); ImGui::End(); }
    if (win_console_) { if (ImGui::Begin("Console", &win_console_)) draw_console(); ImGui::End(); }
    if (win_loadgame_) {
        ImGui::SetNextWindowSize(ImVec2(760, 560), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Load a game", &win_loadgame_)) draw_loadgame();
        ImGui::End();
    }
    if (win_setup_) {
        ImGui::SetNextWindowSize(ImVec2(760, 560), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Setup", &win_setup_)) draw_setup();
        ImGui::End();
    }
    if (win_about_) draw_about();
    if (win_profile_) draw_profile();
    if (win_progress_) {
        // a window newer than the person's saved layout docks beside the workspaces, not floating over them
        if (ImGuiWindow* start = ImGui::FindWindowByName("Start"); start && start->DockId)
            ImGui::SetNextWindowDockID(start->DockId, ImGuiCond_FirstUseEver);
        draw_progress();
    }
    if (win_settings_) draw_settings();
    draw_lock_prompt();
    draw_toasts();
    draw_busy();
}

/* A lock with no live owner is what a crash leaves behind, and nothing in the lock file
 * can tell a crashed window from a live one. So the person is asked, in their words. */
void Studio::draw_lock_prompt() {
    if (locked_path_.empty()) return;
    ImGui::OpenPopup("This project is already open");
    ImGui::SetNextWindowSize(ImVec2(560, 0));
    if (ImGui::BeginPopupModal("This project is already open", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::TextWrapped("%s", fs::path(locked_path_).filename().string().c_str());
        ImGui::TextWrapped("Another Void Squid window has it open, or a window closed without letting go of it (after a crash). "
                           "If no other window is showing it, open it here.");
        ImGui::TextDisabled("%s", session_error_.c_str());
        if (ImGui::Button("Open it here anyway")) {
            std::error_code ec;
            fs::remove(locked_path_ + ".lock", ec);
            const std::string p = locked_path_;
            locked_path_.clear();
            ImGui::CloseCurrentPopup();
            open_project(p);
        }
        ImGui::SameLine();
        if (ImGui::Button("Leave it")) { locked_path_.clear(); ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
}

std::string Studio::autotest(int f, bool& done) {
    done = false;
    auto first_of = [&](const char* m) -> std::string {
        const json* mm = mantle(m);
        return mm && !(*mm)["runes"].empty() ? (*mm)["runes"][0]["spirit"].value("name", "") : std::string();
    };
    switch (f) {
        case 20: focus(Workspace::Start); break;
        case 60: return "01_start";
        case 70:
            sel_turkey_ = rune("turkeys", "smg.Kuribo") ? "smg.Kuribo" : first_of("turkeys");
            if (const char* t = std::getenv("SQUID_SHOTS_TURKEY"); t && *t && rune("turkeys", t)) sel_turkey_ = t;
            turkey_view_ = 0;
            focus(Workspace::Turkey);
            break;
        case 110: return "02_turkey";
        case 115: turkey_view_ = 1; break;
        case 160: return "02b_turkey_3d";
        case 165: turkey_view_ = 0; break;
        case 170:
            sel_table_ = rune("tables", "world.json") ? "world.json" : first_of("tables");
            table_view_ = 0;
            focus(Workspace::Table);
            break;
        case 210: return "03_table";
        case 215: table_view_ = 1; break;
        case 240: return "03b_table_objects";
        case 245: table_view_ = 2; break;
        case 300: return "03c_table_3d";
        case 305: table_view_ = 0; focus(Workspace::Feast); break;
        case 340: return "04_feast";
        case 350: data_mantle_ = "turkeys"; data_rune_ = sel_turkey_; focus(Workspace::Data); break;
        case 390: return "05_data";
        case 400: {
            win_loadgame_ = true;
            const char* g = std::getenv("SQUID_SHOTS_GAME");
            if (g && *g) {
                std::snprintf(game_path_, sizeof game_path_, "%s", g);
                plan_ = json::parse(run("effect loadgame " + maiz::arg(g)).data, nullptr, false);
            }
            if (const char* w = std::getenv("SQUID_SHOTS_INGEST"); w && *w) { ingest_watch_ = w; ingest_poll_ = 0; ingest_tier_ = "minimum"; ingest_named_ = true; }
            ImGui::SetWindowFocus("Load a game");
            break;
        }
        case 440: return "06_loadgame";
        case 450:
            win_setup_ = true;
            inventory_ = json::parse(run("effect setup").data, nullptr, false);
            ImGui::SetWindowFocus("Setup");
            break;
        case 490: return "07_setup";
        case 495: win_settings_ = true; ImGui::SetWindowFocus("Settings"); break;
        case 520: return "08_settings";
        case 525: {
            win_settings_ = false;
            win_progress_ = true;
            const std::string lib = library_dir();
            if (!lib.empty()) progress_ = json::parse(run("effect progress " + maiz::arg(lib)).data, nullptr, false);
            ImGui::SetWindowFocus("Games");
            break;
        }
        case 570: return "09_games";
        case 580: done = true; break;
        default: break;
    }
    return {};
}

}  // namespace squid::studio
