#pragma once

enum class CustomFirmware {
    Atmosphere,
    SXOS,
    ReiNX,
    Unknown,
};

CustomFirmware GetCustomFirmware();
