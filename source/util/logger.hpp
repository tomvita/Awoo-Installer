#pragma once
#include "../hos/fs.hpp"

Result InitializeLog(std::shared_ptr<IFileSystem> &fs);
void ExitLog();

/* Not using variadic templates here because attribute format don't work for it (yet). */
Result Log(const char *path, int line, const char *function, const char *format, ...) __attribute__((format(printf, 4, 5)));

#ifdef DEBUG
#define LOG(format, ...) Log(__FILE__, __LINE__, __PRETTY_FUNCTION__, format, ##__VA_ARGS__);
#define DEBUG_RUN(snippet) snippet
#else
#define LOG(format, ...)
#define DEBUG_RUN(snippet)
#endif

/// Evaluates an expression that returns a result, and returns and logs the result if it would fail.
#define R_TRY_LOG(res_expr)                                                                \
    ({                                                                                     \
        const auto rc = (res_expr);                                                        \
        if (R_FAILED(rc)) {                                                                \
            LOG("failed with rc: 0x%x / 2%03u-%04u", rc, R_MODULE(rc), R_DESCRIPTION(rc)); \
            return rc;                                                                     \
        }                                                                                  \
    })

/// Evaluates an expression that returns a result, and logs the result if it would fail.
#define R_LOG(res_expr)                                                                    \
    ({                                                                                     \
        const auto rc = (res_expr);                                                        \
        if (R_FAILED(rc)) {                                                                \
            LOG("failed with rc: 0x%x / 2%03u-%04u", rc, R_MODULE(rc), R_DESCRIPTION(rc)); \
        }                                                                                  \
    })
