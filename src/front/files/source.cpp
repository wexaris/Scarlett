#include "source_map.hpp"
#include "log/logging.hpp"

namespace scar {
    namespace source {

        SourceFile::SourceFile(std::string_view path) :
            file(path.data()),
            path(std::string(path))
        {
            if (!file.is_open()) {
                log::get_default()->critical("failed to open file {}", path);
            }
        }

        std::string SourceFile::read(size_t start, size_t count) {
            if (!is_open()) {
                open();
            }

            char* buff = new char[count];
            memset(buff, '\0', count);

            file.seekg(start);
            file.read(buff, count);

            std::string str_buff = buff;
            delete[] buff;

            return str_buff;
        }


        bool SourceMap::contains(std::string_view path) const {
            for (auto& f : files) {
                if (f->path == path) {
                    return true;
                }
            }
            return false;
        }

        file_ptr_t SourceMap::load(std::string_view path) {
            // If the file has already been loaded,
            // return that
            auto file_opt = find(path);
            if (file_opt.has_value()) {
                return file_opt.value();
            }

            // Load the file and add it to the list
            auto sf = std::make_shared<SourceFile>(path);
            files.push_back(sf);
            return sf;
        }

        std::optional<file_ptr_t> SourceMap::find(std::string_view path) {
            for (auto& f : files) {
                if (f->path == path) {
                    return f;
                }
            }
            return std::nullopt;
        }

    }
}