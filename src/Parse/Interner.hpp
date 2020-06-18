#pragma once

namespace scar {

    class Interner {
    public:
        using StringID = size_t;

        static StringID Intern(std::string_view str);
        static std::string_view GetString(StringID stringID);

    private:
        static std::vector<std::string> m_Strings;
    };

    static std::ostream& operator<<(std::ostream& os, Interner::StringID id) {
        return os << printf("%llu", (size_t)id);
    }

}
