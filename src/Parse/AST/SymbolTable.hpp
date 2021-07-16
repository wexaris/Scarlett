#pragma once

namespace scar {
    namespace ast {

        template<typename Key, typename Value>
        class SymbolTable {
        public:
            SymbolTable() { PushScope(); }
            virtual ~SymbolTable() = default;

            virtual void PushScope() { m_Symbols.push_back({}); }
            virtual void PopScope()  { m_Symbols.pop_back(); }

        protected:
            std::vector<std::unordered_map<Key, Value>> m_Symbols;

            virtual const Value* TryFind(const Key& key) const {
                for (auto iter = m_Symbols.rbegin(); iter != m_Symbols.rend(); iter++) {
                    auto item = iter->find(key);
                    if (item != iter->end()) {
                        return &item->second;
                    }
                }
                return nullptr;
            }
        };

    }
}