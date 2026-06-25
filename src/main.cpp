#include <iostream>
#include <exception>
#include "config.hpp"
#include "downloader.hpp"
#include <string>

int main(int argc, char *argv[])
{
    try
    {
        transport::Config conf = transport::Config::parse(argc, argv);
        transport::Downloader engine(conf.ip, conf.port, conf.filename, conf.file_size);
        engine.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Configuration error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}