#pragma once
#include <stddef.h>

namespace Hooks {
    using PathResolver = bool (*)(const char* path, char* outPath, size_t outPathSize);

    void Install(PathResolver resolver);
    void Uninstall();
}
