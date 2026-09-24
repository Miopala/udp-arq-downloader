#pragma once

#include "socket.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <poll.h>
#include <string>
#include <string_view>
#include <vector>

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
        void process_datagram(std::string_view packet, const sockaddr_in &sender);
        void process_data(int start, int len, std::string_view payload);
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

        static constexpr int WINDOW_SIZE = 120000;
        static constexpr int ROUND_TIME_MS = 300;
        static constexpr int CHUNK_SIZE = 1000;
        static constexpr int MAX_SENT_PACKETS_BATCH = 100;
    };
} // namespace transport