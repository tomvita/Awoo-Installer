#include "emunand.hpp"

#include "../hos/hos.hpp"
#include "cfw.hpp"
#include "logger.hpp"
#include "smc/smc.hpp"
#include "tx/tx.h"

#define EMUMMC_STORAGE_MAGIC 0x30534645 /* EFS0, EmuFS0 */

namespace {

    EmuNAND status = EmuNAND::Unknown;

    void CacheEmuMMC() {
        static struct
        {
            char storage_path[0x7F + 1];
            char nintendo_path[0x7F + 1];
        } __attribute__((aligned(0x1000))) paths;

        emuMMC::Config cfg;
        R_RETURN(smcGetEmummcConfig(emuMMC::MMC::NAND, &cfg, &paths));

        if (cfg.base_cfg.magic == EMUMMC_STORAGE_MAGIC) {
            status = EmuNAND::Enabled;
        } else {
            status = EmuNAND::Disabled;
        }
    }

    void CacheTxNand() {
        R_RETURN(txInitialize());
        ScopeGuard tx_guard([] { txExit(); });
        bool isEnabled;
        R_RETURN(txIsEmuNand(&isEnabled));

        if (isEnabled) {
            status = EmuNAND::Enabled;
        } else {
            status = EmuNAND::Disabled;
        }
    }

    void CacheEmuNAND() {
        if (status != EmuNAND::Unknown)
            return;

        switch (GetCustomFirmware()) {
            case CustomFirmware::Atmosphere:
                CacheEmuMMC();
                return;
            case CustomFirmware::SXOS:
                CacheTxNand();
                return;
            case CustomFirmware::ReiNX:
                /* ReiNX doesn't have an EmuNAND solution. */
                status = EmuNAND::Disabled;
            default:
                return;
        }
    }

}

EmuNAND GetEmuNAND() {
    CacheEmuNAND();

    return status;
}
