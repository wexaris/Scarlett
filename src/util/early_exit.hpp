#pragma once
#include "err_codes.hpp"
#include <stdexcept>
#include <string>

namespace scar {
	namespace early {

		// Shortcut to the end of the application.
		// For cases when setting up a return circuit isn't worth it.
		class EarlyExit final : public std::exception {

		public:
			const int code;

			EarlyExit(int code) : code(code) {}
			~EarlyExit() = default;

			inline const char* what() const noexcept override {
				return "";
			}
		};

		// Exit after an unrecoverable error.
		// Halt the compilation process and escape.
		class FatalError : public std::exception {

		public:
			const int code;

			FatalError(int code) : code(code) {}
			~FatalError() = default;

			inline const char* what() const noexcept override {
				return "";
			}
		};
	
	}
}