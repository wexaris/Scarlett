#include "lexer.hpp"
#include "errors/errors.hpp"
#include "util/ranges.hpp"
#include <unordered_map>
#include <optional>

const std::unordered_map<std::string, TokenType> keyword_map = {

    /// Literals
    { "true",       TokenType::True },
    { "false",      TokenType::False },

    /// Keywords
    { "var",        TokenType::Var },
    { "fun",        TokenType::Fun },
    { "struct",     TokenType::Struct },
    { "enum",       TokenType::Enum },
    { "union",      TokenType::Union },
    { "trait",      TokenType::Trait },
    { "macro",      TokenType::Macro },

    { "if",         TokenType::If },
    { "else",       TokenType::Else },
    { "loop",       TokenType::Loop },
    { "for",        TokenType::For },
    { "do",         TokenType::Do },
    { "while",      TokenType::While },
    { "switch",     TokenType::Switch },
    { "case",       TokenType::Case },
    { "match",      TokenType::Match },
    { "continue",   TokenType::Continue },
    { "break",      TokenType::Break },
    { "return",     TokenType::Return },

    { "pub",        TokenType::Pub },
    { "priv",       TokenType::Priv },
    { "static",     TokenType::Static },
    { "const",      TokenType::Const },
    { "mut",        TokenType::Mut },

    /// Primitives
    { "str",        TokenType::Str },
    { "char",       TokenType::Char },
    { "bool",       TokenType::Bool },

    { "isize",      TokenType::Isize },
    { "i8",         TokenType::I8 },
    { "i16",        TokenType::I16 },
    { "i32",        TokenType::I32 },
    { "i64",        TokenType::I64 },

    { "usize",      TokenType::Usize },
    { "u8",         TokenType::U8 },
    { "u16",        TokenType::U16 },
    { "u32",        TokenType::U32 },
    { "u64",        TokenType::U64 },

    { "f32",        TokenType::F32 },
    { "f64",        TokenType::F64 },

    /// Meh
    { "_",          TokenType::Meh },
};

/* Search and return the TokenType corresponding to some input text.
If no types match, an std::nullopt is returned. */
inline std::optional<TokenType> find_keyword(const std::string& id) {
    auto iter = keyword_map.find(id);
    if (iter != keyword_map.end()) { return (*iter).second; }
    return std::nullopt;
}


/////////////
/// Lexer ///
/////////////

Lexer::Lexer(const std::string& filepath) :
    reader(filepath),
    curr_c(reader.curr()),
    next_c(reader.next())
{}

void Lexer::bump(uint n) {
    reader.bump(n);
    curr_c = reader.curr();
    next_c = reader.next();
}

inline void Lexer::skip_ws_and_comments() {
    while (curr_c.is_ws()) {
        bump();
    }
    if (curr_c == '/') {
        if (next_c == '/') {
            bump(2);
            while (curr_c != '\n' && !curr_c.is_eof()) {
                bump();
            }
            skip_ws_and_comments();
        }
        else if (next_c == '*') {
            Span comment_sp = Span(get_sf(), get_pos(), get_pos());
            bump(2);
            while (curr_c != '*' && curr_c != '/') {
                // Don't allow unclosed comments at end of file
                if (curr_c.is_eof()) {
                    throw LexError::UnexpectedEOF(comment_sp);
                }
                comment_sp.hi = get_pos();
                bump();
            }
            bump(2);
            skip_ws_and_comments();
        }
    }
}

Token Lexer::next_token() {

    skip_ws_and_comments();

    TextPos start_pos = get_pos();

    // Return 'END' token if we're at the end of the file
    if (curr_c.is_eof()) {
        return Token(END, Span(get_sf(), start_pos, get_pos()));
    }

    return tokenize_next(start_pos);
}

