#include "scarpch.hpp"
#include "Parse/Parser.hpp"
#include "Parse/Lex/Lexer.hpp"

#define SPAN_ERROR(msg, span) SCAR_ERROR("{}: {}", span, msg)

namespace scar {

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    // MISCELANEOUS

    static std::ostream& operator<<(std::ostream& os, const std::vector<Token::TokenType>& types) {
        os << types[0];
        for (size_t i = 1; i < types.size(); i++) { os << "," << types[i]; }
        return os;
    }

    static std::vector<Token::TokenType> operator+(const std::vector<Token::TokenType>& first, const std::vector<Token::TokenType>& second) {
        std::vector<Token::TokenType> ret = first;
        ret.insert(ret.end(), second.begin(), second.end());
        return ret;
    }

    static std::vector<Token::TokenType> operator+(const std::vector<Token::TokenType>& vec, Token::TokenType type) {
        std::vector<Token::TokenType> ret = vec;
        ret.push_back(type);
        return ret;
    }

    static std::vector<Token::TokenType> s_DeclStartTokens = {
        Token::Func,
    };
    static std::vector<Token::TokenType> s_StmtStartTokens = {
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
        Token::TokenType TokenType = Token::Invalid;
        unsigned int Precedence = 0;
        enum Assoc { Left, Right } Associativity = Assoc::Left;

        OperatorInfo() = default;
        OperatorInfo(Token::TokenType tokenType, unsigned int prec, Assoc assoc) :
            TokenType(tokenType),
            Precedence(prec),
            Associativity(assoc) {
        }

        bool IsCast() { return TokenType == Token::As; }
    };

