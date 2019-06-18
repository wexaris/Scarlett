#pragma once
#include <fstream>
#include <string>
#include <memory>

namespace scar {
    namespace source {

        class SourceFile {

        private:
            std::ifstream file;

        public:
            const std::string path;
            std::string text;

            explicit SourceFile(std::string_view path);

            std::string_view read(size_t start, size_t count) const;
            char read(size_t pos) const;

            inline void open()          { file.open(path); }
            inline void close()         { file.close(); }
            inline bool is_open() const { return file.is_open(); }
            inline bool is_eof() const  { return file.eof(); }

            std::string_view name() const;
            inline size_t len() const { return text.length(); }
        };

        using file_ptr_t = std::shared_ptr<SourceFile>;

    }
}