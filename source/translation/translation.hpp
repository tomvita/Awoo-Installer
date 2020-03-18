#pragma once

#include <cstdio>
#include <switch.h>

enum String : u32 {
    /* Misc */
    OK,
    BACK,
    FINISHED,
    EXIT,
    /* Results */
    Invalid_PFS0,
    /* String count. */
    Total_Strings,
};

namespace Translation {

    enum Language : u32 {
        /* Languages offically supported by N. */
        Japanese,
        English_US,
        French,
        German,
        Italian,
        Spanish,
        Chinese_CN,
        Korean,
        Dutch,
        Polish,
        Russian,
        Chinese_TW,
        English_GB,
        French_CA,
        Spanish_Latin,
        Chinese_HANS,
        Chinese_HANT,
        /* Additional languages not officially supported by N. */
        Vong,
        /* Total languages. */
        Total_Languages,
    };

    const char *Translate(String key);
    void SetLanguage(Language lang);

}

inline const char *operator&(String key) {
    return Translation::Translate(key);
}
