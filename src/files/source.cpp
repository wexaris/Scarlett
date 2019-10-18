#include "source_map.hpp"
#include "driver/session.hpp"

namespace scar {
    namespace source {

        SourceFile::SourceFile(std::string_view path) :
            file(path.data()),
            path(path)
        {
            if (!file.is_open()) {
                Session::get().logger().fail(FMT("failed to open file {}", path));
            }

            file.seekg(0, file.end);
            auto len = file.tellg();
            file.seekg(0, file.beg);

            char* buff = new char[1 + len];
            file.read(buff, len);
            buff[file.gcount()] = '\0';
            text = buff;
            delete[] buff;

            file.close();
        }

        std::string_view SourceFile::read_to_endln(size_t start, size_t count) const {
            auto text = read(start, count);
            return (text.substr(0, text.find_first_of('\n')));
        }

        std::string_view SourceFile::read(size_t start, size_t count) const {
            return std::string_view(text).substr(start, count);
        }

        char SourceFile::read(size_t pos) const {
            return text[pos];
        }

        std::string_view SourceFile::name() const {
            auto dash_loc = path.find_last_of('/');
            auto name_loc = (dash_loc == 0 && path[0] != '/') ? 0 : dash_loc + 1;
            return std::string_view(path).substr(name_loc);
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