#pragma once
#include "parse/span.hpp"
#include <vector>

namespace scar {
    namespace log {

        using log_t = std::string_view;

        struct Level {
            enum LevelList {
                Info,
                Warn,
                Error,
                Bug,
                Unimpl,
                Fail
            } level;

            Level(LevelList lvl) : level(lvl) {}

            inline bool operator==(LevelList lvl) const noexcept { return level == lvl; }
            inline bool operator!=(LevelList lvl) const noexcept { return level != lvl; }

            inline bool operator>(LevelList lvl) const noexcept  { return level > lvl; }
            inline bool operator>=(LevelList lvl) const noexcept { return level >= lvl; }
            inline bool operator<(LevelList lvl) const noexcept  { return level < lvl; }
            inline bool operator<=(LevelList lvl) const noexcept { return level <= lvl; }

            log_t to_str();
        };

        struct SubMessage {
            enum Type {
                Warning,
                Note,
                Help
            } type;
            std::string message;

            SubMessage(Type ty, std::string msg);
        };

        class LogBuilder {
            friend class LogManager;

        private:
            class LogManager& manager;

            Level level;
            std::vector<LabledSpan> span;
            std::string message;
            std::vector<std::string_view> preview;
            std::vector<SubMessage> subs;
            uint16_t code = 0;

            std::string compiled;
            bool active = true;

            LogBuilder(LogManager& mgr, Level lvl, std::string msg);

            LogBuilder& add_span(Span sp, std::string label);
            LogBuilder& add_previews();

            log_t build();
            void compile();

        public:
            void emit();

            inline void set_level(Level lvl) noexcept { level = lvl; }

            inline bool is_fail() const noexcept    { return level >= Level::Fail; }
            inline bool is_unimpl() const noexcept  { return level == Level::Warn; }
            inline bool is_bug() const noexcept     { return level == Level::Warn; }
            inline bool is_error() const noexcept   { return level >= Level::Error; }
            inline bool is_warning() const noexcept { return level == Level::Warn; }
            inline bool is_info() const noexcept    { return level == Level::Warn; }
            inline bool is_active() const noexcept  { return active; }
            inline void cancel() noexcept           { active = false; }
        };

    }
}