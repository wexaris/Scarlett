#pragma once
#include <fstream>

namespace scar {

    class SourceFile {
    public:
        explicit SourceFile(const std::string& path);

        std::string_view GetString(size_t start, size_t count, bool stopAtNewline = false) const;
        char GetChar(size_t pos) const { return m_Text[pos]; }

        std::string GetFileName() const;
        const std::string& GetFilePath() const { return m_FilePath; }
        size_t GetLength() const { return m_Text.length(); }

        bool IsEOF() const { return m_File.eof(); }

    private:
        std::ifstream m_File;
        const std::string m_FilePath;
        std::string m_Text;
    };

    class SourceMap {
    public:
        static SourceFile* Load(const std::string& path);
        static SourceFile* Find(const std::string& path);

    private:
        static std::vector<Scope<SourceFile>> s_Files;
    };

}