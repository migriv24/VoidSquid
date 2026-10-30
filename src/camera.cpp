/* camera.cpp — see camera.hpp. */
#include "camera.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <sstream>

namespace squid {

std::string Camera3D::serialize() const {
    char buf[160];
    std::snprintf(buf, sizeof(buf), "%.7g %.7g %.7g %.7g %.7g %.7g", yaw, pitch, dist, tx, ty, tz);
    return buf;
}

bool Camera3D::parse(const std::string& value, Camera3D& out) {
    std::string v = value;
    v.erase(std::remove_if(v.begin(), v.end(), [](char c) { return c == '"' || c == '\''; }), v.end());
    std::istringstream in(v);
    Camera3D c;
    if (!(in >> c.yaw >> c.pitch >> c.dist >> c.tx >> c.ty >> c.tz)) return false;
    if (!(c.dist > 0)) return false;
    out = c;
    return true;
}

voidjelly::Vec3 Camera3D::eye() const {
    const double cp = std::cos(pitch), sp = std::sin(pitch);
    const double cy = std::cos(yaw), sy = std::sin(yaw);
    return {tx + dist * cp * sy, ty + dist * sp, tz + dist * cp * cy};
}

voidjelly::LookCamera Camera3D::to_look(double aspect) const {
    voidjelly::LookCamera c;
    c.eye = eye();
    c.target = {tx, ty, tz};
    c.up = {0, 1, 0};
    c.fov_y_deg = kFovYDeg;
    c.aspect = aspect > 0 ? aspect : 16.0 / 10.0;
    return c;
}

Camera3D frame_all(const voidjelly::GlDrawList& list, const Camera3D& keep_angles) {
    Camera3D cam = keep_angles;
    if (list.draws.empty()) return cam;
    float lo[3] = {1e30f, 1e30f, 1e30f}, hi[3] = {-1e30f, -1e30f, -1e30f};
    const float s = static_cast<float>(list.unit_scale);
    for (const voidjelly::GlDraw& d : list.draws) {
        const voidjelly::GlMeshRange& range = list.meshes[d.mesh];
        for (uint32_t i = 0; i < range.index_count; ++i) {
            const voidjelly::GlVertex& v = list.vertices[list.indices[range.first_index + i]];
            float w[3];
            for (int r = 0; r < 3; ++r)
                w[r] = s * (d.world[r * 4 + 0] * v.position[0] + d.world[r * 4 + 1] * v.position[1] +
                            d.world[r * 4 + 2] * v.position[2] + d.world[r * 4 + 3]);
            for (int k = 0; k < 3; ++k) { lo[k] = std::min(lo[k], w[k]); hi[k] = std::max(hi[k], w[k]); }
        }
    }
    cam.tx = (lo[0] + hi[0]) * 0.5f;
    cam.ty = (lo[1] + hi[1]) * 0.5f;
    cam.tz = (lo[2] + hi[2]) * 0.5f;
    const float dx = hi[0] - lo[0], dy = hi[1] - lo[1], dz = hi[2] - lo[2];
    cam.dist = std::max(0.5f * std::sqrt(dx * dx + dy * dy + dz * dz) * 2.4f, 0.05f);
    return cam;
}

}  // namespace squid
