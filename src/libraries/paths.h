#pragma once

#define PLUGIN_PATH_MAX 260

#define DEFAULT_PLUGIN_ROOT "/data/BedrockLoaderGX"

// Where packs live. Configurable at runtime via the ini; both the directory
// creation in filesystem.cpp and the redirects in loader.cpp derive from this,
// so the two cannot drift.
inline char g_pluginRoot[PLUGIN_PATH_MAX] = DEFAULT_PLUGIN_ROOT;

// The plugin's own files stay at a fixed location - the config cannot live
// inside the directory that the config chooses.
#define PATH_ICON DEFAULT_PLUGIN_ROOT "/res/icon.jpg"
#define PATH_INI  DEFAULT_PLUGIN_ROOT "/settings/BedrockLoaderGX.ini"

struct RedirectRule {
    const char* matchSegment;   // what the game asks for
    const char* suffix;         // where under the root it lives
};

inline const RedirectRule kRedirectRules[] = {
    { "/games/com.mojang/resource_packs", "/resource_packs" },
    { "/games/com.mojang/behavior_packs", "/behavior_packs" },
};
