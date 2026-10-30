cmake_minimum_required(VERSION 3.20)
project(voidsquid VERSION 0.1.0 LANGUAGES CXX C)
# The one place the version is stated. void.json must agree; `mago doctor` checks.

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Void Squid — the host Void Jelly cannot be for itself (okf/index.md).
#
# FIRST CODE: 2026-10-09, rung 2.5 (okf/roadmap.md). A Void Maiz window, a
# viewport, a camera: read a .ink off disk, hand the bytes to Void Jelly, draw what
# comes back, show the residual. Plus the same application headless, for ~40 lines.
#
# ── What this file enforces ──
#
# Three pillars (okf/design/the-three-pillars.md, S7 lean: three targets in one
# repository, promoted to siblings when a second consumer appears):
#
#   squid_app    pillar 1+2's seam for v0: the application declared ONCE as a
#                maiz::HostApp — glyphs, actions, the effect handler — with no
#                window system in it. Links Void Jelly for the computation it
#                performs no part of.
#   squid        pillar 3, the Windows desktop host: GLFW + ImGui + the viewport.
#                The only target that includes a GL header.
#
# Void Squid computes nothing about geometry. If a loop over vertices appears in
# src/, something has gone wrong (CLAUDE.md rule 4); the viewport hands buffers Void
# Jelly already arranged to a GL context the host created, and that is all.

