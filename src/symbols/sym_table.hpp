#pragma once
#include "ast/ast.hpp"
#include <unordered_map>
#include <functional>
#include <optional>
#include <stack>

namespace scar {
    namespace sym {

        enum Type {
            Undef,
            Var     = (int)TokenType::Var,
            Fun     = (int)TokenType::Fun,
            Struct  = (int)TokenType::Struct,
            Enum    = (int)TokenType::Enum,
            Union   = (int)TokenType::Union,
            Trait   = (int)TokenType::Trait,
            Macro   = (int)TokenType::Macro
        };

        /* Key type for the symbol table.
        Contains the symbol's name and type. */
        struct Symbol {
            ast::Name name;
            Type type;

            Symbol(ast::Name name, Type ty) :
                name(name),
                type(ty)
            {}

            inline bool operator==(const Symbol& other) const {
                return name == other.name && type == other.type;
            }
            inline bool operator!=(const Symbol& other) const {
                return !operator==(other);
            }
        };

        /* value type for the symbol table. */
        struct SymInfo {
            Type type = Undef;

            SymInfo() = default;
            SymInfo(ast::VarDecl*) : type(Var) {}
            SymInfo(ast::FunPrototypeDecl*) : type(Fun) {}

            ast::VarDecl* var_info;
            ast::FunPrototypeDecl* fun_info;
        };

    }
}

namespace std {

    template <>
    struct hash<scar::sym::Symbol> {
        size_t operator()(const scar::sym::Symbol& sym) const {
            size_t res = 42;
            res = res * 7 + hash<size_t>()(sym.name.id);
            res = res * 7 + hash<scar::sym::Type>()(sym.type);
            return res;
        }
    };

}

namespace scar {

    class SymbolTable {

    private:
        using table_t = std::unordered_map<sym::Symbol, sym::SymInfo>;
        table_t defined;

    public:
        SymbolTable() = default;

        /* Inserts a `Symbol` and `SymInfo` pair into the symbol table.
        If the key is already mapped, it's value is overridden. */
        inline void insert(const sym::Symbol& sym, const sym::SymInfo& info) {
            defined[sym] = info;
        }

        /* Finds a specific symbol and returns its corresponding `SymInfo`.
        If it's not found, returns a `nullptr`. */
        inline sym::SymInfo* find(const sym::Symbol& sym) {
            auto iter = defined.find(sym);
            if (iter == defined.end()) {
                return nullptr;
            }
            return &iter->second;
        }
    };

    class SymbolStack {

    private:
        using stack_t = std::vector<SymbolTable>;
        stack_t tables;

    public:
        SymbolStack() = default;

        inline void push()  { tables.push_back(SymbolTable()); }
        inline void pop()   { tables.pop_back(); }

        using iterator = stack_t::iterator;
        using const_iterator = stack_t::const_iterator;

        inline iterator top() noexcept                  { return tables.end() - 1; }
        inline const_iterator ctop() const noexcept     { return tables.cend() - 1; }
        inline iterator bottom() noexcept               { return tables.begin(); }
        inline const_iterator cbottom() const noexcept  { return tables.cbegin(); }

        inline iterator end() noexcept              { return tables.end(); }
        inline const_iterator cend() const noexcept { return tables.cend(); }
    };

    class ScopeTree : public std::enable_shared_from_this<ScopeTree> {

    private:
        using scope_t = shared<ScopeTree>;
        std::vector<scope_t> subs;

        ScopeTree(ast::Name name, sym::Type ty, std::vector<scope_t> subs = {}) :
            subs(std::move(subs)),
            name(name),
            type(ty)
        {}

        std::vector<scope_t> find_subs(ast::Name name) const {
            std::vector<scope_t> matches;
            for (auto& decl : subs) {
                if (decl->name == name) {
                    matches.push_back(decl);
                }
            }
            return matches;
        }

    public:
        const ast::Name name;
        const sym::Type type;

        static inline scope_t create(ast::Name name, sym::Type ty, std::vector<scope_t> subs = {}) {
            return scope_t(new ScopeTree(name, ty, std::move(subs)));
        }

        std::vector<scope_t> find(const ast::Path& path) {
            // Return self if path is empty
            if (path.cbegin() == path.cend()) {
                return { shared_from_this() };
            }

            ast::Path::const_iterator path_curr = path.cbegin();
            ast::Path::const_iterator path_end = path.cend();
            std::stack<scope_t> pos;
            pos.push(shared_from_this());

            static const std::function<std::vector<scope_t>()> find_scope = [&]() -> std::vector<scope_t> {
                // Collect all of the scopes with the next name
                // Move path iterator forward
                auto items = pos.top()->find_subs(*path_curr);
                if (items.empty()) {
                    return {};
                }

                if ((++path_curr == path_end)) {
                    return items;
                }

                // Recursively check every scope
                for (auto& item : items) {
                    pos.push(item);
                    // Collect all of the scopes with the next name
                    // Move path iterator forward
                    auto matches = find_scope();
                    if (!matches.empty()) {
                        return matches;
                    }
                    pos.pop();
                }
                return {};
            };

            return find_scope();
        }

        inline scope_t add(scope_t node) {
            subs.push_back(node);
            return subs.back();
        }
        inline scope_t add(ast::Name name, sym::Type ty, std::vector<scope_t> subs = {}) {
            return add(create(name, ty, std::move(subs)));
        }

        inline scope_t& back()              { return subs.back(); }
        inline const scope_t& back() const  { return subs.back(); }

    };

}