/* device.cpp — the device library: manifests, validation, and the route plan. */
#include "forge/forge.hpp"

#include "json.hpp"

#include <string>
#include <utility>

namespace squid::forge {
namespace {

using json = nlohmann::json;

bool get_string(const json& j, const std::string& key, std::string& out) {
    if (!j.contains(key) || !j[key].is_string()) return false;
    out = j[key].get<std::string>();
    return true;
}

}  // namespace

bool parse_device_library(const std::string& json_text, DeviceLibrary& lib,
                          std::string& error) {
    json j;
    try {
        j = json::parse(json_text);
    } catch (const std::exception& e) {
        error = std::string("not JSON: ") + e.what();
        return false;
    }
    if (!j.is_object() || !j.contains("schema") ||
        j["schema"] != "forge.device-library/0.1") {
        error = "schema must be forge.device-library/0.1";
        return false;
    }
    if (!j.contains("devices") || !j["devices"].is_array()) {
        error = "missing devices array";
        return false;
    }

    lib = DeviceLibrary{};
    for (const json& dj : j["devices"]) {
        Device d;
        if (!get_string(dj, "id", d.id)) { error = "device missing id"; return false; }
        if (!get_string(dj, "name", d.name)) d.name = d.id;
        if (!get_string(dj, "cpu", d.cpu)) d.cpu = "";
        if (!get_string(dj, "kind", d.kind)) d.kind = "both";
        if (!get_string(dj, "toolchain", d.toolchain)) d.toolchain = "";
        if (!get_string(dj, "runtime", d.runtime)) d.runtime = "";
        if (dj.contains("subsystems") && dj["subsystems"].is_array()) {
            for (const json& sj : dj["subsystems"]) {
                Subsystem s;
                if (!get_string(sj, "name", s.name)) {
                    error = "subsystem missing name";
                    return false;
                }
                if (!get_string(sj, "impl", s.impl)) s.impl = "";
                d.subsystems.push_back(std::move(s));
            }
        }
        if (dj.contains("registers") && dj["registers"].is_array()) {
            for (const json& rj : dj["registers"]) {
                Register r;
                if (!get_string(rj, "name", r.name)) {
                    error = "register missing name";
                    return false;
                }
                if (!get_string(rj, "impl", r.impl)) r.impl = "";
                d.registers.push_back(std::move(r));
            }
        }
        lib.devices.push_back(std::move(d));
    }
    return true;
}

std::string device_library_to_json(const DeviceLibrary& lib) {
    json j;
    j["schema"] = "forge.device-library/0.1";
    j["devices"] = json::array();
    for (const Device& d : lib.devices) {
        json dj;
        dj["id"] = d.id;
        dj["name"] = d.name;
        dj["cpu"] = d.cpu;
        dj["kind"] = d.kind;
        dj["toolchain"] = d.toolchain;
        dj["runtime"] = d.runtime;
        dj["subsystems"] = json::array();
        for (const Subsystem& s : d.subsystems) {
            json sj;
            sj["name"] = s.name;
            sj["impl"] = s.impl;
            dj["subsystems"].push_back(std::move(sj));
        }
        dj["registers"] = json::array();
        for (const Register& r : d.registers) {
            json rj;
            rj["name"] = r.name;
            rj["impl"] = r.impl;
            dj["registers"].push_back(std::move(rj));
        }
        j["devices"].push_back(std::move(dj));
    }
    return j.dump(2);
}

std::optional<Route> plan_route(const DeviceLibrary& lib, const std::string& source,
                                const std::string& target, std::string& error) {
    const Device* s = nullptr;
    const Device* t = nullptr;
    for (const Device& d : lib.devices) {
        if (d.id == source) s = &d;
        if (d.id == target) t = &d;
    }
    if (!s) { error = "unknown source device '" + source + "'"; return std::nullopt; }
    if (!t) { error = "unknown target device '" + target + "'"; return std::nullopt; }

    Route r;
    r.source = source;
    r.target = target;
    r.cpu_source = s->cpu;
    r.cpu_target = t->cpu;
    r.toolchain = t->toolchain;
    r.runtime = t->runtime;

    /* The cpu always bridges when the ISAs differ: the code is recompiled for
     * the target or binary-translated, either way it is work the route owns. */
    if (!s->cpu.empty() && s->cpu != t->cpu) {
        r.steps.push_back(RouteStep{"cpu", s->cpu, t->cpu});
    }
    for (const Subsystem& ss : s->subsystems) {
        RouteStep step;
        step.subsystem = ss.name;
        step.source_impl = ss.impl;
        bool found = false;
        for (const Subsystem& ts : t->subsystems) {
            if (ts.name == ss.name) {
                step.target_impl = ts.impl;
                found = true;
                break;
            }
        }
        if (!found) step.target_impl = "reimplement";
        r.steps.push_back(std::move(step));
    }
    return r;
}

std::vector<RegisterPlan> plan_registers(const DeviceLibrary& lib,
                                         const std::string& source,
                                         const std::string& target,
                                         std::string& error) {
    const Device* s = nullptr;
    const Device* t = nullptr;
    for (const Device& d : lib.devices) {
        if (d.id == source) s = &d;
        if (d.id == target) t = &d;
    }
    if (!s) { error = "unknown source device '" + source + "'"; return {}; }
    if (!t) { error = "unknown target device '" + target + "'"; return {}; }

    std::vector<RegisterPlan> out;
    for (const Register& sr : s->registers) {
        RegisterPlan p;
        p.name = sr.name;
        p.source_impl = sr.impl;
        bool found = false;
        for (const Register& tr : t->registers) {
            if (tr.name == sr.name) {
                p.target_impl = tr.impl;
                found = true;
                break;
            }
        }
        if (!found) p.target_impl = "reimplement";
        out.push_back(std::move(p));
    }
    return out;
}

}  // namespace squid::forge
