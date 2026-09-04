#pragma once
#include <stddef.h>

namespace Loader {
    void Start();
    void Stop();

    // Passed to Hooks::Install as the resolver.
    bool TryRedirectPath(const char* path, char* outPath, size_t outPathSize);
}
