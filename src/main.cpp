/* main.cpp — Void Squid's two front-ends, one application.
 *
 *     squid                         the studio, on the last project (or a new one)
 *     squid my-game.squid           the studio, on that project
 *     squid scene.ink               the studio, with that scene open in the Scene workspace
 *     squid --describe              the briefing, for an agent arriving cold
 *     squid <command...>            the headless CLI (maiz::run_cli), e.g.
 *     squid --state my-game.squid --allow-effects=setup effect setup
 *     squid --state my-game.squid --import-library <folder>
 *
 * Both halves take the SAME maiz::HostApp (app.hpp). The window is a Human session so
 * that a person outranks an agent on the same project (headless.hpp, SessionKind).
 *
 * `--import-library` is the one front-end composition here, and it adds no verb: it is
 * the two steps the studio's File > Open library runs (`effect library import`, then the
 * splice into the document), so an agent can do from a terminal exactly what a person
 * does from the menu, journalled the same way.
 */
#include "app.hpp"
#include "host/paths.hpp"
#include "library/library.hpp"
#include "library/setup.hpp"
#include "studio/studio.hpp"

#include "voidmaiz/glhost.hpp"
#include "IconsFontAwesome6.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

#ifdef _WIN32
/* Ask for the discrete GPU on a laptop with two (NVIDIA Optimus, AMD PowerXpress).
 * Windows hands an OpenGL program the integrated one unless it exports these; found in
 * the Goomba Galaxy exercise on 2026-10-12, where both programs ran on the Iris Xe. */
