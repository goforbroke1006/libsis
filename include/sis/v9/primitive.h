//
// Created by goforbroke on 8/9/26.
//

#ifndef LIBSIS_SIS_V9_PRIMITIVE_H
#define LIBSIS_SIS_V9_PRIMITIVE_H

#include <cstdint>
#include <string>

namespace sis::v9 {
    struct String {
        // SIS strings are UTF-16LE.
        std::u16string value;

        [[nodiscard]] std::string as_str() const {
            return std::string(value.begin(), value.end());
        }
    };

    struct Uid {
        uint32_t value;
    };

    struct Version {
        uint32_t major;
        uint32_t minor;
        uint32_t build;

        [[nodiscard]] std::string
        as_str() const {
            return std::to_string(major) + "." + std::to_string(minor);
        }
    };

    struct Date {
        uint16_t year;
        uint8_t month;
        uint8_t day;

        [[nodiscard]] std::string as_str() const {
            return std::to_string(year)
                   + "-" + (month < 10 ? "0" : "") + std::to_string(month)
                   + "-" + (day < 10 ? "0" : "") + std::to_string(day);
        }
    };

    struct Time {
        uint8_t hours;
        uint8_t minutes;
        uint8_t seconds;

        [[nodiscard]] std::string as_str() const {
            return (hours < 10 ? "0" : "") + std::to_string(hours)
                   + ":" + (minutes < 10 ? "0" : "") + std::to_string(minutes)
                   + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds);
        }
    };

    struct DateTime {
        Date date;
        Time time;

        [[nodiscard]] std::string as_str() const {
            return date.as_str() + " " + time.as_str();
        }
    };
} //sis::v9

#endif //LIBSIS_SIS_V9_PRIMITIVE_H
