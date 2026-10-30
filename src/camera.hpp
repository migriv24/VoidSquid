/* camera.hpp — the orbit camera as data. No GL here.
 *
 * Split out of viewport.hpp on 2026-10-09 so the headless application can use it:
 * `effect look` asks Void Jelly what is on screen from a camera, and a headless agent
 * has a camera (in the document) and no window. The camera is model content — six
 * numbers under `config view.camera3d` — and this file is the one place they are
 * serialised, parsed, and turned into an eye position.
 */
#pragma once

#include "voidjelly/nibs/gl.hpp"
#include "voidjelly/spatial.hpp"

#include <string>

namespace squid {

struct Camera3D {
    float yaw = 0.6f;       // radians, around +Y
    float pitch = 0.4f;     // radians, above the horizon
    float dist = 6.0f;      // from target
    float tx = 0, ty = 0, tz = 0;   // target

    static constexpr float kFovYDeg = 50.0f;   // the viewport's projection, stated once

    /* "yaw pitch dist tx ty tz" with %.7g, the same precision Maiz keeps. */
    std::string serialize() const;
    /* Tolerates surrounding quotes of either kind. */
    static bool parse(const std::string& value, Camera3D& out);

    /* The eye on its orbit sphere. */
    voidjelly::Vec3 eye() const;
    /* As Void Jelly's `look` wants it, for a view `aspect` wide. */
    voidjelly::LookCamera to_look(double aspect) const;
};

/* A camera that frames the whole draw list: target at the bounds' centre, distance
 * from their radius. Reads Jelly's buffers to aim a camera, never to change them. */
Camera3D frame_all(const voidjelly::GlDrawList& list, const Camera3D& keep_angles);

}  // namespace squid
