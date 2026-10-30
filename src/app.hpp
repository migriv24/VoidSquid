/* app.hpp — Void Squid, declared once.
 *
 * One maiz::HostApp that both front-ends take: the GUI (main.cpp, with a window)
 * and the headless CLI (the same main.cpp, with none). Whatever is missing here is
 * missing from both, which is Void Maiz's anti-drift property and the reason the
 * CLI costs ~40 lines rather than a port (okf/concepts/scope.md).
 *
 * ── What the application IS, at rung 2.5 ──
 *
 * A document that holds VIEW STATE and nothing else: which .ink is open
 * (`config squid.ink`) and where the camera is (`config view.camera3d`). Both are
 * Void Core config, dispatched like everything else, so Void Squid owns no truth
 * (CLAUDE.md rule 6). The .ink's own structure is Jelly's and is reached through
 * Jelly; it is never copied into this document.
 *
 * ── The effects, and why each is one ──
 *
 *   effect open <path>    reads ONE file off disk. The only file read in v0, and the
 *                         first row of okf/concepts/build-surface.md. Then hands the
 *                         bytes to Void Jelly (read_ink) and asks the GL nib for a
 *                         draw list and a residual — pure, cached in `Scene` as
 *                         DERIVED data keyed by the .ink's hash, never as truth.
 *   effect residual       prints the current residual as JSON. Not an effect at all,
 *                         strictly; it rides the effect verb because it needs the
 *                         bytes `open` read, and a headless agent reaches it the
 *                         same way it reached `open`.
 *   effect inspect        Jelly's `inspect` verb over the held scene, for the same
 *                         reason.
 *
 * Effects are refused by default and granted per run (Maiz's headless discipline).
 * Reading a file is harmless; the discipline is kept anyway, because the next row
 * in the build surface is "invoke devkitARM" and the gate must already be a habit.
 */
#pragma once

#include "voidmaiz/headless.hpp"
#include "voidjelly/format.hpp"
#include "voidjelly/nibs/gl.hpp"
#include "voidjelly/spatial.hpp"
#include "table.hpp"

#include <memory>
#include <string>
#include <vector>

namespace squid {

inline constexpr const char* kVersion = VOIDSQUID_VERSION;
inline constexpr const char* kInkKey = "squid.ink";          // config: the open file
inline constexpr const char* kCameraKey = "view.camera3d";   // config: "yaw pitch dist tx ty tz"

/* What the host currently holds, ALL of it derived: bytes a file read produced, the
 * Ink Jelly parsed from them, the GL emission Jelly computed from that. Dropping
 * this struct and re-running `effect open` on the path in the document rebuilds it
 * exactly, which is the test that nothing here is a source of truth. */
struct Scene {
    std::string path;                 // what was opened (also in the document)
    std::vector<uint8_t> bytes;
    voidjelly::Ink ink;
    voidjelly::GlEmission gl;         // drawlist + residual, from the GL nib
    bool loaded = false;
    std::string error;                // the last failure, for the panel and the CLI
    uint64_t generation = 0;          // bumps on every successful open; the viewport re-uploads on change

    /* The last `effect look` answer, for the GUI's Look panel. Derived like the rest:
     * the camera that produced it is in the document. */
    voidjelly::LookResult last_look;
    bool has_look = false;

    /* The simulation, if one was started: the World compiled from this scene and a
     * table's rules, and the instances stepping it. Derived, like everything here;
     * the run is reproducible from (scene, rules, seeds, dt, ticks), which the
     * transcript records command by command. */
    TableCompile table;
    std::vector<sim::Instance> instances;
    double sim_dt = 0.05;
    bool sim_ready = false;
};

struct App {
    maiz::HostApp host;
    std::shared_ptr<Scene> scene;     // shared with the effect handler's closure
};

/* The application. Call once; hand `host` to a Session (GUI) or run_cli (headless). */
App build_app();

/* The one file read, as a function the effect handler calls and a test can call
 * directly: read `path`, parse through Jelly, emit through the GL nib, fill `scene`.
 * Returns false and sets scene.error on any failure; never throws. */
bool open_ink(Scene& scene, const std::string& path);

/* `effect world`: a table's world file, every model read through Jelly and placed at its
 * matrix into one Ink (voidjelly::append_ink). Reads files; computes no geometry. */
bool open_world(Scene& scene, const std::string& library, const std::string& world_file);

/* `effect recon <path>`: what is this artifact? The effects (a bounded folder walk or
 * the first KiB of one file, and a PATH lookup per tool) happen here; everything else
 * is squid_read's and pure. Returns the report as JSON, or {"error":...}. Holds nothing:
 * the answer is derived and re-runnable from the path (okf/concepts/recon.md). */
std::string recon(const std::string& path);

/* A summary of the held scene as JSON, for the effect handler's reply and the CLI. */
std::string scene_summary_json(const Scene& scene);

}  // namespace squid
