#pragma once

namespace scar {

    class Interner {
    public:
        using StringID = uint32_t;

        static StringID Intern(std::string_view str);
        static const std::string& GetString(StringID stringID);

    private:
        static std::vector<std::string> m_Strings;
    };

    static std::ostream& operator<<(std::ostream& os, Interner::StringID id) {
        return os << printf("%u", (uint32_t)id);
    }

}
