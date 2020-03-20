#include "cfw.hpp"

#include <switch.h>
#include "logger.hpp"

namespace {

    CustomFirmware cfw = CustomFirmware::Unknown;

    bool IsServiceRunning(const char *name) {
        Handle handle;
        SmServiceName serviceName = smEncodeName(name);
        /* Try to register service. If this fails, we know that there already is a service with this name. */
        Result rc = smRegisterService(&handle, serviceName, false, 1);

        if (R_FAILED(rc)) {
            /* Service name is already occupied. */
            return true;
        } else {
            /* We registered a service so we should close it now. */
            rc = smUnregisterService(serviceName);
            rc = svcCloseHandle(handle);
            return false;
        }
    }

    void CacheCFW() {
        if (cfw != CustomFirmware::Unknown)
            return;

        /* Service rnx is ReiNX exclusive. */
        if (IsServiceRunning("rnx")) {
            LOG("Running on ReiNX");
            cfw = CustomFirmware::ReiNX;
            return;
        }

        /* Service tx get's spoofed by ReiNX but we already checked that. */
        if (IsServiceRunning("tx")) {
            LOG("Running on SX OS");
            cfw = CustomFirmware::SXOS;
            return;
        }

        /* We seem to be on Atmosphère. */
        LOG("Running on Atmosphère");
        cfw = CustomFirmware::Atmosphere;
    }

}

CustomFirmware GetCustomFirmware() {
    CacheCFW();
    return cfw;
}
