#include "log_manager.hpp"
#include "driver/session.hpp"
#include "util/error.hpp"
#include "util/early_exit.hpp"
#include <sstream>

namespace scar {
    namespace log {

        SubMessage::SubMessage(Type ty, std::string msg) :
            type(ty),
            message(std::move(msg))
        {}

        log_t Level::to_str() {
            switch (inner)
            {
            case Info:
                return "info";
            case Warn:
                return "warning";
            case Error:
                return "error";
            case Bug:
                return "bug";
            case Unimpl:
                return "unimpl";
            case Fail:
                return "error";
            default:
                Session::get().logger().bug(FMT("log_manager.cpp: missing string for level #{}", (int)inner));
                return "unknown";
            }
        }


        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        LogBuilder::LogBuilder(LogManager& mgr, Level lvl, std::string msg) :
            manager(mgr),
            level(lvl),
            message(std::move(msg))
        {}

        LogBuilder& LogBuilder::add_span(Span sp, std::string label) {
            preview = Preview{ LabledSpan(std::move(sp), std::move(label)), sp.file->read(sp.idx, sp.len) };
            return *this;
        }

        LogBuilder& LogBuilder::enable_preview() {
            if (preview.has_value()) {
                preview.value().show_pw = true;
            }
            return *this;
        }

        LogBuilder& LogBuilder::add_sub(SubMessage sub) {
            subs.push_back(std::move(sub));
            return *this;
        }

        log_t LogBuilder::build() {
            compile();
            return compiled;
        }

        std::unordered_map<Level::LevelList, col::color_t> level_colors = {
            { Level::Info,   { col::ansi::reset      }},
            { Level::Warn,   { col::ansi::fg_yellow  }},
            { Level::Error,  { col::ansi::fg_red     }},
            { Level::Bug,    { col::ansi::fg_magenta }},
            { Level::Unimpl, { col::ansi::fg_magenta }},
            { Level::Fail,   { col::ansi::fg_red     }},
        };

        void LogBuilder::compile() {
            // Print span, if any
            if (preview && !preview->show_pw) {
                compiled += FMT("{}: ", preview->span_lb.span);
            }
            // Print log level and message
            auto color = level_colors[level.inner];
            compiled += col::with_color({ color, col::ansi::bold }, FMT("{}: {}\n", level.to_str(), message));
            
            // Print span and preview, if any
            if (preview && preview->show_pw) {
                compiled += FMT(" --> {}\n", preview->span_lb.span);
                // TODO: show preview
            }

            // Print sub-messages
            for (auto& sub : subs) {
                if (sub.preview && !sub.preview->show_pw) {
                    compiled += FMT("{}: ", sub.preview->span_lb.span);
                }
                // Print sub level and message
                compiled += FMT("{}: {}", sub.type_str(), sub.message);
                if (preview && preview->show_pw) {
                    compiled += FMT(" --> {}\n", preview->span_lb.span);
                    // TODO: show preview
                }
            }
        }

        void LogBuilder::emit() {
            manager.emit(*this);
        }


        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        void LogManager::apply_options(const args::ParsedArgs& args) {
            flags.no_warn = args.args.contains("-Woff");
            flags.warn_as_err = args.args.contains("-Werr");
        }

        void LogManager::update_according_to_args(LogBuilder& lb) {
            if (lb.is_warning()) {
                if (flags.no_warn) {
                    lb.cancel();
                }
                if (flags.warn_as_err) {
                    lb.set_level(Level::Error);
                }
            }
        }

        void LogManager::emit(LogBuilder& lb) {
            update_according_to_args(lb);
            if (lb.is_active()) {
                if (lb.is_error()) {
                    error_count++;
                }
                if (lb.is_fail()) {
                    throw err::FatalError(lb.code);
                }
                lb.cancel();
                emitter.log(lb.build());
            }
        }

        void LogManager::emit_delayed() {
            for (auto& lb : delayed) {
                emit(lb);
            }
        }

        void LogManager::finish() {
			emit_delayed();

            if (error_count == 0) {
                return;
            }

            if (error_count == 1) {
                error("failed due to the previous error");
            }
            else {
                error(FMT("failed due to {} errors", error_count));
            }

            throw err::FatalError(err_codes::UNKNOWN);
        }

        LogBuilder& LogManager::make_new(Level lvl, std::string msg) {
            delayed.push_back(LogBuilder(*this, lvl, msg));
            return delayed.back();
        }

        LogBuilder& LogManager::make_info(std::string msg) {
            return make_new(Level::Info, msg);
        }
        LogBuilder& LogManager::make_info_span(std::string msg, const Span& sp, const std::string& label) {
            return make_info(msg).add_span(sp, label);
        }
        LogBuilder& LogManager::make_info_span_preview(std::string msg, const Span& sp, const std::string& label) {
            return make_info(msg).add_span(sp, label).enable_preview();
        }
        void LogManager::info(std::string msg) {
            emit(make_info(msg));
        }

        LogBuilder& LogManager::make_warn(std::string msg) {
            return make_new(Level::Warn, msg);
        }
        LogBuilder& LogManager::make_warn_span(std::string msg, const Span& sp, const std::string& label) {
            return make_warn(msg).add_span(sp, label);
        }
        LogBuilder& LogManager::make_warn_span_preview(std::string msg, const Span& sp, const std::string& label) {
            return make_warn(msg).add_span(sp, label).enable_preview();
        }
        void LogManager::warn(std::string msg) {
            emit(make_warn(msg));
        }

        LogBuilder& LogManager::make_error(std::string msg) {
            return make_new(Level::Error, msg);
        }
        LogBuilder& LogManager::make_error_span(std::string msg, const Span& sp, const std::string& label) {
            return make_error(msg).add_span(sp, label);
        }
        LogBuilder& LogManager::make_error_span_preview(std::string msg, const Span& sp, const std::string& label) {
            return make_error(msg).add_span(sp, label).enable_preview();
        }
        void LogManager::error(std::string msg) {
            emit(make_error(msg));
        }

        LogBuilder& LogManager::make_bug(std::string msg) {
            return make_new(Level::Bug, msg);
        }
        LogBuilder& LogManager::make_bug_span(std::string msg, const Span& sp, const std::string& label) {
            return make_bug(msg).add_span(sp, label);
        }
        LogBuilder& LogManager::make_bug_span_preview(std::string msg, const Span& sp, const std::string& label) {
            return make_bug(msg).add_span(sp, label).enable_preview();
        }
        void LogManager::bug(std::string msg) {
            emit(make_bug(msg));
        }

        LogBuilder& LogManager::make_unimpl(std::string msg) {
            return make_new(Level::Unimpl, msg);
        }
        void LogManager::unimpl(std::string msg) {
            emit(make_unimpl(msg));
        }

        LogBuilder& LogManager::make_fail(std::string msg) {
            return make_new(Level::Fail, msg);
        }
        LogBuilder& LogManager::make_fail_span(std::string msg, const Span& sp, const std::string& label) {
            return make_fail(msg).add_span(sp, label);
        }
        LogBuilder& LogManager::make_fail_span_preview(std::string msg, const Span& sp, const std::string& label) {
            return make_fail(msg).add_span(sp, label).enable_preview();
        }
        void LogManager::fail(std::string msg) {
            emit(make_fail(msg));
        }

    }
}