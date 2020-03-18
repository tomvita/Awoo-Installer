#include "gamecard.hpp"

#include "../util/awoo_result.hpp"

Gamecard::Gamecard() {}

Result Gamecard::Open() {
    /* Open device operator. */
    FsDeviceOperator op;
    R_TRY(fsOpenDeviceOperator(&op));
    ScopeGuard op_guard([&] { fsDeviceOperatorClose(&op); });

    /* Check if gamecard is inserted. */
    bool is_inserted;
    R_TRY(fsDeviceOperatorIsGameCardInserted(&op, &is_inserted));
    if (!is_inserted)
        return ResultGamecardNotInserted();

    /* Receive gamecard handle */
    FsGameCardHandle gc_handle;
    R_TRY(fsDeviceOperatorGetGameCardHandle(&op, &gc_handle));
    ScopeGuard gc_handle_guard([&] { svcCloseHandle(gc_handle.value); });

    /* Open filesystem. */
    FsFileSystem gc;
    R_TRY(fsOpenGameCardFileSystem(&gc, &gc_handle, FsGameCardPartition_Secure));
    m_fs = std::make_unique<IFileSystem>(std::move(gc));

    return ResultSuccess();
}

Result Gamecard::GetEntries(std::vector<FileEntry> *entries) {
    /* Clear entries. */
    entries->clear();

    /* Open root directory. Only read files. Gamecards don't have folders. */
    std::unique_ptr<IDirectory> gc_dir;
    m_fs->OpenDirectoryFormat(&gc_dir, FsDirOpenMode_ReadFiles, "/");

    /* Iterate over the root directory. */
    while (true) {
        /* Read next entry. */
        s64 count;
        FsDirectoryEntry entry;
        R_TRY(gc_dir->Read(&count, &entry, 1));

        /* Finish if nothing was read. */
        if (count == 0) {
            break;
        }

        /* Push back entries. */
        entries->push_back({
            .name = entry.name,
            .offset = 0,
            .size = static_cast<u64>(entry.file_size),
        });
    }
    return ResultSuccess();
}

Result Gamecard::OpenEntry(const FileEntry &entry) {
    /* Only file. */
    return m_fs->OpenFileFormat(&m_file, FsOpenMode_Read, "/%s", entry.name.c_str());
}

Result Gamecard::Read(u64 *bytes_read, u8 *buffer, s64 offset, u64 read_size) {
    /* Read directly from file. */
    return m_file->Read(bytes_read, buffer, offset, read_size);
}

void Gamecard::CloseEntry() {
    /* Close file. */
    m_file->ForceClose();
}
