#pragma once
#include "../common.hpp"
#include "../hos/hos.hpp"

#include <memory>
#include <switch.h>
#include <vector>

class Gamecard {
  private:
    std::unique_ptr<IFileSystem> m_fs;
    std::unique_ptr<IFile> m_file;

  public:
    Gamecard();

    Result Open();
    Result GetEntries(std::vector<FileEntry> *entries);

    Result OpenEntry(const FileEntry &entry);
    Result Read(u64 *bytes_read, u8 *buffer, s64 offset, u64 read_size);
    void CloseEntry();
};
