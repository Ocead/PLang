#include <stdlib.h>
#include <iostream>
#include <getopt.h>
#include <plangcli/cli.hpp>

#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
#endif

using namespace plang;

struct options {
    int color;
    int interactive;
    char word_order[4];
};

int main(int argc, char *argv[], char *envp[]) {
    using namespace plang;
    using namespace plang::op;

#if defined(_WIN32) || defined(WIN32)
    HANDLE h_out  = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dw_mode = 0;
    GetConsoleMode(h_out, &dw_mode);
    dw_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(h_out, dw_mode);
    SetConsoleOutputCP(65001);
#endif

    cli cli;

    auto it = envp;
    std::vector<char *> vec;
    while (*it != nullptr) {
        vec.push_back(*(it++));
    }

    return cli.prompt_loop();
}
