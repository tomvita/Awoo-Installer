#pragma once
#include "hos/hos.hpp"
#include "util/awoo_result.hpp"
#include "util/crypto.hpp"
#include "util/logger.hpp"

#include <switch.h>
#include <utility>
#include <vector>

typedef bool (*ProgressCallback)(int current, int count, double percentage);

template <typename Archive>
class Install {
  private:
    NcmStorageId m_storageId;
    Archive m_archive;

  public:
    template <typename... Args>
    Install(NcmStorageId storageId, Args &&... args)
        : m_storageId(storageId), m_archive(std::forward<Args>(args)...) {}

    Result Verify() {
        LOG("Start verification!");

        /* Try to open Source. */
        R_TRY(m_archive.Open());

        /* Receive entries. */
        std::vector<FileEntry> entries;
        R_TRY(m_archive.GetEntries(&entries));

        /* Iterate over found entries. */
        for (const FileEntry &entry : entries) {
            /* Ignore non-NCA's. */
            if (entry.name.substr(entry.name.size() - 4) != ".nca")
                continue;

            LOG("NCA: %s detected.", entry.name.c_str());

            /* Open entry. */
            LOG("Opening entry.");
            R_TRY(m_archive.OpenEntry(entry));

            /* Read header. */
            LOG("Reading header.");
            u64 read_size;
            NcaHeader header;
            R_TRY(m_archive.Read(&read_size, (u8*)&header, 0, sizeof(NcaHeader)));
            R_UNLESS(read_size == sizeof(NcaHeader), ResultEndOfFile());

            /* Decrypt header. */
            LOG("Decrypting header.");
            auto decryptor = Crypto::GetHeaderDecryptor();
            decryptor.Decrypt(&header, &header, sizeof(NcaHeader), 0, 0x200);

            PRINT_BYTES(&header.magic, 0x10);

            /* Verify header. */
            LOG("Verifying header.");
            R_TRY(Crypto::VerifyNcaHeader(&header));

            LOG("entry: %s", entry.name.c_str());
        }

        LOG("End verification!");
        return ResultSuccess();
    }

    Result Start() {
        /* Receive entries. */
        std::vector<FileEntry> entries;
        R_TRY(m_archive.GetEntries(&entries));

        /* Iterate over found entries. */
        DEBUG_RUN({
            for (const FileEntry &entry : entries)
                LOG("entry: %s", entry.name.c_str());
        })

        LOG("done!");
        return ResultSuccess();
    }

    Result InstallNCA(const FileEntry &entry) {
        NcmContentStorage ncm_storage;
        R_TRY(ncmOpenContentStorage(&ncm_storage, m_storageId));
        IContentStorage storage(ncm_storage);
    }
};
