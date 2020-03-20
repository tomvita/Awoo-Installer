#include "tx.h"

#include <string.h>

Service g_tx;

Result txInitialize() {
    return smGetService(&g_tx, "tx");
}

void txExit() {
    serviceClose(&g_tx);
}

Service *txGetServiceSession() {
    return &g_tx;
}

Result txSelectGamecard(const char *path, size_t path_length) {
    return serviceDispatch(&g_tx, 123,
        .buffer_attrs = { SfBufferAttr_In | SfBufferAttr_HipcMapAlias },
        .buffers = { { path, path_length } },
    );
}

Result txGetGamecardStatus(u32 *out) {
    u32 tmp;
    Result rc = serviceDispatchOut(&g_tx, 124, tmp);
    if (R_SUCCEEDED(rc) && out) *out = tmp;
    return rc;
}

Result tx_125() {
    return serviceDispatch(&g_tx, 125);
}

Result txCheckLicense(u32 *out) {
    u32 tmp;
    Result rc = serviceDispatchOut(&g_tx, 126, tmp);
    if (R_SUCCEEDED(rc) && out) *out = tmp;
    return rc;
}

Result tx_127() {
    return serviceDispatch(&g_tx, 127);
}

Result txGetBuildId(u8 *out) {
    return serviceDispatch(&g_tx, 128,
        .buffer_attrs = { SfBufferAttr_Out | SfBufferAttr_HipcMapAlias },
        .buffers = { { out, 0x40 } },
    );
}

Result txIsPro(bool *out) {
    Result rc = serviceDispatch(&g_tx, 129);
    if (rc == 0xa08) {
        *out = false;
        return 0;
    } else if (rc == 0) {
        *out = true;
    }
    return rc;
}

Result txSetFTP(bool mode) {
    u8 tmp = mode;
    return serviceDispatchIn(&g_tx, 130, tmp);
}

Result txGetFTP(bool *mode) {
    Result rc = serviceDispatch(&g_tx, 131);
    if (rc == 0xa08) {
        *mode = false;
        return 0;
    } else if (rc == 0) {
        *mode = true;
    }
    return rc;
}

Result txGetIPAddress(char *buffer) {
    return serviceDispatch(&g_tx, 132,
        .buffer_attrs = { SfBufferAttr_Out | SfBufferAttr_HipcMapAlias },
        .buffers = { { buffer, 0x100 } },
    );
}

Result tx_133() {
    return serviceDispatch(&g_tx, 133);
}

Result txSetStealthMode(bool mode) {
    u8 tmp = mode;
    return serviceDispatchIn(&g_tx, 134, tmp);
}

Result txGetStealthMode(bool *mode) {
    Result rc = serviceDispatch(&g_tx, 135);
    if (rc == 0xa08) {
        *mode = false;
        return 0;
    } else if (rc == 0) {
        *mode = true;
    }
    return rc;
}

Result txIsEmuNand(bool *mode) {
    Result rc = serviceDispatch(&g_tx, 136);
    if (rc == 0xa08) {
        *mode = false;
        return 0;
    } else if (rc == 0) {
        *mode = true;
    }
    return rc;
}

Result txIsFat32(bool *mode) {
    Result rc = serviceDispatch(&g_tx, 137);
    if (rc == 0xa08) {
        *mode = false;
        return 0;
    } else if (rc == 0) {
        *mode = true;
    }
    return rc;
}

Result txSetLdnMitm(bool mode) {
    u8 tmp = mode;
    return serviceDispatchIn(&g_tx, 138, tmp);
}

Result txGetLdnMitm(bool *mode) {
    Result rc = serviceDispatch(&g_tx, 139);
    if (rc == 0xa08) {
        *mode = false;
        return 0;
    } else if (rc == 0) {
        *mode = true;
    }
    return rc;
}
