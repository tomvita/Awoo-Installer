#pragma once
#include "../hos/hos.hpp"
#include "../util/awoo_result.hpp"
#include "../common.hpp"

#include <utility>
#include <vector>

template <typename Protocol>
class NSP {
  private:
    Protocol m_protocol;
    FileEntry m_open;

  public:
    template <typename... Args>
    NSP(Args &&... args)
        : m_protocol(std::forward<Args>(args)...) {}
    ~NSP() {}

    Result Open() {
        /* File was opened prior to initialization. */
        return ResultSuccess();
    }

    void Close() {
        /* File is provided extern. */
    }

    Result GetEntries(std::vector<FileEntry> *entries) {
        /* Clear entries. */
        entries->clear();

        /* Retrieve the base header. */
        u64 read;
        std::vector<u8> headerBytes(sizeof(PFS0BaseHeader));
        R_TRY(m_protocol.Read(&read, headerBytes.data(), 0, sizeof(PFS0BaseHeader)));
        R_UNLESS(read == sizeof(PFS0BaseHeader), ResultEndOfFile());

        /* Verify header magic. */
        R_UNLESS(VerifyMagic(headerBytes.data(), PFS0Magic), ResultInvalidPFS0Magic());

        /* Extract header. */
        auto *header = reinterpret_cast<PFS0BaseHeader *>(headerBytes.data());

        /* Extend and reload if needed. */
        u64 size = header->GetHeaderSize();
        headerBytes.resize(size, 0);

        R_TRY(m_protocol.Read(&read, headerBytes.data(), 0, size));
        R_UNLESS(read == size, ResultEndOfFile());

        header = reinterpret_cast<PFS0BaseHeader *>(headerBytes.data());

        /* Extract entries. */
        auto *pfs0_entries = header->GetEntries(headerBytes.data());

        /* Extract string table. */
        StringTable stringTable = header->GetStringTable(headerBytes.data());

        /* Store entries. */
        for (u32 i = 0; i < header->numFiles; i++) {
            auto &entry = pfs0_entries[i];
            entries->push_back({
                .name = stringTable.GetName(entry),
                .offset = entry.dataOffset + size,
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
