#include "downloader.hpp"

namespace transport
{

    Downloader::Downloader(const std::string &_ip, int _port, const std::string &_filename, int _file_size) : ip(_ip), port(_port), filename(_filename), file_size(_file_size),
                                                                                                              file_stream(_filename, std::ios::out | std::ios::binary), socket(0), buffer(WINDOW_SIZE * 1.2)
    {
        if (!file_stream.is_open())
        {
            throw std::runtime_error("Problem with opening file!");
        }

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr);
    }

    Downloader::~Downloader() noexcept
    {
        if (file_stream.is_open())
        {
            file_stream.close();
        }
    }

    void Downloader::run()
    {
        while (already_done < file_size)
        {
            send_requests();
            listen_receive();
        }
    }

    void Downloader::send_requests()
    {
        int sent = 0;
        int checkpoint = current_write_pos;
        while (sent < MAX_DOWNLOAD && checkpoint + already_done < file_size && checkpoint < WINDOW_SIZE)
        {
            int to_send = std::min(MAX_DOWNLOAD, file_size - already_done - checkpoint);

            if (chunks.find(checkpoint) == chunks.end())
            {
                std::string msg = "GET " + std::to_string(already_done + checkpoint) + " " + std::to_string(to_send) + "\n";
                // std::cerr << "[CLIENT] Sending: " << msg << std::endl;
                socket.send_to(msg.c_str(), msg.size(), server_addr);
                ++sent;
            }
            checkpoint += to_send;
        }
    }

    void Downloader::listen_receive()
    {
        struct pollfd ps;
        ps.fd = socket.get_fd();
        ps.events = POLLIN;

        int ready = poll(&ps, 1, ROUND_TIME_MS);
        if (ready <= 0)
            return;

        char recv_buf[4096];
        sockaddr_in sender;
        ssize_t datagram_len = socket.recv_from(recv_buf, sizeof(recv_buf), sender, 0);

        if (datagram_len < 5)
            return;

        std::string raw_data(recv_buf, datagram_len);
        if (raw_data.substr(0, 4) != "DATA")
            return;

        size_t space1 = raw_data.find(' ', 5);
        size_t space2 = raw_data.find('\n', space1);

        int start = std::stoi(raw_data.substr(5, space1 - 5));
        int len = std::stoi(raw_data.substr(space1 + 1, space2 - space1 - 1));
        std::string payload = raw_data.substr(space2 + 1);

        process_data(start, len, payload);
    }

    void Downloader::process_data(int start, int len, const std::string &payload)
    {
        int offset = start - already_done;
        if (offset < 0 || chunks.count(offset))
            return;

        chunks[offset] = len;

        std::memcpy(&buffer[offset], payload.data(), len);

        while (chunks.count(current_write_pos))
        {
            current_write_pos += chunks[current_write_pos];
        }

        if (current_write_pos + already_done >= file_size || current_write_pos >= WINDOW_SIZE)
        {
            flush_file();
        }
    }

    void Downloader::flush_file()
    {
        file_stream.write(buffer.data(), current_write_pos);
        already_done += current_write_pos;

        std::cerr << "DONE " << static_cast<double>(already_done) / static_cast<double>(file_size) << "\n";

        current_write_pos = 0;
        chunks.clear();
    }

}