#include "libraries/loader.h"
#include "libraries/hooks.h"
#include "libraries/config.h"
#include "libraries/filesystem.h"
#include "libraries/paths.h"

#include "plugin_common.h"

#include <string.h>
#include <stdio.h>

bool Loader::TryRedirectPath(const char* path, char* outPath, size_t outPathSize)
{
    // Everything we redirect lives under /temp0. Bailing here skips the rule
    if (strncmp(path, "/temp0", 6) != 0)
        return false;

    for (const auto& rule : kRedirectRules)
    {
        const char* match = strstr(path, rule.matchSegment);
        if (match != nullptr)
        {
            // Matching without a trailing slash catches both the directory
            // open (enumeration) and the individual files inside it.
            const char* remainder = match + strlen(rule.matchSegment);

            snprintf(outPath, outPathSize, "%s%s%s",
                     g_pluginRoot, rule.suffix, remainder);
            return true;
        }
    }

    return false;
}

void Loader::Start()
{
    // Order matters: the config can change g_pluginRoot, and the directory
    // tree has to exist before the hooks start redirecting into it.
    Config::Load(PATH_INI);
    FileSystem::EnsureLayout();
    Hooks::Install(&TryRedirectPath);
}

void Loader::Stop()
{
    Hooks::Uninstall();
}