# ── Siblings ────────────────────────────────────────────────────────────────
set(VOIDCORE_ROOT  "${CMAKE_CURRENT_SOURCE_DIR}/../VoidCore/core" CACHE PATH "Void Core's core/ directory")
set(VOIDMAIZ_ROOT  "${CMAKE_CURRENT_SOURCE_DIR}/../VoidMaiz"      CACHE PATH "Void Maiz repository")
set(VOIDJELLY_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../VoidJelly"     CACHE PATH "Void Jelly repository")

foreach(root VOIDCORE_ROOT VOIDMAIZ_ROOT VOIDJELLY_ROOT)
  if(NOT EXISTS "${${root}}")
    message(FATAL_ERROR "${root}=${${root}} does not exist. Clone the sibling beside this repo or set the cache variable.")
  endif()
endforeach()

# ── ONE Void Core ───────────────────────────────────────────────────────────
#
# Void Maiz links the PREBUILT shared Core (its CMake finds the file under
# VOIDCORE_ROOT/build). Void Jelly, standalone, builds Core FROM SOURCE with
# add_subdirectory. In one process that would be two copies of the same library
# with the same exported symbols, so this project settles it: the prebuilt file is
# wrapped as an IMPORTED target named `voidcore` BEFORE Jelly is added, and Jelly's
# `if(NOT TARGET voidcore)` guard (added 2026-10-09 for exactly this) then links
# ours. Both siblings end up on the same libvoidcore.dll, which ships beside squid.
find_library(VOIDCORE_LIBRARY NAMES voidcore libvoidcore
  HINTS "${VOIDCORE_ROOT}/build" "${VOIDCORE_ROOT}/build/bin"
        "${VOIDCORE_ROOT}/build/Release" "${VOIDCORE_ROOT}/build/bin/Release"
  NO_DEFAULT_PATH)
find_file(VOIDCORE_DLL NAMES libvoidcore.dll voidcore.dll
  HINTS "${VOIDCORE_ROOT}/build/bin" "${VOIDCORE_ROOT}/build"
        "${VOIDCORE_ROOT}/build/bin/Release" "${VOIDCORE_ROOT}/build/Release"
  NO_DEFAULT_PATH)
if(NOT VOIDCORE_LIBRARY)
  message(FATAL_ERROR "No built voidcore under ${VOIDCORE_ROOT}/build — build Void Core first")
endif()

add_library(voidcore SHARED IMPORTED GLOBAL)
set_target_properties(voidcore PROPERTIES
  IMPORTED_LOCATION "${VOIDCORE_DLL}"
  IMPORTED_IMPLIB   "${VOIDCORE_LIBRARY}"
  INTERFACE_INCLUDE_DIRECTORIES "${VOIDCORE_ROOT}/include"
  INTERFACE_COMPILE_DEFINITIONS VC_DLL)

# ── Void Maiz: the window, the model adapter, the headless front-end ────────
# EXCLUDE_FROM_ALL: Maiz's own examples and smoke tests are Maiz's to build and run;
# only the targets squid links get built here.
set(MAIZ_RETICULUM OFF CACHE BOOL "" FORCE)   # v0 opens no socket
add_subdirectory("${VOIDMAIZ_ROOT}" voidmaiz EXCLUDE_FROM_ALL)

# ── Void Jelly: every computation about a scene ─────────────────────────────
set(VOIDJELLY_VOIDCORE_DIR "${VOIDCORE_ROOT}" CACHE PATH "" FORCE)
add_subdirectory("${VOIDJELLY_ROOT}" voidjelly EXCLUDE_FROM_ALL)

# ── The application, declared once ──────────────────────────────────────────
# The model-rate simulation runtime (okf/design/simulation.md). Links NOTHING but the
# standard library: no Void Core, no Jelly, no Maiz. It reads a World the forge
# compiled and steps it; railguard §6 made mechanical by the link line.
add_library(squid_sim STATIC src/sim/sim.cpp)
target_include_directories(squid_sim PUBLIC src)
if(NOT MSVC)
  target_compile_options(squid_sim PRIVATE -Wall -Wextra)
endif()

# The ingress arrow's first stage (okf/concepts/recon.md): what is this artifact?
# Pure, like squid_sim: links nothing but the standard library. It never opens a file;
# the host walks the folder or reads the header and hands the bytes in. The link line
# is CLAUDE.md rule 5 made mechanical, and no geometry is anywhere near it (rule 4).
add_library(squid_read STATIC src/read/recon.cpp)
target_include_directories(squid_read PUBLIC src)
if(NOT MSVC)
  target_compile_options(squid_read PRIVATE -Wall -Wextra)
endif()

# ocean: the runtime's motion verdicts, counted for the studio (S34, lean (b);
# src/motion/judge.hpp). Pure like squid_read: the JSON header and nothing else. Void
# Onda computes every verdict in the runtime; the studio does not link it.
add_library(squid_motion STATIC src/motion/judge.cpp)
target_include_directories(squid_motion PUBLIC src)
target_include_directories(squid_motion SYSTEM PUBLIC vendor/json)
if(NOT MSVC)
  target_compile_options(squid_motion PRIVATE -Wall -Wextra)
endif()

# molerat: Void MoleRat's pure half (okf/concepts/molerat.md, molerat/design.md).
# It parses the foreign IR, states the research records, maps them onto two mantles and
# emits a state document. Links the standard library and the vendored JSON header, and
# nothing else: no Core, no Jelly, no Maiz, no filesystem. The host
# (src/molerat/effect.cpp) does the reads and writes.
add_library(squid_molerat STATIC
  src/molerat/ir.cpp
  src/molerat/research.cpp
  src/molerat/emit.cpp
)
target_include_directories(squid_molerat PUBLIC src)
target_include_directories(squid_molerat SYSTEM PUBLIC vendor/json)
if(NOT MSVC)
  target_compile_options(squid_molerat PRIVATE -Wall -Wextra)
endif()

# forge: the device library and the cross-device match loop (okf/concepts/forge.md).
# Pure like squid_molerat: the standard library and the vendored JSON header, and
# nothing else. The host (src/forge/effect.cpp) reads manifests and runs the loop's
# effects; the verifier that compiles-and-compares is injected, never linked.
add_library(squid_forge STATIC
  src/forge/device.cpp
  src/forge/matchloop.cpp
)
target_include_directories(squid_forge PUBLIC src)
target_include_directories(squid_forge SYSTEM PUBLIC vendor/json)
if(NOT MSVC)
  target_compile_options(squid_forge PRIVATE -Wall -Wextra)
endif()

# ── Vendored (vendor/, each with its licence; THIRD-PARTY-NOTICES.md) ───────
# SQLite reads a library's assets.db (Void Hormiga's schema, Hormiga's copy, 3.53.3).
add_library(squid_sqlite3 STATIC vendor/sqlite/sqlite3.c)
target_include_directories(squid_sqlite3 SYSTEM PUBLIC vendor/sqlite)
target_compile_definitions(squid_sqlite3 PRIVATE SQLITE_OMIT_LOAD_EXTENSION SQLITE_THREADSAFE=1)
if(UNIX)
  target_link_libraries(squid_sqlite3 PUBLIC pthread)
endif()

# The library: a game library brought into a project, the catalogue of known
# decompilations, the setup inventory, Load a game's plan, and the host's two effects
# (paths, processes). The pure halves (catalogue matching, the plan, the splice) are
# tested without a file (tests/library_test.cpp); the effect halves are thin.
add_library(squid_library STATIC
  src/host/paths.cpp
  src/host/process.cpp
  src/library/library.cpp
  src/library/catalogue.cpp
  src/library/loadgame.cpp
  src/library/setup.cpp
  src/library/progress.cpp
  src/library/pilot.cpp
)
target_include_directories(squid_library PUBLIC src)
target_include_directories(squid_library SYSTEM PUBLIC vendor/json)
target_link_libraries(squid_library PUBLIC squid_sqlite3)
if(WIN32)
  target_link_libraries(squid_library PUBLIC shell32)
endif()
if(NOT MSVC)
  target_compile_options(squid_library PRIVATE -Wall -Wextra)
endif()

add_library(squid_app STATIC
  src/app.cpp
  src/camera.cpp
  src/effects.cpp
  src/table.cpp
  src/presets.cpp
  src/motion/effect.cpp   # ocean: `effect motion`
  src/molerat/effect.cpp  # molerat: `effect molerat`
  src/forge/effect.cpp    # forge: `effect forge`
)
target_include_directories(squid_app PUBLIC src)
target_link_libraries(squid_app PUBLIC voidmaiz voidmaiz_headless voidjelly squid_sim squid_read squid_library squid_motion squid_molerat squid_forge)
target_compile_definitions(squid_app PUBLIC VOIDSQUID_VERSION="${PROJECT_VERSION}"
                                           SQUID_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
if(NOT MSVC)
  target_compile_options(squid_app PRIVATE -Wall -Wextra)
endif()

# ── The desktop host ────────────────────────────────────────────────────────
# The studio (pillar 2 on this host): the docked window, its workspaces, the dialogs
# and the thumbnail textures. Void Maiz's view module supplies the widget registry.
add_library(squid_studio STATIC
  src/viewport.cpp
  src/studio/studio.cpp
  src/studio/turkey.cpp
  src/studio/table.cpp
  src/studio/data.cpp
  src/studio/panels.cpp
  src/studio/settings.cpp
  src/studio/view3d.cpp
  src/studio/platform.cpp
)
target_include_directories(squid_studio PUBLIC src)
target_include_directories(squid_studio SYSTEM PUBLIC vendor/stb vendor/fonts)
target_link_libraries(squid_studio PUBLIC squid_app voidmaiz_imgui voidmaiz_view)
if(WIN32)
  target_link_libraries(squid_studio PUBLIC comdlg32 ole32 shell32)
endif()

# Two executables, one main: `squid` is a console program (the CLI, and the window
# when run with no command); `voidsquid` is the same program as a windowed application,
# so the desktop shortcut opens no console window. On Linux they are the same thing.
set(SQUID_RC "")
if(WIN32)
  enable_language(RC)
  set(SQUID_RC packaging/squid.rc)
endif()
add_executable(squid src/main.cpp ${SQUID_RC})
target_link_libraries(squid PRIVATE squid_studio)
add_executable(voidsquid WIN32 src/main.cpp ${SQUID_RC})
target_link_libraries(voidsquid PRIVATE squid_studio)
foreach(t squid voidsquid)
  if(NOT MSVC)
    target_compile_options(${t} PRIVATE -Wall -Wextra)
  endif()
  # The shared Core must sit beside the binary (okf/concepts/hosts.md: a host owns
  # what ships with it). Copied, not referenced, so `build/bin/squid` runs from anywhere.
  if(VOIDCORE_DLL)
    add_custom_command(TARGET ${t} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${VOIDCORE_DLL}" "$<TARGET_FILE_DIR:${t}>")
  endif()
endforeach()
# What a release carries beside the binary: the fonts, the seed catalogue, the icon,
# the setup guide. Never a game, never a library (okf/design/load-a-game.md, "Release").
add_custom_command(TARGET squid POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/vendor/fonts" "$<TARGET_FILE_DIR:squid>/fonts"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/data" "$<TARGET_FILE_DIR:squid>/data"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/packaging/squid.png" "$<TARGET_FILE_DIR:squid>/squid.png"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/SETUP.md" "$<TARGET_FILE_DIR:squid>/SETUP.md")

# Carry the compiler runtime INSIDE every executable. Not tuning: the first thing
# that broke (2026-10-09). A MinGW binary otherwise loads whichever libstdc++-6.dll
# the PATH finds first, and on this machine that was a different toolchain's copy
# than the one that compiled us: exit 0xc0000139, entry point not found, before
# main. Void Jelly's shared library and Void Hormiga's first package hit the same
# wall; a host owns what ships with it, and the runtime is part of that.
function(squid_static_runtime target)
  if(MINGW)
    target_link_options(${target} PRIVATE
      -static-libgcc -static-libstdc++
      -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive)
  endif()
endfunction()
squid_static_runtime(squid)
squid_static_runtime(voidsquid)

# ── Tests ───────────────────────────────────────────────────────────────────
# Plain executables, no framework (the family convention). The headless smoke
# exercises the application through the same HostApp the GUI uses, with the window
# system absent, which is the claim okf/concepts/scope.md makes about the CLI.
enable_testing()
add_executable(squid_headless_smoke tests/headless_smoke.cpp)
target_link_libraries(squid_headless_smoke PRIVATE squid_app)
if(NOT MSVC)
  target_compile_options(squid_headless_smoke PRIVATE -Wall -Wextra)
endif()
add_custom_command(TARGET squid_headless_smoke POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${VOIDCORE_DLL}" "$<TARGET_FILE_DIR:squid_headless_smoke>")
squid_static_runtime(squid_headless_smoke)
# Prefixed, because Void Maiz registers a `headless_smoke` of its own and CTest sees
# both trees. Run ours with `ctest --test-dir build -R squid_`; the siblings' tests
# are theirs to run in their own build directories.
add_test(NAME squid_headless_smoke COMMAND squid_headless_smoke)

add_executable(squid_sim_test tests/sim_test.cpp)
target_link_libraries(squid_sim_test PRIVATE squid_app)
squid_static_runtime(squid_sim_test)
add_custom_command(TARGET squid_sim_test POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${VOIDCORE_DLL}" "$<TARGET_FILE_DIR:squid_sim_test>")
add_test(NAME squid_sim COMMAND squid_sim_test)

# Recon, pure: links squid_read alone, so it needs no Void Core DLL beside it. If this
# test ever needs one, something effectful has leaked into the reader.
add_executable(squid_library_test tests/library_test.cpp)
target_link_libraries(squid_library_test PRIVATE squid_library)
squid_static_runtime(squid_library_test)
add_test(NAME squid_library COMMAND squid_library_test)

add_executable(squid_read_test tests/read_test.cpp)
target_link_libraries(squid_read_test PRIVATE squid_read)
if(NOT MSVC)
  target_compile_options(squid_read_test PRIVATE -Wall -Wextra)
endif()
squid_static_runtime(squid_read_test)
add_test(NAME squid_read COMMAND squid_read_test)

# ocean: the motion summary, pure: links squid_motion alone, no Core DLL beside it.
add_executable(squid_motion_test tests/motion_test.cpp)
target_link_libraries(squid_motion_test PRIVATE squid_motion)
if(NOT MSVC)
  target_compile_options(squid_motion_test PRIVATE -Wall -Wextra)
endif()
squid_static_runtime(squid_motion_test)
add_test(NAME squid_motion COMMAND squid_motion_test)

# molerat: the pure slice, links squid_molerat alone. If this test ever needs a Core DLL
# beside it, an effect has leaked into the library.
add_executable(squid_molerat_test tests/molerat_test.cpp)
target_link_libraries(squid_molerat_test PRIVATE squid_molerat)
if(NOT MSVC)
  target_compile_options(squid_molerat_test PRIVATE -Wall -Wextra)
endif()
squid_static_runtime(squid_molerat_test)
add_test(NAME squid_molerat COMMAND squid_molerat_test)

# forge: the pure half, links squid_forge alone. If this test ever needs a Core DLL
# beside it, an effect has leaked into the library.
add_executable(squid_forge_test tests/forge_test.cpp)
target_link_libraries(squid_forge_test PRIVATE squid_forge)
if(NOT MSVC)
  target_compile_options(squid_forge_test PRIVATE -Wall -Wextra)
endif()
squid_static_runtime(squid_forge_test)
add_test(NAME squid_forge COMMAND squid_forge_test)

# Not a test: a measurement (okf/design/smart-logs.md). How emission and the spatial
# log scale with world size, in milliseconds and in bytes of log.
add_executable(squid_scale_bench tests/scale_bench.cpp)
target_link_libraries(squid_scale_bench PRIVATE squid_app)
squid_static_runtime(squid_scale_bench)
add_custom_command(TARGET squid_scale_bench POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${VOIDCORE_DLL}" "$<TARGET_FILE_DIR:squid_scale_bench>")
