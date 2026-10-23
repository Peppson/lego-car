#pragma once
#include "settings.h"

struct ColorSettings
{
    bool hasChanged = true;
    uint32_t frontHeadLight = DEFAULT_HEADLIGHT;
    uint32_t frontTurnSignal = DEFAULT_TURNSIGNAL;
    uint32_t cabinLight = DEFAULT_CABIN;
    uint32_t backBreakLight = DEFAULT_BREAKLIGHT;
};

inline ColorSettings colorSettings;
