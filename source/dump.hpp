#pragma once
#include "common.hpp"
#include "hos/hos.hpp"
#include "util/double_buffer.hpp"
#include "util/logger.hpp"

#include <vector>

template <typename Destination, typename Source, typename Intermediate>
Result doubleDump(Destination &dst, Source &src) {
    /* Try to open Source. */
    R_TRY(src.Open());
    ScopeGuard src_close([&] { src.Close(); });

    /* Receive entries. */
    std::vector<FileEntry> entries;
    R_TRY(src.GetEntries(&entries));

    /* Iterate over found entries. */
    for (const FileEntry &entry : entries) {
        /* Open file for read. */
        LOG("opening entry: %s", entry.name.c_str());
        R_TRY(src.OpenEntry(entry));

        /* Allocate new entry for write. */
        Intermediate fileDst;
        R_TRY(dst.Allocate(&fileDst, entry));

        /* Read and write entry. */
        {
            DoubleBuffer buffer(
                entry.size,
                [](void *user, u64 *read, u8 *buffer, u64 offset, u64 size) {
                    Source *src = static_cast<Source *>(user);
                    return src->Read(read, buffer, offset, size);
                },
                &src,
                [](void *user, const u8 *buffer, u64 offset, u64 size) {
                    Intermediate *dst = static_cast<Intermediate *>(user);
                    return dst->Write(buffer, offset, size);
                },
                &fileDst);

            Thread read_thread, write_thread;
            R_TRY(threadCreate(&read_thread, ReadFunction, &buffer, nullptr, 0x4000, 0x2C, 1));
            R_TRY(threadCreate(&write_thread, WriteFunction, &buffer, nullptr, 0x4000, 0x2C, 2));

            R_TRY(threadStart(&read_thread));
            R_TRY(threadStart(&write_thread));

            R_TRY(threadWaitForExit(&read_thread));
            R_TRY(threadWaitForExit(&write_thread));

            R_TRY(threadClose(&read_thread));
            R_TRY(threadClose(&write_thread));

            R_LOG(buffer.lastResult);
        }

        LOG("closing entry: %s", entry.name.c_str());
        src.CloseEntry();
    }

    return ResultSuccess();
}

template <typename Destination, typename Source, typename Intermediate>
Result dump(Destination &dst, Source &src) {
    /* Try to open Source. */
    R_TRY(src.Open());
    ScopeGuard src_close([&] { src.Close(); });

    /* Allocate temporary buffer and delete on return. */
    u8 *buffer = new u8[DUMP_BUFFER_SIZE];
    ScopeGuard buffer_guard([&] { delete[] buffer; });

    /* Receive entries. */
    std::vector<FileEntry> entries;
    R_TRY(src.GetEntries(&entries));

    /* Iterate over found entries. */
    for (const FileEntry &entry : entries) {
        /* Open file for read. */
        R_TRY(src.OpenEntry(entry));

        /* Allocate new entry for write. */
        std::unique_ptr<Intermediate> fileDst;
        R_TRY(dst.Allocate(&fileDst, entry));

        /* Read and write entry. */
        u64 readSize;
        for (u64 offset = 0; offset < entry.size; offset += readSize) {
            R_TRY(src.Read(&readSize, buffer, offset, DUMP_BUFFER_SIZE));
            R_TRY(fileDst->Write(buffer, offset, readSize));
        }
        src.CloseEntry();
    }
    return ResultSuccess();
}
