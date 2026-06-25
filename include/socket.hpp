#pragma once

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <system_error>
#include <utility>
#include <unistd.h>
namespace transport
{

    class UdpSocket
    {
    public:
        explicit UdpSocket(int protocol = 0);
        ~UdpSocket() noexcept;

        UdpSocket(const UdpSocket &) = delete;
        UdpSocket &operator=(const UdpSocket &) = delete;
        UdpSocket(UdpSocket &&other) noexcept;
        UdpSocket &operator=(UdpSocket &&other) noexcept;

        int get_fd() const noexcept;
        void send_to(const void *buffer, size_t length, const sockaddr_in &recipient);
        ssize_t recv_from(void *buffer, size_t length, sockaddr_in &sender, int flags);

    private:
        int sock_fd = -1;
    };

}