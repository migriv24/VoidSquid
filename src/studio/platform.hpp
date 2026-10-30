/* studio/platform.hpp — the desktop's half of the studio: dialogs and textures.
 *
 * File dialogs: Windows' own; on Linux the helper the desktop already ships (zenity,
 * kdialog, yad, qarma), as Void Hormiga does (its 2026-09-15 lesson: a dialog with a
 * Windows branch and nothing else returns "" on Linux, which reads as "cancelled").
 * When there is no helper, `dialog_problem()` says which one to install.
 *
 * Textures: PNG thumbnails decoded with stb_image and uploaded with GL 1.1 calls, cached
 * by path. Loading a picture to SHOW it is a view's read, like a font, not an effect
 * that changes anything; nothing here writes.
 */
#pragma once

#include <string>

namespace squid::studio {

enum class FileKind { Any, Project, Library, Model, Audio, Game };

std::string pick_file(FileKind kind, const std::string& title = "Choose a file");
std::string pick_folder(const std::string& title = "Choose a folder");
std::string save_file(const std::string& suggested, FileKind kind);
/* Why the last dialog returned nothing when it was not a cancel; cleared by reading. */
std::string dialog_problem();

struct Texture {
    unsigned id = 0;
    int w = 0, h = 0;
    bool ok() const { return id != 0; }
};
/* Load (once) and return the texture for a PNG/JPG on disk; {0} when it cannot. */
Texture texture(const std::string& abs_path);
/* Forget every texture (when the library changes). */
void drop_textures();

}  // namespace squid::studio
