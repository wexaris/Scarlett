#include "scarpch.hpp"
#include "Parse/AST/VerifyVisitor.hpp"

#define SPAN_ERROR(msg, span) SCAR_ERROR("{}: {}", span, msg)

namespace scar {
    namespace ast {

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // MISCELANEOUS

        static bool IsUnknown(TypeInfo type) {
            return type == TypeInfo::Unknown;
        }

        static bool IsVoid(TypeInfo type) {
            return type == TypeInfo::Void;
        }

        static bool IsBool(TypeInfo type) {
            return type == TypeInfo::Bool;
        }

        static bool IsSInt(TypeInfo type) {
            return type == TypeInfo::I8 ||
                type == TypeInfo::I16 ||
                type == TypeInfo::I32 ||
                type == TypeInfo::I64;
        }

        static bool IsUInt(TypeInfo type) {
            return type == TypeInfo::U8 ||
                type == TypeInfo::U16 ||
                type == TypeInfo::U32 ||
                type == TypeInfo::U64;
        }

        static bool IsInt(TypeInfo type) {
            return IsSInt(type) || IsUInt(type);
        }

        static bool IsFloat(TypeInfo type) {
            return type == TypeInfo::F32 || type == TypeInfo::F64;
        }

        static bool IsChar(TypeInfo type) {
            return type == TypeInfo::Char;
        }

