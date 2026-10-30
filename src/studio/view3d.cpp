/* studio/view3d.cpp — the studio's 3D views: a turkey, a table, a scene, through Void Jelly.
 *
 * Everything drawn here is Jelly's GL draw list for the held scene (app.hpp, Scene): an
 * .ink, a glTF read by Jelly's reader, or a whole table assembled by `effect world`. The
 * studio owns the window, the GL context and the camera; it computes no geometry.
 *
 * The view is drawn from inside ImGui's own render pass, by a draw callback. Drawing the
 * scene before ImGui (the rung 2.5 way) only works when nothing covers it; a docked window
 * has a background, popups and tooltips, and the callback puts the scene exactly where the
 * window's content is, in the window's stacking order. The callback steps out of the ImGui
 * backend's state (its shader program, vertex array and buffers) to draw with fixed-function
 * GL, then asks the backend to restore itself (ImDrawCallback_ResetRenderState).
 */
#include "studio/studio.hpp"

#include "viewport.hpp"

#include "IconsFontAwesome6.h"
#include "imgui.h"
#include "stb_image.h"
#include "voidjelly/store.hpp"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <vector>

#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#endif
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

namespace squid::studio {

namespace fs = std::filesystem;

namespace {
/* The three GL 2+/3 entry points stepping out of the backend needs, fetched once (GL 1.1
 * headers on Windows do not declare them). */
using PFN_UseProgram = void (*)(unsigned);
using PFN_BindVertexArray = void (*)(unsigned);
using PFN_BindBuffer = void (*)(unsigned, unsigned);
PFN_UseProgram gl_use_program = nullptr;
PFN_BindVertexArray gl_bind_vao = nullptr;
PFN_BindBuffer gl_bind_buffer = nullptr;
void load_gl() {
    if (gl_use_program) return;
    gl_use_program = reinterpret_cast<PFN_UseProgram>(glfwGetProcAddress("glUseProgram"));
    gl_bind_vao = reinterpret_cast<PFN_BindVertexArray>(glfwGetProcAddress("glBindVertexArray"));
    gl_bind_buffer = reinterpret_cast<PFN_BindBuffer>(glfwGetProcAddress("glBindBuffer"));
}
}  // namespace

unsigned Studio::gl_texture(const std::string& hex) {
    auto it = gltex_.find(hex);
    if (it != gltex_.end()) return it->second;
    unsigned id = 0;
    const auto ref = voidjelly::BlockRef::from_hex(hex);
    const std::vector<uint8_t>* bytes = ref ? app_.scene->ink.get_blob(*ref) : nullptr;
    if (bytes && !bytes->empty()) {
        int w = 0, h = 0, n = 0;
        unsigned char* px = stbi_load_from_memory(bytes->data(), static_cast<int>(bytes->size()), &w, &h, &n, 4);
        if (px) {
            GLuint t = 0;
            glGenTextures(1, &t);
            glBindTexture(GL_TEXTURE_2D, t);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);   // the games' own look
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
            stbi_image_free(px);
            id = t;
        }
    }
    gltex_[hex] = id;   // a failure is remembered too: one bad image is not retried every frame
    return id;
}

void Studio::gl_callback(const ImDrawList*, const ImDrawCmd* cmd) {
    static_cast<Studio*>(cmd->UserCallbackData)->render_view();
}

