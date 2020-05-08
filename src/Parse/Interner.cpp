#include "scarpch.hpp"
#include "Parse/Interner.hpp"

namespace scar {

    std::vector<std::string> Interner::m_Strings;

    Interner::StringID Interner::Intern(std::string_view str) {
        for (size_t i = 0; i < m_Strings.size(); i++) {
            if (m_Strings[i] == str) {
                return i;
            }
        }
        m_Strings.push_back((std::string)str);
        return m_Strings.size() - 1;
    }

    std::string_view Interner::GetString(StringID stringID) {
        return m_Strings[stringID];
    }

}