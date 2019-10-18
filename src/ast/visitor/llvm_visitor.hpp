#pragma once
#include "ast/ast.hpp"
#include "symbols/sym_table.hpp"
#include "log/log_manager.hpp"
#include "ast/llvm.hpp"
#include <optional>

namespace scar {
    namespace ast {

        class LLVMVisitor : public ASTVisitor {

        private:
            llvm::LLVMContext context;
            llvm::IRBuilder<> builder;

			struct VarInfo {
				llvm::Type* type;
				llvm::AllocaInst* value;
			};
			struct VarStack {
				using stack_t = std::vector<std::unordered_map<Name, VarInfo>>;
				stack_t stack;

				VarStack() { push(); }

				inline void push()	{ stack.push_back({}); }
				inline void pop()	{ stack.pop_back(); }

				std::optional<VarInfo> find(Name name) {
					for (auto scope = stack.rbegin(); scope != stack.rend(); scope++) {
						auto info = scope->find(name);
						if (info != scope->end()) {
							return info->second;
						}
					}
					return std::nullopt;
				}

				inline void insert(Name name, VarInfo info) { stack.back()[name] = std::move(info); }
			};

			VarStack variables;

            // Logger shorthand for error emission
            log::LogManager& logger;

        public:
            // Current module being built
            shared<llvm::Module> module;

            LLVMVisitor(log::LogManager& logger);

            bool visit(BinOp& node) override;
            bool visit(UnaryOp& node) override;

			bool visit(StrExpr& node) override;
			bool visit(CharExpr& node) override;
			bool visit(BoolExpr& node) override;
			bool visit(I8Expr& node) override;
			bool visit(I16Expr& node) override;
			bool visit(I32Expr& node) override;
			bool visit(I64Expr& node) override;
			bool visit(U8Expr& node) override;
			bool visit(U16Expr& node) override;
			bool visit(U32Expr& node) override;
			bool visit(U64Expr& node) override;
			bool visit(F32Expr& node) override;
			bool visit(F64Expr& node) override;
			bool visit(VarExpr& node) override;
			bool visit(FunCall& node) override;
            bool visit(Block& node) override;

            bool visit(VarDecl& node) override;
            bool visit(FunPrototypeDecl& node) override;
            bool visit(FunDecl& node) override;
			bool visit(RetVoidStmt& node) override;
			bool visit(RetStmt& node) override;
			bool visit(ExprStmt& node) override;
            bool visit(Module& node) override;

			bool visit(Type& node) override;
        };

    }
}