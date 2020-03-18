#include "sd_dump.hpp"

#include "../util/awoo_result.hpp"

Result SdDump::Allocate(IFile *out, const FileEntry &entry) {
    /* Split and set archive bit if file is over 2 gigabtes. */
    u32 option = entry.size > 2_GB ? FsCreateOption_BigFile : 0;
    Result rc = m_fs->CreateFileFormat(entry.size, option, "%s/%s", m_path.c_str(), entry.name.c_str());

    IFile file;
    if (R_FAILED(rc)) {
        if (rc != 0x402) {
            return rc;
        }

        R_TRY(m_fs->OpenFileFormat(&file, FsOpenMode_Write, "%s/%s", m_path.c_str(), entry.name.c_str()));
        R_TRY(file.SetSize(entry.size));
    } else {
        R_TRY(m_fs->OpenFileFormat(&file, FsOpenMode_Write, "%s/%s", m_path.c_str(), entry.name.c_str()));
    }
    *out = std::move(file);
    return ResultSuccess();
}
