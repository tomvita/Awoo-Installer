#pragma once
#include "../hos/hos.hpp"
#include "../util/awoo_result.hpp"

#include <cstring>
#include <utility>
#include <vector>

#define ROOT_HFS0_OFFSET 0xF000

template <typename Protocol>
class XCI {
  private:
    Protocol m_protocol;
    FileEntry m_open;

  public:
    template <typename... Args>
    XCI(Args &&... args)
        : m_protocol(std::forward<Args>(args)...) {}
    ~XCI() {}

    Result Open() {
        /* File was opened prior to initialization. */
        return ResultSuccess();
    }

    Result GetEntries(std::vector<FileEntry> *entries) {
        /* Clear entries. */
        entries->clear();

        /* Retrieve the base header. */
        u64 partitionOffset = ROOT_HFS0_OFFSET;
        u64 read;
        std::vector<u8> headerBytes(sizeof(HFS0BaseHeader));
        R_TRY(m_protocol.Read(&read, headerBytes.data(), partitionOffset, sizeof(HFS0BaseHeader)));
        R_UNLESS(read == sizeof(HFS0BaseHeader), ResultEndOfFile());

        /* Verify header magic. */
        R_UNLESS(VerifyMagic(headerBytes.data(), HFS0Magic), ResultInvalidHFS0Magic());

        /* Extract header. */
        auto *header = reinterpret_cast<HFS0BaseHeader *>(headerBytes.data());

        /* Extend and reload. */
        u64 rootHeaderSize = header->GetHeaderSize();
        headerBytes.resize(rootHeaderSize, 0);

        R_TRY(m_protocol.Read(&read, headerBytes.data(), partitionOffset, rootHeaderSize));
        R_UNLESS(read == rootHeaderSize, ResultEndOfFile());

        header = reinterpret_cast<HFS0BaseHeader *>(headerBytes.data());

        /* Extract entries. */
        auto *hfs0_entries = header->GetEntries(headerBytes.data());

        /* Extract string table. */
        StringTable stringTable = header->GetStringTable(headerBytes.data());

        u64 secureOffset = 0;
        /* Find secure header. */
        for (u32 i = 0; i < header->numFiles; i++) {
            auto &entry = hfs0_entries[i];
            if (std::strcmp(stringTable.GetName(entry), "secure") == 0) {
                secureOffset = entry.dataOffset;
            }
        }
        R_UNLESS(secureOffset != 0, ResultPartitionNotFound());
        partitionOffset = ROOT_HFS0_OFFSET + rootHeaderSize + secureOffset;

        headerBytes = std::vector<u8>(sizeof(HFS0BaseHeader));
        R_TRY(m_protocol.Read(&read, headerBytes.data(), partitionOffset, sizeof(HFS0BaseHeader)));
        R_UNLESS(read == sizeof(HFS0BaseHeader), ResultEndOfFile());

        /* Verify header magic. */
        R_UNLESS(VerifyMagic(headerBytes.data(), HFS0Magic), ResultInvalidHFS0Magic());

        /* Extract header. */
        header = reinterpret_cast<HFS0BaseHeader *>(headerBytes.data());

        /* Extend and reload. */
        u64 secureHeaderSize = header->GetHeaderSize();
        headerBytes.resize(secureHeaderSize, 0);

        R_TRY(m_protocol.Read(&read, headerBytes.data(), partitionOffset, secureHeaderSize));
        R_UNLESS(read == secureHeaderSize, ResultEndOfFile());

        header = reinterpret_cast<HFS0BaseHeader *>(headerBytes.data());

        /* Extract entries. */
        hfs0_entries = header->GetEntries(headerBytes.data());

        /* Extract string table. */
        stringTable = header->GetStringTable(headerBytes.data());

        /* Store entries. */
        for (u32 i = 0; i < header->numFiles; i++) {
            auto &entry = hfs0_entries[i];
            entries->push_back({
                .name = stringTable.GetName(entry),
                .offset = entry.dataOffset + partitionOffset,
                .size = entry.fileSize,
            });
        }
        return ResultSuccess();
    }

    Result OpenEntry(const FileEntry &entry) {
        /* Only store entry. */
        m_open = entry;
        return ResultSuccess();
    }

    void CloseEntry() {
        /* We don't open any other file. */
    }

    Result Read(u64 *bytes_read, u8 *buffer, s64 offset, u64 read_size) {
        /* limit read to expected filesize. */
        if (read_size + offset > m_open.size) {
            read_size = m_open.size - offset;
        }
        u64 read;
        R_TRY(m_protocol.Read(&read, buffer, offset + m_open.offset, read_size));
        R_UNLESS(read == read_size, ResultEndOfFile());

        *bytes_read = read;
        return ResultSuccess();
    }
};
