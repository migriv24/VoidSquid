/* presets.cpp — see presets.hpp. No geometry is computed here: every mesh is Void
 * Jelly's primitive; this file only places them and tags them. */
#include "presets.hpp"

#include "voidjelly/primitives.hpp"

namespace squid::presets {
namespace {

voidjelly::Object put(voidjelly::Ink& ink, const std::string& id, const voidjelly::BlockRef& mesh,
                      double x, double y, double z, std::vector<voidjelly::Tag> tags,
                      const std::string& material) {
    voidjelly::Object o;
    o.id = id;
    o.rune_id = "rune_preset_" + id;
    o.name = id;
    o.transform.m[3] = x;
    o.transform.m[7] = y;
    o.transform.m[11] = z;
    o.mesh = mesh;
    o.tags = std::move(tags);
    o.material = material;
    ink.objects.push_back(o);
    return o;
}

}  // namespace

voidjelly::Ink traffic() {
    using namespace voidjelly;
    Ink ink;
    ink.name = "traffic";
    ink.mantle_id = "mantle_preset_traffic";
    ink.unit_scale = 1.0;   // metres

    const BlockRef road = ink.put_mesh(primitive::box(80, 0.2, 10));
    const BlockRef car = ink.put_mesh(primitive::box(4, 1.4, 2));
    const BlockRef zone = ink.put_mesh(primitive::box(8, 3, 10));
    const BlockRef wall = ink.put_mesh(primitive::box(1, 2, 4));
    const BlockRef crate = ink.put_mesh(primitive::box(1, 1, 1));

    put(ink, "road", road, 0, -0.1, 0, {{"kind", "road"}}, "asphalt");
    put(ink, "crossing", zone, 24, 1.5, 0, {{"kind", "zone"}, {"role", "goal"}}, "paint");
    /* Slow lane (z = -2.5): three cars nose to tail, 7 m apart. */
    put(ink, "car_a", car, -10, 0.7, -2.5, {{"kind", "car"}, {"lane", "slow"}}, "red");
    put(ink, "car_b", car, -17, 0.7, -2.5, {{"kind", "car"}, {"lane", "slow"}}, "blue");
    put(ink, "car_c", car, -24, 0.7, -2.5, {{"kind", "car"}, {"lane", "slow"}}, "green");
    /* Fast lane (z = +2.5): one car, and a wall across the lane before the crossing. */
    put(ink, "car_fast", car, -20, 0.7, 2.5, {{"kind", "car"}, {"lane", "fast"}}, "yellow");
    put(ink, "roadworks", wall, 8, 1.0, 2.5, {{"kind", "wall"}}, "orange");
    /* A crate dropped from 6 m beside the road. */
    put(ink, "crate", crate, 0, 6, 7, {{"kind", "crate"}}, "wood");
    return ink;
}

voidjelly::Ink shapes() {
    using namespace voidjelly;
    Ink ink;
    ink.name = "shapes";
    ink.mantle_id = "mantle_preset_shapes";
    double x = 0;
    for (const std::string& n : primitive::names()) {
        put(ink, n, ink.put_mesh(primitive::by_name(n)), x, 0, 0, {{"kind", "primitive"}, {"shape", n}}, n);
        x += 1.5;
    }
    return ink;
}

std::vector<std::string> names() { return {"traffic", "shapes"}; }

voidjelly::Ink by_name(const std::string& name) {
    if (name == "traffic") return traffic();
    if (name == "shapes") return shapes();
    return {};
}

}  // namespace squid::presets
