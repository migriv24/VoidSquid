/* studio/panels.cpp — Start, Console, Load a game, Setup, About, and the Scene harness. */
#include "studio/studio.hpp"

#include "effects.hpp"
#include "host/paths.hpp"
#include "studio/platform.hpp"
#include "viewport.hpp"

#include "IconsFontAwesome6.h"
#include "imgui.h"
#include "voidmaiz/glhost.hpp"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <cctype>
#include <fstream>
#include <map>
#include <sstream>

namespace squid::studio {

namespace fs = std::filesystem;

namespace {
ImVec4 good() { return ImVec4(0.45f, 0.86f, 0.6f, 1); }
ImVec4 warn() { return ImVec4(0.95f, 0.78f, 0.35f, 1); }
ImVec4 bad() { return ImVec4(1.0f, 0.48f, 0.42f, 1); }
ImVec4 dim() { return ImVec4(0.55f, 0.6f, 0.72f, 1); }

json parse(const std::string& s) {
    try { return json::parse(s); } catch (...) { return json(); }
}

/* A rounded card with a title; the caller draws its body and calls end_card. */
bool card(const char* id, const char* title, float h) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.09f, 0.11f, 0.21f, 0.85f));
    ImGui::BeginChild(id, ImVec2(0, h), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
    ImGui::TextUnformatted(title);
    ImGui::Separator();
    return true;
}
void end_card() {
    ImGui::EndChild();
    ImGui::PopStyleColor();
}
}  // namespace

// ── Start ───────────────────────────────────────────────────────────────────

void Studio::draw_start() {
    ImGui::Dummy(ImVec2(0, 8));
    if (big) ImGui::PushFont(big);
    ImGui::TextUnformatted("Void Squid");
    if (big) ImGui::PopFont();
    ImGui::TextDisabled("Build new worlds out of the games you own.");
    ImGui::Dummy(ImVec2(0, 6));

    const float w = ImGui::GetContentRegionAvail().x;
    const bool two = w > 760;
    const float cw = two ? (w - 12) * 0.5f : w;
    const std::string lib = library_dir();

    ImGui::BeginGroup();
    ImGui::PushItemWidth(cw);
    ImGui::BeginChild("c1", ImVec2(cw, 0), ImGuiChildFlags_AutoResizeY);
    card("lib", ICON_FA_DATABASE "  1. Open a library", 0);
    ImGui::TextWrapped("A library is a folder on this machine with what was recovered from your games: models, worlds, "
                       "music, and assets.db, the catalogue. Opening it puts every turkey on the Shelf.");
    if (!lib.empty()) ImGui::TextColored(good(), "%s  open: %s", ICON_FA_CIRCLE_CHECK, lib.c_str());
    if (ImGui::Button(ICON_FA_FOLDER_OPEN "  Open library...")) {
        std::string p = pick_folder("Choose a library folder (the one holding assets.db)");
        if (!p.empty()) open_library(p); else if (auto why = dialog_problem(); !why.empty()) toast(why, true);
    }
    end_card();
    ImGui::Dummy(ImVec2(0, 4));
    card("game", ICON_FA_COMPACT_DISC "  2. Load a game", 0);
    ImGui::TextWrapped("Point at a game you own (a ROM, a disc image, an install folder). Void Squid says what it is, "
                       "whether a community already recovered it, and what this machine can do next.");
    if (ImGui::Button(ICON_FA_COMPACT_DISC "  Load a game...")) win_loadgame_ = true;
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_LIST_CHECK "  Games: what each holds, how complete")) { win_progress_ = true; progress_ = json(); }
    end_card();
    ImGui::EndChild();
    ImGui::PopItemWidth();
    ImGui::EndGroup();
    if (two) ImGui::SameLine(0, 12);
    ImGui::BeginGroup();
    ImGui::BeginChild("c2", ImVec2(cw, 0), ImGuiChildFlags_AutoResizeY);
    card("proj", ICON_FA_FILE "  Projects", 0);
    ImGui::TextWrapped("A project (.squid) is your work: the catalogue you curated, the tables, the feast. "
                       "The library's files stay where they are.");
    ImGui::TextDisabled("this one: %s", project_path().c_str());
    if (ImGui::Button("New project...")) new_project();
    ImGui::SameLine();
    if (ImGui::Button("Open project...")) {
        std::string p = pick_file(FileKind::Project, "Open a Void Squid project");
        if (!p.empty()) open_project(p);
    }
    const auto s = host::load_settings();
    int n = 0;
    for (const auto& r : s.recent) {
        if (r == project_path() || n++ >= 5) continue;
        if (ImGui::Selectable((ICON_FA_FILE "  " + fs::path(r).filename().string()).c_str())) open_project(r);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", r.c_str());
    }
    end_card();
    ImGui::Dummy(ImVec2(0, 4));
    card("mach", ICON_FA_MICROCHIP "  This machine", 0);
    if (inventory_.is_object() && inventory_.contains("setups")) {
        for (const auto& st : inventory_["setups"]) {
            const bool ready = st.value("ready", false), planned = st.value("planned", false);
            ImGui::TextColored(ready ? good() : (planned ? dim() : warn()), "%s", ready ? ICON_FA_CIRCLE_CHECK : ICON_FA_TRIANGLE_EXCLAMATION);
            ImGui::SameLine();
            ImGui::Text("%s", st.value("name", "").c_str());
            ImGui::SameLine();
            ImGui::TextDisabled("%s", st.value("needs", "").c_str());
        }
    } else {
        ImGui::TextWrapped("Which setups this machine can run: minimum (no model, no GPU), a local model, vision, a coding agent.");
    }
    if (ImGui::Button(ICON_FA_MICROCHIP "  Check this machine")) { win_setup_ = true; busy("Looking at this machine...", [this] { inventory_ = parse(run("effect setup").data); }); }
    end_card();
    ImGui::EndChild();
    ImGui::EndGroup();

    ImGui::Dummy(ImVec2(0, 10));
    ImGui::TextDisabled("Void Squid ships no game and no game data. Everything you load stays on this machine.");
    ImGui::TextDisabled("Workspaces: Turkey F5 (inspect)  Table F6 (worlds)  Feast F7 (player)  Data F8  Scene F9.");
}

