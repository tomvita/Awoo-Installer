#pragma once

#include <atomic>
#include <mutex>

#define DUMP_BUFFER_SIZE 8_MB

typedef Result (*ReadCallback)(void *user, u64 *read, u8 *buffer, u64 offset, u64 size);
typedef Result (*WriteCallback)(void *user, const u8 *buffer, u64 offset, u64 size);

struct DoubleBuffer {
    /* Shared misc. */
    std::atomic_bool running;
    Result lastResult;
    u64 end;
    /* Read stuff. */
    ReadCallback Read;
    void *readArgs;
    u64 readOffset;

    /* Write stuff. */
    WriteCallback Write;
    void *writeArgs;
    u64 progress;

    /* Shared buffer. */
    struct Buffer {
        std::atomic_bool used;
        u8 *ptr;
        u64 offset, size;
    } buffer[2];

    DoubleBuffer(u64 fileSize, ReadCallback readCB, void *readArgs, WriteCallback writeCB, void *writeArgs)
        : running(true), lastResult(ResultSuccess()), end(fileSize), Read(readCB), readArgs(readArgs), readOffset(0), Write(writeCB), writeArgs(writeArgs), progress(0), buffer{{}, {}} {
        buffer[0].ptr = new u8[DUMP_BUFFER_SIZE];
        buffer[1].ptr = new u8[DUMP_BUFFER_SIZE];
    }
    ~DoubleBuffer() {
        delete[] buffer[0].ptr;
        delete[] buffer[1].ptr;
    }
};

/* Read from file to buffer. */
Result ReadResultFunction(DoubleBuffer *dbl) {
    for (auto &buffer : dbl->buffer) {
        /* Skip buffer if already used. */
        if (buffer.used)
            continue;

        /* Write to buffer. */
        R_TRY(dbl->Read(dbl->readArgs, &buffer.size, buffer.ptr, dbl->readOffset, DUMP_BUFFER_SIZE));

        /* Update offset. */
        buffer.offset = dbl->readOffset;
        dbl->readOffset += buffer.size;

        /* Mark as used. */
        buffer.used = true;

        /* Return if finished. */
        if (dbl->readOffset >= dbl->end) {
            return ResultSuccess();
        }
    }
    return ResultSuccess();
}

void ReadFunction(void *args) {
    /* Cast thread arguments. */
    DoubleBuffer *dbl = reinterpret_cast<DoubleBuffer *>(args);

    /* Run for duration. */
    while (dbl->running) {
        Result rc = ReadResultFunction(dbl);

        /* Check result code and break if failed. */
        if (R_FAILED(rc)) {
            dbl->lastResult = rc;
            dbl->running = false;
            break;
        } else if (dbl->readOffset >= dbl->end) {
            dbl->running = false;
            break;
        }
    }
}

/* Write from buffer to destination. */
Result WriteResultFunction(DoubleBuffer *dbl) {
    for (auto &buffer : dbl->buffer) {
        /* Skip unused buffer. */
        if (!buffer.used)
            continue;

        /* Read from buffer. */
        R_TRY(dbl->Write(dbl->writeArgs, buffer.ptr, buffer.offset, buffer.size));
        dbl->progress += buffer.size;

        /* Mark as unused. */
        buffer.used = false;

        /* Return if finished. */
        if (dbl->progress >= dbl->end) {
            return ResultSuccess();
        }
    }
    return ResultSuccess();
}

void WriteFunction(void *args) {
    /* Cast thread arguments. */
    DoubleBuffer *dbl = reinterpret_cast<DoubleBuffer *>(args);

    /* Run for duration. */
    while (dbl->running) {
        Result rc = WriteResultFunction(dbl);

        /* Check result code and break if failed or finished. */
        if (R_FAILED(rc)) {
            dbl->lastResult = rc;
            dbl->running = false;
            break;
        } else if (dbl->progress >= dbl->end) {
            dbl->running = false;
            break;
        }
    }
}
