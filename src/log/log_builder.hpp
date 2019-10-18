#pragma once
#include "parse/span.hpp"
#include <vector>
#include <optional>

namespace scar {
    namespace log {

        using log_t = std::string_view;

        struct Level {
            enum LevelList {
                Info,
                Warn,
                Error,
                Bug,
                Fail,
                Unimpl
            } inner;

            Level(LevelList lvl) : inner(lvl) {}

            inline bool operator==(LevelList lvl) const noexcept { return inner == lvl; }
            inline bool operator!=(LevelList lvl) const noexcept { return inner != lvl; }

            inline bool operator>(LevelList lvl) const noexcept  { return inner > lvl; }
            inline bool operator>=(LevelList lvl) const noexcept { return inner >= lvl; }
            inline bool operator<(LevelList lvl) const noexcept  { return inner < lvl; }
            inline bool operator<=(LevelList lvl) const noexcept { return inner <= lvl; }

            log_t to_str();
        };

        struct Preview {
            LabledSpan span_lb;
            std::string_view txt;
            bool show_pw = false; // Whether should show preview
        };

        struct SubMessage {
            enum Type {
                Note,
                Help
            } type;
            inline std::string_view type_str() const {
                if (type == Note) { return "note"; }
                else              { return "help"; }
            }
            std::string message;
            std::optional<Preview> preview;

            SubMessage(Type ty, std::string msg);
        };

        class LogBuilder {
            friend class LogManager;

        private:
            class LogManager& manager;

            Level level;
            std::string message;
            std::optional<Preview> preview;
            std::vector<SubMessage> subs;
            uint16_t code = 0;

            std::string compiled;
            bool active = true;

            LogBuilder(LogManager& mgr, Level lvl, std::string msg);

            LogBuilder& add_span(Span sp, std::string label);
            LogBuilder& enable_preview();
            LogBuilder& add_sub(SubMessage sub);

            log_t build();
            void compile();

        public:
            void emit();

            inline void set_level(Level lvl) noexcept { level = lvl; }

            inline bool is_fail() const noexcept    { return level >= Level::Fail; }
            inline bool is_unimpl() const noexcept  { return level == Level::Unimpl; }
            inline bool is_bug() const noexcept     { return level == Level::Bug; }
            inline bool is_error() const noexcept   { return level >= Level::Error; }
            inline bool is_warning() const noexcept { return level == Level::Warn; }
            inline bool is_info() const noexcept    { return level == Level::Info; }
            inline bool is_active() const noexcept  { return active; }
            inline void cancel() noexcept           { active = false; }
        };

    }
}