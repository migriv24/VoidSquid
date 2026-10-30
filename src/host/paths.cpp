/* host/paths.cpp — see paths.hpp. */
#include "host/paths.hpp"

#include "json.hpp"

#include <cstdlib>
#include <fstream>
#include <system_error>

namespace squid::host {

namespace {
fs::path g_ship;

fs::path env_path(const char* name) {
    const char* v = std::getenv(name);
    return (v && *v) ? fs::path(v) : fs::path();
}
}  // namespace

void set_ship_dir(const char* argv0) {
    std::error_code ec;
    if (argv0 && *argv0) {
        fs::path p = fs::absolute(argv0, ec);
        if (!ec) g_ship = p.parent_path();
    }
    if (g_ship.empty()) g_ship = fs::current_path(ec);
}

fs::path ship_dir() { return g_ship.empty() ? fs::current_path() : g_ship; }

fs::path config_dir() {
    // a test run keeps its own settings, so it never edits the person's recent projects
    if (const fs::path o = env_path("SQUID_CONFIG_DIR"); !o.empty()) {
        std::error_code ec;
        fs::create_directories(o, ec);
        return o;
    }
#ifdef _WIN32
    fs::path base = env_path("APPDATA");
    if (base.empty()) base = env_path("USERPROFILE");
    fs::path dir = base / "VoidSquid";
#else
    fs::path base = env_path("XDG_CONFIG_HOME");
    if (base.empty()) base = env_path("HOME") / ".config";
    fs::path dir = base / "voidsquid";
#endif
    std::error_code ec;
    fs::create_directories(dir, ec);
    return dir;
}

fs::path tools_dir() {
    const Settings s = load_settings();
    return s.tools_dir.empty() ? config_dir() / "tools" : fs::path(s.tools_dir);
}

fs::path projects_dir() {
    {
        const Settings s = load_settings();
        if (!s.projects_dir.empty()) return fs::path(s.projects_dir);
    }
#ifdef _WIN32
    fs::path home = env_path("USERPROFILE");
#else
    fs::path home = env_path("HOME");
#endif
    std::error_code ec;
    if (fs::is_directory(home / "Documents", ec)) return home / "Documents" / "Void Squid";
    return home / "Void Squid";
}

Settings load_settings() {
    Settings s;
    std::ifstream in(config_dir() / "settings.json");
    if (!in) return s;
    try {
        nlohmann::json j = nlohmann::json::parse(in);
        if (j.contains("recent") && j["recent"].is_array())
            for (const auto& r : j["recent"]) if (r.is_string()) s.recent.push_back(r.get<std::string>());
        s.light_mode = j.value("light_mode", false);
        s.ui_scale = j.value("ui_scale", 1.0f);
        s.stars = j.value("stars", true);
        s.runtime_dir = j.value("runtime_dir", "");
        s.bicho_root = j.value("bicho_root", "");
        s.toolbox_dir = j.value("toolbox_dir", "");
        s.ollama_model = j.value("ollama_model", "");
        s.projects_dir = j.value("projects_dir", "");
        s.load_tier = j.value("load_tier", std::string("minimum"));
        s.tools_dir = j.value("tools_dir", "");
    } catch (...) {
        return Settings{};   // a broken settings file is not worth refusing to start over
    }
    return s;
}

void save_settings(const Settings& s) {
    nlohmann::json j;
    j["recent"] = s.recent;
    j["light_mode"] = s.light_mode;
    j["ui_scale"] = s.ui_scale;
    j["stars"] = s.stars;
    j["runtime_dir"] = s.runtime_dir;
    j["bicho_root"] = s.bicho_root;
    j["toolbox_dir"] = s.toolbox_dir;
    j["ollama_model"] = s.ollama_model;
    j["projects_dir"] = s.projects_dir;
    j["load_tier"] = s.load_tier;
    j["tools_dir"] = s.tools_dir;
    const fs::path p = config_dir() / "settings.json";
    const fs::path tmp = p.string() + ".tmp";
    {
        std::ofstream out(tmp);
        if (!out) return;
        out << j.dump(1);
    }
    std::error_code ec;
    fs::rename(tmp, p, ec);
}

void remember_recent(const std::string& path) {
    Settings s = load_settings();
    std::error_code ec;
    const std::string abs = fs::absolute(path, ec).lexically_normal().string();
    std::vector<std::string> out{abs};
    for (const auto& r : s.recent)
        if (r != abs && out.size() < 12) out.push_back(r);
    s.recent = out;
    save_settings(s);
}

Profile load_profile() {
    Profile p;
    std::ifstream in(config_dir() / "profile.json");
    if (in) {
        try {
            nlohmann::json j = nlohmann::json::parse(in);
            p.name = j.value("name", "");
            p.about = j.value("about", "");
            if (j.contains("color") && j["color"].is_array() && j["color"].size() == 3)
                for (int i = 0; i < 3; ++i) p.color[static_cast<size_t>(i)] = j["color"][static_cast<size_t>(i)].get<float>();
        } catch (...) {}
    }
    if (p.name.empty()) {
#ifdef _WIN32
        const char* u = std::getenv("USERNAME");
#else
        const char* u = std::getenv("USER");
#endif
        p.name = (u && *u) ? u : "person";
    }
    return p;
}

void save_profile(const Profile& p) {
    nlohmann::json j = {{"name", p.name}, {"about", p.about}, {"color", {p.color[0], p.color[1], p.color[2]}}};
    std::ofstream out(config_dir() / "profile.json");
    if (out) out << j.dump(1);
}

std::string first_existing(const std::vector<fs::path>& candidates) {
    std::error_code ec;
    for (const auto& c : candidates)
        if (!c.empty() && fs::exists(c, ec)) return fs::absolute(c, ec).lexically_normal().string();
    return {};
}

}  // namespace squid::host