void Studio::render_view() {
    Scene& sc = *app_.scene;
    if (!sc.loaded) return;
    load_gl();
    if (gl_use_program) gl_use_program(0);
    if (gl_bind_vao) gl_bind_vao(0);
    if (gl_bind_buffer) { gl_bind_buffer(GL_ARRAY_BUFFER, 0); gl_bind_buffer(GL_ELEMENT_ARRAY_BUFFER, 0); }
    const ImGuiIO& io = ImGui::GetIO();
    const float sx = io.DisplayFramebufferScale.x, sy = io.DisplayFramebufferScale.y;
    const int fb_h = static_cast<int>(io.DisplaySize.y * sy);
    ViewRect r;
    r.x = static_cast<int>(view_min_[0] * sx);
    r.w = static_cast<int>((view_max_[0] - view_min_[0]) * sx);
    r.h = static_cast<int>((view_max_[1] - view_min_[1]) * sy);
    r.y = fb_h - static_cast<int>(view_max_[1] * sy);
    // a dark well behind the scene, inside the rectangle only
    glEnable(GL_SCISSOR_TEST);
    glScissor(r.x, r.y, r.w, r.h);
    glClearColor(0.035f, 0.045f, 0.09f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    draw_scene(sc.gl.drawlist, cam_, r, [this](const std::string& hex) { return gl_texture(hex); });
}

void Studio::view3d(const std::string& key, const std::function<bool()>& load) {
    Scene& sc = *app_.scene;
    if (!key.empty() && key != view_source_) {
        view_source_ = key;
        gltex_.clear();   // the textures belong to the scene that held them
        busy("Reading it through Void Jelly...", [this, load] {
            if (load && !load()) toast(app_.scene->error.empty() ? "it could not be read" : app_.scene->error, true);
        });
    }
    const ImVec2 p0 = ImGui::GetCursorScreenPos();
    ImVec2 sz = ImGui::GetContentRegionAvail();
    sz.x = std::max(sz.x, 64.0f);
    sz.y = std::max(sz.y, 64.0f);
    ImGui::InvisibleButton("##view3d", sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
    if (!sc.loaded) {
        ImGui::GetWindowDrawList()->AddText(ImVec2(p0.x + 12, p0.y + 12), ImGui::GetColorU32(ImGuiCol_TextDisabled),
                                            sc.error.empty() ? "nothing to draw yet" : sc.error.c_str());
        return;
    }
    if (sc.generation != framed_generation_) {   // new content: frame it
        cam_ = frame_all(sc.gl.drawlist, cam_);
        const auto& dr = sc.gl.drawlist.draws;
        if (dr.size() > 20) {   // a whole world: frame the middle 90% of where things are, not the far backdrops
            std::vector<float> xs, ys, zs;
            for (const auto& d : dr) { xs.push_back(d.world[3]); ys.push_back(d.world[7]); zs.push_back(d.world[11]); }
            for (auto* v : {&xs, &ys, &zs}) std::sort(v->begin(), v->end());
            const size_t lo = dr.size() / 20, hi = dr.size() - 1 - dr.size() / 20;
            const float s = static_cast<float>(sc.gl.drawlist.unit_scale);
            cam_.tx = (xs[lo] + xs[hi]) * 0.5f * s;
            cam_.ty = (ys[lo] + ys[hi]) * 0.5f * s;
            cam_.tz = (zs[lo] + zs[hi]) * 0.5f * s;
            const float span = std::max({xs[hi] - xs[lo], ys[hi] - ys[lo], zs[hi] - zs[lo]}) * s;
            cam_.dist = std::max(1.0f, span * 0.9f);
        }
        framed_generation_ = sc.generation;
    }
    ImGuiIO& io = ImGui::GetIO();
    if (ImGui::IsItemActive()) {
        const float dx = io.MouseDelta.x, dy = io.MouseDelta.y;
        if (ImGui::IsMouseDown(0)) {
            cam_.yaw -= dx * 0.008f;
            cam_.pitch = std::clamp(cam_.pitch + dy * 0.008f, -1.5f, 1.5f);
        } else if (ImGui::IsMouseDown(1)) {
            const float k = cam_.dist * 0.002f, cy = std::cos(cam_.yaw), syw = std::sin(cam_.yaw);
            cam_.tx -= dx * k * cy; cam_.tz += dx * k * syw; cam_.ty += dy * k;
        }
    }
    if (ImGui::IsItemHovered() && io.MouseWheel != 0) cam_.dist *= io.MouseWheel > 0 ? 0.9f : 1.1f;
    view_min_[0] = p0.x; view_min_[1] = p0.y;
    view_max_[0] = p0.x + sz.x; view_max_[1] = p0.y + sz.y;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddCallback(&Studio::gl_callback, this);
    dl->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
    const auto& d = sc.gl.drawlist;
    char info[160];
    std::snprintf(info, sizeof info, "%zu objects  %zu triangles   drag: orbit  right-drag: pan  wheel: zoom", d.draws.size(), d.indices.size() / 3);
    dl->AddText(ImVec2(p0.x + 10, p0.y + sz.y - 22), IM_COL32(150, 165, 200, 220), info);
}

void Studio::draw_table_3d(const std::string& world_file) {
    const std::string lib = library_dir();
    view3d("table:" + lib + "|" + world_file, [this, lib, world_file] {
        return run("effect world " + maiz::arg(lib) + " " + maiz::arg(world_file)).ok && app_.scene->loaded;
    });
}

}  // namespace squid::studio
