#include "directory.hpp"

#include "../util/awoo_result.hpp"

Result Directory::Open() {
    return m_fs->OpenDirectoryFormat(&m_dir, FsDirOpenMode_ReadFiles, m_path.c_str());
}

void Directory::Close() {
    /* Filesystem is externally provided. */
    this->m_dir->ForceClose();
    this->m_file->ForceClose();
}

Result Directory::GetEntries(std::vector<FileEntry> *entries) {
    /* Clear entries. */
    entries->clear();

    /* Iterate the directory. */
    for (auto &entry : DirectoryIterator(this->m_dir.get())) {
        /* Push back entries. */
        entries->push_back({
            .name = entry.name,
            .offset = 0,
            .size = static_cast<u64>(entry.file_size),
        });
    }

    return ResultSuccess();
}

Result Directory::OpenEntry(const FileEntry &entry) {
    /* Only file. */
    return m_fs->OpenFileFormat(&m_file, FsOpenMode_Read, "%s/%s", m_path.c_str(), entry.name.c_str());
}

Result Directory::Read(u64 *bytes_read, u8 *buffer, s64 offset, u64 read_size) {
    /* Read directly from file. */
    return m_file->Read(bytes_read, buffer, offset, read_size);
}

void Directory::CloseEntry() {
    /* Close file. */
    m_file->ForceClose();
}
