#pragma once
#include "../common.hpp"
#include "../hos/hos.hpp"

#include <memory>
#include <string>

class SdDump {
  private:
    std::shared_ptr<IFileSystem> m_fs;
    std::string m_path;

  public:
    template <typename Path>
    SdDump(std::shared_ptr<IFileSystem> &fs, Path &&path)
        : m_fs(fs), m_path(std::string(path)) {
        m_fs->CreateDirectoryFormat(m_path.c_str());
    }

    Result Allocate(IFile *out, const FileEntry &entry);
};