Token Lexer::tokenize_next(const TextPos& start_pos) {

    auto Token = [&](TokenType ty, const TokenValue& val = TokenValue()) {
        return ::Token(ty, Span(get_sf(), start_pos, get_pos()), val);
    };

    // Collect characters that would be part of an identifier
    if (is_ident_start(curr_c)) {
        std::string ident;

        do {
            ident += curr_c;
            bump();
        } while (is_ident_cont(curr_c));

        // Check identifier for being a keyword
        auto type = find_keyword(ident);
        if (type.has_value()) {
            return Token(*type);
        }

        return Token(Ident);
    }

    // FIXME: add number lexing
    // Don't forget about binary, octal, and hex prefixes
    if (curr_c.is_dec()) {
        return lex_number(start_pos);
    }

    switch (curr_c.val)
    {
    case ';': bump(); return Token(Semi);
    case ',': bump(); return Token(Comma);
    case '(': bump(); return Token(Lparen);
    case ')': bump(); return Token(Rparen);
    case '[': bump(); return Token(Lbrack);
    case ']': bump(); return Token(Rbrack);
    case '{': bump(); return Token(Lbrace);
    case '}': bump(); return Token(Rbrace);

    case ':':
        bump();
        if (curr_c == ':') {
            bump();
            return Token(Scope);
        }
        return Token(Col);
    case '.':
        if (next_c.is_dec()) {
            return lex_number(start_pos);
        }
        bump();
        if (curr_c == '.') {
            bump();
            if (curr_c == '.') {
                bump();
                return Token(DotDotDot);
            }
            return Token(DotDot);
        }
        return Token(Dot);

    case '!':
        bump();
        if (curr_c == '=') {
            bump();
            return Token(NotEq);
        }
        return Token(Not);

    case '=':
        bump();
        if (curr_c == '=') {
            bump();
            return Token(Eq);
        }
        return Token(Assign);

    case '<':
        bump();
        if (curr_c == '<') {
            bump();
            if (curr_c == '=') {
                bump();
                return Token(ShlAssign);
            }
            return Token(Shl);
        }
        return Token(Lesser);
    case '>':
        bump();
        if (curr_c == '>') {
            bump();
            if (curr_c == '=') {
                bump();
                return Token(ShrAssign);
            }
            return Token(Shr);
        }
        return Token(Greater);

    case '+':
        bump();
        if (curr_c == '=') {
            bump();
            return Token(PlusAssign);
        }
        return Token(Plus);
    case '-':
        bump();
        if (curr_c == '=') {
            bump();
            return Token(MinusAssign);
        }
        return Token(Minus);
    case '*':
        bump();
        if (curr_c == '=') {
            bump();
            return Token(MulAssign);
        }
        return Token(Star);
    case '/':
        bump();
        if (curr_c == '=') {
            bump();
            return Token(DivAssign);
        }
        return Token(Slash);
    case '%':
        bump();
        if (curr_c == '=') {
            bump();
            return Token(ModAssign);
        }
        return Token(Percent);
    case '&':
        bump();
        if (curr_c == '&') {
            bump();
            return Token(LogicAnd);
        }
        else if (curr_c == '=') {
            bump();
            return Token(AndAssign);
        }
        return Token(And);
    case '|':
        bump();
        if (curr_c == '|') {
            bump();
            return Token(LogicOr);
        }
        else if (curr_c == '=') {
            bump();
            return Token(OrAssign);
        }
        return Token(Or);
    case '^':
        bump();
        if (curr_c == '=') {
            bump();
            return Token(XorAssign);
        }
        return Token(Caret);

    case '\'': {
        bump();

        // Don't allow char literal to be empty
        if (curr_c == '\'') {
            TextPos bad_start = get_pos();
            bump();
            throw LexError::UnexpectedSymbol(Span(get_sf(), bad_start, get_pos()), curr_c);
        }

        // Handle lexing a potencial lifetime
        if (is_ident_start(curr_c) && curr_c != '\'') {
            std::string lf;

            do {
                lf += curr_c;
                bump();
            } while (is_ident_cont(curr_c));

            // Check for ending '\''
            if (curr_c == '\'') {
                bump(); // go past ending '\''
                throw LexError::Generic(Span(get_sf(), start_pos, get_pos()),
                    "Character literals can only contain one codepoint");
            }

            return Token(Lifetime);
        }

        // Newlines and tabs aren't allowed inside characters
        if ((curr_c == '\n' || curr_c == '\r' || curr_c == '\t') && curr_c == '\'') {
            bump();
            throw LexError::Generic(Span(get_sf(), start_pos, get_pos()),
                "Special characters need to be written with escape symbols");
        }

        Codepoint ch = curr_c;
        bump();

        // Combine special characters into single symbols
        if (ch == '\\') {
            switch (curr_c.val)
            {
            case 'n':   ch = '\n'; break;
            case 't':   ch = '\t'; break;
            case 'r':   ch = '\r'; break;
            case '\\':  ch = '\\'; break;
            case '\'':  ch = '\''; break;
            case '\"':  ch = '\"'; break;
            case 'x':   ch = read_hex_escape(2, '\'', start_pos); break;
            case 'u':   ch = read_hex_escape(4, '\'', start_pos); break;
            case 'U':   ch = read_hex_escape(8, '\'', start_pos); break;
            default:
                TextPos bad_start = get_pos();
                bump();
                throw LexError::UnexpectedSymbol(Span(get_sf(), bad_start, get_pos()), curr_c);
            }
            bump();
        }

        // Don't allow more than one codepoint
        if (curr_c != '\'') {
            while (true) {
                // The character literal reaches some other ending '\''
                if (curr_c == '\'') {
                    bump();
                    break;
                }
                // The character literal reaches newline or EOF
                if (curr_c == '\n' || curr_c.is_eof()) {
                    break;
                }
                bump();
            }
            throw LexError::Generic(Span(get_sf(), start_pos, get_pos()),
                "Character literals can only contain one codepoint");
        }

        bump(); // go past ending '\''

        return Token(LitChar);
    }

    case '\"': {
        bump();

        std::string str;
        while (curr_c != '\"') {
            // The string literal reaches EOF
            if (curr_c.is_eof()) {
                throw LexError::Generic(Span(get_sf(), start_pos, get_pos()),
                    "String literals missing end quote");
            }

            Codepoint ch = curr_c;
            bump();

            // Combine escape sequences into single symbols
            if (ch == '\\') {
                switch (curr_c.val)
                {
                case 'n':   ch = '\n'; break;
                case 't':   ch = '\t'; break;
                case 'r':   ch = '\r'; break;
                case '\\':  ch = '\\'; break;
                case '\'':  ch = '\''; break;
                case '\"':  ch = '\"'; break;
                case 'x':   ch = read_hex_escape(2, '\"', start_pos); break;
                case 'u':   ch = read_hex_escape(4, '\"', start_pos); break;
                case 'U':   ch = read_hex_escape(8, '\"', start_pos); break;
                default:
                    TextPos bad_start = get_pos();
                    bump();
                    throw LexError::UnexpectedSymbol(Span(get_sf(), bad_start, get_pos()), curr_c);
                }
                bump();
            }

            str += ch;
        }
        bump(); // go past ending '\"'

        return Token(LitString);
    }

    default:
        bump();
        throw LexError::UnexpectedSymbol(Span(get_sf(), start_pos, get_pos()), curr_c);
    }

    return Token(END);
}

