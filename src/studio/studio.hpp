/* studio/studio.hpp — Void Squid's window: the studio (pillar 2) on its desktop host.
 *
 * WHAT THIS IS (okf/design/void-squid-0.1-the-studio.md). A docked application in the
 * shape Void Hormiga proved: a menu bar (File, Edit, View, Windows, Help), a row of
 * WORKSPACES, and windows that are always available. The author's framing for the
 * workspaces, 2026-10-12: "the windows are always available for all of them, but they
 * accomplish different things."
 *
 *   Start      what to do first: open a library, load a game, open a project
 *   Turkey     careful inspection of one thing: its tags and where they came from,
 *              its kind, its checks, its sounds
 *   Table      the worlds: which ones, in what order, what plays there; Play
 *   Feast      the player: who, how fast, how high, how many lives
 *   Data       every mantle of the project, as a table, edited through Void Maiz's
 *              widget registry (Hormiga's Data section, carried over)
 *   Scene      the rung 2.5 harness: an .ink through Void Jelly, its residual, its look
 *
 *   Shelf, Inspector, Console, Load a game, Setup — windows, in every workspace.
 *
 * THE ONE-SYNC RULE. Nothing here changes the project except by dispatching a command
 * through the Session; after every dispatch the views re-project. A view keeps only
 * view state (what is selected, what is scrolled to). So the console, an agent with
 * the CLI, and a click are the same edit in the same journal, and Void Maiz's
 * collaborative editing can carry any of them.
 *
 * THE STUDIO OWNS NO PIXELS of the game: the thumbnails were rendered by the toolbox,
 * the world map is positions read from the world file, and the scene is Jelly's draw
 * list. No loop over a mesh's vertices is anywhere in here (CLAUDE.md rule 4).
 */
#pragma once

#include "app.hpp"
#include "camera.hpp"

#include "json.hpp"
#include "voidmaiz/headless.hpp"
#include "voidmaiz/scene.hpp"
#include "voidmaiz/widget.hpp"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

struct ImFont;

namespace squid::studio {

using json = nlohmann::json;

enum class Workspace { Start, Turkey, Table, Feast, Data, Scene };

struct Toast {
    std::string text;
    bool bad = false;
    double until = 0;
};

class Studio {
public:
    explicit Studio(App& app);
    ~Studio();

    /* After the GL context and ImGui exist. Opens `project` (or the last one, or an
     * untitled one) and, when given, a scene. */
    bool start(const std::string& project, const std::string& ink);
    /* One frame of UI. Call between ImGui::NewFrame and ImGui::Render. */
    void frame();
    /* The 3D scene, when the Scene workspace shows one: after glClear, before ImGui. */
    void draw_world(int width, int height);
    void shutdown();

    /* The screenshot walk (SQUID_SHOTS=<dir>): each workspace in turn, on the project's
     * own data. Returns the name to save this frame's picture as, or "" for none, and
     * sets `done` when the walk is over. A regression check a person can look at. */
    std::string autotest(int frame, bool& done);

    std::function<void(const std::string&)> on_title;
    std::function<void()> on_quit;
    ImFont* mono = nullptr;
    ImFont* big = nullptr;

private:
    // ── the project ──
    bool open_project(const std::string& path);
    void new_project();
    void save_project();
    void save_project_as();
    void open_library(const std::string& path);
    std::string project_path() const;
    std::string library_dir() const;

    // ── dispatching ──
    maiz::Result run(const std::string& command);
    /* Make `mantle` active, then run the commands (one undo frame when several). */
    void edit(const std::string& mantle, const std::vector<std::string>& commands);
    void refresh();                   // re-project when dirty
    const maiz::Scene& scene_of(const std::string& mantle);
    const json* rune(const std::string& mantle, const std::string& name) const;
    const json* mantle(const std::string& name) const;
    void busy(const std::string& label, std::function<void()> fn);
    void toast(const std::string& text, bool bad = false);

    // ── the frame ──
    void apply_theme();
    void draw_stars();
    void draw_menu();
    void draw_workspace_bar();
    void dock_layout();
    void shortcuts();
    void draw_toasts();
    void draw_busy();

