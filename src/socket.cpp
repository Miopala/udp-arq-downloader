#include "socket.hpp"
namespace transport
{
    UdpSocket::UdpSocket(int protocol) : sock_fd(socket(AF_INET, SOCK_DGRAM, protocol))
    {
        if (sock_fd < 0)
        {
            throw std::system_error(errno, std::system_category(), "Failed to create UDP socket");
        }
    }

    UdpSocket::~UdpSocket() noexcept
    {
        if (sock_fd >= 0)
        {
            ::close(sock_fd);
        }
    }

    UdpSocket::UdpSocket(UdpSocket &&other) noexcept : sock_fd(std::exchange(other.sock_fd, -1)) {}

    UdpSocket &UdpSocket::operator=(UdpSocket &&other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        if (sock_fd >= 0)
        {
            close(sock_fd);
        }
        sock_fd = std::exchange(other.sock_fd, -1);
        return *this;
    }

    int UdpSocket::get_fd() const noexcept
    {
        return sock_fd;
    }

    void UdpSocket::send_to(const void *buffer, size_t length, const sockaddr_in &recipient)
    {
        ssize_t bytes_sent = ::sendto(
            sock_fd,
            buffer,
            length,
            0,
            reinterpret_cast<const sockaddr *>(&recipient),
            sizeof(recipient));

        if (bytes_sent < 0)
        {
            throw std::system_error(errno, std::system_category(), "Failed to send packet");
        }
    }

    ssize_t UdpSocket::recv_from(void *buffer, size_t length, sockaddr_in &sender, int flags)
    {
        socklen_t addr_len = sizeof(sender);

        ssize_t bytes_received = ::recvfrom(
            sock_fd,
            buffer,
            length,
            flags,
            reinterpret_cast<sockaddr *>(&sender),
            &addr_len);

        if (bytes_received < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
            {
                return -1;
            }
            throw std::system_error(errno, std::system_category(), "Failed to receive packet");
        }

        return bytes_received;
    }

}