uint Lexer::read_numbers(uint base) {
    uint num = 0;

    std::optional<uint> ret = get_num(curr_c, base);
    while (ret.has_value()) {
        num *= base;
        num += *ret;
        bump();
        ret = get_num(curr_c, base);
    }

    return num;
}

double Lexer::read_fraction(uint base, const TextPos& start_pos) {
    double fract = 0.f;

    if (curr_c == '.' && next_c.is_dec()) {
        bump();

        // Read numbers, then divide until they're less than 1
        fract = read_numbers(10);
        while (fract >= 1) {
            fract /= 10.f;
        }

        // Make sure the base is ten
        // This isn't done at the start, since this'll give better errors
        if (base != 10) {
            bump();
            throw LexError::Generic(Span(get_sf(), start_pos, get_pos()),
                "Only decimal numbers can have floating point values");
        }
    }

    return fract;
}

double Lexer::read_exponent(uint base, const TextPos& start_pos) {
    double expo = 1;

    if (curr_c == 'e' || curr_c == 'E') {
        bump();

        // Get sign of exponent
        int sign = 1;
        if (curr_c == '+') {
            bump();
        }
        else if (curr_c == '-') {
            bump();
            sign = -1;
        }

        // Make sure exponent has value
        if (!curr_c.is_dec()) {
            bump();
            throw LexError::Generic(Span(get_sf(), start_pos, get_pos()),
                "Exponent missing value");
        }

        expo = pow(10, sign * read_numbers(10));

        // Make sure the base is ten
        // This isn't done at the start, since this'll give better errors
        if (base != 10) {
            bump();
            throw LexError::Generic(Span(get_sf(), start_pos, get_pos()),
                "Only decimal numbers can have exponents");
        }
    }

    return expo;
}

