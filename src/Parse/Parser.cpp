#include "scarpch.hpp"
#include "Parse/Parser.hpp"
#include "Parse/Lex/Lexer.hpp"

namespace scar {

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    // MISCELANEOUS

    static std::ostream& operator<<(std::ostream& os, const std::vector<Token::Type>& types) {
        os << types[0];
        for (size_t i = 1; i < types.size(); i++) { os << "," << types[i]; }
        return os;
    }

    static std::vector<Token::Type> operator+(const std::vector<Token::Type>& first, const std::vector<Token::Type>& second) {
        std::vector<Token::Type> ret = first;
        ret.insert(ret.end(), second.begin(), second.end());
        return ret;
    }

    static std::vector<Token::Type> operator+(const std::vector<Token::Type>& vec, Token::Type type) {
        std::vector<Token::Type> ret = vec;
        ret.push_back(type);
        return ret;
    }

    static std::vector<Token::Type> s_DeclStartTokens = {
        Token::Func,
    };
    static std::vector<Token::Type> s_StmtStartTokens = {
        Token::If,
        Token::Else,
        Token::For,
        Token::While,
        Token::Loop,
        Token::Var,
        Token::Continue,
        Token::Break,
        Token::Return,
        Token::Semi,
    };
    
    struct OperatorInfo {
        Token::Type TokenType = Token::Invalid;
        unsigned int Precedence = 0;
        enum Assoc { Left, Right } Associativity = Assoc::Left;

        OperatorInfo() = default;
        OperatorInfo(Token::Type tokenType, unsigned int prec, Assoc assoc) :
            TokenType(tokenType),
            Precedence(prec),
            Associativity(assoc) {
        }
    };

