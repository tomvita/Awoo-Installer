#include "translation.hpp"

#include "languages/chinese.hpp"
#include "languages/dutch.hpp"
#include "languages/english.hpp"
#include "languages/french.hpp"
#include "languages/german.hpp"
#include "languages/italian.hpp"
#include "languages/japanese.hpp"
#include "languages/korean.hpp"
#include "languages/polish.hpp"
#include "languages/russian.hpp"
#include "languages/spanish.hpp"
#include "languages/vong.hpp"

namespace Translation {

    constexpr static const char *const *translations[Total_Languages] = {
        [Japanese] = japanese,
        [English_US] = english,
        [French] = french,
        [German] = german,
        [Italian] = italian,
        [Spanish] = spanish,
        [Chinese_CN] = chinese,
        [Korean] = korean,
        [Dutch] = dutch,
        [Polish] = polish,
        [Russian] = russian,
        [Chinese_TW] = chinese,
        [English_GB] = english,
        [French_CA] = french,
        [Spanish_Latin] = spanish,
        [Chinese_HANS] = chinese,
        [Chinese_HANT] = chinese,
        [Vong] = vong,
    };

    static const char *const *current = translations[English_US];

    const char *Translate(String key) {
        /* Return a snowflake if the key is out of range. */
        if (key >= String::Total_Strings)
            return "\u2744";

        /* Return a snowman if the string isn't defined. */
        const char *temp = current[key];
        if (!temp)
            return "\u2603";

        /* Return translated string. */
        return temp;
    }

    void SetLanguage(Language lang) {
        /* Skip invalid language. */
        if (lang >= Language::Total_Languages)
            return;

        /* Overwrite current translation array. */
        current = translations[lang];
    }
}