// ── Console ─────────────────────────────────────────────────────────────────

void Studio::draw_console() {
    const float input_h = ImGui::GetFrameHeightWithSpacing() + 4;
    ImGui::BeginChild("transcript", ImVec2(0, -input_h));
    if (mono) ImGui::PushFont(mono);
    ImGuiListClipper clip;
    clip.Begin(static_cast<int>(transcript_.size()));
    while (clip.Step())
        for (int i = clip.DisplayStart; i < clip.DisplayEnd; ++i) {
            const std::string& l = transcript_[static_cast<size_t>(i)];
            if (l.rfind("> ", 0) == 0) ImGui::TextColored(ImVec4(0.45f, 0.84f, 0.88f, 1), "%s", l.c_str());
            else if (l.rfind("  ! ", 0) == 0) ImGui::TextColored(bad(), "%s", l.c_str());
            else ImGui::TextUnformatted(l.c_str());
        }
    if (mono) ImGui::PopFont();
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 4) ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();
    ImGui::SetNextItemWidth(-1);
    auto history = [](ImGuiInputTextCallbackData* d) -> int {
        Studio* self = static_cast<Studio*>(d->UserData);
        auto& h = self->console_history_;
        if (h.empty()) return 0;
        int& pos = self->console_history_pos_;
        if (d->EventKey == ImGuiKey_UpArrow) pos = pos < 0 ? static_cast<int>(h.size()) - 1 : std::max(0, pos - 1);
        else if (d->EventKey == ImGuiKey_DownArrow) pos = pos < 0 ? -1 : (pos + 1 >= static_cast<int>(h.size()) ? -1 : pos + 1);
        d->DeleteChars(0, d->BufTextLen);
        if (pos >= 0) d->InsertChars(0, h[static_cast<size_t>(pos)].c_str());
        return 0;
    };
    if (ImGui::InputTextWithHint("##cmd", "a command: ls, find \"enemy AND smg\", set <rune> <field> <value>, help ...", console_line_, sizeof console_line_,
                                 ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory, history, this)) {
        const std::string c = console_line_;
        if (!c.empty()) {
            run(c);
            console_history_.push_back(c);
            console_history_pos_ = -1;
        }
        console_line_[0] = 0;
        ImGui::SetKeyboardFocusHere(-1);
    }
}

// ── Load a game ─────────────────────────────────────────────────────────────