    static OperatorInfo GetPrefixOperatorInfo(Token::Type type) {
        switch (type) {
        case Token::PlusPlus:   return OperatorInfo(type, 12, OperatorInfo::Right);
        case Token::MinusMinus: return OperatorInfo(type, 12, OperatorInfo::Right);
        case Token::Plus:   return OperatorInfo(type, 12, OperatorInfo::Right);
        case Token::Minus:  return OperatorInfo(type, 12, OperatorInfo::Right);
        case Token::Not:    return OperatorInfo(type, 12, OperatorInfo::Right);
        case Token::BitNot: return OperatorInfo(type, 12, OperatorInfo::Right);
        default:
            SCAR_BUG("missing prefix OperatorInfo for Token::Type {}", (int)type);
            return OperatorInfo();
        }
    }
    static OperatorInfo GetSuffixOperatorInfo(Token::Type type) {
        switch (type) {
        case Token::PlusPlus:   return OperatorInfo(Token::PlusPlus,   13, OperatorInfo::Left);
        case Token::MinusMinus: return OperatorInfo(Token::MinusMinus, 13, OperatorInfo::Left);
        default:
            SCAR_BUG("missing suffix OperatorInfo for Token::Type {}", (int)type);
            return OperatorInfo();
        }
    }
    static OperatorInfo GetBinaryOperatorInfo(Token::Type type) {
        switch (type) {
        case Token::Dot:       return OperatorInfo(type, 13, OperatorInfo::Left);
        case Token::Star:      return OperatorInfo(type, 11, OperatorInfo::Left);
        case Token::Slash:     return OperatorInfo(type, 11, OperatorInfo::Left);
        case Token::Percent:   return OperatorInfo(type, 11, OperatorInfo::Left);
        case Token::Plus:      return OperatorInfo(type, 10, OperatorInfo::Left);
        case Token::Minus:     return OperatorInfo(type, 10, OperatorInfo::Left);
        case Token::Greater:   return OperatorInfo(type, 8, OperatorInfo::Left);
        case Token::GreaterEq: return OperatorInfo(type, 8, OperatorInfo::Left);
        case Token::Lesser:    return OperatorInfo(type, 8, OperatorInfo::Left);
        case Token::LesserEq:  return OperatorInfo(type, 8, OperatorInfo::Left);
        case Token::Eq:        return OperatorInfo(type, 7, OperatorInfo::Left);
        case Token::NotEq:     return OperatorInfo(type, 7, OperatorInfo::Left);
        case Token::BitAnd:    return OperatorInfo(type, 6, OperatorInfo::Left);
        case Token::BitXOr:    return OperatorInfo(type, 5, OperatorInfo::Left);
        case Token::BitOr:     return OperatorInfo(type, 4, OperatorInfo::Left);
        case Token::LogicAnd:  return OperatorInfo(type, 3, OperatorInfo::Left);
        case Token::LogicOr:   return OperatorInfo(type, 2, OperatorInfo::Left);
        case Token::Assign:    return OperatorInfo(type, 1, OperatorInfo::Right);
        default:
            SCAR_BUG("missing binary OperatorInfo for Token::Type {}", (int)type);
            return OperatorInfo();
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    // PARSER

    Parser::Parser(const std::string& path) {
        Lexer lexer(path);
        m_TokenStream = lexer.Lex();
        m_Token = m_TokenStream.begin();
    }

    void Parser::Bump() {
        m_Token++;
    }

    bool Parser::Match(const std::vector<Token::Type>& expected) const {
        for (auto type : expected) {
            if (*m_Token == type) {
                return true;
            }
        }
        return false;
    }

    Token& Parser::Expect(const std::vector<Token::Type>& expected) {
        if (!Match(expected)) {
            throw ParseError(ErrorCode::UnexpectedToken, [&]() {
                m_ErrorCount++;
                ThrowError(FMT("unexpected token: {} where {} was expected", *m_Token, expected), m_Token->GetSpan());
            });
        }
        Bump();
        return *(m_Token - 1);
    }

    void Parser::Synchronize(const std::vector<Token::Type>& delims) {
        SCAR_TRACE("synchronizing");
        while (!Match(delims)) {
            Bump();
        }
    }

    void Parser::ThrowError(std::string_view msg, const Span& span) {
        throw ParseError(ErrorCode::UnexpectedToken, [&]() {
            SCAR_ERROR("{}: {}", span, msg);
            m_ErrorCount++;
        });
    }

    Ref<ast::Module> Parser::Parse() {
        TextPosition start;
        std::vector<Ref<ast::Decl>> items;
        while (*m_Token != Token::EndOfFile) {
            if (auto item = Decl()) {
                items.push_back(item);
            }
        }
        return MakeRef<ast::Module>(items, GetSpanFrom(start));
    }

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    // TYPE

    // type : BOOL
    //      | I8 I16 I32 I64
    //      | U8 U16 U32 U64
    //      | F32 F64
    Ref<ast::Type> Parser::Type() {
        Token& token = Expect({ Token::Bool,
            Token::I8, Token::I16, Token::I32, Token::I64,
            Token::U8, Token::U16, Token::U32, Token::U64,
            Token::F32, Token::F64 });
        return MakeRef<ast::Type>((ast::Type::VariableType)token.GetType(), token.GetSpan());
    }

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    // SUPPORT

    // ident : IDENT
    ast::Ident Parser::Ident() {
        Token& token = Expect({ Token::Ident });
        return ast::Ident(token.GetName(), token.GetSpan());
    }

    // arg : ident type
    ast::Arg Parser::Arg() {
        TextPosition start = m_Token->GetTextPos();

        ast::Ident ident = Ident();
        Ref<ast::Type> type = Type();

        return ast::Arg(ident, type, GetSpanFrom(start));
    }

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    // DECLARATIONS

    // decl : function
    Ref<ast::Decl> Parser::Decl() {
        try {
            switch (m_Token->GetType()) {
            case Token::Func: return Function();
            default:
                ThrowError("expected a declaration", m_Token->GetSpan());
                break;
            }
        }
        catch (ParseError& e) {
            e.OnCatch();
            Synchronize({ s_DeclStartTokens });
            return nullptr;
        }

        return nullptr;
    }

    // function : prototype block
    //          | prototype ;
    Ref<ast::Decl> Parser::Function() {
        TextPosition start = m_Token->GetTextPos();

        Ref<ast::FunctionPrototype> prototype = FunctionPrototype();

        if (*m_Token == Token::Semi) {
            Bump();
            return prototype;
        }

        Ref<ast::Block> block = Block();

        return MakeRef<ast::Function>(prototype, block, GetSpanFrom(start));
    }

    // prototype : FUNC ident ( arg* ) -> type
    //           | FUNC ident ( arg* )
    Ref<ast::FunctionPrototype> Parser::FunctionPrototype() {
        TextPosition start = m_Token->GetTextPos();

        Expect({ Token::Func });

        ast::Ident ident = Ident();

        Expect({ Token::LParen });
        std::vector<ast::Arg> args;
        while (*m_Token != Token::RParen) {
            args.push_back(Arg());
        }
        Expect({ Token::RParen });

        Ref<ast::Type> retType;
        if (*m_Token != Token::RArrow) {
            retType = MakeRef<ast::Type>(ast::Type::Void, ident.GetSpan());
        }
        else {
            Expect({ Token::RArrow });
            retType = Type();
        }

        return MakeRef<ast::FunctionPrototype>(ident, args, retType, GetSpanFrom(start));
    }

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    // STATEMENTS

    // stmt : function
    //      | branch
    //      | for_loop
    //      | while_loop
    //      | loop
    //      | return
    //      | expr ;
    //      | ;
    Ref<ast::Stmt> Parser::Stmt() {
        try {
            switch (m_Token->GetType()) {
            case Token::If:       return Branch();
            case Token::For:      return ForLoop();
            case Token::While:    return WhileLoop();
            case Token::Loop:     return Loop();
            case Token::Continue: return Continue();
            case Token::Break:    return Break();
            case Token::Return:   return Return();
            case Token::Semi:     Bump(); return nullptr;
            default:
            {
                TextPosition start = m_Token->GetTextPos();

                if (Ref<ast::Expr> expr = TryExpr()) {
                    Expect({ Token::Semi });
                    return expr;
                }

                ThrowError("expected a statement", GetSpanFrom(start));
                break;
            }
            }
        }
        catch (ParseError& e) {
            e.OnCatch();
            Synchronize({ s_StmtStartTokens });
            return nullptr;
        }

        return nullptr;
    }

    // branch : IF ( expr ) block ELSE block 
    //        | IF ( expr ) block
    Ref<ast::Branch> Parser::Branch() {
        TextPosition start = m_Token->GetTextPos();

        Expect({ Token::If });

        Expect({ Token::LParen });
        Ref<ast::Expr> cond = Expr();
        Expect({ Token::RParen });

        Ref<ast::Block> trueBlock = Block();

        Expect({ Token::Else });

        Ref<ast::Block> falseBlock = Block();
        
        return MakeRef<ast::Branch>(cond, trueBlock, falseBlock, GetSpanFrom(start));
    }

    // for_loop : FOR ( try_expr ; expr ; try_expr ) block
    Ref<ast::ForLoop> Parser::ForLoop() {
        TextPosition start = m_Token->GetTextPos();

        Expect({ Token::For });

        Expect({ Token::LParen });
        Ref<ast::Expr> init = TryExpr();
        Expect({ Token::Semi });
        Ref<ast::Expr> cond = Expr();
        Expect({ Token::Semi });
        Ref<ast::Expr> update = TryExpr();
        Expect({ Token::RParen });

        Ref<ast::Block> block = Block();

        return MakeRef<ast::ForLoop>(init, cond, update, block, GetSpanFrom(start));
    }

    // while_loop : WHILE ( expr ) block
    Ref<ast::WhileLoop> Parser::WhileLoop() {
        TextPosition start = m_Token->GetTextPos();

        Expect({ Token::While });

        Expect({ Token::LParen });
        Ref<ast::Expr> cond = Expr();
        Expect({ Token::RParen });

        Ref<ast::Block> block = Block();

        return MakeRef<ast::WhileLoop>(cond, block, GetSpanFrom(start));
    }

    // loop : LOOP block
    Ref<ast::WhileLoop> Parser::Loop() {
        TextPosition start = m_Token->GetTextPos();

        Expect({ Token::Loop });
        Ref<ast::LiteralFloat> cond = MakeRef<ast::LiteralFloat>(1.0, ast::Type::F64, m_Token->GetSpan());
        Ref<ast::Block> block = Block();

        return MakeRef<ast::WhileLoop>(cond, block, GetSpanFrom(start));
    }

    // block : { stmt* }
    Ref<ast::Block> Parser::Block() {
        TextPosition start = m_Token->GetTextPos();

        Expect({ Token::LBrace });
        std::vector<Ref<ast::Stmt>> items;
        while (*m_Token != Token::RBrace) {
            if (auto item = Stmt()) {
                items.push_back(item);
            }
        }
        Expect({ Token::RBrace });

        return MakeRef<ast::Block>(items, GetSpanFrom(start));
    }

    // continue : CONTINUE ;
    Ref<ast::Continue> Parser::Continue() {
        TextPosition start = m_Token->GetTextPos();
        Expect({ Token::Continue });
        Expect({ Token::Semi });
        return MakeRef<ast::Continue>(GetSpanFrom(start));
    }

    // break : BREAK ;
    Ref<ast::Break> Parser::Break() {
        TextPosition start = m_Token->GetTextPos();
        Expect({ Token::Break });
        Expect({ Token::Semi });
        return MakeRef<ast::Break>(GetSpanFrom(start));
    }

    // return : RETURN try_expr ;
    Ref<ast::Return> Parser::Return() {
        TextPosition start = m_Token->GetTextPos();

        Expect({ Token::Return });
        Ref<ast::Expr> value = TryExpr();
        Expect({ Token::Semi });

        return MakeRef<ast::Return>(value, GetSpanFrom(start));
    }

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    // EXPRESSIONS

    // try_expr : expr?
    Ref<ast::Expr> Parser::TryExpr() {
        return Expr(1, true);
    }

    // expr : atom binary_op expr
    //      | atom
    Ref<ast::Expr> Parser::Expr(unsigned int prec, bool allowEmpty) {
        TextPosition start = m_Token->GetTextPos();

        // Parse left side of expression
        Ref<ast::Expr> lhs = ExprAtom(prec, allowEmpty);
        if (!lhs && allowEmpty) {
            return nullptr;
        }

        while (IsBinaryOperator()) {
            OperatorInfo opInfo = GetBinaryOperatorInfo(m_Token->GetType());
            if (opInfo.Precedence < prec) {
                break;
            }
            Bump();

            // Parse right side of expression
            Ref<ast::Expr> rhs = Expr(opInfo.Associativity == OperatorInfo::Left ?
                                      opInfo.Precedence + 1 :
                                      opInfo.Precedence);

            // Set LHS to complete expression
            lhs = MakeRef<ast::BinaryOperator>((ast::BinaryOperator::OpType)opInfo.TokenType, lhs, rhs, GetSpanFrom(start));
        }

        return lhs;
    }

    // atom : prefix_op expr
    //      | atom suffix_op
    //      | variable
    //      | function_call
    //      | LIT_INT | LIT_FLOAT | LIT_STRing
    Ref<ast::Expr> Parser::ExprAtom(unsigned int prec, bool allowEmpty) {
        TextPosition start = m_Token->GetTextPos();
        Ref<ast::Expr> atom;

        // Parse prefix operator
        if (IsPrefixOperator()) {
            OperatorInfo opInfo = GetPrefixOperatorInfo(m_Token->GetType());
            if (opInfo.Precedence >= prec) {
                Bump();

                // Parse rest of atom
                atom = Expr(opInfo.Associativity == OperatorInfo::Left ?
                            opInfo.Precedence + 1 :
                            opInfo.Precedence);

                return MakeRef<ast::PrefixOperator>((ast::PrefixOperator::OpType)opInfo.TokenType, atom, GetSpanFrom(start));
            }
        }

        // Parse atom body
        switch (m_Token->GetType()) {
        case Token::Ident: {
            ast::Ident ident = Ident();
            if (*m_Token == Token::LParen) {
                Bump();
                std::vector<Ref<ast::Expr>> args;
                while (*m_Token != Token::RParen) {
                    args.push_back(Expr());
                }
                Bump();
                atom = MakeRef<ast::FunctionCall>(ident, args, GetSpanFrom(start));
            }
            else {
                atom = MakeRef<ast::Variable>(ident, ident.GetSpan());
            }
            break;
        }
        case Token::Var: return Var();
        case Token::True:
            atom = MakeRef<ast::LiteralBool>(true, GetSpanFrom(start));
            Bump();
            break;
        case Token::False:
            atom = MakeRef<ast::LiteralBool>(false, GetSpanFrom(start));
            Bump();
            break;
        case Token::LitInt:
            atom = MakeRef<ast::LiteralInteger>(m_Token->GetInt(), (ast::Type::VariableType)m_Token->GetValueType(), GetSpanFrom(start));
            Bump();
            break;
        case Token::LitFloat:
            atom = MakeRef<ast::LiteralFloat>(m_Token->GetFloat(), (ast::Type::VariableType)m_Token->GetValueType(), GetSpanFrom(start));
            Bump();
            break;
        case Token::LitString:
            atom = MakeRef<ast::LiteralString>(m_Token->GetName(), GetSpanFrom(start));
            Bump();
            break;
        default:
            if (allowEmpty) { return nullptr; }
            ThrowError("invalid expression", GetSpanFrom(start));
            break;
        }

        // Parse suffix operator
        if (IsSuffixOperator()) {
            OperatorInfo opInfo = GetSuffixOperatorInfo(m_Token->GetType());
            if (opInfo.Precedence >= prec) {
                Bump();

                return MakeRef<ast::SuffixOperator>((ast::SuffixOperator::OpType)opInfo.TokenType, atom, GetSpanFrom(start));
            }
        }

        return atom;
    }

    Ref<ast::Var> Parser::Var() {
        TextPosition start = m_Token->GetTextPos();

        Expect({ Token::Var });
        ast::Ident ident = Ident();
        Expect({ Token::Colon });
        Ref<ast::Type> type = Type();

        Ref<ast::Expr> lhs = MakeRef<ast::Variable>(ident, ident.GetSpan());
        Ref<ast::BinaryOperator> assign;
        if (*m_Token == Token::Assign) {
            TextPosition s = m_Token->GetTextPos();
            Expect({ Token::Assign });
            Ref<ast::Expr> rhs = Expr();
            assign = MakeRef<ast::BinaryOperator>(ast::BinaryOperator::Assign, lhs, rhs, GetSpanFrom(s));
        }
        else {
            Ref<ast::Expr> rhs = MakeRef<ast::LiteralFloat>(0.0, ast::Type::F64, ident.GetSpan());
            assign = MakeRef<ast::BinaryOperator>(ast::BinaryOperator::Assign, lhs, rhs, ident.GetSpan());
        }

        return MakeRef<ast::Var>(ident, type, assign, GetSpanFrom(start));
    }

    // prefix_op : + -
    //           | ! ~
    //           | ++ --
    bool Parser::IsPrefixOperator() const {
        return Match({
            Token::Plus, Token::Minus,
            Token::Not, Token::BitNot,
            Token::PlusPlus, Token::MinusMinus });
    }

    // suffix_op : ++ --
    bool Parser::IsSuffixOperator() const {
        return Match({
            Token::PlusPlus, Token::MinusMinus });
    }

    // binary_op : + - * / %
    //           | && || & | ^
    //           | == != > >= < <=
    //           | =
    bool Parser::IsBinaryOperator() const {
        return Match({
            Token::Plus, Token::Minus, Token::Star, Token::Slash, Token::Percent,
            Token::LogicAnd, Token::LogicOr, Token::BitAnd, Token::BitOr, Token::BitXOr,
            Token::Eq, Token::NotEq, Token::Greater, Token::GreaterEq, Token::Lesser, Token::LesserEq,
            Token::Assign });
    }

}