#pragma once

namespace scar {

    class Session {
    public:
        std::string InputFile;

        static void Create(int argc, const char* argv[]);
        static Session& Get() {
            if (!s_Instance) { SCAR_BUG("Attempting to get Session instance before creation!"); }
            return *s_Instance;
        }

    private:
        static Session* s_Instance;

        Session(int argc, const char* argv[]);
        Session(const Session& session) = delete;
        Session operator=(const Session& session) = delete;
    };

    class Driver {
    public:
        static void Init(int argc, const char* argv[]);
        static void Compile();

        static int GetReturnState() { return m_ReturnState; }

    private:
        static bool m_Initialized;
        static int m_ReturnState;
        static unsigned int m_ErrorCount;
    };

}