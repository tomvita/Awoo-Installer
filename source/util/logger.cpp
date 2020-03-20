#include "logger.hpp"

#include "awoo_result.hpp"

#include <cstdarg>

namespace {

#ifdef DEBUG
    bool initialized;

#ifdef DEBUG_NXLINK
    int nxlinkSocket;
#endif /* DEBUG_NXLINK */

#ifdef DEBUG_LOG_TO_FILE
    constexpr static inline const char *const log_path = "/tti.txt";
    std::shared_ptr<IFileSystem> sdmc;
    s64 offset;
    constexpr size_t buf_size = 1024;
    char sprint_buf[buf_size];

    __attribute__((format(printf, 2, 3)))
    Result
    fprintf(IFile *file, const char *fmt, ...) {
        va_list args;

        va_start(args, fmt);
        int n = std::vsnprintf(sprint_buf, buf_size, fmt, args);
        va_end(args);

        R_TRY(file->Write(sprint_buf, offset, n, FsWriteOption_None));

        offset += n;
        return ResultSuccess();
    }
#endif /* DEBUG_LOG_TO_FILE */
#endif /* DEBUG*/

}

Result InitializeLog(std::shared_ptr<IFileSystem> &fs) {
#ifdef DEBUG
#ifdef DEBUG_LOG_TO_FILE
    /* Log to file. */
    sdmc = fs;
    Result rc;
    if (R_FAILED(rc = sdmc->CreateFileFormat(0, 0, log_path)) && rc != 0x402)
        return ResultLogCreateFailed();

    std::unique_ptr<IFile> log_file;
    if (R_FAILED(sdmc->OpenFileFormat(&log_file, FsOpenMode_Read, log_path)))
        return ResultLogOpenFailed();

    if (R_FAILED(log_file->GetSize(&offset)))
        return ResultLogStatFailed();
#endif /* DEBUG_LOG_TO_FILE */

#ifdef DEBUG_NXLINK
    /* nxlink debug. */
    nxlinkSocket = nxlinkStdio();
    if (nxlinkSocket < 0) {
        return ResultNxlinkInitFailed();
    }
#endif /* DEBUG_NXLINK */
    initialized = true;
#endif /* DEBUG*/
    return ResultSuccess();
}

void ExitLog() {
#ifdef DEBUG_NXLINK
    close(nxlinkSocket);
#endif /* DEBUG_NXLINK */
}

Result Log(const char *path, int line, const char *function, const char *format, ...) {
#ifdef DEBUG
    R_UNLESS(initialized, ResultLogNotInitialized());

    va_list args;
#ifdef DEBUG_NXLINK
    std::printf("%s:%d %s: ", path, line, function);
    va_start(args, format);
    std::vprintf(format, args);
    va_end(args);
    std::printf("\n");
#endif /* DEBUG_NXLINK */

#ifdef DEBUG_LOG_TO_FILE
    u64 timestamp;
    R_TRY(timeGetCurrentTime(TimeType_Default, &timestamp));
    TimeCalendarTime caltime;
    R_TRY(timeToCalendarTimeWithMyRule(timestamp, &caltime, nullptr));

    std::unique_ptr<IFile> file;
    R_TRY(sdmc->OpenFileFormat(&file, FsOpenMode_Write | FsOpenMode_Append, log_path));

    R_TRY(fprintf(file.get(), "[%02d:%02d:%02d] %s:%d %s: ", caltime.hour, caltime.minute, caltime.second, path, line, function));

    va_start(args, format);
    int n = std::vsnprintf(sprint_buf, buf_size, format, args);
    va_end(args);

    R_TRY(file->Write(sprint_buf, offset, n, FsWriteOption_None));
    offset += n;

    R_TRY(fprintf(file.get(), "\n"));

    file->Flush();
#endif /* DEBUG_LOG_TO_FILE */
#endif /* DEBUG*/

    return ResultSuccess();
}

void PrintBytes(const u8 *bytes, size_t size) {
#ifdef DEBUG
    if (!initialized)
        return;
    if ((size % 0x10)) {
        LOG("INVALID PRINT BYTE SIZE");
        return;
    }

    constexpr const char *header = "__________________________________________________________\n"
                                   "|        |00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F|\n"
                                   "|        |-----------------------------------------------|\n";

    constexpr const char *format = "|%08lX|%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X|\n";

#ifdef DEBUG_NXLINK
    std::printf(header);
#endif /* DEBUG_NXLINK */

#ifdef DEBUG_LOG_TO_FILE
    std::unique_ptr<IFile> file;
    R_RETURN(sdmc->OpenFileFormat(&file, FsOpenMode_Write | FsOpenMode_Append, log_path));

    fprintf(file.get(), header);
#endif /* DEBUG_LOG_TO_FILE */

    char line[60];
    for (size_t i = 0; i < size; i += 0x10) {
        std::snprintf(line, 60, format, i,
                      bytes[0x0], bytes[0x1], bytes[0x2], bytes[0x3], bytes[0x4], bytes[0x5], bytes[0x6], bytes[0x7],
                      bytes[0x8], bytes[0x9], bytes[0xA], bytes[0xB], bytes[0xC], bytes[0xD], bytes[0xE], bytes[0xF]);
        bytes += 0x10;

#ifdef DEBUG_NXLINK
        std::printf(line);
#endif /* DEBUG_NXLINK */

#ifdef DEBUG_LOG_TO_FILE
        fprintf(file.get(), line);
#endif /* DEBUG_LOG_TO_FILE */
    }

#ifdef DEBUG_LOG_TO_FILE
    file->Flush();
#endif /* DEBUG_LOG_TO_FILE */
#endif /* DEBUG*/
}
