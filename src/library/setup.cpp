/* library/setup.cpp — see setup.hpp. Looks; never installs. */
#include "library/setup.hpp"

#include "host/paths.hpp"
#include "host/process.hpp"

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <sstream>
#include <thread>

namespace squid::library {

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace {
std::string g_renderer;

std::string first_line(const std::string& s) {
    std::string l = s.substr(0, s.find('\n'));
    while (!l.empty() && (l.back() == '\r' || l.back() == ' ')) l.pop_back();
    return l;
}

std::string env(const char* k) {
    const char* v = std::getenv(k);
    return v ? v : "";
}

/* Void Bicho's environment: VOIDBICHO_ROOT, else beside this checkout, else Documents. */
fs::path bicho_root() {
    std::vector<fs::path> c;
    if (const std::string set = host::load_settings().bicho_root; !set.empty()) c.push_back(set);   // Settings says
    if (!env("VOIDBICHO_ROOT").empty()) c.push_back(env("VOIDBICHO_ROOT"));
    const fs::path ship = host::ship_dir();
    c.push_back(ship / ".." / ".." / ".." / "VoidBicho");   // VoidSquid/build/bin -> Projects/VoidBicho
    c.push_back(ship / ".." / "VoidBicho");
#ifdef _WIN32
    c.push_back(fs::path(env("USERPROFILE")) / "Documents" / "Projects" / "VoidBicho");
#else
    c.push_back(fs::path(env("HOME")) / "Documents" / "Projects" / "VoidBicho");
#endif
    const std::string f = host::first_existing(c);
    return f.empty() ? fs::path() : fs::path(f);
}

fs::path bicho_python(const fs::path& root) {
#ifdef _WIN32
    return root / ".venv" / "Scripts" / "python.exe";
#else
    return root / ".venv" / "bin" / "python";
#endif
}

/* Claude Code: on PATH, or inside an editor's extension (VS Code, VSCodium, Cursor ship
 * their own copy of the binary and do not put it on PATH). */
std::string find_claude(std::string& where) {
    std::string p = host::which("claude");
    if (!p.empty()) { where = "on PATH"; return p; }
#ifdef _WIN32
    const fs::path home = env("USERPROFILE");
    const char* exe = "claude.exe";
#else
    const fs::path home = env("HOME");
    const char* exe = "claude";
#endif
    std::error_code ec;
    for (const char* ed : {".vscode", ".vscode-oss", ".cursor", ".windsurf"}) {
        const fs::path ext = home / ed / "extensions";
        if (!fs::is_directory(ext, ec)) continue;
        for (const auto& e : fs::directory_iterator(ext, ec)) {
            if (e.path().filename().string().rfind("anthropic.claude-code", 0) != 0) continue;
            const fs::path b = e.path() / "resources" / "native-binary" / exe;
            if (fs::exists(b, ec)) { where = std::string("inside the editor extension (") + ed + "), not on PATH"; return b.string(); }
        }
    }
    return {};
}

json tool(const std::string& id, const std::string& name, const std::string& path, const std::string& version,
          const std::string& used_for, const std::string& get) {
    json t = {{"id", id}, {"name", name}, {"found", !path.empty()}, {"used_for", used_for}, {"get", get}};
    if (!path.empty()) t["path"] = path;
    if (!version.empty()) t["version"] = version;
    return t;
}
}  // namespace

void set_gl_renderer(const std::string& r) { g_renderer = r; }

std::string ollama_path() {
    std::string p = host::which("ollama");
    if (!p.empty()) return p;
#ifdef _WIN32
    const fs::path d = fs::path(env("LOCALAPPDATA")) / "Programs" / "Ollama" / "ollama.exe";
#else
    const fs::path d = "/usr/local/bin/ollama";
#endif
    std::error_code ec;
    return fs::exists(d, ec) ? d.string() : std::string();
}

json inventory(bool deep) {
    json inv;
    // ── the machine ──
    json m;
#ifdef _WIN32
    m["os"] = "Windows";
#elif defined(__APPLE__)
    m["os"] = "macOS";
#else
    m["os"] = "Linux";
#endif
    m["cpu_threads"] = std::thread::hardware_concurrency();
    m["opengl"] = g_renderer.empty() ? json("not asked (headless)") : json(g_renderer);
    {
        const auto smi = host::capture({"nvidia-smi", "--query-gpu=name,memory.total,driver_version", "--format=csv,noheader"}, 4096, 6000);
        if (smi.ran && smi.exit_code == 0) m["nvidia"] = first_line(smi.out);
    }
    inv["machine"] = m;

    // ── the tools ──
    json tools = json::array();
    auto version_of = [](const std::vector<std::string>& argv) {
        const auto c = host::capture(argv, 4096, 8000);
        return c.ran ? first_line(c.out) : std::string();
    };
    std::string py = host::which("python");
    if (py.empty()) py = host::which("python3");
    tools.push_back(tool("python", "Python 3", py, py.empty() ? "" : version_of({py, "--version"}),
                         "the toolbox's ingest routes; Void Bicho", "python.org (3.10 or newer), or your package manager"));
    tools.push_back(tool("git", "Git", host::which("git"), "", "fetching community decompilations", "git-scm.com"));
    tools.push_back(tool("curl", "curl", host::which("curl"), "", "refreshing the community decompilation list",
                         "ships with Windows 10 and later, and with most Linux systems"));

    const std::string ol = ollama_path();
    json oll = tool("ollama", "Ollama", ol, "", "the local-model setup: naming, tagging, reading logs",
                    "ollama.com/download, then `ollama pull qwen3:0.6b` (small) or a larger model your memory allows");
    if (!ol.empty()) {
        const auto list = host::capture({ol, "list"}, 16384, 8000);
        json models = json::array();
        if (list.ran && list.exit_code == 0) {
            std::istringstream in(list.out);
            std::string line;
            std::getline(in, line);   // header
            while (std::getline(in, line)) {
                std::istringstream ls(line);
                std::string name, id, size, unit;
                if (ls >> name >> id >> size >> unit) models.push_back({{"name", name}, {"size", size + " " + unit}});
            }
            oll["running"] = true;
        } else {
            oll["running"] = false;
            oll["note"] = "installed, but the server did not answer: start Ollama, then check again";
        }
        oll["models"] = models;
    }
    tools.push_back(oll);

    const fs::path broot = bicho_root();
    json bicho = tool("bicho", "Void Bicho (vision: CLIP, SAM)", broot.empty() ? "" : broot.string(), "",
                      "tagging thumbnails by what they look like", "see SETUP.md, \"Vision\"");
    if (!broot.empty()) {
        std::error_code ec;
        const fs::path bp = bicho_python(broot);
        bicho["environment"] = fs::exists(bp, ec);
        if (deep && fs::exists(bp, ec)) {
            const auto t = host::capture({bp.string(), "-c",
                "import torch;print('cuda' if torch.cuda.is_available() else 'cpu', torch.__version__, torch.cuda.get_device_name(0) if torch.cuda.is_available() else '')"},
                4096, 120000);
            bicho["torch"] = t.ran ? first_line(t.out) : "did not run";
        }
    }
    tools.push_back(bicho);

    {
        std::string where;
        const std::string cl = find_claude(where);
        json t = tool("claude", "Claude Code", cl, "", "the agent setup: drive Void Squid as a library", "claude.com/claude-code");
        if (!cl.empty()) t["note"] = where;
        tools.push_back(t);
    }
    tools.push_back(tool("codex", "Codex CLI", host::which("codex"), "", "the agent setup", "github.com/openai/codex"));
    tools.push_back(tool("um", "universal-modder (um)", host::which("um"), "", "fingerprinting PC games; the agent's modding skills",
                         "github.com/rehan-remade/universal-modder"));
    tools.push_back(tool("rea", "REA", host::which("rea"), "", "agent-driven reverse engineering over Ghidra and others",
                         "github.com/morluto/rea (npx rea-agents)"));
    tools.push_back(tool("ghidra", "Ghidra (headless)", host::which("analyzeHeadless"), "", "decompiling native code nobody has decompiled",
                         "ghidra-sre.org"));
    const std::string dkp = env("DEVKITPRO");
    tools.push_back(tool("devkitpro", "devkitPro", dkp, "", "building Nintendo DS artifacts (later)", "devkitpro.org"));
    tools.push_back(tool("blender", "Blender", host::which("blender"), "", "authoring scenes for Void Jelly", "blender.org"));
    inv["tools"] = tools;

    // ── the setups ──
    auto found = [&](const char* id) {
        for (const auto& t : tools) if (t["id"] == id) return t.value("found", false);
        return false;
    };
    bool has_model = false;
    for (const auto& t : tools)
        if (t["id"] == "ollama" && t.contains("models") && !t["models"].empty()) has_model = true;
    json setups = json::array();
    setups.push_back({{"id", "minimum"}, {"name", "Minimum"}, {"ready", true},
                      {"what", "Recon, the catalogue of known decompilations, the routes, and checks. No model and no GPU."},
                      {"needs", "nothing more"}});
    setups.push_back({{"id", "local"}, {"name", "Local model"}, {"ready", has_model},
                      {"what", "A model on this machine helps name, tag and read logs; the same checks hold it to account."},
                      {"needs", has_model ? "ready" : (found("ollama") ? "pull a model: `ollama pull qwen3:0.6b`" : "install Ollama and pull a model")}});
    setups.push_back({{"id", "vision"}, {"name", "Vision"}, {"ready", found("bicho") && found("python")},
                      {"what", "Void Bicho tags thumbnails by what they look like (CLIP) and cuts out shapes (SAM)."},
                      {"needs", found("bicho") ? "ready (a GPU makes it faster; the CPU works)" : "Python and Void Bicho's environment (SETUP.md)"}});
    setups.push_back({{"id", "agent"}, {"name", "Coding agent"}, {"ready", found("claude") || found("codex")},
                      {"what", "An agent drives Void Squid's tools from a terminal and can decompile what no community has."},
                      {"needs", (found("claude") || found("codex")) ? "ready: run `squid --describe` in the agent" : "install Claude Code or Codex"}});
    setups.push_back({{"id", "api"}, {"name", "Hosted model (API key)"}, {"ready", false}, {"planned", true},
                      {"what", "A hosted model through your own key. Planned, not built."}, {"needs", "not built yet"}});
    inv["setups"] = setups;

    const std::time_t now = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof buf, "%Y-%m-%d %H:%M", std::localtime(&now));
    inv["checked"] = buf;
    return inv;
}

}  // namespace squid::library
