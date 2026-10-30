/* studio/platform.cpp — see platform.hpp. */
#include "studio/platform.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#include "stb_image.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#endif
#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <map>

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

namespace squid::studio {

namespace {
std::string g_problem;

#ifdef _WIN32
std::wstring widen(const std::string& s) {
    if (s.empty()) return {};
    const int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring w(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), w.data(), n);
    return w;
}
std::string narrow(const wchar_t* w) {
    const int n = WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
    if (n <= 1) return {};
    std::string s(n - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w, -1, s.data(), n, nullptr, nullptr);
    return s;
}
/* Filters are double-NUL-terminated lists; built as wide strings with explicit NULs. */
std::wstring filter_for(FileKind k) {
    std::wstring f;
    auto add = [&](const wchar_t* label, const wchar_t* pattern) {
        f += label; f.push_back(L'\0'); f += pattern; f.push_back(L'\0');
    };
    switch (k) {
        case FileKind::Project: add(L"Void Squid project (.squid)", L"*.squid"); break;
        case FileKind::Library: add(L"Asset database (assets.db)", L"*.db"); break;
        case FileKind::Model: add(L"Models (.obj, .glb, .gltf)", L"*.obj;*.glb;*.gltf"); break;
        case FileKind::Audio: add(L"Audio (.wav, .ogg, .mp3)", L"*.wav;*.ogg;*.mp3"); break;
        case FileKind::Game: add(L"Games (ROMs, disc images)", L"*.z64;*.n64;*.v64;*.nds;*.gba;*.gb;*.gbc;*.nes;*.sfc;*.smc;*.iso;*.rvz;*.gcm;*.wbfs;*.exe"); break;
        default: break;
    }
    add(L"All files", L"*.*");
    f.push_back(L'\0');
    return f;
}
#else
std::string shell_quoted(const std::string& s) {
    std::string q = "'";
    for (char c : s) q += (c == '\'') ? std::string("'\\''") : std::string(1, c);
    return q + "'";
}
bool have_tool(const char* tool) {
    return std::system((std::string("command -v ") + tool + " >/dev/null 2>&1").c_str()) == 0;
}
std::string run_dialog(const std::string& cmd) {
    std::string out;
    FILE* p = popen((cmd + " 2>/dev/null").c_str(), "r");
    if (!p) return out;
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof buf, p)) > 0) out.append(buf, n);
    pclose(p);
    while (!out.empty() && (out.back() == '\n' || out.back() == '\r')) out.pop_back();
    return out;
}
const char* kNoDialog =
    "No file dialog is available on this computer. Install zenity (for example `sudo apt install zenity`) or kdialog, then try again.";
#endif
}  // namespace

std::string dialog_problem() {
    std::string p = g_problem;
    g_problem.clear();
    return p;
}

std::string pick_file(FileKind kind, const std::string& title) {
#ifdef _WIN32
    wchar_t buf[MAX_PATH * 4] = {};
    const std::wstring filter = filter_for(kind), wt = widen(title);
    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof ofn;
    ofn.lpstrFile = buf;
    ofn.nMaxFile = MAX_PATH * 4;
    ofn.lpstrFilter = filter.c_str();
    ofn.lpstrTitle = wt.c_str();
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    if (GetOpenFileNameW(&ofn)) return narrow(buf);
    return {};
#else
    (void)kind;
    const std::string t = shell_quoted(title);
    if (have_tool("zenity")) return run_dialog("zenity --file-selection --title=" + t);
    if (have_tool("kdialog")) return run_dialog("kdialog --getopenfilename \"$HOME\"");
    if (have_tool("yad")) return run_dialog("yad --file --title=" + t);
    if (have_tool("qarma")) return run_dialog("qarma --file-selection --title=" + t);
    g_problem = kNoDialog;
    return {};
#endif
}

std::string pick_folder(const std::string& title) {
#ifdef _WIN32
    const HRESULT co = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    wchar_t buf[MAX_PATH] = {};
    const std::wstring wt = widen(title);
    BROWSEINFOW bi = {};
    bi.lpszTitle = wt.c_str();
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    std::string picked;
    if (LPITEMIDLIST pidl = SHBrowseForFolderW(&bi)) {
        if (SHGetPathFromIDListW(pidl, buf)) picked = narrow(buf);
        CoTaskMemFree(pidl);
    }
    if (SUCCEEDED(co)) CoUninitialize();
    return picked;
#else
    const std::string t = shell_quoted(title);
    if (have_tool("zenity")) return run_dialog("zenity --file-selection --directory --title=" + t);
    if (have_tool("kdialog")) return run_dialog("kdialog --getexistingdirectory \"$HOME\"");
    if (have_tool("yad")) return run_dialog("yad --file --directory --title=" + t);
    if (have_tool("qarma")) return run_dialog("qarma --file-selection --directory --title=" + t);
    g_problem = kNoDialog;
    return {};
#endif
}

std::string save_file(const std::string& suggested, FileKind kind) {
    std::string picked;
#ifdef _WIN32
    wchar_t buf[MAX_PATH * 4] = {};
    const std::wstring s = widen(suggested);
    wcsncpy(buf, s.c_str(), MAX_PATH * 4 - 1);
    const std::wstring filter = filter_for(kind);
    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof ofn;
    ofn.lpstrFile = buf;
    ofn.nMaxFile = MAX_PATH * 4;
    ofn.lpstrFilter = filter.c_str();
    ofn.lpstrDefExt = kind == FileKind::Project ? L"squid" : nullptr;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
    if (GetSaveFileNameW(&ofn)) picked = narrow(buf);
#else
    if (have_tool("zenity"))
        picked = run_dialog("zenity --file-selection --save --confirm-overwrite --filename=" + shell_quoted(suggested));
    else if (have_tool("kdialog"))
        picked = run_dialog("kdialog --getsavefilename " + shell_quoted(suggested));
    else if (have_tool("yad"))
        picked = run_dialog("yad --file --save --confirm-overwrite --filename=" + shell_quoted(suggested));
    else
        g_problem = kNoDialog;
    if (kind == FileKind::Project && !picked.empty() && std::filesystem::path(picked).extension().empty()) picked += ".squid";
#endif
    return picked;
}

// ── textures ──
namespace {
std::map<std::string, Texture>& cache() {
    static std::map<std::string, Texture> c;
    return c;
}
}  // namespace

Texture texture(const std::string& abs_path) {
    auto& c = cache();
    auto it = c.find(abs_path);
    if (it != c.end()) return it->second;
    Texture t;
    int w = 0, h = 0, comp = 0;
    unsigned char* px = stbi_load(abs_path.c_str(), &w, &h, &comp, 4);
    if (px) {
        GLuint id = 0;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
        stbi_image_free(px);
        t = {id, w, h};
    }
    c[abs_path] = t;   // a failure is cached too: one missing file is not retried every frame
    return t;
}

void drop_textures() {
    for (auto& [p, t] : cache())
        if (t.id) { GLuint id = t.id; glDeleteTextures(1, &id); }
    cache().clear();
}

}  // namespace squid::studio
