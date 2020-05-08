#include "scarpch.hpp"
#include "Parse/Lex/Lexer.hpp"

namespace scar {

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    // MISCELANEOUS
    
    static std::unordered_map<std::string_view, Token::Type> KeywordMap = {
        { "func",     Token::Func },
        { "if",       Token::If },
        { "else",     Token::Else },
        { "for",      Token::For },
        { "while",    Token::While },
        { "loop",     Token::Loop },
        { "var",      Token::Var },
        { "continue", Token::Continue },
        { "break",    Token::Break },
        { "return",   Token::Return },

        { "true",  Token::True },
        { "false", Token::False },

        { "bool", Token::Bool },

        { "i8",   Token::I8 },
        { "i16",  Token::I16 },
        { "i32",  Token::I32 },
        { "i64",  Token::I64 },

        { "u8",   Token::U8 },
        { "u16",  Token::U16 },
        { "u32",  Token::U32 },
        { "u64",  Token::U64 },

        { "f32",  Token::F32 },
        { "f64",  Token::F64 },
    };

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    // LEXER

    Lexer::Lexer(const std::string& path) :
        m_Reader(path)
    {}

    void Lexer::Bump(unsigned int n) {
        m_Reader.Bump(n);
    }

    void Lexer::Error(std::string_view msg) {
        SCAR_ERROR("{}: {}", GetSpan(), msg);
        m_ErrorCount++;
    }

    void Lexer::ThrowError(std::string_view msg) {
        throw ParseError(ErrorCode::Unknown, [&]() { Error(msg); });
    }

    TokenStream Lexer::Lex() {
        TokenStream tokenStream;
        do {
            tokenStream.push_back(GetNextToken());
        } while (!tokenStream.back().IsEOF()); // Check if last token was EOF
        return tokenStream;
    }

    Token Lexer::GetNextToken() {
        SkipWhitespaceAndComments();
        m_TokenStartPosition = GetPosition();

        // EOF
        if (GetCurr().IsEOF()) {
            return Token(Token::EndOfFile, GetSpan());
        }

        // Numbers
        if (GetCurr().IsDec()) {
            return LexNumber();
        }

        if (range::IsIdentStart(GetCurr())) {
            do {
                Bump();
            } while (range::IsIdentBody(GetCurr()));

            std::string_view ident = m_Reader.GetSourceFile()->GetString(m_TokenStartPosition.Index, GetPosition().Index - m_TokenStartPosition.Index);
            auto& iter = KeywordMap.find(ident);
            if (iter != KeywordMap.end()) {
                return Token(iter->second, GetSpan());
            }

            return Token(Token::Ident, GetString(), GetSpan());
        }

        // Symbols
        switch (GetCurr().Value)
        {
        case '(':
            Bump();
            return Token(Token::LParen, GetSpan());
        case ')':
            Bump();
            return Token(Token::RParen, GetSpan());
        case '{':
            Bump();
            return Token(Token::LBrace, GetSpan());
        case '}':
            Bump();
            return Token(Token::RBrace, GetSpan());
        case '[':
            Bump();
            return Token(Token::LBracket, GetSpan());
        case ']':
            Bump();
            return Token(Token::RBracket, GetSpan());

        case ';':
            Bump();
            return Token(Token::Semi, GetSpan());
        case ':':
            Bump();
            if (GetCurr() == ':') { Bump(); return Token(Token::Scope, GetSpan()); }
            return Token(Token::Colon, GetSpan());
        case ',':
            Bump();
            return Token(Token::Comma, GetSpan());
        case '.':
            Bump();
            return Token(Token::Dot, GetSpan());

        case '!':
            Bump();
            if (GetCurr() == '=') { Bump(); return Token(Token::NotEq, GetSpan()); }
            return Token(Token::Not, GetSpan());
        case '~':
            Bump();
            return Token(Token::BitNot, GetSpan());

        case '>':
            Bump();
            if (GetCurr() == '=') { Bump(); return Token(Token::GreaterEq, GetSpan()); }
            return Token(Token::Greater, GetSpan());
        case '<':
            Bump();
            if (GetCurr() == '=') { Bump(); return Token(Token::LesserEq, GetSpan()); }
            return Token(Token::Lesser, GetSpan());

        case '+':
            Bump();
            if (GetCurr() == '+') { Bump(); return Token(Token::PlusPlus, GetSpan()); }
            return Token(Token::Plus, GetSpan());
        case '-':
            Bump();
            if (GetCurr() == '-') { Bump(); return Token(Token::MinusMinus, GetSpan()); }
            if (GetCurr() == '>') { Bump(); return Token(Token::RArrow, GetSpan()); }
            return Token(Token::Minus, GetSpan());
        case '*':
            Bump();
            return Token(Token::Star, GetSpan());
        case '/':
            Bump();
            return Token(Token::Slash, GetSpan());
        case '%':
            Bump();
            return Token(Token::Percent, GetSpan());
        case '&':
            Bump();
            if (GetCurr() == '&') { Bump(); return Token(Token::LogicAnd, GetSpan()); }
            return Token(Token::BitAnd, GetSpan());
        case '|':
            Bump();
            if (GetCurr() == '|') { Bump(); return Token(Token::LogicOr, GetSpan()); }
            return Token(Token::BitOr, GetSpan());
        case '^':
            Bump();
            return Token(Token::BitXOr, GetSpan());

        case '=':
            Bump();
            if (GetCurr() == '=') { Bump(); return Token(Token::Eq, GetSpan()); }
            return Token(Token::Assign, GetSpan());

        default:
            Bump();
            Error(FMT("unrecognized symbol '{}'", GetString()));
            return Token(GetSpan());
        }
    }

