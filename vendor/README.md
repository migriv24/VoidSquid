# vendor — third-party code Void Squid compiles in

The family rule: vendor, don't depend. Each piece is checked in with its licence, and
nothing is fetched at build time. Listed again in `THIRD-PARTY-NOTICES.md`.

| folder | what | version | licence | from |
|---|---|---|---|---|
| `sqlite/` | SQLite, to read a library's `assets.db` | 3.53.3 | public domain (`sqlite/LICENSE.md`) | Void Hormiga's `vendor/sqlite`, unchanged |
| `json/` | nlohmann/json, the studio's JSON | 3.12.0 | MIT (`json/LICENSE.md`) | Void Hormiga's `vendor/json`, unchanged |
| `stb/` | `stb_image.h` (thumbnails, the window icon) and `stb_image_write.h` (the screenshot walk) | as in Hormiga | public domain or MIT (`stb/LICENSE.md`) | Void Hormiga's `vendor/stb`, unchanged |
| `fonts/` | Lato (UI), JetBrains Mono (console), Font Awesome 6 Free Solid (icons), `IconsFontAwesome6.h` | as in Hormiga | SIL OFL 1.1 (fonts), zlib (the header); `fonts/LICENSE.md`, `fonts/OFL-*.txt` | Void Hormiga's `vendor/fonts`, unchanged |

Dear ImGui and GLFW come through Void Maiz (`../VoidMaiz/vendor`), not from here.