    static OperatorInfo GetPrefixOperatorInfo(Token::TokenType type) {
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
    static OperatorInfo GetSuffixOperatorInfo(Token::TokenType type) {
        switch (type) {
        case Token::PlusPlus:   return OperatorInfo(Token::PlusPlus,   13, OperatorInfo::Left);
        case Token::MinusMinus: return OperatorInfo(Token::MinusMinus, 13, OperatorInfo::Left);
        case Token::As:         return OperatorInfo(Token::As, 11, OperatorInfo::Left);
        default:
            SCAR_BUG("missing suffix OperatorInfo for Token::Type {}", (int)type);
            return OperatorInfo();
        }
    }
    static OperatorInfo GetBinaryOperatorInfo(Token::TokenType type) {
        switch (type) {
        case Token::Dot:       return OperatorInfo(type, 13, OperatorInfo::Left);
        case Token::Star:      return OperatorInfo(type, 10, OperatorInfo::Left);
        case Token::Slash:     return OperatorInfo(type, 10, OperatorInfo::Left);
        case Token::Percent:   return OperatorInfo(type, 10, OperatorInfo::Left);
        case Token::Plus:      return OperatorInfo(type, 9, OperatorInfo::Left);
        case Token::Minus:     return OperatorInfo(type, 9, OperatorInfo::Left);
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

    ast::TypeInfo ASTType(Token::TokenType tokenType) {
        return (ast::TypeInfo)tokenType;
    }

    ast::BinaryOperator::OpType ASTBinaryOp(Token::TokenType tokenType) {
        return (ast::BinaryOperator::OpType)tokenType;
    }

    ast::PrefixOperator::OpType ASTPrefixOp(Token::TokenType tokenType) {
        return (ast::PrefixOperator::OpType)tokenType;
    }

    ast::SuffixOperator::OpType ASTSuffixOp(Token::TokenType tokenType) {
        return (ast::SuffixOperator::OpType)tokenType;
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

    bool Parser::Match(const std::vector<Token::TokenType>& expected) const {
        for (auto type : expected) {
            if (*m_Token == type) {
                return true;
            }
        }
        return false;
    }

    Token& Parser::Expect(const std::vector<Token::TokenType>& expected) {
        if (!Match(expected)) {
            SPAN_ERROR(FMT("unexpected token: {} where {} was expected", *m_Token, expected), m_Token->Span);
        }
        Bump();
        return *(m_Token - 1);
    }

    void Parser::Synchronize(const std::vector<Token::TokenType>& delims) {
        SCAR_TRACE("synchronizing");
        while (!Match(delims)) {
            Bump();
        }
    }

    Ref<ast::Module> Parser::Parse() {
        TextPosition start;
        std::vector<Ref<ast::Stmt>> items;
        while (*m_Token != Token::EndOfFile) {
            if (auto item = Global()) {
                items.push_back(item);
            }
        }
        return MakeRef<ast::Module>(items, GetSpanFrom(start));
    }

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    // TYPE

    Token& Parser::ExpectTypeToken() {
        return Expect({ Token::Bool,
            Token::I8, Token::I16, Token::I32, Token::I64,
            Token::U8, Token::U16, Token::U32, Token::U64,
            Token::F32, Token::F64 });
    }

    // type : BOOL
    //      | I8 I16 I32 I64
    //      | U8 U16 U32 U64
    //      | F32 F64
    Ref<ast::Type> Parser::Type() {
        Token& token = ExpectTypeToken();
        return MakeRef<ast::Type>((ast::TypeInfo)token.Type, token.Span);
    }

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    // SUPPORT

    // ident : IDENT
    ast::Ident Parser::Ident() {
        Token& token = Expect({ Token::Ident });
        return ast::Ident(token.GetName(), token.Span);
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

    // global : function
    Ref<ast::Stmt> Parser::Global() {
        try {
            switch (m_Token->Type) {
            case Token::Func: return Function();
            default:
                SPAN_ERROR("expected a declaration", m_Token->Span);
                break;
            }
        }
        catch (CompilerError& e) {
            e.OnCatch();
            Synchronize({ s_DeclStartTokens });
            return nullptr;
        }

        return nullptr;
    }

    // function : prototype block
    //          | prototype ;
    Ref<ast::Stmt> Parser::Function() {
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
            retType = MakeRef<ast::Type>(ast::TypeInfo::Void, ident.GetSpan());
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
            switch (m_Token->Type) {
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

                SPAN_ERROR("expected a statement", GetSpanFrom(start));
                break;
            }
            }
        }
        catch (CompilerError& e) {
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
        Ref<ast::LiteralBool> cond = MakeRef<ast::LiteralBool>(true, m_Token->Span);
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
            OperatorInfo opInfo = GetBinaryOperatorInfo(m_Token->Type);
            if (opInfo.Precedence < prec) {
                break;
            }
            Bump();

            // Parse right side of expression
            Ref<ast::Expr> rhs = Expr(opInfo.Associativity == OperatorInfo::Left ?
                                      opInfo.Precedence + 1 :
                                      opInfo.Precedence);

            // Set LHS to complete expression
            lhs = MakeRef<ast::BinaryOperator>(ASTBinaryOp(opInfo.TokenType), lhs, rhs, GetSpanFrom(start));
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
            OperatorInfo opInfo = GetPrefixOperatorInfo(m_Token->Type);
            if (opInfo.Precedence >= prec) {
                Bump();

                // Parse rest of atom
                atom = Expr(opInfo.Associativity == OperatorInfo::Left ?
                            opInfo.Precedence + 1 :
                            opInfo.Precedence);

                return MakeRef<ast::PrefixOperator>(ASTPrefixOp(opInfo.TokenType), atom, GetSpanFrom(start));
            }
        }

        // Parse atom body
        switch (m_Token->Type) {
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
                atom = MakeRef<ast::VarAccess>(ident, ident.GetSpan());
            }
            break;
        }
        case Token::Var: return VarDecl();
        case Token::True:
            atom = MakeRef<ast::LiteralBool>(true, GetSpanFrom(start));
            Bump();
            break;
        case Token::False:
            atom = MakeRef<ast::LiteralBool>(false, GetSpanFrom(start));
            Bump();
            break;
        case Token::LitInt:
            atom = MakeRef<ast::LiteralInteger>(m_Token->GetInt(), ASTType(m_Token->LiteralType), GetSpanFrom(start));
            Bump();
            break;
        case Token::LitFloat:
            atom = MakeRef<ast::LiteralFloat>(m_Token->GetFloat(), ASTType(m_Token->LiteralType), GetSpanFrom(start));
            Bump();
            break;
        case Token::LitString:
            atom = MakeRef<ast::LiteralString>(m_Token->GetName(), GetSpanFrom(start));
            Bump();
            break;
        default:
            if (allowEmpty) { return nullptr; }
            SPAN_ERROR("invalid expression", GetSpanFrom(start));
            break;
        }

        // Parse suffix operator
        if (IsSuffixOperator()) {
            OperatorInfo opInfo = GetSuffixOperatorInfo(m_Token->Type);
            if (opInfo.Precedence >= prec) {
                Bump();
                // Suffix operators also include type casts (x as i32),
                // so we get the SuffixOp type and check if it's a cast.
                // If it is, we expect the next token to be a type.
                // We pass this type to the ast::SuffixOperator as the
                // expression's result type.
                ast::SuffixOperator::OpType suffixOp = ASTSuffixOp(opInfo.TokenType);
                if (suffixOp == ast::SuffixOperator::Cast) {
                    ast::TypeInfo targetType = ASTType(ExpectTypeToken().Type);
                    return MakeRef<ast::SuffixOperator>(suffixOp, atom, targetType, GetSpanFrom(start));
                }

                return MakeRef<ast::SuffixOperator>(suffixOp, atom, GetSpanFrom(start));
            }
        }

        return atom;
    }

    Ref<ast::VarDecl> Parser::VarDecl() {
        TextPosition start = m_Token->GetTextPos();

        Expect({ Token::Var });
        ast::Ident ident = Ident();
        Expect({ Token::Colon });
        Ref<ast::Type> type = Type();

        return MakeRef<ast::VarDecl>(ident, type, GetSpanFrom(start));
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
            Token::PlusPlus, Token::MinusMinus, Token::As });
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