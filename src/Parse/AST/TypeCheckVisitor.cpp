#pragma once
#include "Parse/AST/TypeCheckVisitor.hpp"

namespace scar {
    namespace ast {

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // MISCELANEOUS

        static bool IsUnknown(Type::VariableType type) {
            return type == Type::Unknown;
        }

        static bool IsVoid(Type::VariableType type) {
            return type == Type::Void;
        }

        static bool IsBool(Type::VariableType type) {
            return type == Type::Bool;
        }

        static bool IsSInt(Type::VariableType type) {
            return type == Type::I8 || type == Type::I16 || type == Type::I32 || type == Type::I64;
        }

        static bool IsUInt(Type::VariableType type) {
            return type == Type::U8 || type == Type::U16 || type == Type::U32 || type == Type::U64;
        }

        static bool IsInt(Type::VariableType type) {
            return IsSInt(type) || IsUInt(type);
        }

        static bool IsFloat(Type::VariableType type) {
            return type == Type::F32 || type == Type::F64;
        }

        static bool IsChar(Type::VariableType type) {
            return type == Type::Char;
        }

        static bool IsString(Type::VariableType type) {
            return type == Type::String;
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // VISITOR

        class SymbolTable {
        public:
            void Add(std::string_view name, Type::VariableType type) { m_Symbols.back()[name] = type; }
            void PushScope() { m_Symbols.push_back({}); }
            void PopScope() { m_Symbols.pop_back(); }

            Type::VariableType Find(std::string_view name) const {
                for (auto iter = m_Symbols.rbegin(); iter != m_Symbols.rend(); iter++) {
                    auto item = iter->find(name);
                    if (item != iter->end()) {
                        return item->second;
                    }
                }
                return Type::Unknown;
            }

        private:
            std::vector<std::unordered_map<std::string_view, Type::VariableType>> m_Symbols = { {} };
        };

        struct VisitorData {
            SymbolTable Symbols;
            FunctionPrototype* CurrentFunction;
        };
        static VisitorData s_Data;

        void TypeCheckVisitor::Error(std::string_view msg, const Span& span) {
            SCAR_ERROR("{}: {}", span, msg);
            m_ErrorCount++;
        }

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
            Type::VariableType type = node.ReturnType->VarType;
            s_Data.Symbols.Add(node.Name.GetString(), type);

            for (auto& arg : node.Args) {
                s_Data.Symbols.Add(arg.Name.GetString(), arg.VarType->VarType);
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
            if (node.Value->GetValueType() != s_Data.CurrentFunction->ReturnType->VarType) {
                Error("return value does not match function type", node.GetSpan());
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
            Type::VariableType type = s_Data.Symbols.Find(node.Name.GetString());
            node.SetValueType(type);
        }

        void TypeCheckVisitor::Visit(Var& node) {
            node.VarType->Accept(*this);
            if (IsVoid(node.GetValueType())) {
                Error("invalid void type variable", node.VarType->GetSpan());
            }
            s_Data.Symbols.Add(node.Name.GetString(), node.GetValueType());
            node.Assign->Accept(*this);
        }

        void TypeCheckVisitor::Visit(Variable& node) {
            Type::VariableType type = s_Data.Symbols.Find(node.Name.GetString());
            node.SetValueType(type);
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // OPERATORS

        void TypeCheckVisitor::Visit(PrefixOperator& node) {
            node.RHS->Accept(*this);
            Type::VariableType rhsType = node.RHS->GetValueType();
            
            if (IsUnknown(rhsType)) {
                return;
            }
            if (IsVoid(rhsType)) {
                Error("invalid expression type", node.GetSpan());
                return;
            }

            switch (node.Type) {
            case PrefixOperator::Plus:  [[fallthrough]];
            case PrefixOperator::Minus:
                if (IsInt(rhsType) || IsFloat(rhsType)) { node.SetValueType(rhsType); }
                else {
                    Error(FMT("prefix operator {} not defined for {} type", node.Type, node.RHS->GetValueType()), node.GetSpan());
                }
                break;
            case PrefixOperator::Not:
                if (IsBool(rhsType)) { node.SetValueType(rhsType); }
                else {
                    Error(FMT("prefix operator {} not defined for {} type", node.Type, node.RHS->GetValueType()), node.GetSpan());
                }
                break;
            case PrefixOperator::BitNot:
                if (IsBool(rhsType) || IsInt(rhsType) || IsFloat(rhsType)) { node.SetValueType(rhsType); }
                else {
                    Error(FMT("prefix operator {} not defined for {} type", node.Type, node.RHS->GetValueType()), node.GetSpan());
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
            Type::VariableType lhsType = node.LHS->GetValueType();

            if (IsUnknown(lhsType)) {
                return;
            }
            if (IsVoid(lhsType)) {
                Error("invalid expression type", node.GetSpan());
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
                ast::Variable* lhsNode = dynamic_cast<ast::Variable*>(node.LHS.get());
                if (lhsNode) {
                    node.SetValueType(node.LHS->GetValueType());
                    node.RHS->Accept(*this);
                }
                else {
                    Error(FMT("binary operator {} requires a variable", node.Type), node.LHS->GetSpan());
                }
                return;
            }

            node.LHS->Accept(*this);
            node.RHS->Accept(*this);
            Type::VariableType lhsType = node.LHS->GetValueType();
            Type::VariableType rhsType = node.RHS->GetValueType();

            if (IsUnknown(lhsType) || IsUnknown(rhsType)) {
                return;
            }
            if (IsVoid(lhsType) || IsVoid(rhsType)) {
                Error("invalid expression type", node.GetSpan());
                return;
            }

            if (lhsType != rhsType) {
                SCAR_BUG("type casting not supported");
                return;
            }

            switch (node.Type) {
            case BinaryOperator::MemberAccess:
                SCAR_BUG("BinaryOperator::MemberAccess not implemented");
                break;
            case BinaryOperator::Multiply:  [[fallthrough]];
            case BinaryOperator::Divide:    [[fallthrough]];
            case BinaryOperator::Reminder:  [[fallthrough]];
            case BinaryOperator::Plus:      [[fallthrough]];
            case BinaryOperator::Minus:
                if (IsInt(lhsType) || IsFloat(lhsType)) { node.SetValueType(lhsType); }
                else {
                    Error(FMT("binary operator {} not defined for {} type", node.Type, node.RHS->GetValueType()), node.GetSpan());
                }
                break;
            case BinaryOperator::Greater:   [[fallthrough]];
            case BinaryOperator::GreaterEq: [[fallthrough]];
            case BinaryOperator::Lesser:    [[fallthrough]];
            case BinaryOperator::LesserEq:
                if (IsBool(lhsType)) { node.SetValueType(lhsType); }
                else {
                    Error(FMT("binary operator {} not defined for {} type", node.Type, node.RHS->GetValueType()), node.GetSpan());
                }
                break;
            case BinaryOperator::Eq:        [[fallthrough]];
            case BinaryOperator::NotEq:
                if (IsBool(rhsType) || IsInt(rhsType) || IsFloat(rhsType)) { node.SetValueType(lhsType); }
                else {
                    Error(FMT("binary operator {} not defined for {} type", node.Type, node.RHS->GetValueType()), node.GetSpan());
                }
                break;
            case BinaryOperator::BitAnd:    [[fallthrough]];
            case BinaryOperator::BitXOr:    [[fallthrough]];
            case BinaryOperator::BitOr:
                if (IsBool(rhsType) || IsInt(rhsType) || IsFloat(rhsType)) { node.SetValueType(lhsType); }
                else {
                    Error(FMT("binary operator {} not defined for {} type", node.Type, node.RHS->GetValueType()), node.GetSpan());
                }
                break;
            case BinaryOperator::LogicAnd:  [[fallthrough]];
            case BinaryOperator::LogicOr:
                if (IsBool(lhsType)) { node.SetValueType(lhsType); }
                else {
                    Error("", node.GetSpan());
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