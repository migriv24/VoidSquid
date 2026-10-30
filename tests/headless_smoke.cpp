/* headless_smoke.cpp — the application with no window, through the same HostApp.
 *
 * What it checks, in the order each would be most expensive to be wrong:
 *
 *   1. A refused effect does not run. `effect open` under the default policy must
 *      leave the scene unloaded. The gate is the whole review story for agents.
 *   2. Under Allow, `effect open` reads the file, Jelly parses it, the GL nib emits,
 *      and the reply carries the verdict. That is the three verbs, minus the window.
 *   3. The document remembers the path and the camera as config, and a SECOND
 *      session over the same state file sees both. There is only one document.
 *   4. `effect residual` and `effect inspect` answer from the held bytes.
 *
 * The fixture is Jelly's own `sample` verb, written to a temp file by this test,
 * which is a host and may touch the disk. No framework; plain checks.
 */
#include "app.hpp"

#include "voidjelly/verbs.hpp"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

static int failures = 0;
#define CHECK(cond)                                                             \
    do {                                                                        \
        if (!(cond)) { ++failures; std::printf("  FAIL  %s:%d: %s\n", __FILE__, __LINE__, #cond); } \
    } while (0)

static bool contains(const std::string& h, const std::string& n) { return h.find(n) != std::string::npos; }

int main() {
    const fs::path dir = fs::temp_directory_path() / "voidsquid_headless_smoke";
    std::error_code ec;
    fs::remove_all(dir, ec);
    fs::create_directories(dir, ec);
    const fs::path ink_path = dir / "room.ink";
    const fs::path state_path = dir / "squid.state.json";

    /* The fixture: Jelly computes the bytes, this host writes them. */
    {
        const voidjelly::VerbResult s = voidjelly::dispatch("sample", "{}", nullptr, 0);
        CHECK(s.ok && !s.blob.empty());
        std::ofstream out(ink_path, std::ios::binary);
        out.write(reinterpret_cast<const char*>(s.blob.data()), static_cast<std::streamsize>(s.blob.size()));
    }
    const std::string ink = ink_path.string();

    /* 1. Refused by default. */
    {
        squid::App app = squid::build_app();
        maiz::SessionOptions opts;
        opts.state_path = state_path.string();
        opts.lock = false;
        maiz::Session session(app.host, opts);
        CHECK(session.start());
        const maiz::Result r = session.dispatch("effect open " + maiz::arg(ink));
        CHECK(!r.ok);
        CHECK(!app.scene->loaded);
        session.close();
    }

    /* 2 and 3. Allowed: open, remember, and persist. */
    {
        squid::App app = squid::build_app();
        maiz::SessionOptions opts;
        opts.state_path = state_path.string();
        opts.lock = false;
        opts.effects = maiz::EffectPolicy::Allow;
        maiz::Session session(app.host, opts);
        CHECK(session.start());

        const maiz::Result r = session.dispatch("effect open " + maiz::arg(ink));
        CHECK(r.ok);
        CHECK(app.scene->loaded);
        CHECK(contains(r.data, "\"verdict\":\"fits\""));
        CHECK(contains(r.data, "\"draws\":4"));
        CHECK(app.scene->gl.drawlist.draws.size() == 4);
        CHECK(app.scene->gl.residual.count("dropped") == 0);

        CHECK(session.dispatch(std::string("config set ") + squid::kInkKey + " " + maiz::arg(ink)).ok);
        CHECK(session.dispatch(std::string("config set ") + squid::kCameraKey + " " + maiz::arg("0.6 0.4 6 0 0 0")).ok);

        /* 4. The held bytes answer. */
        const maiz::Result res = session.dispatch("effect residual");
        CHECK(res.ok);
        CHECK(contains(res.data, "\"nib\":\"gl\""));
        CHECK(contains(res.data, "\"kind\":\"triangulated\""));
        const maiz::Result insp = session.dispatch("effect inspect");
        CHECK(insp.ok);
        CHECK(contains(insp.data, "\"polygons\":19"));

        /* The spatial log: survey, near, look, all answered by Jelly's pure geometry. */
        const maiz::Result sv = session.dispatch("effect survey 2");
        CHECK(sv.ok && contains(sv.data, "\"objects\":5") && contains(sv.data, "\"cells_per_axis\":2"));
        const maiz::Result nr = session.dispatch("effect near 0.05 0 0 0.025");
        CHECK(nr.ok && contains(nr.data, "\"count\":2") && contains(nr.data, "crate_2"));
        const maiz::Result lk = session.dispatch("effect look");     // the framing camera
        CHECK(lk.ok && contains(lk.data, "\"visible_total\":5"));
        CHECK(contains(lk.data, "\"occlusion\":\"not computed\""));
        CHECK(app.scene->has_look && app.scene->last_look.visible_total == 5);
        /* yaw pi: the eye sits at z = 0.3 and looks toward +z at the target z = 0.5,
         * so the crates near z = 0 are all behind it. */
        const maiz::Result away = session.dispatch("effect look " + maiz::arg("3.1415927 0 0.2 0.02 0 0.5"));
        CHECK(away.ok && contains(away.data, "\"visible_total\":0") && contains(away.data, "\"behind\":5"));
        const maiz::Result junk = session.dispatch("effect look " + maiz::arg("not a camera"));
        CHECK(contains(junk.data, "cannot parse camera"));

        /* Simulation through the same surface an agent uses: make a preset, open it,
         * two instances, step, compare, fork, log. */
        const std::string traffic = (dir / "traffic.ink").string();
        CHECK(session.dispatch("effect make traffic " + maiz::arg(traffic)).ok);
        CHECK(session.dispatch("effect open " + maiz::arg(traffic)).ok);
        const maiz::Result sn = session.dispatch("effect sim new 2 1");
        CHECK(sn.ok && contains(sn.data, "\"dynamic\":5") && contains(sn.data, "\"diagnostics\":[]"));
        const maiz::Result st = session.dispatch("effect sim step 200");
        CHECK(st.ok && contains(st.data, "\"car_a\":") && contains(st.data, "\"identical\":false"));
        CHECK(session.dispatch("effect sim fork 0").ok);                    // same seed: a replay
        const maiz::Result lg = session.dispatch("effect sim log 0 3");
        CHECK(lg.ok && contains(lg.data, "\"counts\""));
        const maiz::Result unknown = session.dispatch("effect sim warp");
        CHECK(contains(unknown.data, "needs new|step|fork|log|compare"));

        /* Recon, the ingress arrow's first stage, through the same surface. An .ink is
         * not a game artifact recon knows, and it says so in the residual rather than
         * guessing; a folder with a GameMaker data file is fingerprinted by its listing. */
        const maiz::Result ri = session.dispatch("effect recon " + maiz::arg(traffic));
        CHECK(ri.ok && contains(ri.data, "\"kind\":\"unknown\"") && contains(ri.data, "no known header"));
        const fs::path gm = dir / "gm_install";
        fs::create_directories(gm, ec);
        { std::ofstream(gm / "data.win") << "FORM"; std::ofstream(gm / "Game.exe") << "MZ"; }
        const maiz::Result rg = session.dispatch("effect recon " + maiz::arg(gm.string()));
        CHECK(rg.ok && contains(rg.data, "\"engine\":\"gamemaker\"") && contains(rg.data, "\"depth\":\"structure\""));
        CHECK(contains(session.dispatch("effect recon " + maiz::arg((dir / "nope").string())).data, "no such file"));

        /* Void MoleRat, through the same surface an agent uses: a synthetic IR in, a
         * state document out, and the document validates. MOLERAT_OUT keeps the run in
         * this test's temp directory. */
#if defined(_WIN32)
        _putenv_s("MOLERAT_OUT", (dir / "molerat").string().c_str());
#else
        setenv("MOLERAT_OUT", (dir / "molerat").string().c_str(), 1);
#endif
        const fs::path ir_path = dir / "toy.ir.json";
        {
            std::ofstream out(ir_path);
            out << R"IR({"schema":"molerat.foreign-ir/0.1",
                "target":{"id":"smoke","title":"Smoke","platform":"synthetic","origin":"authored smoke fixture","depth":"source"},
                "functions":[{"name":"step","addr":"0x0","blocks":[{"id":"e","ops":[{"kind":"load","src":"stats.gravity"},{"kind":"store","dst":"stats.vy"}]}],"calls":[],"reads":["stats.gravity"],"writes":["stats.vy"],"evidence":["writes vy"]}],
                "tables":[{"name":"stats","fields":[{"name":"gravity","type":"f32","unit":"m/s^2","value":-9.8}]}],
                "constants":[],"residual":["tick rate is not in the IR"]})IR";
        }
        const maiz::Result mg = session.dispatch(
            "effect molerat gather " + maiz::arg(ir_path.string()) +
            " --run smoke --seed smoke-seed --ir " + maiz::arg(ir_path.string()));
        CHECK(mg.ok && contains(mg.data, "\"stage\":\"gathered\""));
        CHECK(session.dispatch("effect molerat research smoke").ok);
        CHECK(session.dispatch("effect molerat compose smoke").ok);
        const maiz::Result mb = session.dispatch(
            "effect molerat build smoke --out " + maiz::arg((dir / "molerat" / "smoke").string()));
        CHECK(mb.ok && contains(mb.data, "\"ok\":true"));
        CHECK(fs::exists(dir / "molerat" / "smoke" / "smoke.squid"));
        const maiz::Result mv = session.dispatch("effect molerat verify smoke");
        CHECK(mv.ok && contains(mv.data, "\"document_valid\""));

        /* The answer gate: a claim with no evidence is refused, writes nothing, and says
         * ok:false in the op's own reply. A complete claim is stored, one file. */
        const maiz::Result bad = session.dispatch(
            "effect molerat answer smoke " +
            maiz::arg(R"({"claims":[{"claim":"x","confidence":"weak","citations":["learned-decompilers"]}]})"));
        CHECK(contains(bad.data, "\"ok\":false"));
        CHECK(!fs::exists(dir / "molerat" / "smoke" / "answers"));
        const maiz::Result good = session.dispatch(
            "effect molerat answer smoke " +
            maiz::arg(R"({"claims":[{"claim":"gravity is -9.8 m/s^2","evidence":"stats.gravity in the fixture","confidence":"weak","citations":["learned-decompilers"]}]})"));
        CHECK(contains(good.data, "\"accepted\":1"));
        CHECK(fs::exists(dir / "molerat" / "smoke" / "answers" / "001.json"));

        session.close();
        CHECK(fs::exists(state_path));
    }

    /* 3, continued: a second session sees the document. */
    {
        squid::App app = squid::build_app();
        maiz::SessionOptions opts;
        opts.state_path = state_path.string();
        opts.lock = false;
        maiz::Session session(app.host, opts);
        CHECK(session.start());
        const maiz::Result p = session.dispatch(std::string("config get ") + squid::kInkKey);
        CHECK(p.ok && contains(p.data, "room.ink"));
        const maiz::Result c = session.dispatch(std::string("config get ") + squid::kCameraKey);
        CHECK(c.ok && contains(c.data, "0.6 0.4 6"));
        /* The scene itself is NOT in the document: it is derived, and re-opening is
         * how it comes back. Owning no truth, checked. */
        CHECK(!app.scene->loaded);
        session.close();
    }

    /* A bad path is an error with a sentence, not a crash. */
    {
        squid::Scene scene;
        CHECK(!squid::open_ink(scene, (dir / "missing.ink").string()));
        CHECK(contains(scene.error, "cannot read"));
        CHECK(!scene.loaded);
    }

    fs::remove_all(dir, ec);
    if (failures == 0) { std::printf("ok    squid headless smoke\n"); return 0; }
    std::printf("FAIL  squid headless smoke (%d)\n", failures);
    return 1;
}
