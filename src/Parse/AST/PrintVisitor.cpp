#include "scarpch.hpp"
#include "Parse/AST/PrintVisitor.hpp"

namespace scar {
    namespace ast {

#define PRINT_BRANCH(...) SCAR_TRACE("{}`{} {}", GetIndent(), FMT(__VA_ARGS__), node.GetSpan())
#define PRINT_LEAF(...) SCAR_TRACE("{}|{} {}", GetIndent(), FMT(__VA_ARGS__), node.GetSpan())

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // TYPE

        void PrintVisitor::Visit(Type& node) {
            PRINT_LEAF("Type {}", node.ValType);
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // DECLARAIONS

        void PrintVisitor::Visit(Module& node) {
            PRINT_BRANCH("Module");
            m_IndentCount++;
            for (auto& item : node.Items) {
                item->Accept(*this);
            }
            m_IndentCount--;
        }

        void PrintVisitor::Visit(Function& node) {
            PRINT_BRANCH("Function");
            m_IndentCount++;
            node.Prototype->Accept(*this);
            node.CodeBlock->Accept(*this);
            m_IndentCount--;
        }

        void PrintVisitor::Visit(FunctionPrototype& node) {
            PRINT_BRANCH("FunctionPrototype {} \"{}\"({})", node.ReturnType->ValType, node.Name.GetString(), node.Name.StringID);
            m_IndentCount++;
            for (auto& arg : node.Args) {
                PRINT_LEAF("Arg {} \"{}\"({})", arg.VarType->ValType, arg.Name.GetString(), arg.Name.StringID);
            }
            m_IndentCount--;
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // STATEMENTS

        void PrintVisitor::Visit(Branch& node) {
            PRINT_BRANCH("Branch");
            m_IndentCount++;
            node.Condition->Accept(*this);
            node.TrueBlock->Accept(*this);
            if (node.FalseBlock) {
                node.FalseBlock->Accept(*this);
            }
            m_IndentCount--;
        }

        void PrintVisitor::Visit(ForLoop& node) {
            PRINT_BRANCH("ForLoop");
            m_IndentCount++;
            node.Init->Accept(*this);
            node.Condition->Accept(*this);
            node.Update->Accept(*this);
            node.CodeBlock->Accept(*this);
            m_IndentCount--;
        }

        void PrintVisitor::Visit(WhileLoop& node) {
            PRINT_BRANCH("WhileLoop");
            m_IndentCount++;
            node.Condition->Accept(*this);
            node.CodeBlock->Accept(*this);
            m_IndentCount--;
        }

        void PrintVisitor::Visit(Block& node) {
            PRINT_BRANCH("Block");
            m_IndentCount++;
            for (auto& item : node.Items) {
                item->Accept(*this);
            }
            m_IndentCount--;
        }

        void PrintVisitor::Visit(Continue& node) {
            PRINT_LEAF("Continue");
        }

        void PrintVisitor::Visit(Break& node) {
            PRINT_LEAF("Break");
        }

        void PrintVisitor::Visit(Return& node) {
            PRINT_BRANCH("Return");
            m_IndentCount++;
            node.Value->Accept(*this);
            m_IndentCount--;
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // EXPRESSIONS

        void PrintVisitor::Visit(FunctionCall& node) {
            PRINT_BRANCH("FunctionCall {} \"{}\"({})", node.ValueType, node.Name.GetString(), node.Name.StringID);
            m_IndentCount++;
            for (auto& arg : node.Args) {
                arg->Accept(*this);
            }
            m_IndentCount--;
        }

        void PrintVisitor::Visit(Var& node) {
            PRINT_BRANCH("Var {} \"{}\"({})", node.ValueType, node.Name.GetString(), node.Name.StringID);
            m_IndentCount++;
            node.Assign->RHS->Accept(*this);
            m_IndentCount--;
        }

        void PrintVisitor::Visit(Variable& node) {
            PRINT_LEAF("Variable {} \"{}\"({})", node.ValueType, node.Name.GetString(), node.Name.StringID);
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // OPERATORS

        void PrintVisitor::Visit(PrefixOperator& node) {
            PRINT_BRANCH("Prefix {}", node.Type);
            m_IndentCount++;
            node.RHS->Accept(*this);
            m_IndentCount--;
        }

        void PrintVisitor::Visit(SuffixOperator& node) {
            PRINT_BRANCH("Suffix {}", node.Type);
            m_IndentCount++;
            node.LHS->Accept(*this);
            m_IndentCount--;
        }

        void PrintVisitor::Visit(BinaryOperator& node) {
            PRINT_BRANCH("Binary {}", node.Type);
            m_IndentCount++;
            node.LHS->Accept(*this);
            node.RHS->Accept(*this);
            m_IndentCount--;
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // LITERALS

        void PrintVisitor::Visit(LiteralBool& node) {
            PRINT_LEAF("Bool {}", node.Value);
        }

        void PrintVisitor::Visit(LiteralInteger& node) {
            PRINT_LEAF("Int {}", node.Value);
        }

        void PrintVisitor::Visit(LiteralFloat& node) {
            PRINT_LEAF("Float {}", node.Value);
        }

        void PrintVisitor::Visit(LiteralString& node) {
            PRINT_LEAF("String \"{}\"({})", Interner::GetString(node.StringID), node.StringID);
        }

    }
}