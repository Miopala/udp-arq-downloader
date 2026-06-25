#pragma once

#include <string>
#include <vector>
#include "arpa/inet.h"
#include <algorithm>
#include <string_view>
#include <charconv>
namespace transport
{
    namespace utils
    {
        static constexpr int MAX_IP_PART = 255;
        bool validate_ipv4(const std::string &ip_str) noexcept;

    }
}