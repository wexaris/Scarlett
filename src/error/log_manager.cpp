#include "log_manager.hpp"
#include "error.hpp"
#include "driver/session.hpp"
#include "util/early_exit.hpp"

namespace scar {
    namespace log {

        SubMessage::SubMessage(Type ty, std::string msg) :
            type(ty),
            message(std::move(msg))
        {}

        log_t Level::to_str() {
            switch (level)
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
                Session::get().logger().bug(format("log_manager.cpp: missing string for level #{}", (int)level));
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
            span.push_back(LabledSpan(std::move(sp), std::move(label)));
            return *this;
        }

        LogBuilder& LogBuilder::add_previews() {
            for (auto& sp : span) {
                preview.push_back(sp.span.file->read(sp.span.start, sp.span.len));
            }
            return *this;
        }

        log_t LogBuilder::build() {
            compile();
            return compiled;
        }

        void LogBuilder::compile() {

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
                    throw err::FatalError();
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

        bool LogManager::print_error_count() {
            if (error_count == 0) {
                return false;
            }

            if (error_count == 1) {
                error("failed due to the previous error");
                return true;
            }
            else {
                error(fmt::format("failed due to {} errors", error_count));
                return true;
            }

            throw EarlyExit(ERR_FAILED_BUILD);
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
            return make_info(msg).add_span(sp, label).add_previews();
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
            return make_warn(msg).add_span(sp, label).add_previews();
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
            return make_error(msg).add_span(sp, label).add_previews();
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
            return make_bug(msg).add_span(sp, label).add_previews();
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
            return make_fail(msg).add_span(sp, label).add_previews();
        }
        void LogManager::fail(std::string msg) {
            emit(make_fail(msg));
        }

    }
}