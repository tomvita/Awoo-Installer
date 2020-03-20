#pragma once

#include <switch.h>

#ifdef __cplusplus
extern "C" {
#endif

Result txInitialize();

void txExit();

Service *txGetServiceSession();

Result txSelectGamecard(const char *path, size_t path_length); /* Needs to include leading sdmc: */
Result txGetGamecardStatus(u32 *out);                          /* 0 not detected, 0x100 detected. */
Result tx_125();                                               /* Not used by ROMMENU */
Result txCheckLicense(u32 *out);
Result tx_127();                  /* Called after receiving license and before writing it to sd. */
Result txGetBuildId(u8 *buildId); /* Needs to be 0x40 wide. */
Result txIsPro(bool *out);
Result txSetFTP(bool mode);
Result txGetFTP(bool *mode);
Result txGetIPAddress(char *buffer); /* Needs to be 0x100 wide. */
Result tx_133();                     /* Installation related. */
Result txSetStealthMode(bool mode);
Result txGetStealthMode(bool *mode);
Result txIsEmuNand(bool *mode);
Result txIsFat32(bool *mode);
Result txSetLdnMitm(bool mode);
Result txGetLdnMitm(bool *mode);

#ifdef __cplusplus
}
#endif
