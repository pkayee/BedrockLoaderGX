#include "libraries/config.h"
#include "libraries/paths.h"

#include "plugin_common.h"

#include <fcntl.h>
#include <unistd.h>
#include <string.h>

static char* trim(char* s)
{
    while (*s == ' ' || *s == '\t') s++;

    char* end = s + strlen(s);
    while (end > s && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r'))
        end--;
    *end = '\0';

    return s;
}

static void applySetting(const char* key, const char* value)
{
    if (strcmp(key, "PluginRoot") == 0) {
        if (value[0] != '/') {
            final_printf("BedrockLoaderGX: ignoring PluginRoot '%s' (must be absolute)\n", value);
            return;
        }

        strncpy(g_pluginRoot, value, PLUGIN_PATH_MAX - 1);
        g_pluginRoot[PLUGIN_PATH_MAX - 1] = '\0';

        final_printf("BedrockLoaderGX: plugin root set to %s\n", g_pluginRoot);
    }
}

void Config::Load(const char* path)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        debug_printf("BedrockLoaderGX: no ini at %s, using defaults\n", path);
        return;
    }

    char buf[1024];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    close(fd);

    if (n <= 0)
        return;

    buf[n] = '\0';

    char* line = buf;
    while (line && *line)
    {
        char* next = strchr(line, '\n');
        if (next)
            *next++ = '\0';

        char* trimmed = trim(line);

        // skip blanks, comments and section headers
        if (*trimmed && *trimmed != ';' && *trimmed != '#' && *trimmed != '[')
        {
            char* eq = strchr(trimmed, '=');
            if (eq) {
                *eq = '\0';
                applySetting(trim(trimmed), trim(eq + 1));
            }
        }

        line = next;
    }
}
