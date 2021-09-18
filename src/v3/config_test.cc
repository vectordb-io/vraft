#include <iostream>
#include "config.h"

int main(int argc, char **argv) {
    auto s = vraft::Config::GetInstance().Init(argc, argv);
    assert(s.ok());

    if (argc < 2) {
        vraft::Config::GetInstance().PrintHelp();
        exit(0);
    }

    std::cout << vraft::Config::GetInstance().ToString() << std::endl;

    return 0;
}
