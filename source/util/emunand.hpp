#pragma once

#include <switch.h>

enum class EmuNAND {
    Disabled,
    Enabled,
    Unknown,
};

EmuNAND GetEmuNAND();
