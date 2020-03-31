#include "dst/sd_dump.hpp"
#include "dump.hpp"
#include "hos/hos.hpp"
#include "install.hpp"
#include "protocol/gdrive.hpp"
#include "src/directory.hpp"
#include "src/gamecard.hpp"
#include "src/nsp.hpp"
#include "src/xci.hpp"
#include "translation/translation.hpp"
#include "util/logger.hpp"

#include <unistd.h>

extern "C" {
void __appInit();
void __appExit();
}

void __appInit() {
    R_ASSERT(smInitialize());

    /* Setup HOS version. */
    if (hosversionGet() == 0) {
        R_ASSERT(setsysInitialize());
        SetSysFirmwareVersion fw;
        R_ASSERT(setsysGetFirmwareVersion(&fw));
        hosversionSet(MAKEHOSVERSION(fw.major, fw.minor, fw.micro));
        setsysExit();
    }
    /* Set language. */
    {
        SetLanguage ourLang;
        u64 lcode = 0;
        R_ASSERT(setInitialize());
        R_ASSERT(setGetSystemLanguage(&lcode));
        R_ASSERT(setMakeLanguage(lcode, &ourLang));
        Translation::SetLanguage(Translation::Language(ourLang));
        setExit();
    }

    R_ASSERT(appletInitialize());
    R_ASSERT(timeInitialize());
    R_ASSERT(fsInitialize());
    R_ASSERT(splCryptoInitialize());

    R_ASSERT(socketInitializeDefault());
}

void __appExit() {
    socketExit();

    splCryptoExit();
    fsExit();
    timeExit();
    appletExit();
    smExit();
}

Result Test(std::shared_ptr<IFileSystem> &fs) {
    {
        IFile file;
        R_TRY(fs->OpenFileFormat(&file, FsOpenMode_Read, "/atmosphere/hbl.nsp"));
        Install<NSP<IFile>> install(NcmStorageId_SdCard, std::move(file));
        R_TRY(install.Verify());
        R_TRY(install.Start());
    }
    {
        Install<Directory> dir_install(NcmStorageId_SdCard, fs, "/");
        R_TRY(dir_install.Verify());
        R_TRY(dir_install.Start());
    }
    {
        IFile file;
        R_TRY(fs->OpenFileFormat(&file, FsOpenMode_Read, "/atmosphere/hbl.nsp"));
        NSP<IFile> nsp(std::move(file));
        SdDump sd_dump(fs, "/tti-dump");
        R_TRY((doubleDump<SdDump, NSP<IFile>, IFile>(sd_dump, nsp)));
    }
    {
        Install<Gamecard> install(NcmStorageId_SdCard);
        R_TRY(install.Verify());
        R_TRY(install.Start());
    }
    {
        IFile file;
        R_TRY(fs->OpenFileFormat(&file, FsOpenMode_Read, "/Labo.xci"));
        Install<XCI<IFile>> install(NcmStorageId_SdCard, std::move(file));
        R_TRY(install.Verify());
        R_TRY(install.Start());
    }

    return ResultSuccess();
}

int main(int argc, char *argv[]) {
    auto fs = std::make_shared<IFileSystem>();
    fs->OpenSdCardFileSystem();

    InitializeLog(fs);

    Crypto::Initialize();

    LOG(&OK);

    R_LOG(Test(fs));

    LOG(&EXIT);

    ExitLog();
    return 0;
}
