#pragma once

namespace scar {
	namespace err_codes {
		
		constexpr int UNKNOWN = -1;
		constexpr int NO_JOB = 0;

		constexpr int UNRECOGNIZED_OPT = 1;
		constexpr int INVALID_PARAM = 1;
		constexpr int MISSING_PARAM = 1;

		constexpr int ERR_FAILED_PARSE = 1000;
		constexpr int ERR_FAILED_CODEGEN = 2000;
		constexpr int ERR_FAILED_OPTIMIZE = 3000;

	}
}