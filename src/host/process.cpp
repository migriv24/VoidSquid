/* host/process.cpp — see process.hpp. One Windows half, one POSIX half. */
#include "host/process.hpp"

#include <chrono>
#include <cstdlib>
#include <filesystem>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shellapi.h>
#else
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
extern char** environ;
#endif

namespace squid::host {

namespace fs = std::filesystem;

std::string which(const std::string& exe) {
    if (exe.empty()) return {};
    std::error_code ec;
    if (exe.find('/') != std::string::npos || exe.find('\\') != std::string::npos)
        return fs::exists(exe, ec) ? exe : std::string();
    const char* path = std::getenv("PATH");
    if (!path) return {};
#ifdef _WIN32
    const char sep = ';';
    const std::vector<std::string> exts = {".exe", ".cmd", ".bat", ""};
#else
    const char sep = ':';
    const std::vector<std::string> exts = {""};
#endif
    std::string p = path;
    size_t start = 0;
    while (start <= p.size()) {
        size_t end = p.find(sep, start);
        if (end == std::string::npos) end = p.size();
        const std::string dir = p.substr(start, end - start);
        if (!dir.empty()) {
            for (const auto& e : exts) {
                const fs::path cand = fs::path(dir) / (exe + e);
                if (fs::is_regular_file(cand, ec)) return cand.string();
            }
        }
        start = end + 1;
    }
    return {};
}

#ifdef _WIN32

namespace {
std::wstring widen(const std::string& s) {
    if (s.empty()) return {};
    const int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring w(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), w.data(), n);
    return w;
}

/* CommandLineToArgvW's quoting rules, so a path with spaces or quotes survives. */
std::wstring quote_arg(const std::wstring& a) {
    if (!a.empty() && a.find_first_of(L" \t\n\v\"") == std::wstring::npos) return a;
    std::wstring q = L"\"";
    for (size_t i = 0;; ++i) {
        size_t backslashes = 0;
        while (i < a.size() && a[i] == L'\\') { ++i; ++backslashes; }
        if (i == a.size()) { q.append(backslashes * 2, L'\\'); break; }
        if (a[i] == L'"') { q.append(backslashes * 2 + 1, L'\\'); q.push_back(L'"'); }
        else { q.append(backslashes, L'\\'); q.push_back(a[i]); }
    }
    q.push_back(L'"');
    return q;
}

std::wstring command_line(const std::vector<std::string>& argv, const std::string& resolved) {
    std::wstring cl;
    for (size_t i = 0; i < argv.size(); ++i) {
        if (i) cl += L' ';
        cl += quote_arg(widen(i == 0 ? resolved : argv[i]));
    }
    return cl;
}

/* A .cmd or .bat is not an executable: CreateProcess needs cmd.exe in front of it. */
std::string resolve(const std::vector<std::string>& argv, std::vector<std::string>& real) {
    real = argv;
    std::string exe = which(argv[0]);
    if (exe.empty()) return {};
    const std::string ext = fs::path(exe).extension().string();
    if (ext == ".cmd" || ext == ".bat") {
        real.insert(real.begin(), {"cmd.exe", "/d", "/c"});
        real[3] = exe;
        return which("cmd.exe").empty() ? std::string("C:\\Windows\\System32\\cmd.exe") : which("cmd.exe");
    }
    return exe;
}
}  // namespace

