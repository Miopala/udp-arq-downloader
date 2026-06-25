#pragma once
#include <string>
#include <stdexcept>
#include "ip_validator.hpp"
namespace transport
{
    struct Config
    {
        std::string ip;
        int port;
        std::string filename;
        int file_size;

        static Config parse(int argc, char *argv[])
        {
            if (argc != 5)
            {
                throw std::runtime_error("Usage: transport <IPv4> <PORT> <filename> <file_size>");
            }

            std::string ip = argv[1];
            if (!transport::utils::validate_ipv4(ip))
            {
                throw std::invalid_argument("Invalid IPv4 address format specified.");
            }

            int port = std::stoi(argv[2]);
            if (port < 0 || port > 65535)
            {
                throw std::out_of_range("Port number must be between 0 and 65535.");
            }

            return Config{
                ip,
                port,
                argv[3],
                std::stoi(argv[4])};
        }
    };
}