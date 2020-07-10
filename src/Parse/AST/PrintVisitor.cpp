#include "scarpch.hpp"
#include "Parse/AST/PrintVisitor.hpp"

namespace scar {
    namespace ast {

        struct PrintVisitorData {
            std::vector<bool> BranchTracker; // True enables pipe printing for that level
            uint32_t IndentCount = 0;
        };
        static PrintVisitorData s_Data;
        
        static void EnableBranch(bool enabled) {
            s_Data.BranchTracker.back() = enabled;
        }

        static std::string GetIndent() {
            if (s_Data.IndentCount == 0)
                return "";

            std::string indent((size_t)s_Data.IndentCount * 2, ' ');
            for (size_t i = 1, j = 0; j < s_Data.BranchTracker.size(); i += 2, j++) {
                if (s_Data.BranchTracker[j])
                    indent[i] = '|';
            }
            indent.back() = s_Data.BranchTracker.back() ? '|' : '`';
            return indent;
        }

        struct Scoper {
            Scoper() {
                s_Data.IndentCount++;
                s_Data.BranchTracker.push_back(true);
            }
            ~Scoper() {
                s_Data.IndentCount--;
                s_Data.BranchTracker.pop_back();
            }
        };


#define PRINT(...) \
    SCAR_TRACE("{}-{} <{}>", GetIndent(), FMT(__VA_ARGS__), node.GetSpan()); \
    Scoper scoper;

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // TYPE

        void PrintVisitor::Visit(Type& node) {
            PRINT("Type {}", node.ResultType);
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // DECLARAIONS

        void PrintVisitor::Visit(Module& node) {
            PRINT("Module");
            for (size_t i = 0; i < node.Items.size(); i++) {
                EnableBranch(i != node.Items.size() - 1);
                node.Items[i]->Accept(*this);
            }
        }

        void PrintVisitor::Visit(Function& node) {
            PRINT("Function");
            node.Prototype->Accept(*this);
            EnableBranch(false);
            node.CodeBlock->Accept(*this);
        }

        void PrintVisitor::Visit(FunctionPrototype& node) {
            PRINT("FunctionPrototype {} \"{}\"({})", node.ReturnType->ResultType, node.Name, node.Name.StringID);
            for (size_t i = 0; i < node.Args.size(); i++) {
                EnableBranch(i != node.Args.size() - 1);
                PRINT("Arg {} \"{}\"({})",
                      node.Args[i].VarType->ResultType,
                      node.Args[i].Name,
                      node.Args[i].Name.StringID);
            }
        }

        void PrintVisitor::Visit(VarDecl& node) {
            PRINT("VarDecl {} \"{}\"({})", node.ResultType, node.Name, node.Name.StringID);
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // STATEMENTS

        void PrintVisitor::Visit(Branch& node) {
            PRINT("Branch");
            node.Condition->Accept(*this);
            EnableBranch((bool)node.FalseBlock);
            node.TrueBlock->Accept(*this);
            if (node.FalseBlock) {
                EnableBranch(false);
                node.FalseBlock->Accept(*this);
            }
        }

        void PrintVisitor::Visit(ForLoop& node) {
            PRINT("ForLoop");
            node.Init->Accept(*this);
            node.Condition->Accept(*this);
            node.Update->Accept(*this);
            EnableBranch(false);
            node.CodeBlock->Accept(*this);
        }

        void PrintVisitor::Visit(WhileLoop& node) {
            PRINT("WhileLoop");
            node.Condition->Accept(*this);
            EnableBranch(false);
            node.CodeBlock->Accept(*this);
        }

        void PrintVisitor::Visit(Block& node) {
            PRINT("Block");
            for (size_t i = 0; i < node.Items.size(); i++) {
                EnableBranch(i != node.Items.size() - 1);
                node.Items[i]->Accept(*this);
            }
        }

        void PrintVisitor::Visit(Continue& node) {
            PRINT("Continue");
        }

        void PrintVisitor::Visit(Break& node) {
            PRINT("Break");
        }

        void PrintVisitor::Visit(Return& node) {
            PRINT("Return");
            EnableBranch(false);
            node.Value->Accept(*this);
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // EXPRESSIONS

        void PrintVisitor::Visit(FunctionCall& node) {
            PRINT("FunctionCall {} \"{}\"({})", node.ResultType, node.Name, node.Name.StringID);
            for (size_t i = 0; i < node.Args.size(); i++) {
                EnableBranch(i != node.Args.size() - 1);
                node.Args[i]->Accept(*this);
            }
        }

        void PrintVisitor::Visit(VarAccess& node) {
            PRINT("VarAccess {} \"{}\"({})", node.ResultType, node.Name, node.Name.StringID);
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // OPERATORS

        void PrintVisitor::Visit(PrefixOperator& node) {
            PRINT("Prefix {}", node.Type);
            EnableBranch(false);
            node.RHS->Accept(*this);
        }

        void PrintVisitor::Visit(SuffixOperator& node) {
            PRINT("Suffix {}", node.Type);
            EnableBranch(false);
            node.LHS->Accept(*this);
        }

        void PrintVisitor::Visit(BinaryOperator& node) {
            PRINT("Binary {}", node.Type);
            node.LHS->Accept(*this);
            EnableBranch(false);
            node.RHS->Accept(*this);
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // LITERALS

        void PrintVisitor::Visit(LiteralBool& node) {
            PRINT("Bool {}", node.Value);
        }

        void PrintVisitor::Visit(LiteralInteger& node) {
            PRINT("Int {}", node.Value);
        }

        void PrintVisitor::Visit(LiteralFloat& node) {
            PRINT("Float {}", node.Value);
        }

        void PrintVisitor::Visit(LiteralString& node) {
            PRINT("String \"{}\"({})", Interner::GetString(node.StringID), node.StringID);
        }

    }
}
