#include "downloader.hpp"
#include <algorithm>
#include <charconv>

namespace transport
{

    Downloader::Downloader(const std::string &_ip, int _port, const std::string &_filename, int _file_size)
        : ip(_ip), port(_port), filename(_filename), file_size(_file_size),
          file_stream(_filename, std::ios::out | std::ios::binary), socket(0), buffer(WINDOW_SIZE)
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
        while (sent < MAX_SENT_PACKETS_BATCH && checkpoint + already_done < file_size && checkpoint < WINDOW_SIZE)
        {
            int to_send = std::min(CHUNK_SIZE, file_size - already_done - checkpoint);

            if (chunks.find(checkpoint) == chunks.end())
            {
                std::string msg =
                    "GET " + std::to_string(already_done + checkpoint) + " " + std::to_string(to_send) + "\n";
                // std::cerr << "[CLIENT] Sending: " << msg << std::endl;
                socket.send_to(msg.c_str(), msg.size(), server_addr);
                ++sent;
            }
            checkpoint += to_send;
        }
    }

    void Downloader::listen_receive()
    {
        struct pollfd ps = {};
        ps.fd = socket.get_fd();
        ps.events = POLLIN;

        int ready = poll(&ps, 1, ROUND_TIME_MS);
        if (ready <= 0 || !(ps.revents & POLLIN))
            return;

        while (true)
        {
            char recv_buf[4096];
            sockaddr_in sender{};
            ssize_t datagram_len = socket.recv_from(recv_buf, sizeof(recv_buf), sender, MSG_DONTWAIT);

            if (datagram_len < 0)
                break;

            process_datagram(std::string_view(recv_buf, static_cast<std::size_t>(datagram_len)), sender);
        }
    }

    void Downloader::process_datagram(std::string_view packet, const sockaddr_in &sender)
    {
        if (packet.size() < 5)
            return;

        if (sender.sin_family != server_addr.sin_family || sender.sin_port != server_addr.sin_port ||
            sender.sin_addr.s_addr != server_addr.sin_addr.s_addr)
        {
            return;
        }

        constexpr std::string_view prefix = "DATA ";
        if (!packet.starts_with(prefix))
            return;

        const std::size_t separator = packet.find(' ', prefix.size());
        if (separator == std::string_view::npos)
            return;

        const std::size_t newline = packet.find('\n', separator + 1);
        if (newline == std::string_view::npos)
            return;

        const std::string_view start_text = packet.substr(prefix.size(), separator - prefix.size());
        const std::string_view len_text = packet.substr(separator + 1, newline - separator - 1);
        const std::string_view payload = packet.substr(newline + 1);

        auto parse_int = [](std::string_view text, int &value)
        {
            const char *begin = text.data();
            const char *end = begin + text.size();

            const auto [ptr, ec] = std::from_chars(begin, end, value);
            return ec == std::errc{} && ptr == end;
        };

        int start = 0;
        int len = 0;

        if (!parse_int(start_text, start) || !parse_int(len_text, len))
            return;

        if (len <= 0)
            return;

        if (payload.size() != static_cast<std::size_t>(len))
            return;

        if (start < already_done || start > file_size)
            return;

        if (len > file_size - start)
            return;

        const int offset = start - already_done;

        if (offset >= WINDOW_SIZE || len > WINDOW_SIZE - offset)
            return;

        process_data(start, len, payload);
    }

    void Downloader::process_data(int start, int len, std::string_view payload)
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

        const double progress = 100.0 * static_cast<double>(already_done) / static_cast<double>(file_size);

        std::cerr << "Downloaded: " << progress << "%\n";

        current_write_pos = 0;
        chunks.clear();
    }

} // namespace transport