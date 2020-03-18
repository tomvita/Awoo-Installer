#pragma once
#include "hos/hos.hpp"
#include "util/awoo_result.hpp"
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
        /* TODO */
        return ResultSuccess();
    }

    Result Start() {
        /* Try to open Source. */
        R_TRY(m_archive.Open());

        /* Receive entries. */
        std::vector<FileEntry> entries;
        R_TRY(m_archive.GetEntries(&entries));

        /* Iterate over found entries. */
        DEBUG_RUN({
            for (const FileEntry &entry : entries)
                std::printf("entry: %s\n", entry.name.c_str());
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
