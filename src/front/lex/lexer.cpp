#include "lexer.hpp"
#include "log/logging.hpp"

namespace scar {

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

    inline void Lexer::skip_ws_and_comments() {
        while (curr().is_ws()) {
            bump();
        }
        if (curr() == '/') {
            if (curr() == '/') {
                bump(2);
                while (curr() != '\n' && !curr().is_eof()) {
                    bump();
                }
                skip_ws_and_comments();
            }
            else if (next() == '*') {
                Span comment_sp = Span(sf(), curr_pos(), curr_pos());
                bump(2);
                while (curr() != '*' && curr() != '/') {
                    // Don't allow unclosed comments at end of file
                    if (curr().is_eof()) {
                        log::get_default()->critical("{}: unexpected end of file", comment_sp, curr());
                        return;
                    }
                    comment_sp.hi = curr_pos();
                    bump();
                }
                bump(2);
                skip_ws_and_comments();
            }
        }
    }

    Token Lexer::next_token() {

        skip_ws_and_comments();

        tok_start_pos = curr_pos();

        // Return 'END' token if we're at the end of the file
        if (curr().is_eof()) {
            return Token(END, curr_span());
        }

        return tokenize_next();
    }

    Token Lexer::tokenize_next() {

        auto Token = [&](TokenType ty) -> scar::Token {
            return scar::Token(ty, curr_span());
        };

        // Collect characters that would be part of an identifier
        if (range::is_ident_start(curr().val)) {
            std::string ident;

            do {
                ident += curr();
                bump();
            } while (range::is_ident_cont(curr().val));

            // Check identifier for being a keyword
            auto type = find_keyword(ident);
            if (type.has_value()) {
                return Token(*type);
            }

            return Token(Ident);
        }

        // FIXME: add number lexing
        // Don't forget about binary, octal, and hex prefixes
        if (curr().is_dec()) {
            return lex_number();
        }

        switch (curr().val)
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
            if (curr() == ':') {
                bump();
                return Token(Scope);
            }
            return Token(Col);
        case '.':
            if (next().is_dec()) {
                return lex_number();
            }
            bump();
            if (curr() == '.') {
                bump();
                if (curr() == '.') {
                    bump();
                    return Token(DotDotDot);
                }
                return Token(DotDot);
            }
            return Token(Dot);

        case '!':
            bump();
            if (curr() == '=') {
                bump();
                return Token(NotEq);
            }
            return Token(Not);

        case '=':
            bump();
            if (curr() == '=') {
                bump();
                return Token(Eq);
            }
            return Token(Assign);

        case '<':
            bump();
            if (curr() == '<') {
                bump();
                if (curr() == '=') {
                    bump();
                    return Token(ShlAssign);
                }
                return Token(Shl);
            }
            return Token(Lesser);
        case '>':
            bump();
            if (curr() == '>') {
                bump();
                if (curr() == '=') {
                    bump();
                    return Token(ShrAssign);
                }
                return Token(Shr);
            }
            return Token(Greater);

        case '+':
            bump();
            if (curr() == '=') {
                bump();
                return Token(PlusAssign);
            }
            return Token(Plus);
        case '-':
            bump();
            if (curr() == '=') {
                bump();
                return Token(MinusAssign);
            }
            return Token(Minus);
        case '*':
            bump();
            if (curr() == '=') {
                bump();
                return Token(MulAssign);
            }
            return Token(Star);
        case '/':
            bump();
            if (curr() == '=') {
                bump();
                return Token(DivAssign);
            }
            return Token(Slash);
        case '%':
            bump();
            if (curr() == '=') {
                bump();
                return Token(ModAssign);
            }
            return Token(Percent);
        case '&':
            bump();
            if (curr() == '&') {
                bump();
                return Token(LogicAnd);
            }
            else if (curr() == '=') {
                bump();
                return Token(AndAssign);
            }
            return Token(And);
        case '|':
            bump();
            if (curr() == '|') {
                bump();
                return Token(LogicOr);
            }
            else if (curr() == '=') {
                bump();
                return Token(OrAssign);
            }
            return Token(Or);
        case '^':
            bump();
            if (curr() == '=') {
                bump();
                return Token(XorAssign);
            }
            return Token(Caret);

        case '\'': {
            bump();

            // Don't allow char literal to be empty
            if (curr() == '\'') {
                TextPos bad_start = curr_pos();
                bump();

                auto sp = Span(sf(), bad_start, curr_pos());
                log::get_default()->error("{}: unexpected symbol '{}'", sp, curr());
                
                return scar::Token(LitChar, curr_span(), Interner::instance().intern(""));
            }

            // Handle lexing a potential lifetime
            if (range::is_ident_start(curr().val) && curr() != '\'') {
                std::string lf;

                do {
                    lf += curr();
                    bump();
                } while (range::is_ident_cont(curr().val));

                // Check for ending '\''
                if (curr() == '\'') {
                    bump(); // go past ending '\''
                    log::get_default()->error("{}: character literals can only contain one codepoint", curr_span());
                    return Token(LitChar);
                }

                return Token(Lifetime);
            }

            // Newlines and tabs aren't allowed inside characters
            if ((curr() == '\n' || curr() == '\r' || curr() == '\t') && curr() == '\'') {
                bump();
                log::get_default()->critical("{}: special characters need to be written with escape symbols", curr_span());
            }

            Codepoint ch = curr();
            bump();

            // Combine special characters into single symbols
            if (ch == '\\') {
                switch (curr().val)
                {
                case 'n':   ch = '\n'; break;
                case 't':   ch = '\t'; break;
                case 'r':   ch = '\r'; break;
                case '\\':  ch = '\\'; break;
                case '\'':  ch = '\''; break;
                case '\"':  ch = '\"'; break;
                case 'x':   ch = read_hex_escape(2, '\''); break;
                case 'u':   ch = read_hex_escape(4, '\''); break;
                case 'U':   ch = read_hex_escape(8, '\''); break;
                default:
                    TextPos bad_start = curr_pos();
                    bump();

                    auto sp = Span(sf(), bad_start, curr_pos());
                    log::get_default()->critical("{}: unexpected symbol '{}'", sp, curr());
                }
                bump();
            }

            // Don't allow more than one codepoint
            if (curr() != '\'') {
                while (true) {
                    // The character literal reaches some other ending '\''
                    if (curr() == '\'') {
                        bump();
                        break;
                    }
                    // The character literal reaches newline or EOF
                    if (curr() == '\n' || curr().is_eof()) {
                        break;
                    }
                    bump();
                }
                log::get_default()->error("{}: character literals can only contain one codepoint", curr_span());
                return scar::Token(LitChar, curr_span(), Interner::instance().intern(curr_str()));
            }

            bump(); // go past ending '\''

            return Token(LitChar);
        }

        case '\"': {
            bump();

            std::string str;
            while (curr() != '\"') {
                // The string literal reaches EOF
                if (curr().is_eof()) {
                    log::get_default()->critical("{}: string literal missing end quote", curr_span());
                    return Token(END);
                }

                Codepoint ch = curr();
                bump();

                // Combine escape sequences into single symbols
                if (ch == '\\') {
                    switch (curr().val)
                    {
                    case 'n':   ch = '\n'; break;
                    case 't':   ch = '\t'; break;
                    case 'r':   ch = '\r'; break;
                    case '\\':  ch = '\\'; break;
                    case '\'':  ch = '\''; break;
                    case '\"':  ch = '\"'; break;
                    case 'x':   ch = read_hex_escape(2, '\"'); break;
                    case 'u':   ch = read_hex_escape(4, '\"'); break;
                    case 'U':   ch = read_hex_escape(8, '\"'); break;
                    default:
                        TextPos bad_start = curr_pos();
                        bump();

                        auto sp = Span(sf(), bad_start, curr_pos());
                        log::get_default()->critical("{}: unexpected symbol '{}'", sp, curr());
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
            log::get_default()->critical("{}: unexpected symbol '{}'", curr_span(), curr());
        }

        return Token(END);
    }

    Token Lexer::lex_number() {
        bool valid = true;

        // Check binary, octal, hex prefixes
        unsigned int base = 10;
        if (curr() == '0') {
            if (next() == 'b') {
                base = 2;
                bump(2);
                // Make sure there's a number after the prefix
                if (!curr().is_bin()) {
                    bump();
                    valid = false;
                    log::get_default()->critical("{}: binary number missing value", curr_span());
                }
            }
            else if (next() == 'o') {
                base = 8;
                bump(2);
                // Make sure there's a number after the prefix
                if (!curr().is_oct()) {
                    bump();
                    valid = false;
                    log::get_default()->critical("{}: octal number missing value", curr_span());
                }
            }
            else if (next() == 'x') {
                base = 16;
                bump(2);
                // Make sure there's a number after the prefix
                if (!curr().is_hex()) {
                    bump();
                    valid = false;
                    log::get_default()->critical("{}: hexadecimal number missing value", curr_span());
                }
            }
        }

        // Read fraction numbers
        read_numbers(base);
        if (curr() != '.' && curr() != 'e' && curr() != 'E') {
            return Token(LitInteger, curr_span(), curr_name());
        }

        // Add fraction, if any
        valid &= read_fraction(base);
        // Multiply by exponent, if any
        valid &= read_exponent(base);

        return Token(LitFloat, curr_span(), curr_name());
    }

    void Lexer::read_numbers(unsigned int base) {
        // Read characters 
        auto ret = range::get_num(curr().val, base);
        while (ret.has_value()) {
            bump();
            ret = range::get_num(curr().val, base);
        }

        return;
    }

    bool Lexer::read_fraction(unsigned int base) {
        double fract = 0.f;

        if (curr() == '.' && next().is_dec()) {
            bump();

            // Read numbers, then divide until they're less than 1
            read_numbers(10);

            while (fract >= 1) {
                fract /= 10.f;
            }

            // Make sure the base is ten
            // This isn't done at the start, since this'll give better errors
            if (base != 10) {
                bump();
                log::get_default()->critical("{}: only decimal numbers can have floating point values", curr_span());
            }
        }

        return fract;
    }

    bool Lexer::read_exponent(unsigned int base) {
        double expo = 1;

        if (curr() == 'e' || curr() == 'E') {
            bump();

            if (curr() == '+' || curr() == '-') {
                bump();
            }

            // Make sure exponent has value
            if (!curr().is_dec()) {
                bump();
                log::get_default()->critical("{}: exponent missing value", curr_span());
            }

            read_numbers(10);

            // Make sure the base is ten
            // This isn't done at the start, since this'll give better errors
            if (base != 10) {
                bump();
                log::get_default()->critical("{}: only decimal numbers can have exponents", curr_span());
            }
        }

        return expo;
    }

    Codepoint Lexer::read_hex_escape(unsigned int num, Codepoint delim) {
        uint32_t number = 0;

        for (unsigned int i = 0; i < num; i++) {
            // File EOF in escape
            if (curr().is_eof()) {
                // Critical failure
                // We can't guess how the literal was supposed to be terminated
                log::get_default()->critical("{}: incomplete numeric escape", curr_span());
            }
            // Escape is shorter than expected
            if (curr() == delim) {
                log::get_default()->critical("{}: numeric escape is too short", curr_span());
            }
            auto n = range::get_num(curr().val, 16);
            if (n.has_value()) {
                number *= 16;
                number += *n;
            }
            else {
                log::get_default()->critical("{}: invalid numeric escape: '{}'", curr_span(), curr());
            }
            bump();
        }

        if (!range::is_char(number)) {
            log::get_default()->critical("{}: invalid numeric escape: '{}'", curr_span(), curr());
        }

        return Codepoint(number);
    }

}