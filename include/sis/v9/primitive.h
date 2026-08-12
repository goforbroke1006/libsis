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
        std::string str;
    };

    struct Uid {
        uint32_t value;
    };

    struct Version {
        uint32_t major;
        uint32_t minor;
        uint32_t build;
    };

    struct Date {
        uint16_t year;
        uint8_t month;
        uint8_t day;
    };

    struct Time {
        uint8_t hours;
        uint8_t minutes;
        uint8_t seconds;
    };

    struct DateTime {
        Date date;
        Time time;
    };
} //sis::v9

#endif //LIBSIS_SIS_V9_PRIMITIVE_H
