#include "scarpch.hpp"
#include "Core/Session.hpp"

namespace scar {

    void Session::Init(const std::vector<const char*>& args) {
        if (args.size() == 1) {
            SCAR_ERROR("no input file specified!");
            return;
        }
        GetProperties().InputFile = args[1];
    }

    void Session::Trace(const std::string& message) {
        Log::GetLogger()->trace(message);
    }
    void Session::Info(const std::string& message) {
        Log::GetLogger()->info(message);
    }
    void Session::Warn(const std::string& message) {
        Log::GetLogger()->warn(message);
    }
    void Session::Error(const std::string& message) {
        Log::GetLogger()->error(message);
        GetProperties().ErrorCount++;
    }

}