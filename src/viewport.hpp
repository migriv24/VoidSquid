/* viewport.hpp — the control experiment, on screen.
 *
 * okf/concepts/viewport.md: Void Jelly emits a draw list; Void Squid owns the
 * window, the GL context, the frame loop and the camera, and issues the calls.
 * Nothing here computes geometry. It draws what Jelly arranged.
 *
 * ── Why fixed-function GL, honestly ──
 *
 * Void Maiz's substrate is a GL 3.0 compatibility context on Windows and Linux
 * (voidmaiz/glhost.hpp). GL 1.1 entry points come straight from opengl32 with no
 * loader, so client-side arrays plus `glDrawElements` plus one directional light draw
 * "flat lit geometry, is this the scene?" with zero vendored code. The VBO-and-shader
 * path belongs to Jelly's open question §10 (`voidjelly_gl`) and arrives with a second
 * host, or with macOS, whichever asks first; this file is written to be replaced.
 *
 * The camera is data and lives in camera.hpp, so the headless build has it too.
 */
#pragma once

#include "camera.hpp"
#include "voidjelly/nibs/gl.hpp"

#include <functional>
#include <string>

namespace squid {

/* Draw the list with the camera into the CURRENT context, over `width` x `height`
 * pixels. Call after glClear and before ImGui renders on top. Applies the Ink's
 * unit_scale so a centimetre scene and a metre scene share one camera. */
void draw_scene(const voidjelly::GlDrawList& list, const Camera3D& cam, int width, int height);

/* Where, and with what pictures (2026-10-12, the studio's 3D views). `x, y, w, h` is
 * a rectangle of the framebuffer in GL's convention (origin bottom left); the scene is
 * drawn inside it alone, depth cleared there first. `texture` turns a draw's texture
 * block (hex) into a GL texture the host uploaded, or 0. Jelly's textured profile puts
 * the material colour, texture and alpha on each draw and vertex colours on vertices;
 * this draws them with fixed-function GL: texture modulated by the lit vertex colour,
 * the material colour folded into the light, alpha tested or blended as the material
 * says. Still no geometry computed here. */
struct ViewRect { int x = 0, y = 0, w = 0, h = 0; };
using TextureLookup = std::function<unsigned(const std::string& hex)>;
void draw_scene(const voidjelly::GlDrawList& list, const Camera3D& cam, const ViewRect& rect,
                const TextureLookup& texture);

}  // namespace squid
