#pragma once
#include "Parse/Token.hpp"

namespace scar {
    namespace ast {

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // VISITOR

        class Visitor {
        public:
            Visitor() = default;
            virtual ~Visitor() = default;

            virtual void Visit(class Type& node) = 0;

            virtual void Visit(class Module& node) = 0;
            virtual void Visit(class Function& node) = 0;
            virtual void Visit(class FunctionPrototype& node) = 0;
            virtual void Visit(class VarDecl& node) = 0;

            virtual void Visit(class Branch& node) = 0;
            virtual void Visit(class ForLoop& node) = 0;
            virtual void Visit(class WhileLoop& node) = 0;
            virtual void Visit(class Block& node) = 0;
            virtual void Visit(class Continue& node) = 0;
            virtual void Visit(class Break& node) = 0;
            virtual void Visit(class Return& node) = 0;

            virtual void Visit(class FunctionCall& node) = 0;
            virtual void Visit(class VarAccess& node) = 0;

            virtual void Visit(class PrefixOperator& node) = 0;
            virtual void Visit(class SuffixOperator& node) = 0;
            virtual void Visit(class BinaryOperator& node) = 0;

            virtual void Visit(class LiteralBool& node) = 0;
            virtual void Visit(class LiteralInteger& node) = 0;
            virtual void Visit(class LiteralFloat& node) = 0;
            virtual void Visit(class LiteralString& node) = 0;
        };

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // TYPE

        struct TypeInfo {
            enum Ty {
                Invalid = Token::Invalid,
                Void = Token::Void,

                Bool = Token::Bool,

                I8 = Token::I8,
                I16 = Token::I16,
                I32 = Token::I32,
                I64 = Token::I64,

                U8 = Token::U8,
                U16 = Token::U16,
                U32 = Token::U32,
                U64 = Token::U64,

                F32 = Token::F32,
                F64 = Token::F64,

                Char = Token::Char,
                String = Token::String
            } Type = Invalid;

            TypeInfo() = default;
            TypeInfo(TypeInfo::Ty type) : Type(type) {}
            explicit TypeInfo(Token::TokenType type) : Type((TypeInfo::Ty)type) {}

            operator TypeInfo::Ty() const { return Type; }
            
            bool IsValid() const    { return Type != Invalid; }
            bool IsVoid() const     { return Type == Void; }
            bool IsBool() const     { return Type == Bool; }
            bool IsInt() const      { return IsSInt() || IsUInt(); }
            bool IsSInt() const     { return Type == I8 || Type == I16 || Type == I32 || Type == I64; }
            bool IsUInt() const     { return Type == U8 || Type == U16 || Type == U32 || Type == U64; }
            bool IsFloat() const    { return Type == F32 || Type == F64; }
            bool IsChar() const     { return Type == Char; }
            bool IsString() const   { return Type == String; }
        };

