/**
    @file LimeUtil.cpp
    @author Lime Microsystems
    @brief Command line test app
*/

#include <ConnectionRegistry.h>
#include <IConnection.h>
#include <iostream>
#include <cstdlib>

int main(void)
{
    std::cout << "Welcome to LimeUtil" << std::endl;

    std::cout << "Discovery available connections..." << std::endl;
    auto handles = ConnectionRegistry::findConnections();
    for (const auto &handle : handles)
    {
        std::cout << "  Found handle [" << handle.serialize() << "]" << std::endl;

        std::cout << "  Make connection... " << std::flush;
        auto conn = ConnectionRegistry::makeConnection(handle);
        std::cout << "OK" << std::endl;

        std::cout << "  Conn? " << size_t(conn) << std::endl;
        std::cout << "  IsOpen? " << conn->IsOpen() << std::endl;

        std::cout << "  Free connection... " << std::flush;
        ConnectionRegistry::freeConnection(conn);
        std::cout << "OK" << std::endl;
    }

    std::cout << "Done!" << std::endl;
    return EXIT_SUCCESS;
}
