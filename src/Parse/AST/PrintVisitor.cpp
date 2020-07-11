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

#define PRINT(...) SCAR_TRACE("{}-{} <{}>", GetIndent(), FMT(__VA_ARGS__), node.GetSpan())
#define ADD_SCOPE() Scoper scoper##__LINE__
#define PRINT_AND_SCOPE(...) PRINT(__VA_ARGS__); ADD_SCOPE();

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // TYPE

        void PrintVisitor::Visit(Type& node) {
            PRINT_AND_SCOPE("Type {}", node.ResultType);
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // DECLARAIONS

        void PrintVisitor::Visit(Module& node) {
            PRINT_AND_SCOPE("Module");
            for (size_t i = 0; i < node.Items.size(); i++) {
                EnableBranch(i != node.Items.size() - 1);
                node.Items[i]->Accept(*this);
            }
        }

        void PrintVisitor::Visit(Function& node) {
            PRINT_AND_SCOPE("Function");
            node.Prototype->Accept(*this);
            EnableBranch(false);
            node.CodeBlock->Accept(*this);
        }

        void PrintVisitor::Visit(FunctionPrototype& node) {
            PRINT_AND_SCOPE("FunctionPrototype {} \"{}\"({})", node.ReturnType->ResultType, node.Name, node.Name.StringID);
            for (size_t i = 0; i < node.Args.size(); i++) {
                EnableBranch(i != node.Args.size() - 1);
                PRINT("Arg {} \"{}\"({})",
                      node.Args[i].VarType->ResultType,
                      node.Args[i].Name, node.Args[i].Name.StringID);
            }
        }

        void PrintVisitor::Visit(VarDecl& node) {
            PRINT_AND_SCOPE("VarDecl {} \"{}\"({})",
                            node.ResultType,
                            node.Name, node.Name.StringID);
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // STATEMENTS

        void PrintVisitor::Visit(Branch& node) {
            PRINT_AND_SCOPE("Branch");
            node.Condition->Accept(*this);
            EnableBranch((bool)node.FalseBlock);
            node.TrueBlock->Accept(*this);
            if (node.FalseBlock) {
                EnableBranch(false);
                node.FalseBlock->Accept(*this);
            }
        }

        void PrintVisitor::Visit(ForLoop& node) {
            PRINT_AND_SCOPE("ForLoop");
            node.Init->Accept(*this);
            node.Condition->Accept(*this);
            node.Update->Accept(*this);
            EnableBranch(false);
            node.CodeBlock->Accept(*this);
        }

        void PrintVisitor::Visit(WhileLoop& node) {
            PRINT_AND_SCOPE("WhileLoop");
            node.Condition->Accept(*this);
            EnableBranch(false);
            node.CodeBlock->Accept(*this);
        }

        void PrintVisitor::Visit(Block& node) {
            PRINT_AND_SCOPE("Block");
            for (size_t i = 0; i < node.Items.size(); i++) {
                EnableBranch(i != node.Items.size() - 1);
                node.Items[i]->Accept(*this);
            }
        }

        void PrintVisitor::Visit(Continue& node) {
            PRINT_AND_SCOPE("Continue");
        }

        void PrintVisitor::Visit(Break& node) {
            PRINT_AND_SCOPE("Break");
        }

        void PrintVisitor::Visit(Return& node) {
            PRINT_AND_SCOPE("Return");
            EnableBranch(false);
            node.Value->Accept(*this);
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // EXPRESSIONS

        void PrintVisitor::Visit(FunctionCall& node) {
            PRINT_AND_SCOPE("FunctionCall {} \"{}\"({})",
                            node.ResultType,
                            node.Name, node.Name.StringID);

            for (size_t i = 0; i < node.Args.size(); i++) {
                EnableBranch(i != node.Args.size() - 1);
                node.Args[i]->Accept(*this);
            }
        }

        void PrintVisitor::Visit(VarAccess& node) {
            PRINT_AND_SCOPE("VarAccess {} \"{}\"({})",
                            node.ResultType,
                            node.Name, node.Name.StringID);
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // OPERATORS

        void PrintVisitor::Visit(PrefixOperator& node) {
            PRINT_AND_SCOPE("Prefix {}", node.Type);
            EnableBranch(false);
            node.RHS->Accept(*this);
        }

        void PrintVisitor::Visit(SuffixOperator& node) {
            switch (node.Type)
            {
            case SuffixOperator::Cast: PRINT("Cast {}", node.ResultType); break;
            default:
                PRINT("Suffix {}", node.Type);
                break;
            }
            ADD_SCOPE();
            EnableBranch(false);
            node.LHS->Accept(*this);
        }

        void PrintVisitor::Visit(BinaryOperator& node) {
            PRINT_AND_SCOPE("Binary {}", node.Type);
            node.LHS->Accept(*this);
            EnableBranch(false);
            node.RHS->Accept(*this);
        }

        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        // LITERALS

        void PrintVisitor::Visit(LiteralBool& node) {
            PRINT_AND_SCOPE("Bool {} {}", node.ResultType, node.Value);
        }

        void PrintVisitor::Visit(LiteralInteger& node) {
            PRINT_AND_SCOPE("Int {} {}", node.ResultType, node.Value);
        }

        void PrintVisitor::Visit(LiteralFloat& node) {
            PRINT_AND_SCOPE("Float {} {}", node.ResultType, node.Value);
        }

        void PrintVisitor::Visit(LiteralString& node) {
            PRINT_AND_SCOPE("String \"{}\"({})",
                            Interner::GetString(node.StringID),
                            node.StringID);
        }

    }
}
