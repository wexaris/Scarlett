#pragma once
#include "log_builder.hpp"
#include "emitter.hpp"
#include <memory>

namespace scar {

    namespace args {
        struct ParsedArgs;
    }

    namespace log {

        class LogManager {

            Emitter emitter = Emitter(std::make_shared<sinks::StdOut>());
			size_t error_count = 0;

            std::vector<LogBuilder> delayed;

            struct Options {
                bool no_warn;
                bool warn_as_err;
            } flags;

            /* Modifies the given `LogBuilder` to fit command line parameters. */
            void update_according_to_args(LogBuilder& lb);


        public:
            LogManager() = default;

            void apply_options(const args::ParsedArgs& args);

            /* Emits the error built my a `LogBuilder`.
            Applies effects specified by command line parameters.
            The `LogBuilder` is disabled after emission. */
            void emit(LogBuilder& lb);
			/* Emits all of the `LogBuilder` instances that haven't emitted yet. */
			void emit_delayed();

			/* Prints delayed errors and throws an `EarlyExit` exception, if there were any. */
			void finish();

			/* Returns true if there have been errors. */
			inline bool has_delayed() const noexcept		{ return delayed.size() > 0; }
			/* Returns true if there have been errors. */
			inline size_t delayed_count() const noexcept	{ return delayed.size(); }
            /* Returns true if there have been errors. */
            inline bool had_error() const noexcept			{ return error_count > 0; }
            /* Returns error count. */
            inline size_t err_count() const noexcept		{ return error_count; }

            LogBuilder& make_new(Level lvl, std::string msg);

            LogBuilder& make_info(std::string msg);
            LogBuilder& make_info_span(std::string msg, const Span& sp, const std::string& label = "");
            LogBuilder& make_info_span_preview(std::string msg, const Span& sp, const std::string& label = "");
            void info(std::string msg);

            LogBuilder& make_warn(std::string msg);
            LogBuilder& make_warn_span(std::string msg, const Span& sp, const std::string& label = "");
            LogBuilder& make_warn_span_preview(std::string msg, const Span& sp, const std::string& label = "");
            void warn(std::string msg);

            LogBuilder& make_error(std::string msg);
            LogBuilder& make_error_span(std::string msg, const Span& sp, const std::string& label = "");
            LogBuilder& make_error_span_preview(std::string msg, const Span& sp, const std::string& label = "");
            void error(std::string msg);

            LogBuilder& make_bug(std::string msg);
            LogBuilder& make_bug_span(std::string msg, const Span& sp, const std::string& label = "");
            LogBuilder& make_bug_span_preview(std::string msg, const Span& sp, const std::string& label = "");
            void bug(std::string msg);

            LogBuilder& make_unimpl(std::string msg);
            void unimpl(std::string msg);

            LogBuilder& make_fail(std::string msg);
            LogBuilder& make_fail_span(std::string msg, const Span& sp, const std::string& label = "");
            LogBuilder& make_fail_span_preview(std::string msg, const Span& sp, const std::string& label = "");
            void fail(std::string msg);
        };

    }
}