void Studio::draw_loadgame() {
    ImGui::TextWrapped("Loading a game is the first step, not the point: what comes out becomes turkeys and tables you build "
                       "new things with. Void Squid never sends your game anywhere.");
    ImGui::SetNextItemWidth(-260);
    ImGui::InputTextWithHint("##game", "a ROM, a disc image or an install folder you own", game_path_, sizeof game_path_);
    ImGui::SameLine();
    auto read_game = [this](const std::string& p) { busy("Reading the game...", [this, p] { plan_ = parse(run("effect loadgame " + maiz::arg(p)).data); }); };
    if (ImGui::Button("File...")) {
        std::string p = pick_file(FileKind::Game, "Choose a game file you own");
        if (!p.empty()) { std::snprintf(game_path_, sizeof game_path_, "%s", p.c_str()); read_game(p); }
    }
    ImGui::SameLine();
    if (ImGui::Button("Folder...")) {
        std::string p = pick_folder("Choose a game folder");
        if (!p.empty()) { std::snprintf(game_path_, sizeof game_path_, "%s", p.c_str()); read_game(p); }
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(!game_path_[0]);
    if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS "  Read it")) {
        const std::string p = game_path_;
        busy("Reading the game...", [this, p] { plan_ = parse(run("effect loadgame " + maiz::arg(p)).data); });
    }
    ImGui::EndDisabled();
    ImGui::TextDisabled("Known projects: Void Squid's own list, plus the community's when downloaded.");
    ImGui::SameLine();
    if (ImGui::SmallButton(ICON_FA_ROTATE_RIGHT " Download the community list")) {
        busy("Downloading the community list...", [this] {
            json r = parse(run("effect catalogue refresh").data);
            if (r.contains("error")) toast(r["error"].get<std::string>(), true);
            else toast("community list: " + std::to_string(r.value("entries", 0)) + " projects");
        });
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Download the community's list of game decompilations\n(github.com/CharlotteCross1998/awesome-game-decompilations)\ninto your settings folder. One request.");

    if (!plan_.is_object()) return;
    if (plan_.contains("error")) { ImGui::TextColored(bad(), "%s", plan_["error"].get<std::string>().c_str()); return; }
    ImGui::Separator();
    if (big) ImGui::PushFont(big);
    ImGui::TextWrapped("%s", plan_.value("summary", "").c_str());
    if (big) ImGui::PopFont();
    const json& rc = plan_["recon"];
    auto str = [](const json& v) { return v.is_string() ? v.get<std::string>() : std::string("-"); };
    ImGui::Text("It is:  %s   platform %s   title %s", str(rc["kind"]).c_str(), str(rc["platform"]).c_str(), str(rc["title"]).c_str());
    if (rc.contains("header") && rc["header"].is_object()) {
        std::string h;
        for (auto it = rc["header"].begin(); it != rc["header"].end(); ++it) h += it.key() + " " + (it.value().is_string() ? it.value().get<std::string>() : it.value().dump()) + "   ";
        ImGui::TextDisabled("%s", h.c_str());
    }
    ImGui::TextDisabled("file: %s   community list: %s", plan_.value("target", "").c_str(), plan_.value("community_list", "").c_str());

    ImGui::SeparatorText("Has someone already recovered it?");
    if (plan_["matches"].empty()) ImGui::TextDisabled("no match in the lists Void Squid has");
    else if (ImGui::BeginTable("matches", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("project", ImGuiTableColumnFlags_WidthStretch, 3.0f);
        ImGui::TableSetupColumn("kind", ImGuiTableColumnFlags_WidthFixed, 110);
        ImGui::TableSetupColumn("why it matched", ImGuiTableColumnFlags_WidthStretch, 1.6f);
        ImGui::TableSetupColumn("score", ImGuiTableColumnFlags_WidthFixed, 44);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 60);
        ImGui::TableHeadersRow();
        for (const auto& m : plan_["matches"]) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", m.value("title", "").c_str());
            ImGui::TextDisabled("%s  %s%s", m.value("platform", "").c_str(), m.value("from", "").c_str(), m.value("ai_flagged", false) ? "  (the list marks AI involvement)" : "");
            if (m.contains("route")) ImGui::TextColored(good(), "route: %s", m["route"].get<std::string>().c_str());
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(m.value("kind", "").c_str());
            ImGui::TableNextColumn();
            ImGui::TextWrapped("%s", m.value("why", "").c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%d", m.value("score", 0));
            ImGui::TableNextColumn();
            const std::string url = m.value("url", "");
            ImGui::PushID(&m);
            if (!url.empty() && ImGui::SmallButton(ICON_FA_LINK " open")) run("effect reveal " + maiz::arg(url));
            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    ImGui::SeparatorText("What happens next");
    for (const auto& s : plan_["steps"]) {
        const std::string state = s.value("state", "");
        ImVec4 col = dim();
        const char* icon = ICON_FA_CIRCLE_INFO;
        if (state == "done") { col = good(); icon = ICON_FA_CIRCLE_CHECK; }
        else if (state == "ready") { col = ImVec4(0.45f, 0.84f, 0.88f, 1); icon = ICON_FA_PLAY; }
        else if (state == "check" || state == "manual") { col = warn(); icon = ICON_FA_TRIANGLE_EXCLAMATION; }
        else if (state == "needs" || state == "none") { col = bad(); icon = ICON_FA_XMARK; }
        ImGui::TextColored(col, "%s", icon);
        ImGui::SameLine();
        ImGui::Text("%s", s.value("step", "").c_str());
        ImGui::SameLine();
        ImGui::TextDisabled("[%s setup, %s]", s.value("setup", "").c_str(), state.c_str());
        ImGui::Indent();
        ImGui::TextWrapped("%s", s.value("how", "").c_str());
        ImGui::Unindent();
    }
    // ── carry on: one button, then loading bars ──
    const std::string route = plan_.value("route_id", "");
    if (!route.empty()) {
        ImGui::SeparatorText(ICON_FA_PLAY "  Load it");
        if (!ingest_lib_[0]) {
            const std::string lib = library_dir();
            std::snprintf(ingest_lib_, sizeof ingest_lib_, "%s", lib.empty() ? (host::projects_dir() / "library").string().c_str() : lib.c_str());
        }
        const host::Settings st = host::load_settings();
        const std::string tier = st.load_tier.empty() ? "minimum" : st.load_tier;
        const char* tier_name = tier == "local" ? "Local model" : tier == "vision" ? "Vision" : "Minimum (no model)";
        ImGui::TextWrapped("Void Squid fetches the tools this game needs, takes the game apart with them, and builds a library you can "
                           "make new things from. Minutes, on its own; you can keep working.");
        ImGui::TextDisabled("Setup: %s.", tier_name);
        ImGui::SameLine();
        if (ImGui::SmallButton("change##tier")) win_settings_ = true;
        ImGui::SetNextItemWidth(-180);
        ImGui::InputText("##ingestlib", ingest_lib_, sizeof ingest_lib_);
        ImGui::SameLine();
        if (ImGui::Button("Folder...##il")) {
            std::string p = pick_folder("Where should the library go?");
            if (!p.empty()) std::snprintf(ingest_lib_, sizeof ingest_lib_, "%s", p.c_str());
        }
        const bool running = !ingest_watch_.empty() && !ingest_done_.is_object();
        ImGui::BeginDisabled(running);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.55f, 0.36f, 1));
        if (ImGui::Button(ICON_FA_PLAY "  Load it", ImVec2(180, 34))) {
            const maiz::Result r = run("effect ingest " + maiz::arg(route) + " " + maiz::arg(plan_.value("target", "")) + " " + maiz::arg(ingest_lib_) + " " + maiz::arg(tier));
            json j = parse(r.data);
            if (j.contains("error")) toast(j["error"].get<std::string>(), true);
            else { ingest_watch_ = ingest_lib_; ingest_tail_.clear(); ingest_done_ = json(); ingest_prog_ = json(); ingest_poll_ = 0; ingest_tier_ = tier; ingest_named_ = false; }
        }
        ImGui::PopStyleColor();
        ImGui::EndDisabled();
    }
    if (!ingest_watch_.empty()) {
        const double now = ImGui::GetTime();
        if (now > ingest_poll_) {   // the route's own progress, four times a second
            ingest_poll_ = now + 0.25;
            auto slurp = [](const fs::path& f) {
                std::string body;
                if (std::FILE* h = std::fopen(f.string().c_str(), "rb")) {
                    char buf[4096];
                    size_t n;
                    while ((n = std::fread(buf, 1, sizeof buf, h)) > 0) body.append(buf, n);
                    std::fclose(h);
                }
                return body;
            };
            const std::string pj = slurp(fs::path(ingest_watch_) / "ingest.progress.json");
            if (!pj.empty()) { json j = parse(pj); if (j.is_object()) ingest_prog_ = j; }
            ingest_tail_.clear();
            std::string log = slurp(fs::path(ingest_watch_) / "ingest.log");
            size_t at = 0;
            while (at < log.size()) {
                size_t e = log.find(char(10), at);
                if (e == std::string::npos) e = log.size();
                std::string l = log.substr(at, e - at);
                while (!l.empty() && l.back() == char(13)) l.pop_back();
                ingest_tail_.push_back(l);
                if (ingest_tail_.size() > 14) ingest_tail_.erase(ingest_tail_.begin());
                at = e + 1;
            }
            const std::string dj = slurp(fs::path(ingest_watch_) / "ingest.done");
            if (!dj.empty()) ingest_done_ = parse(dj);
        }
        // one bar per step: a fraction when the route knows one, a moving bar when it does not
        if (ingest_prog_.is_object()) {
            int finished = 0, total = 0;
            for (const auto& s : ingest_prog_.value("steps", json::array())) {
                ++total;
                const std::string state = s.value("state", "");
                finished += state == "done" || state == "skipped";
                ImVec4 col = dim();
                const char* icon = ICON_FA_CIRCLE;
                if (state == "done") { col = good(); icon = ICON_FA_CIRCLE_CHECK; }
                else if (state == "skipped") { col = dim(); icon = ICON_FA_CIRCLE_CHECK; }
                else if (state == "running") { col = ImVec4(0.45f, 0.84f, 0.88f, 1); icon = ICON_FA_SPINNER; }
                else if (state == "failed") { col = bad(); icon = ICON_FA_XMARK; }
                ImGui::TextColored(col, "%s", icon);
                ImGui::SameLine();
                ImGui::SetNextItemWidth(260);
                ImGui::TextUnformatted(s.value("title", "").c_str());
                ImGui::SameLine(320);
                float frac = s.contains("fraction") && s["fraction"].is_number() ? s["fraction"].get<float>() : -1.f;
                std::string over;
                if (state == "running" && frac < 0) {   // indeterminate: a band that moves
                    frac = static_cast<float>(std::fmod(now * 0.6, 1.0));
                    over = "working";
                } else if (state == "waiting") frac = 0.f;
                else if (state == "skipped") { frac = 1.f; over = "already here"; }
                else if (frac < 0) frac = state == "done" ? 1.f : 0.f;
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, state == "failed" ? bad() : state == "skipped" ? ImVec4(0.35f, 0.42f, 0.55f, 1) : ImVec4(0.30f, 0.72f, 0.80f, 1));
                ImGui::ProgressBar(frac, ImVec2(220, 0), over.empty() ? nullptr : over.c_str());
                ImGui::PopStyleColor();
                const std::string detail = s.value("detail", "");
                if (!detail.empty() && state != "waiting") {
                    ImGui::SameLine();
                    ImGui::TextDisabled("%s", detail.c_str());
                }
            }
            ImGui::Spacing();
            ImGui::ProgressBar(total ? static_cast<float>(finished) / total : 0.f, ImVec2(-1, 0),
                               (std::to_string(finished) + " of " + std::to_string(total) + " steps").c_str());
        }
        if (ImGui::TreeNode("The log")) {
            ImGui::BeginChild("ingestlog", ImVec2(0, 160), ImGuiChildFlags_Borders);
            if (mono) ImGui::PushFont(mono);
            for (const auto& l : ingest_tail_) ImGui::TextUnformatted(l.c_str());
            if (mono) ImGui::PopFont();
            ImGui::EndChild();
            ImGui::TreePop();
        }
        if (ingest_done_.is_object()) {
            if (ingest_done_.value("ok", false)) {
                // the local-model setup: proposals for things with no wiki page yet, held to account (library/pilot.cpp)
                if (ingest_tier_ == "local" && !ingest_named_) {
                    ingest_named_ = true;
                    const std::string lib = ingest_watch_;
                    busy("Asking the local model for names...", [this, lib] {
                        json r = parse(run("effect pilot propose " + maiz::arg(lib)).data);
                        toast(r.contains("error") ? "naming: " + r["error"].get<std::string>()
                                                  : "the local model proposed names; review them in the Turkey workspace");
                    });
                }
                ImGui::TextColored(good(), "%s  loaded", ICON_FA_CIRCLE_CHECK);
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_DATABASE "  Open the library")) { open_library(ingest_watch_); ingest_watch_.clear(); }
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_LIST_CHECK "  How complete is it?")) win_progress_ = true;
            } else {
                ImGui::TextColored(bad(), "%s  stopped: %s", ICON_FA_TRIANGLE_EXCLAMATION, ingest_done_.value("error", std::string("see the log")).c_str());
                ImGui::SameLine();
                if (ImGui::SmallButton("dismiss")) ingest_watch_.clear();
            }
        }
    }

    ImGui::Spacing();
    ImGui::TextDisabled("Every step above is a rule, not a guess: a header read, a title match, a lookup on this machine.");
    ImGui::TextDisabled("A model may propose later steps; the checks decide what is accepted.");
}

