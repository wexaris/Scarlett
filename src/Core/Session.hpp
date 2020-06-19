#pragma once

namespace scar {

    struct SessionProperties {
        uint32_t ErrorCount = 0;
        const char* InputFile;
        std::vector<const char*> Args;
    };

    class Session {
    public:
        static void Init(const std::vector<const char*>& args);

        static bool IsGood() { return GetProperties().ErrorCount == 0; }
        static uint32_t GetErrorCount() { return GetProperties().ErrorCount; }
        static const char* const GetInputFile() { return GetProperties().InputFile; }

        static void Trace(const std::string& message);
        static void Info(const std::string& message);
        static void Warn(const std::string& message);
        static void Error(const std::string& message);

        static SessionProperties& GetProperties() {
            static SessionProperties Properties;
            return Properties;
        }

    private:
        Session() = delete;
    };

}