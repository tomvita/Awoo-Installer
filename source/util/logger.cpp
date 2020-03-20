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
    constexpr static inline const char *const log_path = "/log.txt";
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
