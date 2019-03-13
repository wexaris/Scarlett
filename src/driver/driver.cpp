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

#define LOG_TIMED(X, Y) LOG(FMT(X << " : DONE (" << std::fixed << std::setprecision(2) << timed_exec(Y) << "ms)"))


//////////////
/// DRIVER ///
//////////////

void Driver::compile() {
    // Fail compilation if no input file was given
    if (sess->infile.empty()) {
        ERR("Input file missing");
    }

    LOG_TIMED("Read file chars", [&]() {
        UTFBumpReader reader(sess->infile);
        while (!reader.is_eof()) {
            LOG(reader.curr() << " (" << reader.get_pos() << ":" << reader.get_pos().idx << ")");
            reader.bump();
        }
        reader.free_sf();
    });

    LOG_TIMED("Read lexer chars", [&]() {
        Lexer lex(sess->infile);
        Token tok = lex.next_token();
        while (!tok.is_eof()) {
            LOG(tok);
            tok = lex.next_token();;
        }
        lex.free_sf();
    });


    Parser parser = Parser(sess->infile);

    parser.parse();
}