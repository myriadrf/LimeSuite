/**
    @file LimeUtil.cpp
    @author Lime Microsystems
    @brief Command line test app
*/

#include "ConnectionRegistry.h"
#include <iostream>
#include <cstdlib>

void __loadConnectionEVB7COMEntry(void); //TODO fixme replace with LoadLibrary/dlopen

int main(void)
{
    std::cout << "Welcome to LimeUtil" << std::endl;

    __loadConnectionEVB7COMEntry();

    std::cout << "Discovery available connections..." << std::endl;
    auto handles = ConnectionRegistry::findConnections();
    for (const auto &handle : handles)
    {
        std::cout << "Handle " << handle.serialize() << std::endl;
    }

    std::cout << "Done!" << std::endl;
    return EXIT_SUCCESS;
}
