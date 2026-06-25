#include "ip_validator.hpp"

namespace transport
{
    namespace utils
    {
        bool validate_ipv4(const std::string &ip_str) noexcept
        {
            if (std::count(ip_str.begin(), ip_str.end(), '.') != 3)
                return false;

            std::string_view ip_str_v(ip_str);
            while (!ip_str_v.empty())
            {
                auto first_dot_pos = ip_str_v.find('.');
                const size_t segment_len = (first_dot_pos == std::string_view::npos) ? ip_str_v.size() : first_dot_pos;
                if (segment_len < 1 || segment_len > 3)
                    return false;

                int parsed_value = 0;
                const auto [ptr, ec] = std::from_chars(ip_str_v.data(), ip_str_v.data() + segment_len, parsed_value);

                if (ec != std::errc{} || ptr != ip_str_v.data() + segment_len)
                {
                    return false;
                }

                if (parsed_value < 0 || parsed_value > MAX_IP_PART)
                {
                    return false;
                }

                if (first_dot_pos == std::string_view::npos)
                {
                    ip_str_v.remove_prefix(ip_str_v.size());
                }
                else
                {
                    ip_str_v.remove_prefix(first_dot_pos + 1);
                }
            }

            return true;
        }
    }
}