// ── Setup ───────────────────────────────────────────────────────────────────

void Studio::draw_setup() {
    ImGui::TextWrapped("What this machine can do. Void Squid works with nothing extra installed (the minimum setup) and "
                       "gets better with more: a local model, vision, a coding agent. It only looks; it installs nothing.");
    if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS "  Check this machine")) busy("Looking at this machine...", [this] { inventory_ = parse(run("effect setup").data); });
    ImGui::SameLine();
    if (ImGui::Button("Check vision deeply")) busy("Asking PyTorch about the GPU (this takes a few seconds)...", [this] { inventory_ = parse(run("effect setup deep").data); });
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_BOOK "  The setup guide")) {
        const std::string g = host::first_existing({host::ship_dir() / "SETUP.md", fs::path(SQUID_SOURCE_DIR) / "SETUP.md"});
        if (!g.empty()) run("effect reveal " + maiz::arg(g));
    }
    if (!inventory_.is_object() || !inventory_.contains("setups")) return;
    ImGui::TextDisabled("checked %s", inventory_.value("checked", "").c_str());

    ImGui::SeparatorText("Setups");
    for (const auto& s : inventory_["setups"]) {
        const bool ready = s.value("ready", false), planned = s.value("planned", false);
        ImGui::PushID(s.value("id", "").c_str());
        ImGui::TextColored(ready ? good() : (planned ? dim() : warn()), "%s", ready ? ICON_FA_CIRCLE_CHECK : (planned ? ICON_FA_CIRCLE_INFO : ICON_FA_TRIANGLE_EXCLAMATION));
        ImGui::SameLine();
        ImGui::Text("%s", s.value("name", "").c_str());
        ImGui::SameLine(200);
        ImGui::TextDisabled("%s", ready ? "ready" : s.value("needs", "").c_str());
        ImGui::Indent();
        ImGui::TextWrapped("%s", s.value("what", "").c_str());
        ImGui::Unindent();
        ImGui::PopID();
    }
    ImGui::SeparatorText("This machine");
    const json& m = inventory_["machine"];
    ImGui::Text("%s, %d threads", m.value("os", "").c_str(), m.value("cpu_threads", 0));
    ImGui::Text("OpenGL: %s", m["opengl"].is_string() ? m["opengl"].get<std::string>().c_str() : "-");
    if (m.contains("nvidia")) ImGui::Text("NVIDIA: %s", m["nvidia"].get<std::string>().c_str());

    ImGui::SeparatorText("Tools");
    if (ImGui::BeginTable("tools", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("tool", ImGuiTableColumnFlags_WidthFixed, 200);
        ImGui::TableSetupColumn("here?");
        ImGui::TableSetupColumn("for");
        ImGui::TableSetupColumn("how to get it");
        ImGui::TableHeadersRow();
        for (const auto& t : inventory_["tools"]) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(t.value("name", "").c_str());
            ImGui::TableNextColumn();
            const bool f = t.value("found", false);
            ImGui::TextColored(f ? good() : dim(), "%s", f ? ICON_FA_CIRCLE_CHECK " yes" : "no");
            if (t.contains("version")) { ImGui::SameLine(); ImGui::TextDisabled("%s", t["version"].get<std::string>().c_str()); }
            if (t.contains("models")) {
                std::string ms;
                for (const auto& x : t["models"]) ms += (ms.empty() ? "" : ", ") + x.value("name", "");
                ImGui::TextDisabled("models: %s", ms.empty() ? "none pulled" : ms.c_str());
            }
            if (t.contains("torch")) ImGui::TextDisabled("torch: %s", t["torch"].get<std::string>().c_str());
            if (t.contains("note")) ImGui::TextColored(warn(), "%s", t["note"].get<std::string>().c_str());
            ImGui::TableNextColumn();
            ImGui::TextWrapped("%s", t.value("used_for", "").c_str());
            ImGui::TableNextColumn();
            ImGui::TextWrapped("%s", f ? "" : t.value("get", "").c_str());
        }
        ImGui::EndTable();
    }
}

