#include "ast.hpp"
#include "symbols/interner.hpp"

namespace scar {
    namespace ast {

        // std::vector<lex::Token>
        // _SN3stdS6vectorG1N3lexS5Token
        // | |    |       | |    |
        // |scarlet       |generics(1)
        //   |namespace     |namespace
        //        |struct        |struct
        //
        // std::mem::align_of<&mut (&str,())>()
        // _SN3stdN3memF8align_ofG1RmT2RcsvA0
        // | |    |    |         | | | | |||
        // |scarlet    |function |generics(1)
        //   |namespace            |ref_m|||
        //        |struct            |tuple(2)
        //                             |ref_c
        //                               |str
        //                                |void
        //                                 |args(0)
        //
        // std::vector<&mut (&str,())>::iterator
        // _SN3stdS6vectorG1RmT2RcsvS8iterator
        // | |    |       | | | | |||
        // |scarlet       |generic(1)
        //   |namespace     |ref_m|||
        //        |struct     |tuple(2)
        //                        |str
        //                         |void
        //                          |struct


        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        Name::Name(size_t id) : id(id) {}

        interned_str_t Name::get_str() const {
            return Interner::instance().find(*this);
        }

        std::string& Name::get_strref() const { return *get_str(); }

        bool Name::operator==(const Name& other) const { return id == other.id; }
        bool Name::operator!=(const Name& other) const { return !operator==(other); }


        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        UnaryOp::UnaryOp(expr_ptr expr, UnaryOp::Kind kind) :
            expr(std::move(expr)),
			kind(kind)
        {}

        BinOp::BinOp(expr_ptr lhs, expr_ptr rhs, BinOp::Kind kind) :
            lhs(std::move(lhs)),
            rhs(std::move(rhs)),
			kind(kind)
        {}


        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        StrExpr::StrExpr(Name val) : val(val) {}
        CharExpr::CharExpr(Name val) : val(val) {}
        BoolExpr::BoolExpr(int8_t val) : val(val) {}

        I8Expr::I8Expr(int8_t val) : val(val) {}
        I16Expr::I16Expr(int16_t val) : val(val) {}
        I32Expr::I32Expr(int32_t val) : val(val) {}
        I64Expr::I64Expr(int64_t val) : val(val) {}

        U8Expr::U8Expr(int8_t val) : val(val) {}
        U16Expr::U16Expr(int16_t val) : val(val) {}
        U32Expr::U32Expr(int32_t val) : val(val) {}
        U64Expr::U64Expr(int64_t val) : val(val) {}

        F32Expr::F32Expr(double val) : val(val) {}
        F64Expr::F64Expr(double val) : val(val) {}

        VarExpr::VarExpr(Name name) : name(name) {}

        FunCall::FunCall(Path path, ArgList args) :
            path(std::move(path)),
            args(std::move(args))
        {}

        Block::Block(std::vector<stmt_ptr> stmts) : stmts(std::move(stmts)) {}


        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        VarDecl::VarDecl(Name name, unique<Type> ty, expr_ptr val) :
            name(name),
            type(std::move(ty)),
            val(std::move(val))
        {}

        Param::Param(Name name, unique<Type> ty) :
            name(name),
            type(std::move(ty))
        {}

        FunPrototypeDecl::FunPrototypeDecl(Name name, ParamList params, unique<Type> ret) :
            name(name),
            params(std::move(params)),
            ret(std::move(ret))
        {}

        std::string FunPrototypeDecl::signature() const {
            return "";
        }

        FunDecl::FunDecl(unique<FunPrototypeDecl> proto, unique<Block> blk) :
            prototype(std::move(proto)),
            block(std::move(blk))
        {}

        ExprStmt::ExprStmt(expr_ptr e) :
            expr(std::move(e))
        {}

    }
}