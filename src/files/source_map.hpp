#include "source_file.hpp"
#include <optional>
#include <vector>

namespace scar {
    namespace source {

        class SourceMap {

        private:
            std::vector<file_ptr_t> files;

            SourceMap() = default;
            SourceMap(const SourceMap&) = delete;

            void operator=(const SourceMap&) = delete;

        public:
            static SourceMap& instace() {
                static SourceMap map;
                return map;
            }

            bool contains(std::string_view path) const;
            file_ptr_t load(std::string_view path);
            std::optional<file_ptr_t> find(std::string_view path);
        };

    }
}