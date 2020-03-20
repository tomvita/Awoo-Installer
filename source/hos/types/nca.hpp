#pragma once

#include <switch.h>

#define MAGIC_NCA3 0x3341434E

struct HierarchicalSha256 {
    u8 hash[0x20];
    u32 blockSize;
    u32 unk_x24;
    u64 hashTableOffset;
    u64 hashTableSize;
    u64 relativeOffset;
    u64 relativeSize;
};
static_assert(sizeof(HierarchicalSha256) == 0x48);

struct HierarchicalIntegrity {
    u64 pad_x0;
    u32 magic;
    u32 unk_xc;
    u32 masterHashSize;
    u32 unk_x14;
    struct {
        u64 offset;
        u64 size;
        u32 blockSize;
        u32 pad_x14;
    } level[6];
    u8 pad_xa8[0x20];
    u8 hash[0x20];
    u8 pad_xe8[0x10];
};
static_assert(sizeof(HierarchicalIntegrity) == 0xF8);

struct PatchInfo {
    struct {
        u64 offset;
        u64 size;
        u32 magic;
        u32 unk_x14;
        u32 unk_x18;
        u32 unk_x1c;
    } entry[2];
};

enum class NcaFsType : u8 {
    RomFs = 0,
    PartitionFs = 1,
};

enum class NcaFsHashType : u8 {
    Auto = 0,
    HierarchicalSha256 = 2,
    HierarchicalIntegrity = 3,
};

enum class NcaFsEncryptionType : u8 {
    Auto = 0,
    None = 1,
    AesCtrOld = 2,
    AesCtr = 3,
    AesCtrEx = 4,
};

struct NcaFsEntry {
    u32 startOffset;
    u32 endOffset;
    u8 pad_x8[0x8];
};
static_assert(sizeof(NcaFsEntry) == 0x10);

struct NcaFsHeader {
    u16 version;
    NcaFsType fsType;
    NcaFsHashType hashType;
    NcaFsEncryptionType encryptionType;
    u8 pad_x5[3];
    union HashInfo {
        HierarchicalSha256 sha256;
        HierarchicalIntegrity integrity;
    };
    PatchInfo pathInfo;
    u32 generation;
    u32 secureValue;
    u8 sparseInfo[0x30];
    u8 pad_x178[0x88];
};

enum class NcaDistributionType : u8 {
    System = 0,
    Gamecard = 1,
};

enum class NcaContentType : u8 {
    Program = 0,
    Meta = 1,
    Control = 2,
    Manual = 3,
    Data = 4,
    PublicData = 5,
};

enum class NcaKeyAreaEncryptionKeyIndex : u8 {
    Application = 0,
    Ocean = 1,
    System = 2,
};

enum class NcaKeyGenerationOld : u8 {
    Version100 = 0,
    Version200 = 2,
};

enum class NcaKeyGeneration : u8 {
    Version301 = 3,
    Version400 = 4,
    Version500 = 5,
    Version600 = 6,
    Version620 = 7,
    Version700 = 8,
    Version810 = 9,
    Version900 = 10,
    Version910 = 11,
    Invalid = 0xff,
};

enum class NcaFixedKeyGeneration : u8 {
    Version100 = 0,
    Version910 = 1,
};

struct NcaHeader {
    u8 fixedKeySignature[0x100]; /* RSA-2048 signature over the header using a fixed key. */
    u8 npdmKeySignature[0x100];  /* RSA-2048 signature over the header using a key from NPDM. */
    u32 magic;                   /* Magicnum "NCA3" ("NCA2", "NCA1" or "NCA0" for pre-1.0.0 NCAs) */
    NcaDistributionType distribution;
    NcaContentType content;
    NcaKeyGenerationOld oldKeyGeneration;
    NcaKeyAreaEncryptionKeyIndex keyIndex;
    u64 contentSize; /* Entire archive size. */
    u64 programId;
    u32 contentIndex;
    union {
        u32 sdkVersion; /* What SDK was this built with? */
        struct {
            u8 sdkRevision;
            u8 sdkMicro;
            u8 sdkMinor;
            u8 sdkMajor;
        };
    };
    NcaKeyGeneration keyGeneration;           /* Which key generation do we need. */
    NcaFixedKeyGeneration fixedKeyGeneration; /* Which fixed key to use. */
    u8 pad_x220[0xe];
    FsRightsId rightsId;
    NcaFsEntry fsEntries[4];
    u8 fsEntryHashes[0x20][4];
    u8 encryptedKeyArea[0x10][4];
    u8 pad[0xc0];
};
//static_assert(sizeof(NcaHeader) == 0x340);
static_assert(sizeof(NcaHeader) == 0x400);
