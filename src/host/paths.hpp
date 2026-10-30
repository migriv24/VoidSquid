/* host/paths.hpp — where things live on this machine, Windows and Linux.
 *
 * Three places, never confused (Void Hormiga learned each of these the hard way):
 *
 *   ship_dir      beside the binary: the fonts, the seed catalogue, the icon. What a
 *                 release carries. Never the working directory, because nobody launches
 *                 an installed application from a checkout.
 *   config_dir    this person's settings on this machine: the recent projects, the
 *                 cached decompilation list. %APPDATA%\VoidSquid, or
 *                 $XDG_CONFIG_HOME/voidsquid (~/.config/voidsquid).
 *   projects_dir  where a new project is suggested: Documents\Void Squid, or
 *                 ~/Documents/Void Squid (~/Void Squid when there is no Documents).
 *
 * None of these is ever inside the repository, and none of them holds game data unless
 * the person put it there (okf/design/void-squid-0.1-the-studio.md, "Release").
 */
#pragma once

#include <array>
#include <filesystem>
#include <string>
#include <vector>

namespace squid::host {

namespace fs = std::filesystem;

/* Set once from argv[0] at start. */
void set_ship_dir(const char* argv0);
fs::path ship_dir();
fs::path config_dir();     // created on first call
fs::path projects_dir();   // not created until a project is saved there

/* The person's settings: recent projects, newest first, and a few preferences.
 * A small JSON file in config_dir(); a missing or broken file reads as empty. */
struct Settings {
    std::vector<std::string> recent;   // .squid paths
    bool light_mode = false;
    float ui_scale = 1.0f;
    bool stars = true;
    // where things are, when the person says so (empty: Void Squid looks for itself)
    std::string runtime_dir;            // the folder holding the runtime Play starts
    std::string bicho_root;             // Void Bicho's checkout (vision)
    std::string toolbox_dir;            // the ingest routes (today: ../ds-mashup/toolbox)
    std::string ollama_model;           // the local model the local-model setup asks
    std::string projects_dir;           // where a new project is suggested
    std::string load_tier = "minimum";  // Load a game's setup: minimum (no model), local (a local model names things), vision (Void Bicho)
    std::string tools_dir;              // where Load a game keeps the tools it downloads (empty: <settings>/tools)
};
std::filesystem::path tools_dir();   // Load a game's downloaded tools

/* The person, on this machine: who signs the journal, and who a Coral reef will say
 * they are. Local only; nothing here leaves the machine. */
struct Profile {
    std::string name;                   // shown, and the journal's actor ("human:<name>")
    std::string about;
    std::array<float, 3> color{0.34f, 0.84f, 0.87f};
};
Profile load_profile();
void save_profile(const Profile& p);
Settings load_settings();
void save_settings(const Settings& s);
/* Put `path` first in the recent list (deduplicated, at most 12) and save. */
void remember_recent(const std::string& path);

/* The first existing path among candidates, or "". */
std::string first_existing(const std::vector<fs::path>& candidates);

}  // namespace squid::host
