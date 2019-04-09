#include "driver.hpp"
#include "parse/parser.hpp"
#include <functional>   // timed_exec()
#include <chrono>       // timed_exec()
#include <iomanip>      // std::setprecision()

/* Measures the time it takes to run the given function. */
long long timed_exec(const std::function<void(void)>& fun) {
    // Run the provided function
    // Keep track of it's start and end times
    auto start = std::chrono::system_clock::now();
    fun();
    auto end = std::chrono::system_clock::now();

    // Calculate duration of function execution in milliseconds
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

inline void log_timed(const std::string& name, const std::function<void(void)>& fun) {
    auto time = timed_exec(fun);
    LOG(name << " : DONE (" << std::fixed << std::setprecision(2) << time << "ms)");
}


//////////////
/// DRIVER ///
//////////////

#include "files/source_map.hpp"

void Driver::compile() {
    // Fail compilation if no input file was given
    if (sess->infile.empty()) {
        ERR("Input file missing");
    }

    log_timed("Read lexer chars", [&]() {
        Lexer lex(sess->infile);
        Token tok = lex.next_token();
        while (!tok.is_eof()) {
            LOG(tok.raw());
            tok = lex.next_token();;
        }
    });

    Parser parser = Parser(sess->infile);

    parser.parse();
}