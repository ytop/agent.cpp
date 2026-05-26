// pie CLI entry point — Phase 1 stub with --version support via CLI11.
// Satisfies Req 1.3 (pie --version exits 0), Req 1.9 (no node/npm/bun/deno spawned).

#include <CLI/CLI.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

static constexpr const char* kVersion = "pie 0.1.0";

int main(int argc, char* argv[]) {
    CLI::App app{"pie — C++20 coding agent"};

    // Allow extras so that unrecognized flags/positionals don't error out yet.
    // Later tasks will wire up the full argument grammar.
    app.allow_extras(true);

    // --version / -v: print version string and exit 0.
    bool show_version = false;
    app.add_flag("-v,--version", show_version, "Print version and exit");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return app.exit(e);
    }

    if (show_version) {
        std::cout << kVersion << "\n";
        return EXIT_SUCCESS;
    }

    // If no mode-selecting flag was given, print a brief message for now.
    // Later tasks will wire up interactive mode, print mode, etc.
    std::cout << kVersion << " — use --help for usage information.\n";
    return EXIT_SUCCESS;
}