Token Lexer::lex_number(const TextPos& start_pos) {
    // Check binary, octal, hex prefixes
    uint base = 10;
    if (curr_c == '0') {
        if (next_c == 'b') {
            base = 2;
            bump(2);
            // Make sure there's a number after the prefix
            if (!curr_c.is_bin()) {
                bump();
                throw LexError::Generic(Span(get_sf(), start_pos, get_pos()),
                    "Binary number missing value");
            }
        }
        else if (next_c == 'o') {
            base = 8;
            bump(2);
            // Make sure there's a number after the prefix
            if (!curr_c.is_oct()) {
                bump();
                throw LexError::Generic(Span(get_sf(), start_pos, get_pos()),
                    "Octal number missing value");
            }
        }
        else if (next_c == 'x') {
            base = 16;
            bump(2);
            // Make sure there's a number after the prefix
            if (!curr_c.is_hex()) {
                bump();
                throw LexError::Generic(Span(get_sf(), start_pos, get_pos()),
                    "Hexadecimal number missing value");
            }
        }
    }

    // Read fraction numbers
    double num = read_numbers(base);
    if (curr_c != '.' && curr_c != 'e' && curr_c != 'E') {
        return Token(LitInteger, Span(get_sf(), start_pos, get_pos()), static_cast<TokenValue::IntType>(num));
    }

    // Add fraction, if any
    num += read_fraction(base, start_pos);
    // Multiply by exponent, if any
    num *= read_exponent(base, start_pos);

    return Token(LitFloat, Span(get_sf(), start_pos, get_pos()), num);
}

Codepoint Lexer::read_hex_escape(uint num, Codepoint delim, const TextPos& start_pos) {
    uint number = 0;

    for (uint i = 0; i < num; i++) {
        // File EOF in escape
        if (curr_c.is_eof()) {
            // Critical failure
            // We can't guess how the literal was supposed to be terminated
            throw LexError::Generic(Span(get_sf(), start_pos, get_pos()),
                "Incomplete numeric escape");
        }
        // Escape is shorter than expected
        if (curr_c == delim) {
            throw LexError::Generic(Span(get_sf(), start_pos, get_pos()),
                "Numeric escape is too short");
        }
        auto n = get_num(curr_c, 16);
        if (n.has_value()) {
            number *= 16;
            number += *n;
        }
        else {
            throw LexError::Generic(Span(get_sf(), start_pos, get_pos()),
                FMT("Invalid numeric escape: " << curr_c));
        }
        bump();
    }

    if (!is_char(number)) {
        throw LexError::Generic(Span(get_sf(), start_pos, get_pos()),
            FMT("Invalid numeric escape: " << curr_c));
    }

    return Codepoint(number);
}