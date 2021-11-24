#include "HttpProxy.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "you must enter port number" << std::endl;
        return 0;
    }

    try {
        auto proxy = new HttpProxy(std::stoi(argv[1]));
        proxy->start();
    } catch (std::invalid_argument &exception) {
        std::cerr << "port must be an integer value" << std::endl;
    }
    return 0;
}