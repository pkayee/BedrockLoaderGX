#include <libraries/filesystem.h>
#include <libraries/paths.h>
#include <res/icon_data.h>

#include "plugin_common.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>


static bool ensureDir(const char* path)
{
    struct stat st;
    if (stat(path, &st) == 0)
        return true;

    if (mkdir(path, 0777) == 0) {
        debug_printf("BedrockLoaderGX: created %s\n", path);
        return true;
    }

    final_printf("BedrockLoaderGX: FAILED to create %s\n", path);
    return false;
}

static void writeIfAbsent(const char* path, const void* data, size_t len)
{
    int fd = open(path, O_RDONLY);
    if (fd >= 0) { close(fd); return; }   // already there, leave it alone

    fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if (fd < 0) {
        final_printf("BedrockLoaderGX: could not write %s\n", path);
        return;
    }

    const char* p = (const char*)data;
    size_t written = 0;
    while (written < len) {
        ssize_t n = write(fd, p + written, len - written);
        if (n <= 0) break;
        written += (size_t)n;
    }
    close(fd);

    if (written != len)
        final_printf("BedrockLoaderGX: short write to %s (%zu/%zu)\n", path, written, len);
    else
        debug_printf("BedrockLoaderGX: wrote %s\n", path);
}

void FileSystem::EnsureLayout()
{
    ensureDir(DEFAULT_PLUGIN_ROOT);
    ensureDir(DEFAULT_PLUGIN_ROOT "/res");
    ensureDir(DEFAULT_PLUGIN_ROOT "/settings");

    if (!ensureDir(g_pluginRoot)) {
        final_printf("BedrockLoaderGX: falling back to %s\n", DEFAULT_PLUGIN_ROOT);
        strncpy(g_pluginRoot, DEFAULT_PLUGIN_ROOT, PLUGIN_PATH_MAX - 1);
        g_pluginRoot[PLUGIN_PATH_MAX - 1] = '\0';
        ensureDir(g_pluginRoot);
    }

    char path[PLUGIN_PATH_MAX];
    for (const auto& rule : kRedirectRules) {
        snprintf(path, sizeof(path), "%s%s", g_pluginRoot, rule.suffix);
        ensureDir(path);
    }

    static const char* kDefaultIni =
        "[Settings]\n"
        "PluginRoot=" DEFAULT_PLUGIN_ROOT "\n";

    writeIfAbsent(PATH_INI,  kDefaultIni, strlen(kDefaultIni));
    writeIfAbsent(PATH_ICON, icon_data, icon_data_len);
}
