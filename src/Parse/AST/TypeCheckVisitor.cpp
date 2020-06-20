#include "scarpch.hpp"
#include "Parse/AST/TypeCheckVisitor.hpp"

#define SPAN_ERROR(msg, span) SCAR_ERROR("{}: {}", span, msg)

namespace scar {
    namespace ast {

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // MISCELANEOUS

        static bool IsUnknown(Type::ValueType type) {
            return type == Type::Unknown;
        }

        static bool IsVoid(Type::ValueType type) {
            return type == Type::Void;
        }

        static bool IsBool(Type::ValueType type) {
            return type == Type::Bool;
        }

        static bool IsSInt(Type::ValueType type) {
            return type == Type::I8 || type == Type::I16 || type == Type::I32 || type == Type::I64;
        }

        static bool IsUInt(Type::ValueType type) {
            return type == Type::U8 || type == Type::U16 || type == Type::U32 || type == Type::U64;
        }

        static bool IsInt(Type::ValueType type) {
            return IsSInt(type) || IsUInt(type);
        }

        static bool IsFloat(Type::ValueType type) {
            return type == Type::F32 || type == Type::F64;
        }

        static bool IsChar(Type::ValueType type) {
            return type == Type::Char;
        }

        static bool IsString(Type::ValueType type) {
            return type == Type::String;
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // VISITOR

        class SymbolTable {
        public:
            SymbolTable() {
                PushScope();
            }

            void Add(const Ident& name, Type::ValueType type) { Add(name.GetString(), type); }
            void Add(const std::string & name, Type::ValueType type) {
                m_Symbols.back()[name] = type;
            }
            
            void PushScope() { m_Symbols.push_back({}); }
            void PopScope() { m_Symbols.pop_back(); }

            Type::ValueType Find(const Ident& name) const { return Find(name.GetString()); }
            Type::ValueType Find(const std::string & name) const {
                for (auto iter = m_Symbols.rbegin(); iter != m_Symbols.rend(); iter++) {
                    auto item = iter->find(name);
                    if (item != iter->end()) {
                        return item->second;
                    }
                }
                return Type::Unknown;
            }

        private:
            std::vector<std::unordered_map<std::string, Type::ValueType>> m_Symbols;
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
            Type::ValueType type = node.ReturnType->ValType;
            s_Data.Symbols.Add(node.Name, type);

            for (auto& arg : node.Args) {
                s_Data.Symbols.Add(arg.Name, arg.VarType->ValType);
            }
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
            if (node.Value->ValueType != s_Data.CurrentFunction->ReturnType->ValType) {
                SPAN_ERROR("return value does not match function type", node.GetSpan());
            }
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // EXPRESSIONS

        void TypeCheckVisitor::Visit(FunctionCall& node) {
            for (auto& arg : node.Args) {
                arg->Accept(*this);
            }
            // TODO: match arguments
            Type::ValueType type = s_Data.Symbols.Find(node.Name);
            node.ValueType = type;
        }

        void TypeCheckVisitor::Visit(Var& node) {
            node.VarType->Accept(*this);
            if (IsVoid(node.ValueType)) {
                SPAN_ERROR("invalid void type variable", node.VarType->GetSpan());
            }
            s_Data.Symbols.Add(node.Name, node.ValueType);
            node.Assign->Accept(*this);
        }

        void TypeCheckVisitor::Visit(Variable& node) {
            Type::ValueType type = s_Data.Symbols.Find(node.Name);
            node.ValueType = type;
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // OPERATORS

        void TypeCheckVisitor::Visit(PrefixOperator& node) {
            node.RHS->Accept(*this);
            Type::ValueType rhsType = node.RHS->ValueType;
            
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
                if (IsInt(rhsType) || IsFloat(rhsType)) { node.ValueType = rhsType; }
                else {
                    SPAN_ERROR(FMT("prefix operator {} not defined for {} type", node.Type, node.RHS->ValueType), node.GetSpan());
                }
                break;
            case PrefixOperator::Not:
                if (IsBool(rhsType)) { node.ValueType = rhsType; }
                else {
                    SPAN_ERROR(FMT("prefix operator {} not defined for {} type", node.Type, node.RHS->ValueType), node.GetSpan());
                }
                break;
            case PrefixOperator::BitNot:
                if (IsBool(rhsType) || IsInt(rhsType) || IsFloat(rhsType)) { node.ValueType = rhsType; }
                else {
                    SPAN_ERROR(FMT("prefix operator {} not defined for {} type", node.Type, node.RHS->ValueType), node.GetSpan());
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
            Type::ValueType lhsType = node.LHS->ValueType;

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
                node.ValueType = node.LHS->ValueType;

                // Make sure LHS is a variable
                ast::Variable* lhsNode = dynamic_cast<ast::Variable*>(node.LHS.get());
                if (!lhsNode) {
                    SPAN_ERROR(FMT("binary operator {} requires a variable", node.Type), node.LHS->GetSpan());
                    return;
                }
                // Visit RHS
                node.RHS->Accept(*this);
                
                // Make sure LHS and RHS types are the same
                if (node.LHS->ValueType != node.RHS->ValueType) {
                    SPAN_ERROR(FMT("type mismatch: {} and {}", node.LHS->ValueType, node.RHS->ValueType), node.GetSpan());
                }

                return;
            }

            node.LHS->Accept(*this);
            node.RHS->Accept(*this);
            Type::ValueType lhsType = node.LHS->ValueType;
            Type::ValueType rhsType = node.RHS->ValueType;

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
                if (IsInt(lhsType) || IsFloat(lhsType)) { node.ValueType = lhsType; }
                else {
                    SCAR_BUG("binary operator {} not defined for {} type", node.Type, node.RHS->ValueType);
                }
                break;
            case BinaryOperator::Greater:   [[fallthrough]];
            case BinaryOperator::GreaterEq: [[fallthrough]];
            case BinaryOperator::Lesser:    [[fallthrough]];
            case BinaryOperator::LesserEq:
                if (IsBool(lhsType)) { node.ValueType = lhsType; }
                else {
                    SCAR_BUG("binary operator {} not defined for {} type", node.Type, node.RHS->ValueType);
                }
                break;
            case BinaryOperator::Eq:        [[fallthrough]];
            case BinaryOperator::NotEq:
                if (IsBool(rhsType) || IsInt(rhsType) || IsFloat(rhsType)) { node.ValueType = lhsType; }
                else {
                    SCAR_BUG("binary operator {} not defined for {} type", node.Type, node.RHS->ValueType);
                }
                break;
            case BinaryOperator::BitAnd:    [[fallthrough]];
            case BinaryOperator::BitXOr:    [[fallthrough]];
            case BinaryOperator::BitOr:
                if (IsBool(rhsType) || IsInt(rhsType) || IsFloat(rhsType)) { node.ValueType = lhsType; }
                else {
                    SCAR_BUG("binary operator {} not defined for {} type", node.Type, node.RHS->ValueType);
                }
                break;
            case BinaryOperator::LogicAnd:  [[fallthrough]];
            case BinaryOperator::LogicOr:
                if (IsBool(lhsType)) { node.ValueType = lhsType; }
                else {
                    SCAR_BUG("binary operator {} not defined for {} type", node.Type, node.RHS->ValueType);
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