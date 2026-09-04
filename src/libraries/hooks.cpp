#include "libraries/hooks.h"

#include <GoldHEN/Common.h>
#include "plugin_common.h"

#include <sys/stat.h>

// All three use HOOK32, matching afr.prx. HOOK (x64) faults in stub memory
// on these functions - see README.
HOOK_INIT(fopen);
HOOK_INIT(sceKernelOpen);
HOOK_INIT(sceKernelStat);

static Hooks::PathResolver g_resolver = nullptr;

FILE* fopen_hook(const char* path, const char* mode)
{
    char redirected[MAX_PATH_];

    if (g_resolver && g_resolver(path, redirected, sizeof(redirected)))
    {
        FILE* fp = HOOK_CONTINUE(fopen,
                                 FILE *(*)(const char*, const char*),
                                 redirected, mode);
        if (fp) {
            final_printf("redirected %s -> %s\n", path, redirected);
            return fp;
        }
        // redirect target absent - fall through to the real path
    }

    debug_printf("fopen: %s\n", path);
    return HOOK_CONTINUE(fopen,
                         FILE *(*)(const char*, const char*),
                         path, mode);
}

s32 sceKernelOpen_hook(const char* path, s32 flags, OrbisKernelMode mode)
{
    char redirected[MAX_PATH_];

    if (g_resolver && g_resolver(path, redirected, sizeof(redirected)))
    {
        s32 fd = HOOK_CONTINUE(sceKernelOpen,
                               s32 (*)(const char*, s32, OrbisKernelMode),
                               redirected, flags, mode);
        if (fd >= 0) {
            final_printf("redirected %s -> %s (fd=0x%08x)\n", path, redirected, fd);
            return fd;
        }
    }

    debug_printf("open: %s (flags=0x%x)\n", path, flags);
    return HOOK_CONTINUE(sceKernelOpen,
                         s32 (*)(const char*, s32, OrbisKernelMode),
                         path, flags, mode);
}

// Needed for loose-folder packs: a directory walk stats each entry before
// descending, and an unhooked stat reports the redirected pack as missing.
//
// NOTE: calls stat() directly rather than HOOK_CONTINUE. Going through the
// trampoline here faults; this matches how afr.prx does it.
s32 sceKernelStat_hook(char* path, struct stat* buf)
{
    char redirected[MAX_PATH_];

    if (g_resolver && g_resolver(path, redirected, sizeof(redirected)))
    {
        s32 ret = stat(redirected, buf);
        if (ret >= 0)
            return ret;
    }

    return stat(path, buf);
}

void Hooks::Install(PathResolver resolver)
{
    g_resolver = resolver;

    HOOK32(sceKernelOpen);
    HOOK32(sceKernelStat);
    HOOK32(fopen);
}

void Hooks::Uninstall()
{
    UNHOOK(sceKernelOpen);
    UNHOOK(sceKernelStat);
    UNHOOK(fopen);

    g_resolver = nullptr;
}