        static std::ostream& operator<<(std::ostream& os, TypeInfo type) {
            if (!type.IsValid())
                return os << "<unknown>";
            return os << (Token::TokenType)type.Type;
        }
        static std::ostream& operator<<(std::ostream& os, TypeInfo::Ty type) {
            return os << TypeInfo(type);
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // NODES

        class Node {
        public:
            Node(const TextSpan& span) : m_Span(span) {}
            virtual ~Node() = default;
            virtual void Accept(Visitor& visitor) = 0;
            const TextSpan& GetSpan() const { return m_Span; }
        private:
            const TextSpan m_Span;
        };

#define SCAR_GENERATE_NODE public: void Accept(Visitor& visitor) override { visitor.Visit(*this); }

        class Stmt : public Node {
        public:
            Stmt(const TextSpan& span) : Node(span) {}
        };

        class Expr : public Stmt {
        public:
            TypeInfo ResultType;
            Expr(TypeInfo type, const TextSpan& span) : Stmt(span), ResultType(type) {}
        };

        class Type : public Stmt {
            SCAR_GENERATE_NODE;
        public:
            const TypeInfo ResultType;
            Type(TypeInfo type, const TextSpan& span) : Stmt(span), ResultType(type) {}
        };

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // SUPPORT

        struct Ident {
            const Interner::StringID StringID;
            Ident(Interner::StringID id, const TextSpan& span) :
                 StringID(id), m_Span(span) {}
            const std::string& GetString() const { return Interner::GetString(StringID); }
            const TextSpan& GetSpan() const { return m_Span; }
        private:
            const TextSpan m_Span;
        };

        static std::ostream& operator<<(std::ostream& os, const Ident& ident) {
            return os << ident.GetString();
        }

        struct Arg {
            const Ident Name;
            Ref<Type> VarType;
            Arg(Ident name, const Ref<Type>& type, const TextSpan& span) :
                Name(name), VarType(type), m_Span(span) {}
            const TextSpan& GetSpan() const { return m_Span; }
        private:
            const TextSpan m_Span;
        };

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // DECLARATIONS

        class Module : public Stmt {
            SCAR_GENERATE_NODE;
        public:
            const std::vector<Ref<Stmt>> Items;
            Module(const std::vector<Ref<Stmt>>& items, const TextSpan& span) :
                Stmt(span), Items(items) {}
        };

        class Function : public Stmt {
            SCAR_GENERATE_NODE;
        public:
            Ref<FunctionPrototype> Prototype;
            Ref<Block> CodeBlock;
            Function(const Ref<FunctionPrototype>& prototype, const Ref<Block>& block, const TextSpan& span) :
                Stmt(span), Prototype(prototype), CodeBlock(block) {}
        };

        class FunctionPrototype : public Stmt {
            SCAR_GENERATE_NODE;
        public:
            Ident Name;
            std::vector<Arg> Args;
            Ref<Type> ReturnType;
            FunctionPrototype(Ident name, const std::vector<Arg>& args, const Ref<Type>& retType, const TextSpan& span) :
                Stmt(span), Name(name), Args(args), ReturnType(retType) {}
        };

        class VarDecl : public Expr {
            SCAR_GENERATE_NODE;
        public:
            Ident Name;
            Ref<Type> VarType;
            VarDecl(Ident name, const Ref<Type>& type, const TextSpan& span) :
                Expr(type->ResultType, span), Name(name), VarType(type) {}
        };

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // STATEMENTS

        class Block : public Stmt {
            SCAR_GENERATE_NODE;
        public:
            const std::vector<Ref<Stmt>> Items;
            Block(const std::vector<Ref<Stmt>>& items, const TextSpan& span) :
                Stmt(span), Items(items) {}
        };

        class Branch : public Stmt {
            SCAR_GENERATE_NODE;
        public:
            Ref<Expr> Condition;
            Ref<Block> TrueBlock;
            Ref<Block> FalseBlock;
            Branch(const Ref<Expr>& cond, const Ref<Block>& trueBlock, const Ref<Block>& falseBlock, const TextSpan& span) :
                Stmt(span), Condition(cond), TrueBlock(trueBlock), FalseBlock(falseBlock) {}
        };

        class ForLoop : public Stmt {
            SCAR_GENERATE_NODE;
        public:
            Ref<Expr> Init;
            Ref<Expr> Condition;
            Ref<Expr> Update;
            Ref<Block> CodeBlock;
            ForLoop(const Ref<Expr>& init, const Ref<Expr>& cond, const Ref<Expr>& update, const Ref<Block>& block, const TextSpan& span) :
                Stmt(span), Init(init), Condition(cond), Update(update), CodeBlock(block) {}
        };

        class WhileLoop : public Stmt {
            SCAR_GENERATE_NODE;
        public:
            Ref<Expr> Condition;
            Ref<Block> CodeBlock;
            WhileLoop(const Ref<Expr>& cond, const Ref<Block>& block, const TextSpan& span) :
                Stmt(span), Condition(cond), CodeBlock(block) {}
        };

        class Continue : public Stmt {
            SCAR_GENERATE_NODE;
        public:
            Continue(const TextSpan& span) : Stmt(span) {}
        };

        class Break : public Stmt {
            SCAR_GENERATE_NODE;
        public:
            Break(const TextSpan& span) : Stmt(span) {}
        };

        class Return : public Stmt {
            SCAR_GENERATE_NODE;
        public:
            Ref<Expr> Value;
            Return(const Ref<Expr>& value, const TextSpan& span) :
                Stmt(span), Value(value) {}
        };

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // EXPRESSIONS

        class FunctionCall : public Expr {
            SCAR_GENERATE_NODE;
        public:
            Ident Name;
            std::vector<Ref<Expr>> Args;
            FunctionCall(const Ident& name, const std::vector<Ref<Expr>>& args, const TextSpan& span) :
                Expr(TypeInfo::Invalid, span), Name(name), Args(args) {}
        };

        class VarAccess : public Expr {
            SCAR_GENERATE_NODE;
        public:
            Ident Name;
            VarAccess(const Ident& name, const TextSpan& span) :
                Expr(TypeInfo::Invalid, span), Name(name) {}
        };

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // OPERATORS

        class PrefixOperator : public Expr {
            SCAR_GENERATE_NODE;
        public:
            enum OpType {
                Increment = Token::PlusPlus,
                Decrement = Token::MinusMinus,
                Plus   = Token::Plus,
                Minus  = Token::Minus,
                Not    = Token::Not,
                BitNot = Token::BitNot,
            };

            const OpType Type;
            Ref<Expr> RHS;
            PrefixOperator(PrefixOperator::OpType type, const Ref<Expr>& expr, const TextSpan& span) :
                Expr(TypeInfo::Invalid, span), Type(type), RHS(expr) {}
        };

        class SuffixOperator : public Expr {
            SCAR_GENERATE_NODE;
        public:
            enum OpType {
                Increment = Token::PlusPlus,
                Decrement = Token::MinusMinus,
                Cast      = Token::As,
            };

            const OpType Type;
            Ref<Expr> LHS;
            SuffixOperator(SuffixOperator::OpType type, const Ref<Expr>& expr, TypeInfo resultType, const TextSpan& span) :
                Expr(resultType, span), Type(type), LHS(expr) {}
            SuffixOperator(SuffixOperator::OpType type, const Ref<Expr>& expr, const TextSpan& span) :
                SuffixOperator(type, expr, TypeInfo::Invalid, span) {}
            
        };

        class BinaryOperator : public Expr {
            SCAR_GENERATE_NODE;
        public:
            enum OpType {
                MemberAccess = Token::Dot,
                Multiply  = Token::Star,
                Divide    = Token::Slash,
                Remainder = Token::Percent,
                Plus      = Token::Plus,
                Minus     = Token::Minus,
                Greater   = Token::Greater,
                GreaterEq = Token::GreaterEq,
                Lesser    = Token::Lesser,
                LesserEq  = Token::LesserEq,
                Eq        = Token::Eq,
                NotEq     = Token::NotEq,
                BitAnd    = Token::BitAnd,
                BitXOr    = Token::BitXOr,
                BitOr     = Token::BitOr,
                LogicAnd  = Token::LogicAnd,
                LogicOr   = Token::LogicOr,
                Assign    = Token::Assign,
            };

            const OpType Type;
            Ref<Expr> LHS;
            Ref<Expr> RHS;
            BinaryOperator(BinaryOperator::OpType type, const Ref<Expr>& lhs, const Ref<Expr>& rhs, const TextSpan& span) :
                Expr(TypeInfo::Invalid, span), Type(type), LHS(lhs), RHS(rhs) {}
        };

        static std::ostream& operator<<(std::ostream& os, PrefixOperator::OpType opType) {
            return os << (Token::TokenType)opType;
        }
        static std::ostream& operator<<(std::ostream& os, SuffixOperator::OpType opType) {
            return os << (Token::TokenType)opType;
        }
        static std::ostream& operator<<(std::ostream& os, BinaryOperator::OpType opType) {
            return os << (Token::TokenType)opType;
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // LITERALS

        class LiteralBool : public Expr {
            SCAR_GENERATE_NODE;
        public:
            const bool Value;
            LiteralBool(bool val, const TextSpan& span) :
                Expr(TypeInfo::Bool, span), Value(val) {
            }
        };

        class LiteralInteger : public Expr {
            SCAR_GENERATE_NODE;
        public:
            const uint64_t Value;
            LiteralInteger(uint64_t val, TypeInfo type, const TextSpan& span) :
                Expr(type, span), Value(val) {}
        };

        class LiteralFloat : public Expr {
            SCAR_GENERATE_NODE;
        public:
            const double Value;
            LiteralFloat(double val, TypeInfo type, const TextSpan& span) :
                Expr(type, span), Value(val) {}
        };

        class LiteralString : public Expr {
            SCAR_GENERATE_NODE;
        public:
            const Interner::StringID StringID;
            LiteralString(Interner::StringID id, const TextSpan& span) :
                Expr(TypeInfo::String, span), StringID(id) {}
        };

    }
}

template <>
struct fmt::formatter<scar::ast::TypeInfo> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const scar::ast::TypeInfo& type, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "{}", (scar::Token::TokenType)type.Type);
    }
};
