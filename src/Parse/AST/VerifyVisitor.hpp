#pragma once
#include "Parse/AST/AST.hpp"

namespace scar {
    namespace ast {

        struct VisitorData;

        class TypeCheckVisitor : public Visitor {
        public:
            TypeCheckVisitor() = default;

            void Visit(Type& node) override;

            void Visit(Module& node) override;
            void Visit(Function& node) override;
            void Visit(FunctionPrototype& node) override;
            void Visit(VarDecl& node) override;

            void Visit(Branch& node) override;
            void Visit(ForLoop& node) override;
            void Visit(WhileLoop& node) override;
            void Visit(Block& node) override;
            void Visit(Continue& node) override;
            void Visit(Break& node) override;
            void Visit(Return& node) override;

            void Visit(FunctionCall& node) override;
            void Visit(VarAccess& node) override;

            void Visit(PrefixOperator& node) override;
            void Visit(SuffixOperator& node) override;
            void Visit(BinaryOperator& node) override;

            void Visit(LiteralBool& node) override;
            void Visit(LiteralInteger& node) override;
            void Visit(LiteralFloat& node) override;
            void Visit(LiteralString& node) override;
        };

    }
}