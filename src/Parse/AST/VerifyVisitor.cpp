#include "scarpch.hpp"
#include "Parse/AST/VerifyVisitor.hpp"
#include "Parse/AST/SymbolTable.hpp"

#define SPAN_ERROR(msg, span) SCAR_ERROR("{}: {}", span, msg)

namespace scar {
    namespace ast {

        std::string_view BaseTypeName(TypeInfo x) {
            if (x.IsBool()) return "boolean";
            if (x.IsInt()) return "integer";
            if (x.IsFloat()) return "float";
            if (x.IsChar()) return "char";
            if (x.IsString()) return "string";
            if (x.IsVoid()) return "void";
            SCAR_BUG("missing base type name for {}", x);
            return AsString((Token::TokenType)x.Type);
        }

        bool IsSameBaseType(TypeInfo x, TypeInfo y) {
            return
                (x.IsBool()   && y.IsBool())   ||
                (x.IsInt()    && y.IsInt())    ||
                (x.IsFloat()  && y.IsFloat())  ||
                (x.IsChar()   && y.IsChar())   ||
                (x.IsString() && y.IsString()) ||
                (x.IsVoid()   && y.IsVoid());
        }

        TypeInfo LargestType(TypeInfo x, TypeInfo y) {
            if (!IsSameBaseType(x, y))
                return TypeInfo::Invalid;
            return (int)x >= (int)y ? x : y;
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // VISITOR

        class VerifyVisitorSymbolTable : public SymbolTable<std::string, TypeInfo> {
        public:
            VerifyVisitorSymbolTable() = default;
            ~VerifyVisitorSymbolTable() = default;

            void Add(const Ident& key, TypeInfo value) { Add(key.GetString(), value); }
            void Add(const std::string& key, TypeInfo value) {
                m_Symbols.back()[key] = value;
            }

            TypeInfo Find(const Ident& key) const { return Find(key.GetString()); }
            TypeInfo Find(const std::string& key) const {
                auto ret = TryFind(key);
                if (!ret) return TypeInfo::Invalid;
                return *ret;
            }
        };

        struct VerifyVisitorData {
            VerifyVisitorSymbolTable Symbols;
            FunctionPrototype* CurrentFunction;
        };
        static VerifyVisitorData s_Data;

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // TYPE

        void VerifyVisitor::Visit(Type& node) {

        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // DECLARATIONS

        void VerifyVisitor::Visit(Module& node) {
            for (auto& item : node.Items) {
                item->Accept(*this);
            }
        }

        void VerifyVisitor::Visit(Function& node) {
            s_Data.Symbols.PushScope();
            node.Prototype->Accept(*this);

            s_Data.CurrentFunction = node.Prototype.get();
            node.CodeBlock->Accept(*this);
            s_Data.CurrentFunction = nullptr;

            s_Data.Symbols.PopScope();
        }

        void VerifyVisitor::Visit(FunctionPrototype& node) {
            TypeInfo type = node.ReturnType->ResultType;
            s_Data.Symbols.Add(node.Name, type);

            for (auto& arg : node.Args) {
                s_Data.Symbols.Add(arg.Name, arg.VarType->ResultType);
            }
        }

        void VerifyVisitor::Visit(VarDecl& node) {
            node.VarType->Accept(*this);
            if (node.ResultType.IsVoid())
                SPAN_ERROR("invalid void type variable", node.VarType->GetSpan());
            s_Data.Symbols.Add(node.Name, node.ResultType);
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // STATEMENTS

        void VerifyVisitor::Visit(Branch& node) {
            node.Condition->Accept(*this);
            node.TrueBlock->Accept(*this);
            node.FalseBlock->Accept(*this);

            if (!node.Condition->ResultType.IsBool())
                SPAN_ERROR(FMT("expectead a boolean, found {}", node.Condition->ResultType), node.Condition->GetSpan());
        }

        void VerifyVisitor::Visit(ForLoop& node) {
            node.Init->Accept(*this);
            node.Condition->Accept(*this);
            node.Update->Accept(*this);
            node.CodeBlock->Accept(*this);

            if (!node.Condition->ResultType.IsBool())
                SPAN_ERROR(FMT("expectead a boolean, found {}", node.Condition->ResultType), node.Condition->GetSpan());
        }

        void VerifyVisitor::Visit(WhileLoop& node) {
            node.Condition->Accept(*this);
            node.CodeBlock->Accept(*this);

            if (!node.Condition->ResultType.IsBool())
                SPAN_ERROR(FMT("expectead a boolean, found {}", node.Condition->ResultType), node.Condition->GetSpan());
        }

        void VerifyVisitor::Visit(Block& node) {
            s_Data.Symbols.PushScope();
            for (auto& item : node.Items) {
                item->Accept(*this);
            }
            s_Data.Symbols.PopScope();
        }

        void VerifyVisitor::Visit(Continue& node) {

        }

        void VerifyVisitor::Visit(Break& node) {

        }

        void VerifyVisitor::Visit(Return& node) {
            node.Value->Accept(*this);
            if (node.Value->ResultType != s_Data.CurrentFunction->ReturnType->ResultType) {
                SPAN_ERROR("return value does not match function type", node.GetSpan());
            }
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // EXPRESSIONS

        void VerifyVisitor::Visit(FunctionCall& node) {
            TypeInfo type = s_Data.Symbols.Find(node.Name);
            // TODO: Verify function argument count and types match called function

            for (auto& arg : node.Args) {
                arg->Accept(*this);
            }
            node.ResultType = type;
        }

        void VerifyVisitor::Visit(VarAccess& node) {
            TypeInfo type = s_Data.Symbols.Find(node.Name);
            if (!type.IsValid()) {
                SPAN_ERROR(FMT("undeclared variable: {}", node.Name), node.GetSpan());
            }
            node.ResultType = type;
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // OPERATORS

        void VerifyVisitor::Visit(PrefixOperator& node) {
            node.RHS->Accept(*this);
            TypeInfo rhsType = node.RHS->ResultType;

            if (!rhsType.IsValid()) {
                return;
            }
            if (rhsType.IsVoid()) {
                SPAN_ERROR("invalid expression type", node.GetSpan());
                return;
            }

            switch (node.Type) {
            case PrefixOperator::Plus: [[fallthrough]];
            case PrefixOperator::Minus:
                if (!rhsType.IsInt() && !rhsType.IsFloat())
                    SPAN_ERROR(FMT("expected an integer or float, found {}", rhsType), node.GetSpan());
                node.ResultType = rhsType;
                break;
            case PrefixOperator::Not:
                if (!rhsType.IsBool())
                    SPAN_ERROR(FMT("expected a bool, found {}", rhsType), node.RHS->GetSpan());
                node.ResultType = TypeInfo::Bool;
                break;
            case PrefixOperator::BitNot:
                if (!rhsType.IsInt())
                    SPAN_ERROR(FMT("expected an integer, found {}", rhsType), node.GetSpan());
                node.ResultType = rhsType;
                break;
            case PrefixOperator::Increment:
                SCAR_UNIMPL("PrefixOperator::Increment");
                break;
            case PrefixOperator::Decrement:
                SCAR_UNIMPL("PrefixOperator::Decrement");
                break;

            default:
                SCAR_BUG("missing verification for PrefixOperator {} node", node.Type);
                break;
            }
        }

        void VerifyVisitor::Visit(SuffixOperator& node) {
            node.LHS->Accept(*this);
            TypeInfo lhsType = node.LHS->ResultType;

            if (!lhsType.IsValid()) {
                return;
            }
            if (lhsType.IsVoid()) {
                SPAN_ERROR("invalid expression type", node.GetSpan());
                return;
            }

            switch (node.Type) {
            case SuffixOperator::Increment:
                SCAR_UNIMPL("SuffixOperator::Increment");
                break;
            case SuffixOperator::Decrement:
                SCAR_UNIMPL("SuffixOperator::Decrement");
                break;
            case SuffixOperator::Cast:
                // TODO: check cast validity. Largely already done in LLVMVisitor.
                // node.ResultType is already set to target type
                break;

            default:
                SCAR_BUG("missing verification for SuffixOperator {} node", node.Type);
                break;
            }
        }

        void VerifyVisitor::Visit(BinaryOperator& node) {
            if (node.Type == BinaryOperator::Assign) {
                node.LHS->Accept(*this);
                node.ResultType = node.LHS->ResultType;

                // Make sure LHS is a variable
                if (dynamic_cast<ast::VarAccess*>(node.LHS.get()) || dynamic_cast<ast::VarDecl*>(node.LHS.get())) {
                    // Visit RHS
                    node.RHS->Accept(*this);

                    // Make sure LHS and RHS types are the same
                    if (node.LHS->ResultType != node.RHS->ResultType) {
                        SPAN_ERROR(FMT("type mismatch: {} and {}", node.LHS->ResultType, node.RHS->ResultType), node.GetSpan());
                    }
                }
                else {
                    SPAN_ERROR(FMT("binary operator {} requires a variable", node.Type), node.LHS->GetSpan());
                }

                return;
            }

            node.LHS->Accept(*this);
            node.RHS->Accept(*this);
            TypeInfo lhsType = node.LHS->ResultType;
            TypeInfo rhsType = node.RHS->ResultType;

            if (!lhsType.IsValid() || !rhsType.IsValid())
                return;
            if (lhsType.IsVoid() || rhsType.IsVoid()) {
                SPAN_ERROR("invalid expression type", node.GetSpan());
                return;
            }

            switch (node.Type) {
            case BinaryOperator::MemberAccess:
                SCAR_UNIMPL("BinaryOperator::MemberAccess");
                break;
            case BinaryOperator::Multiply:  [[fallthrough]];
            case BinaryOperator::Divide:    [[fallthrough]];
            case BinaryOperator::Remainder: [[fallthrough]];
            case BinaryOperator::Plus:      [[fallthrough]];
            case BinaryOperator::Minus:
                if (lhsType != rhsType)
                    SPAN_ERROR(FMT("type mismatch: {} and {}", lhsType, rhsType), node.LHS->GetSpan());
                node.ResultType = lhsType;
                break;
            case BinaryOperator::Greater:   [[fallthrough]];
            case BinaryOperator::GreaterEq: [[fallthrough]];
            case BinaryOperator::Lesser:    [[fallthrough]];
            case BinaryOperator::LesserEq:
                if (lhsType != rhsType)
                    SPAN_ERROR(FMT("type mismatch: {} and {}", lhsType, rhsType), node.LHS->GetSpan());
                node.ResultType = TypeInfo::Bool;
                break;
            case BinaryOperator::Eq:        [[fallthrough]];
            case BinaryOperator::NotEq:
                if (lhsType != rhsType)
                    SPAN_ERROR(FMT("type mismatch: {} and {}", lhsType, rhsType), node.LHS->GetSpan());
                node.ResultType = TypeInfo::Bool;
                break;
            case BinaryOperator::BitAnd:    [[fallthrough]];
            case BinaryOperator::BitXOr:    [[fallthrough]];
            case BinaryOperator::BitOr:
                if (!lhsType.IsInt())
                    SPAN_ERROR(FMT("expected an integer, found {}", lhsType), node.LHS->GetSpan());
                if (!rhsType.IsInt())
                    SPAN_ERROR(FMT("expected an integer, found {}", rhsType), node.RHS->GetSpan());
                node.ResultType = LargestType(lhsType, rhsType);
                break;
            case BinaryOperator::LogicAnd:  [[fallthrough]];
            case BinaryOperator::LogicOr:
                if (!lhsType.IsBool())
                    SPAN_ERROR(FMT("expected a bool, found {}", lhsType), node.LHS->GetSpan());
                if (!rhsType.IsBool())
                    SPAN_ERROR(FMT("expected a bool, found {}", rhsType), node.RHS->GetSpan());
                node.ResultType = TypeInfo::Bool;
                break;

            default:
                SCAR_BUG("missing verification for BinaryOperator {} node", node.Type);
                break;
            }
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // LITERALS

        void VerifyVisitor::Visit(LiteralBool& node) {

        }

        void VerifyVisitor::Visit(LiteralInteger& node) {

        }

        void VerifyVisitor::Visit(LiteralFloat& node) {

        }

        void VerifyVisitor::Visit(LiteralString& node) {

        }

    }
}