    // ── workspaces and windows (one file each) ──
    void draw_start();
    void draw_turkey();
    void draw_shelf();
    void draw_table();
    void draw_table_3d(const std::string& world_file);   // the Table's 3D view, through Void Jelly
    /* A 3D view in the current window: loads `key` once through `load` (an effect), then
     * draws the held scene in the space that is left, through an ImGui draw callback so it
     * sits in the window's own stacking order. Orbit, pan and zoom by mouse. */
    void view3d(const std::string& key, const std::function<bool()>& load);
    static void gl_callback(const struct ImDrawList* list, const struct ImDrawCmd* cmd);
    void render_view();
    unsigned gl_texture(const std::string& hex);
    void draw_feast();
    void draw_data();
    void draw_scene_window();
    void draw_inspector();
    void draw_console();
    void draw_loadgame();
    void draw_setup();
    void draw_about();
    void draw_profile();
    void draw_progress();
    void draw_settings();

    // helpers shared by the windows
    void tag_chips(const std::string& mantle, const json& rune, bool editable);
    void add_tag(const std::string& mantle, const std::string& rune, const std::string& tag);
    void remove_tag(const std::string& mantle, const std::string& rune, const std::string& tag);
    bool thumbnail(const json& turkey, float size);
    bool thumb_at(const json& turkey, float x, float y, float size);   // drawn on the window's list, no layout
    void focus(Workspace w);
    std::string turkey_label(const json& r) const;

    App& app_;
    std::unique_ptr<maiz::Session> session_;
    std::string session_error_;
    std::string locked_path_;          // a project another (or a crashed) window holds: ask the person
    void draw_lock_prompt();
    maiz::WidgetRegistry widgets_;

    // the projection: the whole state document, parsed, and a Maiz Scene per mantle
    bool dirty_ = true;
    json doc_;
    std::map<std::string, maiz::Scene> scenes_;
    std::map<std::string, bool> scene_dirty_;
    int undo_depth_ = 0;

    // view state
    Workspace ws_ = Workspace::Start;
    Workspace focus_request_ = Workspace::Start;
    bool focus_pending_ = true;
    bool dock_seeded_ = false;
    bool win_shelf_ = true, win_inspector_ = true, win_console_ = true, win_loadgame_ = false,
         win_setup_ = false, win_about_ = false, win_demo_ = false, win_profile_ = false, win_settings_ = false;
    bool win_progress_ = false;
    json progress_;
    int table_view_ = 0;      // the Table's views: 0 the map from above, 1 the objects, 2 in 3D
    int map_pick_ = -1;       // the object picked on the map or in the list
    std::string sel_turkey_, sel_table_, data_mantle_ = "turkeys", data_rune_;
    std::string inspect_mantle_, inspect_rune_;   // what the Inspector shows
    char shelf_filter_[256] = {};
    std::string shelf_kind_, shelf_game_;
    char new_tag_[64] = {};
    char data_filter_[256] = {};
    char console_line_[1024] = {};
    std::vector<std::string> console_history_;
    int console_history_pos_ = -1;
    std::vector<std::string> transcript_;

    // Load a game, Setup
    char game_path_[1024] = {};
    json plan_;
    json inventory_;
    std::string catalogue_note_;
    char ingest_lib_[1024] = {};        // where a route brings the game
    std::string ingest_watch_;          // the library a running route writes into ("" = none)
    std::vector<std::string> ingest_tail_;
    double ingest_poll_ = 0;
    json ingest_done_;
    json ingest_prog_;
    json proposals_;                    // the library's pilot_proposals.json, by turkey (read when the library changes)
    std::string proposals_lib_;                  // the route's steps, states and fractions (ingest.progress.json)
    std::string ingest_tier_;           // the setup the running route was started with
    bool ingest_named_ = false;          // the local-model setup's naming step has been started

    // Table: the world file of the selected table, read for its map
    std::string map_file_;
    json map_;
    float map_zoom_ = 1.0f;
    float map_pan_x_ = 0, map_pan_y_ = 0;

    // the 3D view: what is loaded into the held scene, and where it is drawn this frame
    std::string view_source_;
    float view_min_[2] = {0, 0}, view_max_[2] = {0, 0};
    std::map<std::string, unsigned> gltex_;
    int turkey_view_ = 0;     // the Turkey's views: 0 the card, 1 in 3D

    // Scene (rung 2.5)
    Camera3D cam_;
    uint64_t framed_generation_ = 0;
    double look_due_ = -1;
    char ink_path_[1024] = {};
    std::string ink_reply_;

    // feedback
    std::vector<Toast> toasts_;
    std::string busy_label_;
    std::function<void()> busy_fn_;
    int busy_frames_ = 0;
    bool light_ = false;
    bool stars_ = true;
};

}  // namespace squid::studio
