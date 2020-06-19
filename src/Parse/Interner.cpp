#include "scarpch.hpp"
#include "Parse/Interner.hpp"

namespace scar {

    std::vector<std::string> Interner::m_Strings;

    Interner::StringID Interner::Intern(std::string_view str) {
        for (uint32_t i = 0; i < m_Strings.size(); i++) {
            if (m_Strings[i] == str) {
                return i;
            }
        }
        m_Strings.push_back((std::string)str);
        return (uint32_t)m_Strings.size() - 1;
    }

    const std::string& Interner::GetString(StringID stringID) {
        return m_Strings[stringID];
    }

}