// ── Progress ────────────────────────────────────────────────────────────────

void Studio::draw_progress() {
    ImGui::SetNextWindowSize(ImVec2(900, 680), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Games", &win_progress_)) { ImGui::End(); return; }
    const std::string lib = library_dir();
    ImGui::TextWrapped("The games this library was made from, and how much of each is accounted for, three ways that measure "
                       "different things: the community's decompilation (code and data matched to the original, from decomp.dev), "
                       "what this library holds (turkeys, tables, characters), and what the game's wiki and Wikipedia say is in "
                       "the game that the library has a turkey or a table for. The rest is to do by hand, or by the next route.");
    ImGui::BeginDisabled(lib.empty());
    if (ImGui::Button(ICON_FA_ROTATE_RIGHT "  Measure") || (!progress_.is_object() && !lib.empty()))
        busy("Measuring the library against the wikis...", [this, lib] { progress_ = parse(run("effect progress " + maiz::arg(lib)).data); });
    ImGui::EndDisabled();
    ImGui::SameLine();
    // decomp.dev's figures: downloaded only when asked
    static json decomp;
    static bool decomp_read = false;
    if (!decomp_read) {
        decomp_read = true;
        std::ifstream f((host::config_dir() / "decomp_dev.json").string(), std::ios::binary);
        if (f) { std::stringstream ss; ss << f.rdbuf(); decomp = json::parse(ss.str(), nullptr, false);
                if (decomp.is_object() && decomp.contains("projects")) decomp = decomp["projects"]; }
    }
    if (ImGui::Button(ICON_FA_CLOUD_ARROW_DOWN "  Community progress (decomp.dev)"))
        busy("Asking decomp.dev...", [this] {
            json r = parse(run("effect decomp refresh").data);
            if (r.contains("error")) toast(r["error"].get<std::string>(), true);
            else {
                std::ifstream f((host::config_dir() / "decomp_dev.json").string(), std::ios::binary);
                std::stringstream ss;
                ss << f.rdbuf();
                decomp = json::parse(ss.str(), nullptr, false);
                if (decomp.is_object() && decomp.contains("projects")) decomp = decomp["projects"];
                toast("decomp.dev: " + std::to_string(r.value("projects", 0)) + " projects");
            }
        });
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("One request to decomp.dev/projects.json, kept in your settings folder.");
    if (lib.empty()) { ImGui::TextDisabled("open a library first"); ImGui::End(); return; }
    if (!progress_.is_object() || !progress_.contains("games")) { ImGui::End(); return; }
    // what the routes recorded loading (library/games.json)
    json loaded;
    {
        std::ifstream f((fs::path(lib) / "games.json").string(), std::ios::binary);
        if (f) { std::stringstream ss; ss << f.rdbuf(); loaded = json::parse(ss.str(), nullptr, false); }
    }
    auto fold = [](const std::string& s) {
        std::string o;
        for (char c : s) if (std::isalnum(static_cast<unsigned char>(c))) o += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return o;
    };
    for (const auto& g : progress_["games"]) {
        const json& w = g["wiki"];
        if (w.value("total", 0) == 0 && g["library"].value("turkeys", 0) < 3) continue;
        const std::string title = g.value("title", "");
        ImGui::PushID(title.c_str());
        ImGui::SeparatorText(title.c_str());
        // 1. the community's decompilation
        const json* dp = nullptr;
        if (decomp.is_array())
            for (const auto& p : decomp)
                if (fold(p.value("name", "")) == fold(title)) { dp = &p; break; }
        if (dp) {
            const json& m = (*dp)["measures"];
            const float code = static_cast<float>(m.value("matched_code_percent", 0.0)), data = static_cast<float>(m.value("matched_data_percent", 0.0));
            char l1[96], l2[96];
            std::snprintf(l1, sizeof l1, "code matched %.1f%%", code);
            std::snprintf(l2, sizeof l2, "data matched %.1f%%", data);
            ImGui::Text("Decompilation");
            ImGui::SameLine(130);
            ImGui::ProgressBar(code / 100.f, ImVec2(220, 0), l1);
            ImGui::SameLine();
            ImGui::ProgressBar(data / 100.f, ImVec2(220, 0), l2);
            ImGui::SameLine();
            ImGui::TextDisabled("%s/%s", dp->value("owner", "").c_str(), dp->value("repo", "").c_str());
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("decomp.dev, as of the commit %s", (*dp)["commit"].value("timestamp", std::string("?")).c_str());
        } else {
            ImGui::Text("Decompilation");
            ImGui::SameLine(130);
            ImGui::TextDisabled(decomp.is_array() ? "not tracked on decomp.dev" : "press Community progress to see the community's figures");
        }
        const float pct = static_cast<float>(w.value("percent", 0.0) / 100.0);
        char label[96];
        std::snprintf(label, sizeof label, "%d of %d known pages accounted for (%.0f%%)", w.value("recovered", 0), w.value("total", 0), pct * 100);
        ImGui::Text("The wiki");
        ImGui::SameLine(130);
        ImGui::ProgressBar(pct, ImVec2(-1, 0), label);
        if (w.contains("classes"))
            for (const auto& [cls, v] : w["classes"].items()) {
                const int t = v.value("total", 0), r = v.value("recovered", 0);
                ImGui::Text("%-7s", cls.c_str());
                ImGui::SameLine(80);
                char l2[64];
                std::snprintf(l2, sizeof l2, "%d / %d", r, t);
                ImGui::ProgressBar(t ? static_cast<float>(r) / t : 0.f, ImVec2(220, 0), l2);
                ImGui::SameLine();
                ImGui::TextDisabled("%s", cls == "thing" ? "as turkeys" : cls == "place" ? "as tables" : "as logic (not recovered yet)");
            }
        const json& wp = g["wikipedia"];
        if (wp.value("total", 0) > 0) ImGui::Text("Wikipedia's gameplay and plot links: %d of %d", wp.value("recovered", 0), wp.value("total", 0));
        const json& L = g["library"];
        ImGui::Text("Library: %d turkeys, %d playable characters (%d animation states), %d worlds", L.value("turkeys", 0), L.value("playable", 0),
                    L.value("animations", 0), L.value("worlds", 0));
        // where it was loaded from, and loading it again
        if (loaded.is_object())
            for (const auto& e : loaded.value("games", json::array()))
                if (e.value("title", "") == title) {
                    ImGui::TextDisabled("loaded %s from %s (route %s, setup %s)", e.value("loaded", "").c_str(), e.value("from", "").c_str(),
                                        e.value("route", "").c_str(), e.value("tier", "").c_str());
                    ImGui::SameLine();
                    if (ImGui::SmallButton(ICON_FA_ROTATE_RIGHT " load again")) {
                        const maiz::Result r = run("effect ingest " + maiz::arg(e.value("route", "")) + " " + maiz::arg(e.value("from", "")) + " " + maiz::arg(lib));
                        json j = parse(r.data);
                        if (j.contains("error")) toast(j["error"].get<std::string>(), true);
                        else { ingest_watch_ = lib; ingest_done_ = json(); ingest_prog_ = json(); ingest_poll_ = 0; ingest_tier_ = host::load_settings().load_tier; ingest_named_ = false; win_loadgame_ = true; }
                    }
                    break;
                }
        if (g.contains("project") && !g["project"].value("url", "").empty()) {
            ImGui::Text("Community project: %s", g["project"].value("name", "").c_str());
            ImGui::SameLine();
            if (ImGui::SmallButton(ICON_FA_LINK " open")) run("effect reveal " + maiz::arg(g["project"].value("url", "")));
        }
        if (ImGui::TreeNode("missing", "The most-mentioned pages not recovered yet (%zu shown)", w["missing"].size())) {
            for (const auto& m : w["missing"]) {
                ImGui::BulletText("%s", m.value("name", "").c_str());
                ImGui::SameLine();
                ImGui::TextDisabled("%s, mentioned by %d pages", m.value("section", "").c_str(), m.value("mentions", 0));
                if (ImGui::IsItemHovered() && !m.value("description", "").empty()) ImGui::SetTooltip("%s", m.value("description", "").c_str());
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
    ImGui::End();
}

// ── About ───────────────────────────────────────────────────────────────────

void Studio::draw_about() {
    ImGui::SetNextWindowSize(ImVec2(560, 380), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("About Void Squid", &win_about_, ImGuiWindowFlags_NoDocking)) {
        if (big) ImGui::PushFont(big);
        ImGui::Text("Void Squid %s", kVersion);
        if (big) ImGui::PopFont();
        ImGui::TextWrapped("The game engine of the Void family: build new worlds out of the games you own, and export new games.");
        ImGui::Text("Void Maiz %.*s   Void Core %.*s", static_cast<int>(maiz::kVoidmaizVersion.size()), maiz::kVoidmaizVersion.data(),
                    static_cast<int>(maiz::Core::core_version().size()), maiz::Core::core_version().data());
        ImGui::Text("GPU: %s", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
        ImGui::SeparatorText("Carries");
        ImGui::BulletText("Dear ImGui, GLFW (MIT, zlib), through Void Maiz");
        ImGui::BulletText("SQLite (public domain), nlohmann/json (MIT), stb_image (public domain)");
        ImGui::BulletText("Lato and JetBrains Mono (SIL OFL 1.1), Font Awesome 6 Free (SIL OFL 1.1)");
        ImGui::SeparatorText("Does not carry");
        ImGui::TextWrapped("Any game, any game's data, any decompiled code, any model weights. A library is yours and stays on your machine.");
    }
    ImGui::End();
}

// ── the Scene harness (rung 2.5) ────────────────────────────────────────────

void Studio::draw_scene_window() {
    Scene& sc = *app_.scene;
    ImGui::SetNextItemWidth(-200);
    ImGui::InputTextWithHint("##ink", "an .ink scene (Void Jelly's format)", ink_path_, sizeof ink_path_);
    ImGui::SameLine();
    if (ImGui::Button("Browse...")) {
        std::string p = pick_file(FileKind::Any, "Open an .ink scene");
        if (!p.empty()) std::snprintf(ink_path_, sizeof ink_path_, "%s", p.c_str());
    }
    ImGui::SameLine();
    if (ImGui::Button("Open") && ink_path_[0]) {
        ink_reply_ = run("effect open " + maiz::arg(ink_path_)).text();
        if (sc.loaded) run(std::string("config set ") + kInkKey + " " + maiz::arg(ink_path_));
    }
    if (sc.loaded) {
        const auto& dl = sc.gl.drawlist;
        ImGui::Text("%s   %zu objects, %zu polygons   GL nib: %zu draws, %zu triangles", sc.ink.name.c_str(), sc.ink.objects.size(),
                    sc.ink.polygon_count(), dl.draws.size(), dl.indices.size() / 3);
        if (ImGui::SmallButton("Frame all")) { cam_ = frame_all(dl, cam_); run(std::string("config set ") + kCameraKey + " " + maiz::arg(cam_.serialize())); }
        ImGui::SameLine();
        ImGui::TextDisabled("drag: orbit   right-drag: pan   wheel: zoom");
        if (ImGui::CollapsingHeader("Residual")) {
            const voidjelly::Residual& r = sc.gl.residual;
            ImGui::Text("nib %s   verdict: %s   %zu entries", r.nib.c_str(), voidjelly::verdict_name(r.verdict), r.losses.size());
            std::map<std::string, std::vector<const voidjelly::Loss*>> by_kind;
            for (const voidjelly::Loss& l : r.losses) by_kind[l.kind].push_back(&l);
            for (const auto& [kind, losses] : by_kind) {
                char head[96];
                std::snprintf(head, sizeof head, "%s (%zu)###%s", kind.c_str(), losses.size(), kind.c_str());
                if (ImGui::TreeNode(head)) {
                    for (const voidjelly::Loss* l : losses) ImGui::BulletText("%s  because %s", l->subject.c_str(), l->because.c_str());
                    ImGui::TreePop();
                }
            }
        }
        if (ImGui::CollapsingHeader("Look") && sc.has_look) {
            const voidjelly::LookResult& lr = sc.last_look;
            ImGui::Text("%zu in view, %zu off screen, %zu behind", lr.visible_total, lr.off_screen, lr.behind);
            for (const voidjelly::Seen& s : lr.visible) ImGui::BulletText("%s  r %.2f  d %.3g", s.id.c_str(), s.screen_radius, s.distance);
        }
    } else {
        if (!sc.error.empty()) ImGui::TextColored(bad(), "%s", sc.error.c_str());
        ImGui::TextWrapped("The Scene workspace is Void Squid's first job, still kept: open a Void Jelly scene, draw Jelly's GL "
                           "draw list, and show what the target could not carry (the residual). Make a test scene with the Console: "
                           "effect make shapes shapes.ink");
    }
    if (!ink_reply_.empty()) ImGui::TextDisabled("%s", ink_reply_.substr(0, 300).c_str());

    // the rest of the window is the 3D view (view3d.cpp), the same one the Turkey and the Table use
    if (sc.loaded) view3d("", {});
}

/* The 3D views draw inside ImGui's render pass now (view3d.cpp); nothing is drawn behind
 * the windows. Kept for the host's frame loop, which still calls it. */
void Studio::draw_world(int, int) {}

}  // namespace squid::studio
