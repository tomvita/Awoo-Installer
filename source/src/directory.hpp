#pragma once
#include "../common.hpp"
#include "../hos/hos.hpp"

#include <memory>
#include <vector>

class Directory {
  private:
    std::shared_ptr<IFileSystem> m_fs;
    std::unique_ptr<IDirectory> m_dir;
    std::unique_ptr<IFile> m_file;
    std::string m_path;

  public:
    template <typename Path>
    Directory(std::shared_ptr<IFileSystem> &fs, Path &path)
        : m_fs(fs), m_path(std::string(path)) {}

    Result Open();
    void Close();
    Result GetEntries(std::vector<FileEntry> *entries);

    /* Open an entry for read. */
    Result OpenEntry(const FileEntry &entry);
    Result Read(u64 *bytes_read, u8 *buffer, s64 offset, u64 read_size);
    void CloseEntry();
};
