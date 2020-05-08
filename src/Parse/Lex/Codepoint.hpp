#pragma once

namespace scar {

    struct Codepoint {
        uint32_t Value;

        Codepoint(unsigned int x = 0) : Value(x) {}
        Codepoint(int x)              : Value(x) {}

        bool IsWhitespace() const;
        bool IsChar() const;
        bool IsBin() const;
        bool IsOct() const;
        bool IsDec() const;
        bool IsHex() const;
        bool IsEOF() const;

        bool operator==(Codepoint x) const { return Value == x.Value; }
        bool operator!=(Codepoint x) const { return Value != x.Value; }

        bool operator<(Codepoint x) const  { return Value < x.Value; }
        bool operator>(Codepoint x) const  { return Value > x.Value; }
        bool operator<=(Codepoint x) const { return Value <= x.Value; }
        bool operator>=(Codepoint x) const { return Value >= x.Value; }

        uint32_t operator+(Codepoint x) const { return Value + x.Value; }
        uint32_t operator-(Codepoint x) const { return Value - x.Value; }
    };

    std::string AsString(const Codepoint& cp);

    static std::ostream& operator<<(std::ostream& os, Codepoint cp) {
        return os << AsString(cp);
    }
    static std::string& operator+=(std::string& s, Codepoint cp) {
        return s += AsString(cp);
    }


    namespace range {
        // Check if the Codepoint is a C-style char
        static bool IsChar(Codepoint cp) { return cp <= 0xffff; }

        // Check if the Codepoint is within a certain range
        static bool InRange(Codepoint cp, Codepoint lo, Codepoint hi) { return (lo <= cp) && (cp <= hi); }

        static bool IsWhitespace(Codepoint cp) {
            return
                cp == ' ' || cp == '\t' || cp == '\r' || cp == '\n' ||
                cp == 0xC ||     // ^L
                cp == 0x85 ||
                cp == 0x200E ||  // LTR
                cp == 0x200F ||  // RTL
                cp == 0x2028 ||  // Line Separator
                cp == 0x2029;    // Paragraph Separator
        }
        static bool IsBin(Codepoint cp) { return cp == '0' || cp == '1'; }
        static bool IsOct(Codepoint cp) { return InRange(cp, '0', '7'); }
        static bool IsDec(Codepoint cp) { return InRange(cp, '0', '9'); }
        static bool IsHex(Codepoint cp) { return InRange(cp, '0', '9') || InRange(cp, 'a', 'f') || InRange(cp, 'A', 'F'); }
        static bool IsAlpha(Codepoint cp) { return InRange(cp, 'a', 'z') || InRange(cp, 'A', 'Z'); }
        static bool IsAlnum(Codepoint cp) { return IsAlpha(cp) || IsDec(cp); }
        static bool IsEOF(Codepoint cp) { return cp.Value == '\0'; }

        // Check if the Codepoint can start an identifier
        static bool IsIdentStart(Codepoint cp) { return IsAlpha(cp) || cp == '_'; }
        // Check if the Codepoint can continue an identifier
        static bool IsIdentBody(Codepoint cp) { return IsAlnum(cp) || cp == '_'; }

        // Get a number from a Codepoint
        // Returns (-1) if it's not a valid number
        static int GetNum(Codepoint cp, unsigned int base) {
            unsigned int val = 0;

            if (range::InRange(cp, '0', '9'))      { val = cp - '0'; }
            else if (range::InRange(cp, 'a', 'f')) { val = cp - 'a' + 10; }
            else if (range::InRange(cp, 'A', 'F')) { val = cp - 'A' + 10; }
            else { return -1; }

            return (val < base) ? val : -1;
        }
    }


    inline bool Codepoint::IsWhitespace() const { return range::IsWhitespace(Value); }
    inline bool Codepoint::IsChar() const { return range::IsChar(Value); }
    inline bool Codepoint::IsBin() const { return range::IsBin(Value); }
    inline bool Codepoint::IsOct() const { return range::IsOct(Value); }
    inline bool Codepoint::IsDec() const { return range::IsDec(Value); }
    inline bool Codepoint::IsHex() const { return range::IsHex(Value); }
    inline bool Codepoint::IsEOF() const { return range::IsEOF(Value); }

}