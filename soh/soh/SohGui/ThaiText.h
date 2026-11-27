#pragma once

#include <string>
#include <unordered_map>

// Thai UI translation for the Ship of Harkinian menu.
//
//   T("Autosave")  ->  "เซฟอัตโนมัติ"   (the English text is returned unchanged if it is not in the table)
//
// The table lives in ThaiTable.inc as { "English", "ภาษาไทย" } pairs. The English string is only used as the
// lookup key, so it must match the string in the code exactly. Define SOH_UI_ENGLISH to turn translation off.
namespace SohGui {

inline const std::unordered_map<std::string, const char*>& ThaiTable() {
    static const std::unordered_map<std::string, const char*> table = {
#include "ThaiTable.inc"
    };
    return table;
}

inline const char* T(const char* english) {
#ifndef SOH_UI_ENGLISH
    const auto& table = ThaiTable();
    auto it = table.find(english);
    if (it != table.end()) {
        return it->second;
    }
#endif
    return english;
}

} // namespace SohGui
