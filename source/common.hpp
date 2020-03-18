#pragma once

#include <string>
#include <switch.h>

struct FileEntry {
    std::string name;
    u64 offset;
    u64 size;
};