extern "C" {
__declspec(dllexport) unsigned long NvOptimusEnablement = 1;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

#ifndef SQUID_SOURCE_DIR
#define SQUID_SOURCE_DIR "."
#endif

namespace {

namespace fs = std::filesystem;

bool ends_with(const std::string& s, const char* suffix) {
    const size_t n = std::strlen(suffix);
    return s.size() >= n && s.compare(s.size() - n, n, suffix) == 0;
}

/* The fonts ship beside the binary (CMake copies them to bin/fonts); a checkout's
 * vendor/fonts is the fallback, so a build run from the tree still finds them. */
fs::path fonts_dir() {
    const std::string d = squid::host::first_existing({squid::host::ship_dir() / "fonts", fs::path(SQUID_SOURCE_DIR) / "vendor" / "fonts"});
    return d.empty() ? fs::path() : fs::path(d);
}

void load_fonts(squid::studio::Studio& studio) {
    ImGuiIO& io = ImGui::GetIO();
    const fs::path fd = fonts_dir();
    const fs::path lato = fd / "Lato-Regular.ttf", fa = fd / "fa-solid-900.ttf", mono = fd / "JetBrainsMono.ttf";
    std::error_code ec;
    static const ImWchar fa_range[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
    auto with_icons = [&](float size) -> ImFont* {
        ImFontConfig base;
        base.OversampleH = 2;
        base.OversampleV = 2;
        ImFont* f = fs::exists(lato, ec) ? io.Fonts->AddFontFromFileTTF(lato.string().c_str(), size, &base) : io.Fonts->AddFontDefault();
        if (fs::exists(fa, ec)) {
            ImFontConfig cfg;
            cfg.MergeMode = true;
            cfg.PixelSnapH = true;
            cfg.GlyphMinAdvanceX = size;
            io.Fonts->AddFontFromFileTTF(fa.string().c_str(), size * 0.9f, &cfg, fa_range);
        }
        return f;
    };
    with_icons(17.0f);                  // the first font is the default
    studio.big = with_icons(28.0f);
    if (fs::exists(mono, ec)) studio.mono = io.Fonts->AddFontFromFileTTF(mono.string().c_str(), 15.0f);
}

void set_icon(GLFWwindow* window) {
    const std::string p = squid::host::first_existing({squid::host::ship_dir() / "squid.png", fs::path(SQUID_SOURCE_DIR) / "packaging" / "squid.png"});
    if (p.empty()) return;
    int w = 0, h = 0, n = 0;
    unsigned char* px = stbi_load(p.c_str(), &w, &h, &n, 4);
    if (!px) return;
    GLFWimage img{w, h, px};
    glfwSetWindowIcon(window, 1, &img);
    stbi_image_free(px);
}

int run_gui(squid::App& app, const std::string& project, const std::string& ink) {
    glfwSetErrorCallback([](int code, const char* desc) { std::fprintf(stderr, "glfw error %d: %s\n", code, desc); });
    if (!glfwInit()) return 2;
    const char* glsl = maiz::gl_context_hints();
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(1500, 900, "Void Squid", nullptr, nullptr);
    if (!window) { glfwTerminate(); return 2; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    set_icon(window);
    squid::library::set_gl_renderer(std::string(reinterpret_cast<const char*>(glGetString(GL_VENDOR))) + " / " +
                                    reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // the layout is this person's, on this machine: beside their settings, not in the cwd
    static std::string ini = (squid::host::config_dir() / "layout.ini").string();
    io.IniFilename = ini.c_str();

    squid::studio::Studio studio(app);
    load_fonts(studio);
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl);
    studio.on_title = [window](const std::string& t) { glfwSetWindowTitle(window, t.c_str()); };
    studio.on_quit = [window] { glfwSetWindowShouldClose(window, 1); };
    if (!studio.start(project, ink)) {
        std::fprintf(stderr, "squid: could not open a project\n");
        return 2;
    }
    std::fprintf(stderr, "gpu: %s\n", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

    const char* shots = std::getenv("SQUID_SHOTS");   // the screenshot walk (Studio::autotest)
    int frame_no = 0;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED)) { glfwWaitEvents(); continue; }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        studio.frame();
        ImGui::Render();
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.025f, 0.035f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        studio.draw_world(w, h);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (shots && *shots) {
            bool done = false;
            const std::string name = studio.autotest(++frame_no, done);
            if (!name.empty() && w > 0 && h > 0) {
                std::vector<unsigned char> px(static_cast<size_t>(w) * h * 4);
                glPixelStorei(GL_PACK_ALIGNMENT, 1);
                glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
                stbi_flip_vertically_on_write(1);
                const std::string out = (fs::path(shots) / (name + ".png")).string();
                stbi_write_png(out.c_str(), w, h, 4, px.data(), w * 4);
                std::fprintf(stderr, "shot: %s\n", out.c_str());
            }
            if (done) glfwSetWindowShouldClose(window, 1);
        }
        glfwSwapBuffers(window);
    }
    studio.shutdown();   // persists the project
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

/* `squid [--state p.squid] --import-library <folder>`: File > Open library, from a terminal. */
int import_library(squid::App& app, int argc, char** argv) {
    std::string state = "squid.state.json", lib, actor = "agent";
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--state" && i + 1 < argc) state = argv[++i];
        else if (a.rfind("--state=", 0) == 0) state = a.substr(8);
        else if (a == "--import-library" && i + 1 < argc) lib = argv[++i];
        else if (a == "--actor" && i + 1 < argc) actor = argv[++i];
    }
    maiz::SessionOptions o;
    o.state_path = state;
    o.actor = actor;
    o.effects = maiz::EffectPolicy::Allow;
    o.allowed_effects = {"library"};
    maiz::Session s(app.host, o);
    if (!s.start()) { std::fprintf(stderr, "squid: %s\n", s.error().c_str()); return 2; }
    const maiz::Result r = s.dispatch("effect library import " + maiz::arg(lib));
    nlohmann::json reply;
    try { reply = nlohmann::json::parse(r.data); } catch (...) {}
    if (!r.ok || !reply.is_object() || reply.contains("error")) {
        std::fprintf(stderr, "squid: %s\n", reply.is_object() && reply.contains("error") ? reply["error"].get<std::string>().c_str() : r.text().c_str());
        return 1;
    }
    const std::string next = squid::library::splice(s.core().export_state(), reply);
    if (next.empty() || !s.core().replace_state(next)) { std::fprintf(stderr, "squid: the splice failed\n"); return 1; }
    s.close();
    std::printf("%s\n", nlohmann::json{{"imported", reply["library"]}, {"counts", reply["counts"]}, {"state", state}}.dump().c_str());
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    squid::host::set_ship_dir(argc > 0 ? argv[0] : nullptr);
    squid::App app = squid::build_app();

    if (argc == 1) return run_gui(app, "", "");
    if (argc == 2 && ends_with(argv[1], ".ink")) return run_gui(app, "", argv[1]);
    if (argc == 2 && ends_with(argv[1], ".squid")) return run_gui(app, argv[1], "");
    for (int i = 1; i < argc; ++i)
        if (std::strcmp(argv[i], "--import-library") == 0) return import_library(app, argc, argv);

    /* Everything else is the terminal front-end, and it is Maiz's. */
    return maiz::run_cli(app.host, argc, argv);
}