    void Lexer::SkipWhitespaceAndComments() {
        while (GetCurr().IsWhitespace()) {
            Bump();
        }
        if (GetCurr() == '/') {
            if (GetNext() == '/') {
                Bump(2);
                while (GetCurr() != '\n' && !GetCurr().IsEOF()) {
                    Bump();
                }
                SkipWhitespaceAndComments();
            }
            else if (GetNext() == '*') {
                m_TokenStartPosition = GetPosition();
                Bump(2);
                while (!(GetCurr() == '*' && GetNext() == '/')) {
                    // Don't allow unclosed comments at end of file
                    if (GetCurr().IsEOF()) {
                        ThrowError("unexpected end of file in comment");
                    }
                    Bump();
                }
                Bump(2);
                SkipWhitespaceAndComments();
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    // NUMBERS

    static uint64_t StringToInt(std::string_view str) {
        uint64_t value = 0;
        unsigned int base = 10;
        unsigned int index = 0;

        if (str.size() > 2) {
            if (str[0] == '0') {
                if (str[1] == 'b') {
                    base = 2;
                    index = 2;
                }
                else if (str[1] == '0') {
                    base = 8;
                    index = 2;
                }
                else if (str[1] == 'x') {
                    base = 16;
                    index = 2;
                }
            }
        }

        for (; index < str.size(); index++) {
            value *= base;
            value += range::GetNum(str[index], base);
        }

        return value;
    }

    static double StringToFloat(std::string_view str) {
        double value = 0;
        unsigned int index = 0;

        for (; range::IsDec(str[index]); index++) {
            value *= 10;
            value += range::GetNum(str[index], 10);
        }

        if (str[index] == '.') {
            index++;
            double power = 1.0;

            for (; range::IsDec(str[index]); index++) {
                power *= 10.0;
                value += range::GetNum(str[index], 10) / power;

                if (index == str.size() - 1) {
                    return value;
                }
            }
        }

        if (str[index] == 'e' || str[index] == 'E') {
            index++;
            unsigned int exp = 0;

            int sign = str[index] == '-' ? -1 : 1;
            if (str[index] == '-' || str[index] == '+') {
                index++;
            }

            for (; index < str.size(); index++) {
                exp *= 10;
                exp += range::GetNum(str[index], 10);
            }

            if (sign > 0) {
                value *= pow(10, exp);
            }
            else {
                value /= pow(10, exp);
            }
        }

        return value;
    }

    Token Lexer::LexNumber() {
        // Check binary, octal, hex prefixes
        unsigned int base = 10;
        if (GetCurr() == '0') {
            if (GetNext() == 'b') {
                base = 2;
                Bump(2);
                // Make sure there's a number after the prefix
                if (!GetCurr().IsBin()) {
                    Bump();
                    ThrowError("binary number missing value");
                }
            }
            else if (GetNext() == 'o') {
                base = 8;
                Bump(2);
                // Make sure there's a number after the prefix
                if (!GetCurr().IsOct()) {
                    Bump();
                    ThrowError("octal number missing value");
                }
            }
            else if (GetNext() == 'x') {
                base = 16;
                Bump(2);
                // Make sure there's a number after the prefix
                if (!GetCurr().IsHex()) {
                    Bump();
                    ThrowError("hexadecimal number missing value");
                }
            }
        }

        ReadDigits(base);

        bool isFloat = ((GetCurr() == '.' && GetNext().IsDec()) || GetCurr() == 'e' || GetCurr() == 'E');
        if (!isFloat) {
            return Token(Token::LitInt, Token::I32, StringToInt(GetString()), GetSpan());
        }

        ReadFraction();
        ReadExponent();

        if (base != 10) {
            Error("only decimal numbers support fractions and exponents");
            return Token(GetSpan());
        }

        return Token(Token::LitFloat, Token::F64, StringToFloat(GetString()), GetSpan());
    }

    void Lexer::ReadDigits(unsigned int base) {
        // Keep reading as long as Codepoints are valid numbers
        while (range::GetNum(GetCurr().Value, base) >= 0) {
            Bump();
        }
    }

    void Lexer::ReadFraction() {
        // Make sure this is a fraction, not a method call
        if (GetCurr() == '.' && GetNext().IsDec()) {
            Bump();
            ReadDigits(10);
        }
    }

    void Lexer::ReadExponent() {
        if (GetCurr() == 'e' || GetCurr() == 'E') {
            Bump();

            if (GetCurr() == '+' || GetCurr() == '-') {
                Bump();
            }

            // Make sure exponent has valid value
            if (GetCurr().IsDec()) {
                ReadDigits(10);
            }
            else {
                ThrowError("exponent requires a value");
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    // STRINGS

    Codepoint Lexer::ReadHexEscape(unsigned int length, Codepoint delim) {
        uint32_t value = 0;

        for (; length > 0; length--) {
            if (GetCurr().IsEOF()) {
                return Codepoint(0);
            }
            if (GetCurr() == delim) {
                value = 0xFFFD;
                break;
            }

            int n = range::GetNum(GetCurr().Value, 16);
            if (n >= 0) {
                value *= 16;
                value += n;
            }
            else {
                value = 0xFFFD;
                break;
            }
            Bump();
        }

        if (!range::IsChar(value)) {
            Error(FMT("invalid numeric escape : '{}'", GetString()));
            value = 0xFFFD;
        }

        return Codepoint(value);
    }

}