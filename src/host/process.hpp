/* host/process.hpp — running another program: the effect every toolchain row is made of.
 *
 * Two shapes, and only two:
 *
 *   capture   run a command, wait, return what it printed (bounded) and its exit code.
 *             For questions: `ollama list`, `nvidia-smi`, `python --version`.
 *   launch    start a program and do not wait. For the runtime (Play) and the 3D table,
 *             which are their own windows with their own lifetimes.
 *
 * Arguments are a vector, never a command line the caller assembled, so a path with a
 * space or an apostrophe (every Windows game folder has both) cannot split. On Windows
 * the quoting is CommandLineToArgvW's rules; on Linux it is execvp, which has none.
 */
#pragma once

#include <string>
#include <vector>

namespace squid::host {

struct Captured {
    bool ran = false;        // false: the program was not found or could not start
    int exit_code = -1;
    std::string out;         // stdout and stderr together, at most `limit` bytes
};

/* Run argv[0] (looked up on PATH unless it is a path) with the rest as arguments. */
Captured capture(const std::vector<std::string>& argv, size_t limit = 64 * 1024, int timeout_ms = 20000);

/* Start argv[0] detached, in `cwd` when given. True when it started. */
bool launch(const std::vector<std::string>& argv, const std::string& cwd = {});

/* The full path of `exe` on PATH (adds .exe on Windows), or "". */
std::string which(const std::string& exe);

/* Open a file or folder with the system's handler (Explorer, xdg-open). */
bool open_with_system(const std::string& path);

}  // namespace squid::host
