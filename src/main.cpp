#include <iostream>
#include <chrono>
#include <iomanip>
#include <string>
#include <sstream>
#include <cctype>

#include "move_generator/move_generation.h"
#include "perft/perft_testing.h"
#include "game.h"

#include "temp_cmd_manager.h"

#if ENABLE_DEBUG == 1
DEBUG_INIT;
#endif

#include "config.h"
#include "debug.h"

void perft_test(const std::vector<std::string>& args);
void detailed_perft_test(const std::vector<std::string>& args);
void speed_test(const std::vector<std::string>& args);
void uci_interface();

int main(int argc, char** argv)
{
    std::vector<std::string> args(argv, argv + argc);
    initializePrecomputedStuff();

    if ( argc > 1 ) {
        if ( args[1] == "-test" ) {
            PerftTestSuite perft_tests;
            perft_tests.runTests();
        }
        else if ( args[1] == "-perft" ) {
            perft_test(args);
        }
        else if ( args[1] == "-speed" ) {
            speed_test(args);
        }
        else if ( args[1] == "-perftd" ) {
            detailed_perft_test(args);
        }
        else {
            std::cout << "Usage:\n"
                << "-test" << '\n'
                << "-perft <depth> [\"fen\"|startpos] <expected>" << '\n'
                << "-speed <depth> [\"fen\"|startpos]" << '\n'
                << "-perftd <depth> [\"fen\"|startpos]"
                << '\n';
        }
    }
    else {
        uci_interface();
    }


#if ENABLE_DEBUG == 1
    printBenchmarkInfo();
#endif

    return 0;
}

void uci_interface()
{
    std::cout << '\n'
        << "  ********   **           *******     **     **\n"
        << " **//////   /**          **/////**   /**    /**\n"
        << "/**         /**         **     //**  /**    /**\n"
        << "/*********  /**        /**      /**  /**    /**\n"
        << "////////**  /**        /**      /**  /**    /**\n"
        << "       /**  /**        //**     **   /**    /**\n"
        << " ********   /********   //*******    //*******\n"
        << "////////    ////////     ///////      ///////\n\n"
        << "A chess egine by ama - 2024\n\n"
        << "try 'help' if you are lost <3\n\n";


    CommandManager cmd_manager;
    cmd_manager.parseCommand();
}

void detailed_perft_test(const std::vector<std::string>& args)
{
    const static std::string usage = "-perftd <depth> [\"fen\"|startpos]";
    if ( args.size() != 4 ) {
        std::cout << "usage: " << usage << '\n';
        return;
    }
    int depth = 0;
    try {
        depth = std::stoi(args[2]);
    }
    catch ( std::exception& e ) {
        std::cout << "\'depth\' must be a number!\n"
            << "usage: " << usage << '\n';
        return;
    }
    const std::string fen = args[3];
    Game game;
    try {
        game = Game(fen);
    }
    catch ( std::string& e ) {
        std::cout << e << '\n'
            << "usage: " << usage << '\n';
        return;
    }

    game.perftDetailEntry(depth);
}

// -perft <depth> ["fen"|startpos] <expected>
void perft_test(const std::vector<std::string>& args)
{
    const static std::string usage = "-perft <depth> [\"fen\"|startpos] <expected>";
    if ( args.size() < 4 || args.size() > 5 ) {
        std::cout << "usage: " << usage << '\n';
        return;
    }

    int depth = 0;
    try {
        depth = std::stoi(args[2]);
    }
    catch ( std::exception& e ) {
        std::cout << "\'depth\' must be a number!\n"
            << "usage: " << usage << '\n';
        return;
    }

    const std::string fen = args[3];

    Game game;
    try {
        game = Game(fen);
    }
    catch ( std::string& e ) {
        std::cout << e << '\n'
            << "usage: " << usage << '\n';
        return;
    }

    uint64_t perft_result = game.perftSimpleEntry(depth);

    if ( args.size() == 4 ) {
        std::cout << perft_result << '\n';
    }
    else {
        uint64_t expected = 0ULL;
        try {
            expected = std::stoull(args[4]);
        }
        catch ( std::exception& e ) {
            std::cout << "\'expected\' must be a number!\n"
                << "usage: " << usage << '\n';
            return;
        }

        if ( perft_result == expected ) {
            std::cout << GREEN << "passed: " << RESET << perft_result << '\n';
        }
        else {
            std::cout << RED << "failed: " << RESET << std::left << std::setw(10) << expected
                << std::left << std::setw(10) << perft_result
                << std::left << std::setw(10) << " "
                << "input: " << "-perft " << depth << " " << fen << " " << expected
                << '\n';
        }
    }
}

// -speed <depth> ["fen"|startpos]
void speed_test(const std::vector<std::string>& args)
{
    const static std::string usage = "-speed <depth> [\"fen\"|startpos]";
    if ( args.size() != 4 ) {
        std::cout << "usage: " << usage << '\n';
        return;
    }

    int depth = 0;
    try {
        depth = std::stoi(args[2]);
    }
    catch ( std::exception& e ) {
        std::cout << "\'depth\' must be a number!\n"
            << "usage: " << usage << '\n';
        return;
    }

    const std::string fen = args[3];

    Game game;
    try {
        game = Game(fen);
    }
    catch ( std::string& e ) {
        std::cout << e << '\n'
            << "usage: " << usage << '\n';
        return;
    }

    auto begin = std::chrono::high_resolution_clock::now();
    uint64_t perft_result = game.perftSimpleEntry(depth);
    auto end = std::chrono::high_resolution_clock::now();

    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    const auto nps = perft_result * 1000 / duration;

    std::cout << perft_result << " nodes in " << duration << "ms (" << nps << "nps)\n";
}