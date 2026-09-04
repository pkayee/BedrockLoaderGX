#include "libraries/loader.h"

#include "plugin_common.h"

#include <string.h>
#include <stdio.h>

attr_public const char *g_pluginName    = "BedrockLoaderGX";
attr_public const char *g_pluginDesc    = "Redirects Bedrock resource/behavior packs";
attr_public const char *g_pluginAuth    = "unknown";
attr_public u32         g_pluginVersion = 0x00000100; // 1.00

// Icon URIs: "cxml://psnotification/tex_icon_system" and friends, or a path
// to an image ShellUI can read. See the PS4-Notify README for the full list.
static void notify(const char *msg,
                   const char *iconUri = "cxml://psnotification/tex_icon_system")
{
    OrbisNotificationRequest req;

    memset(&req, 0, sizeof(req));

    req.type            = NotificationRequest;
    req.targetId        = -1;
    req.useIconImageUri = 1;

    strncpy(req.iconUri, iconUri, sizeof(req.iconUri) - 1);
    strncpy(req.message, msg,     sizeof(req.message) - 1);

    // Non-blocking: module_start must not stall the title's startup.
    sceKernelSendNotificationRequest(0, &req, sizeof(req), 0);
}

// GoldHEN's plugin loader looks for plugin_load/plugin_unload but fails to
// find them even in the official plugins, so the real work goes in
// module_start - which is what game_patch.prx does too.
extern "C" {

    s32 attr_module_hidden module_start(s64 argc, const void *args)
    {
        final_printf("[GoldHEN] <%s\\Ver.0x%08x> %s\n",
                     g_pluginName, g_pluginVersion, __func__);

        Loader::Start();

        char msg[128];
        snprintf(msg, sizeof(msg), "%s v%d.%02d loaded",
                 g_pluginName,
                 (g_pluginVersion >> 8) & 0xFF,
                 g_pluginVersion & 0xFF);
        notify(msg);

        return 0;
    }

    s32 attr_module_hidden module_stop(s64 argc, const void *args)
    {
        final_printf("[GoldHEN] <%s\\Ver.0x%08x> %s\n",
                     g_pluginName, g_pluginVersion, __func__);

        Loader::Stop();

        return 0;
    }

}
