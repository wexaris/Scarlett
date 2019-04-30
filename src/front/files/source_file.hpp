#pragma once
#include <fstream>
#include <string>
#include <memory>

namespace scar {
    namespace source {

        struct SourceFile {

        private:
            std::ifstream file;

        public:
            const std::string path;

            explicit SourceFile(std::string_view path);

            inline void open()          { file.open(path); }
            inline void close()         { file.close(); }
            inline bool is_open() const { return file.is_open(); }
            inline bool is_eof() const  { return file.eof(); }

            std::string name() const {
                auto dash_loc = path.find_last_of('/');
                auto name_loc = (dash_loc == 0 && path[0] != '/') ? 0 : dash_loc + 1;
                return path.substr(name_loc);
            }

            std::string read(size_t start, size_t count);
        };

        using file_ptr_t = std::shared_ptr<SourceFile>;

    }
}