        static bool IsString(TypeInfo type) {
            return type == TypeInfo::String;
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // VISITOR

        class SymbolTable {
        public:
            SymbolTable() {
                PushScope();
            }

            void Add(const Ident& name, TypeInfo type) { Add(name.GetString(), type); }
            void Add(const std::string & name, TypeInfo type) {
                m_Symbols.back()[name] = type;
            }
            
            void PushScope() { m_Symbols.push_back({}); }
            void PopScope() { m_Symbols.pop_back(); }

            TypeInfo Find(const Ident& name) const { return Find(name.GetString()); }
            TypeInfo Find(const std::string & name) const {
                for (auto iter = m_Symbols.rbegin(); iter != m_Symbols.rend(); iter++) {
                    auto item = iter->find(name);
                    if (item != iter->end()) {
                        return item->second;
                    }
                }
                return TypeInfo::Unknown;
            }

        private:
            std::vector<std::unordered_map<std::string, TypeInfo>> m_Symbols;
        };

        struct TypeCheckVisitorData {
            SymbolTable Symbols;
            FunctionPrototype* CurrentFunction;
        };
        static TypeCheckVisitorData s_Data{};

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // TYPE

        void TypeCheckVisitor::Visit(Type& node) {

        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // DECLARATIONS

        void TypeCheckVisitor::Visit(Module& node) {
            for (auto& item : node.Items) {
                item->Accept(*this);
            }
        }

        void TypeCheckVisitor::Visit(Function& node) {
            s_Data.Symbols.PushScope();
            node.Prototype->Accept(*this);

            s_Data.CurrentFunction = node.Prototype.get();
            node.CodeBlock->Accept(*this);
            s_Data.CurrentFunction = nullptr;

            s_Data.Symbols.PopScope();
        }

        void TypeCheckVisitor::Visit(FunctionPrototype& node) {
            TypeInfo type = node.ReturnType->ResultType;
            s_Data.Symbols.Add(node.Name, type);

            for (auto& arg : node.Args) {
                s_Data.Symbols.Add(arg.Name, arg.VarType->ResultType);
            }
        }

        void TypeCheckVisitor::Visit(VarDecl& node) {
            node.VarType->Accept(*this);
            if (IsVoid(node.ResultType)) {
                SPAN_ERROR("invalid void type variable", node.VarType->GetSpan());
            }
            s_Data.Symbols.Add(node.Name, node.ResultType);
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // STATEMENTS

        void TypeCheckVisitor::Visit(Branch& node) {
            node.Condition->Accept(*this);
            node.TrueBlock->Accept(*this);
            node.FalseBlock->Accept(*this);
        }

        void TypeCheckVisitor::Visit(ForLoop& node) {
            node.Init->Accept(*this);
            node.Condition->Accept(*this);
            node.Update->Accept(*this);
            node.CodeBlock->Accept(*this);
        }

        void TypeCheckVisitor::Visit(WhileLoop& node) {
            node.Condition->Accept(*this);
            node.CodeBlock->Accept(*this);
        }

        void TypeCheckVisitor::Visit(Block& node) {
            s_Data.Symbols.PushScope();
            for (auto& item : node.Items) {
                item->Accept(*this);
            }
            s_Data.Symbols.PopScope();
        }

        void TypeCheckVisitor::Visit(Continue& node) {

        }

        void TypeCheckVisitor::Visit(Break& node) {

        }

        void TypeCheckVisitor::Visit(Return& node) {
            node.Value->Accept(*this);
            if (node.Value->ResultType != s_Data.CurrentFunction->ReturnType->ResultType) {
                SPAN_ERROR("return value does not match function type", node.GetSpan());
            }
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // EXPRESSIONS

        void TypeCheckVisitor::Visit(FunctionCall& node) {
            TypeInfo type = s_Data.Symbols.Find(node.Name);
            // TODO: Verify function argument count and types match called function

            for (auto& arg : node.Args) {
                arg->Accept(*this);
            }
            node.ResultType = type;
        }

        void TypeCheckVisitor::Visit(VarAccess& node) {
            TypeInfo type = s_Data.Symbols.Find(node.Name);
            if (type == TypeInfo::Unknown) {
                SPAN_ERROR(FMT("undeclared variable: {}", node.Name), node.GetSpan());
            }
            node.ResultType = type;
        }

        void TypeCheckVisitor::Visit(Cast& node) {

        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // OPERATORS

        void TypeCheckVisitor::Visit(PrefixOperator& node) {
            node.RHS->Accept(*this);
            TypeInfo rhsType = node.RHS->ResultType;
            
            if (IsUnknown(rhsType)) {
                return;
            }
            if (IsVoid(rhsType)) {
                SPAN_ERROR("invalid expression type", node.GetSpan());
                return;
            }

            switch (node.Type) {
            case PrefixOperator::Plus:  [[fallthrough]];
            case PrefixOperator::Minus:
                if (IsInt(rhsType) || IsFloat(rhsType)) { node.ResultType = rhsType; }
                else {
                    SPAN_ERROR(FMT("prefix operator {} not defined for {} type", node.Type, node.RHS->ResultType), node.GetSpan());
                }
                break;
            case PrefixOperator::Not:
                if (IsBool(rhsType)) { node.ResultType = rhsType; }
                else {
                    SPAN_ERROR(FMT("prefix operator {} not defined for {} type", node.Type, node.RHS->ResultType), node.GetSpan());
                }
                break;
            case PrefixOperator::BitNot:
                if (IsBool(rhsType) || IsInt(rhsType) || IsFloat(rhsType)) { node.ResultType = rhsType; }
                else {
                    SPAN_ERROR(FMT("prefix operator {} not defined for {} type", node.Type, node.RHS->ResultType), node.GetSpan());
                }
                break;
            case PrefixOperator::Increment:
                SCAR_BUG("PrefixOperator::Increment not implemented");
                break;
            case PrefixOperator::Decrement:
                SCAR_BUG("PrefixOperator::Decrement not implemented");
                break;

            default:
                SCAR_BUG("missing Type::VariableType for PrefixOperator {}", node.Type);
                break;
            }
        }

        void TypeCheckVisitor::Visit(SuffixOperator& node) {
            node.LHS->Accept(*this);
            TypeInfo lhsType = node.LHS->ResultType;

            if (IsUnknown(lhsType)) {
                return;
            }
            if (IsVoid(lhsType)) {
                SPAN_ERROR("invalid expression type", node.GetSpan());
                return;
            }

            switch (node.Type) {
            case SuffixOperator::Increment:
                SCAR_BUG("SuffixOperator::Increment not implemented");
                break;
            case SuffixOperator::Decrement:
                SCAR_BUG("SuffixOperator::Decrement not implemented");
                break;

            default:
                SCAR_BUG("missing Type::VariableType for SuffixOperator {}", node.Type);
                break;
            }
        }

        void TypeCheckVisitor::Visit(BinaryOperator& node) {
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

            if (IsUnknown(lhsType) || IsUnknown(rhsType)) {
                return;
            }
            if (IsVoid(lhsType) || IsVoid(rhsType)) {
                SPAN_ERROR("invalid expression type", node.GetSpan());
                return;
            }

            switch (node.Type) {
            case BinaryOperator::MemberAccess:
                SCAR_BUG("BinaryOperator::MemberAccess not implemented");
                break;
            case BinaryOperator::Multiply:  [[fallthrough]];
            case BinaryOperator::Divide:    [[fallthrough]];
            case BinaryOperator::Remainder: [[fallthrough]];
            case BinaryOperator::Plus:      [[fallthrough]];
            case BinaryOperator::Minus:
                if (IsInt(lhsType) || IsFloat(lhsType)) { node.ResultType = lhsType; }
                else {
                    SCAR_BUG("binary operator {} not defined for {} type", node.Type, node.RHS->ResultType);
                }
                break;
            case BinaryOperator::Greater:   [[fallthrough]];
            case BinaryOperator::GreaterEq: [[fallthrough]];
            case BinaryOperator::Lesser:    [[fallthrough]];
            case BinaryOperator::LesserEq:
                if (IsBool(lhsType)) { node.ResultType = lhsType; }
                else {
                    SCAR_BUG("binary operator {} not defined for {} type", node.Type, node.RHS->ResultType);
                }
                break;
            case BinaryOperator::Eq:        [[fallthrough]];
            case BinaryOperator::NotEq:
                if (IsBool(rhsType) || IsInt(rhsType) || IsFloat(rhsType)) { node.ResultType = lhsType; }
                else {
                    SCAR_BUG("binary operator {} not defined for {} type", node.Type, node.RHS->ResultType);
                }
                break;
            case BinaryOperator::BitAnd:    [[fallthrough]];
            case BinaryOperator::BitXOr:    [[fallthrough]];
            case BinaryOperator::BitOr:
                if (IsBool(rhsType) || IsInt(rhsType) || IsFloat(rhsType)) { node.ResultType = lhsType; }
                else {
                    SCAR_BUG("binary operator {} not defined for {} type", node.Type, node.RHS->ResultType);
                }
                break;
            case BinaryOperator::LogicAnd:  [[fallthrough]];
            case BinaryOperator::LogicOr:
                if (IsBool(lhsType)) { node.ResultType = lhsType; }
                else {
                    SCAR_BUG("binary operator {} not defined for {} type", node.Type, node.RHS->ResultType);
                }
                break;

            default:
                SCAR_BUG("missing Type::VariableType for BinaryOperator {}", node.Type);
                break;
            }
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // LITERALS

        void TypeCheckVisitor::Visit(LiteralBool& node) {

        }

        void TypeCheckVisitor::Visit(LiteralInteger& node) {

        }

        void TypeCheckVisitor::Visit(LiteralFloat& node) {

        }

        void TypeCheckVisitor::Visit(LiteralString& node) {

        }

    }
}