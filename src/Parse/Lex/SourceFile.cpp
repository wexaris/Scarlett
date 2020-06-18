#include "scarpch.hpp"
#include "Parse/Lex/SourceFile.hpp"

namespace scar {

    std::vector<Scope<SourceFile>> SourceMap::s_Files;

    SourceFile::SourceFile(const std::string& path) :
        m_File(path),
        m_FilePath(path)
    {
        if (!m_File.is_open()) {
            throw ParseError(ErrorCode::SourceFileError, [&]() { SCAR_ERROR("failed to open file: {}", path); });
        }

        m_File.seekg(0, std::ios::end);
        auto length = m_File.tellg();
        m_File.seekg(0, std::ios::beg);

        m_Text.resize(length);
        m_File.read(&m_Text[0], length);

        m_File.close();
    }

    std::string_view SourceFile::GetString(size_t start, size_t count, bool stopAtNewline) const {
        std::string_view str = std::string_view(m_Text).substr(start, count);
        return stopAtNewline ? str.substr(0, str.find_first_of('\n')) : str;
    }

    std::string SourceFile::GetFileName() const {
        auto dashPos = m_FilePath.find_last_of('/');
        auto namePos = (dashPos == 0 && m_FilePath[0] != '/') ? 0 : dashPos + 1;
        return m_FilePath.substr(namePos);
    }

    SourceFile* SourceMap::Load(const std::string& path) {
        if (SourceFile* file = Find(path)) {
            return file;
        }

        // Load the file and add it to the list
        s_Files.push_back(MakeScope<SourceFile>(path));
        return s_Files.back().get();
    }

    SourceFile* SourceMap::Find(const std::string& path) {
        for (auto& f : s_Files) {
            if (f->GetFilePath() == path) {
                return f.get();
            }
        }
        return nullptr;
    }

}