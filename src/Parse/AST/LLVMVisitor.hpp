#pragma once
#include "Parse/AST/AST.hpp"

#pragma warning(push, 0)
#pragma warning(disable:4996) // Deprecation
#pragma warning(disable:4146) // Operator minus on unsigned type
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#pragma warning(pop)

namespace scar {
    namespace ast {

        class LLVMVisitor : public Visitor {
        public:
            LLVMVisitor();

            void Print() const { m_Module->print(llvm::outs(), nullptr); }

            void Visit(Type& node) override;

            void Visit(Module& node) override;
            void Visit(Function& node) override;
            void Visit(FunctionPrototype& node) override;

            void Visit(Branch& node) override;
            void Visit(ForLoop& node) override;
            void Visit(WhileLoop& node) override;
            void Visit(Block& node) override;
            void Visit(Continue& node) override;
            void Visit(Break& node) override;
            void Visit(Return& node) override;

            void Visit(FunctionCall& node) override;
            void Visit(Var& node) override;
            void Visit(Variable& node) override;

            void Visit(PrefixOperator& node) override;
            void Visit(SuffixOperator& node) override;
            void Visit(BinaryOperator& node) override;

            void Visit(LiteralBool& node) override;
            void Visit(LiteralInteger& node) override;
            void Visit(LiteralFloat& node) override;
            void Visit(LiteralString& node) override;

        private:
            llvm::LLVMContext m_Context;
            llvm::IRBuilder<> m_Builder;
            Scope<llvm::Module> m_Module;
            Scope<llvm::legacy::FunctionPassManager> m_FunctionPassManager;

            // Return values across codegen functions
            llvm::Value* m_ReturnValue = nullptr;
            llvm::Type* m_ReturnType = nullptr;

            bool m_BlockReturned = false;

            [[noreturn]] void ThrowError(std::string_view msg, const Span& span);
        };

    }
}