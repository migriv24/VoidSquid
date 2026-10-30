/* viewport.cpp — upload nothing, retain nothing: client-side arrays straight from
 * Jelly's buffers every frame. At rung 2.5 scenes are tens of objects; the day a
 * profile says otherwise is the day this becomes VBOs, measured rather than assumed
 * (okf/concepts/viewport.md, "measure first"). */
#include "viewport.hpp"

#include <GLFW/glfw3.h>   // brings <GL/gl.h> with the platform headers in the right order

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace squid {

namespace {

/* Row-major (Jelly's) -> column-major (GL's), once per draw. */
void load_world(const std::array<float, 16>& m) {
    float col[16];
    for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c) col[c * 4 + r] = m[r * 4 + c];
    glMultMatrixf(col);
}

void perspective(double fovy_deg, double aspect, double znear, double zfar) {
    const double f = 1.0 / std::tan(fovy_deg * 3.14159265358979323846 / 360.0);
    const double m[16] = {
        f / aspect, 0, 0, 0,
        0, f, 0, 0,
        0, 0, (zfar + znear) / (znear - zfar), -1,
        0, 0, (2 * zfar * znear) / (znear - zfar), 0,
    };
    glMultMatrixd(m);
}

void look_at(const Camera3D& cam) {
    /* Orbit: eye on a sphere around the target. */
    const float cp = std::cos(cam.pitch), sp = std::sin(cam.pitch);
    const float cy = std::cos(cam.yaw), sy = std::sin(cam.yaw);
    const float ex = cam.tx + cam.dist * cp * sy;
    const float ey = cam.ty + cam.dist * sp;
    const float ez = cam.tz + cam.dist * cp * cy;

    float f[3] = {cam.tx - ex, cam.ty - ey, cam.tz - ez};
    const float fl = std::sqrt(f[0] * f[0] + f[1] * f[1] + f[2] * f[2]);
    for (float& v : f) v /= fl;
    const float up[3] = {0, 1, 0};
    float s[3] = {f[1] * up[2] - f[2] * up[1], f[2] * up[0] - f[0] * up[2], f[0] * up[1] - f[1] * up[0]};
    const float sl = std::sqrt(s[0] * s[0] + s[1] * s[1] + s[2] * s[2]);
    for (float& v : s) v /= (sl > 0 ? sl : 1);
    const float u[3] = {s[1] * f[2] - s[2] * f[1], s[2] * f[0] - s[0] * f[2], s[0] * f[1] - s[1] * f[0]};

    const float m[16] = {
        s[0], u[0], -f[0], 0,
        s[1], u[1], -f[1], 0,
        s[2], u[2], -f[2], 0,
        0, 0, 0, 1,
    };
    glMultMatrixf(m);
    glTranslatef(-ex, -ey, -ez);
}

}  // namespace

void draw_scene(const voidjelly::GlDrawList& list, const Camera3D& cam, int width, int height) {
    draw_scene(list, cam, ViewRect{0, 0, width, height}, {});
}

void draw_scene(const voidjelly::GlDrawList& list, const Camera3D& cam, const ViewRect& rect, const TextureLookup& texture) {
    const int width = rect.w, height = rect.h;
    if (width <= 0 || height <= 0 || list.indices.empty()) return;
    glViewport(rect.x, rect.y, width, height);
    glEnable(GL_SCISSOR_TEST);
    glScissor(rect.x, rect.y, width, height);
    glClear(GL_DEPTH_BUFFER_BIT);

    /* No back-face culling, and two-sided lighting. The viewport answers "is this
     * the scene?", and a face an artist wound backwards is still in the scene: culling
     * would make it vanish, which is a silent loss in the one instrument meant to
     * show losses. (Found 2026-10-09: Jelly's own sample cube had two inward faces.) */
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    const double aspect = static_cast<double>(width) / height;
    perspective(Camera3D::kFovYDeg, aspect, cam.dist * 0.01, cam.dist * 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    look_at(cam);

    /* One directional light that rides with the camera, so flat shading reads the
     * same from every angle. Lighting on, colour material on: the stand-in colour
     * Jelly chose per material is the diffuse term, and the residual says it is a
     * stand-in. */
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    const float light_dir[4] = {0.3f, 0.8f, 0.6f, 0.0f};   // after look_at: world-space
    const float ambient[4] = {0.22f, 0.22f, 0.24f, 1.0f};
    const float diffuse[4] = {0.9f, 0.9f, 0.88f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light_dir);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glEnable(GL_NORMALIZE);   // unit_scale and object scale must not dim the lighting

    const float s = static_cast<float>(list.unit_scale);
    glScalef(s, s, s);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    const voidjelly::GlVertex* base = list.vertices.data();
    glVertexPointer(3, GL_FLOAT, sizeof(voidjelly::GlVertex), base->position.data());
    glNormalPointer(GL_FLOAT, sizeof(voidjelly::GlVertex), base->normal.data());
    const bool textured = static_cast<bool>(texture);
    if (textured) {   // vertex colours and uvs ride along; the material's colour goes into the light
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glColorPointer(4, GL_FLOAT, sizeof(voidjelly::GlVertex), base->color.data());
        glTexCoordPointer(2, GL_FLOAT, sizeof(voidjelly::GlVertex), base->uv.data());
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }

    for (const voidjelly::GlDraw& d : list.draws) {
        const voidjelly::GlMeshRange& range = list.meshes[d.mesh];
        glPushMatrix();
        load_world(d.world);
        if (textured) {
            const float dif[4] = {diffuse[0] * d.color[0], diffuse[1] * d.color[1], diffuse[2] * d.color[2], d.color[3]};
            const float amb[4] = {ambient[0] * d.color[0], ambient[1] * d.color[1], ambient[2] * d.color[2], 1.0f};
            glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
            glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
            const unsigned tex = d.texture.empty() ? 0u : texture(d.texture);
            if (tex) { glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, tex); }
            if (tex) {   // the material's wrap: the DS mirrors half-textures, and some clamp
                const auto comma = d.wrap.find(',');
                const std::string ws = d.wrap.empty() ? "repeat" : d.wrap.substr(0, comma);
                const std::string wt = d.wrap.empty() ? "repeat" : (comma == std::string::npos ? ws : d.wrap.substr(comma + 1));
                auto gl = [](const std::string& w) { return w == "mirror" ? 0x8370 /* GL_MIRRORED_REPEAT */ : w == "clamp" ? 0x812F /* GL_CLAMP_TO_EDGE */ : GL_REPEAT; };
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gl(ws));
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gl(wt));
            }
            else glDisable(GL_TEXTURE_2D);
            if (d.alpha == "mask") { glEnable(GL_ALPHA_TEST); glAlphaFunc(GL_GREATER, 0.4f); }
            else glDisable(GL_ALPHA_TEST);
            if (d.alpha == "blend") { glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); }
            else glDisable(GL_BLEND);
        } else {
            glColor4fv(d.color.data());
        }
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(range.index_count), GL_UNSIGNED_INT,
                       list.indices.data() + range.first_index);
        glPopMatrix();
    }

    if (textured) {
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_ALPHA_TEST);
        glDisable(GL_BLEND);
    }
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_LIGHTING);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
}


}  // namespace squid
