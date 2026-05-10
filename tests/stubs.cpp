// Stub definitions for globals normally provided by ColAndreas.cpp and amxplugin.cpp.
// The test binary is not a SA-MP plugin, so we provide minimal no-op replacements.

#include "DynamicWorld.h"

static void noop_logprintf(char*, ...) {}

logprintf_t        logprintf    = noop_logprintf;
bool               colInit      = false;
bool               colDataLoaded = false;
cell               nullAddress  = 0;
ColAndreasWorld*   collisionWorld = nullptr;
