#pragma once

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <netinet/in.h>
#include "socket.hpp"
#include <poll.h>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>

namespace transport
{
    class Downloader
    {
    public:
        Downloader(const std::string &_ip, int _port, const std::string &_filename, int _file_size);
        ~Downloader() noexcept;
        Downloader(const Downloader &) = delete;
        Downloader &operator=(const Downloader &) = delete;
        void run();

    private:
        void send_requests();
        void listen_receive();
        void process_data(int start, int len, const std::string &payload);
        void flush_file();
        sockaddr_in server_addr;
        std::string ip;
        int port;
        std::string filename;
        int file_size;
        std::ofstream file_stream;
        UdpSocket socket;

        std::vector<char> buffer;
        std::map<int, int> chunks;

        int already_done = 0;
        int current_write_pos = 0;

        static constexpr int WINDOW_SIZE = 1200000;
        static constexpr int ROUND_TIME_MS = 300;
        static constexpr int MAX_DOWNLOAD = 1000;
    };
}