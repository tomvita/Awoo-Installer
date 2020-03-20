/**
 * @file smc.h
 * @brief Wrappers for secure monitor calls.
 * @copyright libnx Authors
 */
#pragma once
#include <switch.h>

namespace emuMMC {

    enum class Type : u32 {
        NONE = 0,
        PARTITION = 1,
        FILES = 2,
    };

    enum class MMC : u32 {
        NAND = 0,
        SD = 1,
        GC = 2,
    };

    struct BaseConfig {
        u32 magic;
        Type type;
        u32 id;
        u32 fs_version;
    };

    struct PartitionConfig {
        u64 startSector;
    };

    struct Config {
        BaseConfig base_cfg;
        union {
            PartitionConfig partition_cfg;
        };
    };

}

enum class AmsSplConfigItem {
    DisableProgramVerification = 1,
    DramId = 2,
    SecurityEngineIrqNumber = 3,
    Version = 4,
    HardwareType = 5,
    IsRetail = 6,
    IsRecoveryBoot = 7,
    DeviceId = 8,
    BootReason = 9,
    MemoryArrange = 10,
    IsDebugMode = 11,
    KernelMemoryConfiguration = 12,
    IsChargerHiZModeEnabled = 13,
    IsKiosk = 14,
    NewHardwareType = 15,
    NewKeyGeneration = 16,
    Package2Hash = 17,

    ExosphereVersion = 65000,
    NeedsReboot      = 65001,
    NeedsShutdown    = 65002,
    ExosphereVerHash = 65003,
    HasRcmBugPatch   = 65004,
};

Result smcGetConfig(AmsSplConfigItem config_item, u64 *out_config);

void smcRebootToRcm(void);
void smcRebootToIramPayload(void);
void smcPerformShutdown(void);

Result smcCopyToIram(uintptr_t iram_addr, const void *src_addr, u32 size);
Result smcCopyFromIram(void *dst_addr, uintptr_t iram_addr, u32 size);

Result smcReadWriteRegister(u32 phys_addr, u32 value, u32 mask);

Result smcWriteAddress8(void *dst_addr, u8 val);
Result smcWriteAddress16(void *dst_addr, u16 val);
Result smcWriteAddress32(void *dst_addr, u32 val);
Result smcWriteAddress64(void *dst_addr, u64 val);

Result smcGetEmummcConfig(emuMMC::MMC mmc_id, emuMMC::Config *out_cfg, void *out_paths);