Captured capture(const std::vector<std::string>& argv, size_t limit, int timeout_ms) {
    Captured c;
    if (argv.empty()) return c;
    std::vector<std::string> real;
    const std::string exe = resolve(argv, real);
    if (exe.empty()) return c;

    SECURITY_ATTRIBUTES sa{sizeof(sa), nullptr, TRUE};
    HANDLE rd = nullptr, wr = nullptr;
    if (!CreatePipe(&rd, &wr, &sa, 0)) return c;
    SetHandleInformation(rd, HANDLE_FLAG_INHERIT, 0);
    STARTUPINFOW si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = wr;
    si.hStdError = wr;
    si.hStdInput = nullptr;
    PROCESS_INFORMATION pi{};
    std::wstring cl = command_line(real, exe);
    const BOOL ok = CreateProcessW(widen(exe).c_str(), cl.data(), nullptr, nullptr, TRUE,
                                   CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
    CloseHandle(wr);
    if (!ok) { CloseHandle(rd); return c; }
    c.ran = true;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
    char buf[4096];
    for (;;) {
        DWORD avail = 0;
        if (!PeekNamedPipe(rd, nullptr, 0, nullptr, &avail, nullptr)) break;   // closed: the child is done
        if (avail > 0) {
            DWORD got = 0;
            if (!ReadFile(rd, buf, sizeof(buf), &got, nullptr) || got == 0) break;
            if (c.out.size() < limit) c.out.append(buf, std::min<size_t>(got, limit - c.out.size()));
            continue;
        }
        if (WaitForSingleObject(pi.hProcess, 15) == WAIT_OBJECT_0) {
            // drain what is left
            while (PeekNamedPipe(rd, nullptr, 0, nullptr, &avail, nullptr) && avail > 0) {
                DWORD got = 0;
                if (!ReadFile(rd, buf, sizeof(buf), &got, nullptr) || got == 0) break;
                if (c.out.size() < limit) c.out.append(buf, std::min<size_t>(got, limit - c.out.size()));
            }
            break;
        }
        if (std::chrono::steady_clock::now() > deadline) {
            TerminateProcess(pi.hProcess, 1);
            c.out += "\n[stopped: no answer within the time limit]";
            break;
        }
    }
    WaitForSingleObject(pi.hProcess, 2000);
    DWORD code = 0;
    if (GetExitCodeProcess(pi.hProcess, &code)) c.exit_code = (int)code;
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(rd);
    return c;
}

bool launch(const std::vector<std::string>& argv, const std::string& cwd) {
    if (argv.empty()) return false;
    std::vector<std::string> real;
    const std::string exe = resolve(argv, real);
    if (exe.empty()) return false;
    STARTUPINFOW si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};
    std::wstring cl = command_line(real, exe);
    const std::wstring wcwd = widen(cwd);
    const BOOL ok = CreateProcessW(widen(exe).c_str(), cl.data(), nullptr, nullptr, FALSE,
                                   DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP, nullptr,
                                   cwd.empty() ? nullptr : wcwd.c_str(), &si, &pi);
    if (!ok) return false;
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
}

bool open_with_system(const std::string& path) {
    return (INT_PTR)ShellExecuteW(nullptr, L"open", widen(path).c_str(), nullptr, nullptr, SW_SHOWNORMAL) > 32;
}

#else  // POSIX

Captured capture(const std::vector<std::string>& argv, size_t limit, int timeout_ms) {
    Captured c;
    if (argv.empty() || which(argv[0]).empty()) return c;
    int fds[2];
    if (pipe(fds) != 0) return c;
    posix_spawn_file_actions_t fa;
    posix_spawn_file_actions_init(&fa);
    posix_spawn_file_actions_adddup2(&fa, fds[1], 1);
    posix_spawn_file_actions_adddup2(&fa, fds[1], 2);
    posix_spawn_file_actions_addclose(&fa, fds[0]);
    std::vector<char*> args;
    for (const auto& a : argv) args.push_back(const_cast<char*>(a.c_str()));
    args.push_back(nullptr);
    pid_t pid = 0;
    const int rc = posix_spawnp(&pid, argv[0].c_str(), &fa, nullptr, args.data(), environ);
    posix_spawn_file_actions_destroy(&fa);
    close(fds[1]);
    if (rc != 0) { close(fds[0]); return c; }
    c.ran = true;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
    char buf[4096];
    for (;;) {
        pollfd p{fds[0], POLLIN, 0};
        const int r = poll(&p, 1, 50);
        if (r > 0) {
            const ssize_t got = read(fds[0], buf, sizeof(buf));
            if (got <= 0) break;
            if (c.out.size() < limit) c.out.append(buf, std::min<size_t>((size_t)got, limit - c.out.size()));
        }
        if (std::chrono::steady_clock::now() > deadline) {
            kill(pid, SIGTERM);
            c.out += "\n[stopped: no answer within the time limit]";
            break;
        }
    }
    close(fds[0]);
    int status = 0;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) c.exit_code = WEXITSTATUS(status);
    return c;
}

bool launch(const std::vector<std::string>& argv, const std::string& cwd) {
    if (argv.empty() || which(argv[0]).empty()) return false;
    const pid_t pid = fork();
    if (pid < 0) return false;
    if (pid == 0) {
        setsid();
        if (!cwd.empty() && chdir(cwd.c_str()) != 0) _exit(127);
        std::vector<char*> args;
        for (const auto& a : argv) args.push_back(const_cast<char*>(a.c_str()));
        args.push_back(nullptr);
        // a second fork so the launched program is not our child (no zombie when it exits)
        if (fork() == 0) { execvp(args[0], args.data()); _exit(127); }
        _exit(0);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    return true;
}

bool open_with_system(const std::string& path) {
    return launch({"xdg-open", path});
}

#endif

}  // namespace squid::host
