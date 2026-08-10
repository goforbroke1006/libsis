//
// Created by goforbroke on 8/9/26.
//

#include "../../../include/sis/legacy/Parser.h"

#include <iostream>

sis::legacy::SisFile sis::legacy::Parser::parse() {
    const auto header = this->read_header();
    reader_.seek(header.filesPointer);
    // const auto files = this->read_content();

    return sis::legacy::SisFile{
        .header = header,
        // .files = files,
    };
}

sis::legacy::SisHeader sis::legacy::Parser::read_header() {
    SisHeader h{
        reader_.read_u32_le(),
        reader_.read_u32_le(),
        reader_.read_u32_le(),
        reader_.read_u32_le(),
        reader_.read_u16_le(),
        reader_.read_u16_le(),
        reader_.read_u16_le(),
        reader_.read_u16_le(),
        reader_.read_u16_le(),
        reader_.read_u16_le(),
        reader_.read_u16_le(),
        reader_.read_u16_le(),
        reader_.read_u32_le(),
        reader_.read_u16_le(),
        reader_.read_u16_le(),
        reader_.read_u16_le(),
        reader_.read_u16_le(),
        reader_.read_u32_le(),
        reader_.read_u32_le(),
        reader_.read_u32_le(),
        reader_.read_u32_le(),
        reader_.read_u32_le(),
        reader_.read_u32_le(),

        reader_.read_u32_le(),
        reader_.read_u32_le(),
        reader_.read_u32_le(),
        reader_.read_u32_le(),
        reader_.read_bytes(16),
    };

    // if (h.uid1 != SIS_UID1)
    //     throw std::runtime_error("Not a SISX file");

    return h;
}

sis::legacy::SisField sis::legacy::Parser::read_field() {
    SisField f;

    f.type = reader_.read_u32_le();
    f.length = reader_.read_u32_le();

    std::cout << "  Field: " << std::endl;
    std::cout << "    offset: 0x"
            << std::hex << reader_.position()
            << std::dec << '\n';
    std::cout << "    type   = 0x"
            << std::hex << f.type
            << std::dec << '\n';
    std::cout << "    length = 0x"
            << std::hex << f.length
            << std::dec << " (" << f.length << ")\n";

    f.data = reader_.read_bytes(f.length);


    return f;
}


std::vector<sis::legacy::SISFileDescription>
sis::legacy::Parser::read_content() {
    std::vector<SISFileDescription> result;

    while (reader_.remaining() > 0) {
        const auto field = this->read_field();

        if (field.type == static_cast<uint32_t>(SisType::Date)) {
            uint16_t year = reader_.read_u16_le();
            uint8_t month = reader_.read_u8();
            uint8_t day = reader_.read_u8();

            std::cout << "  Date: "
                    << year << "-" << month << "-" << day << std::endl;
        }

        // if (field.type == SisType::FileDescription)
        //     parse_file_description();
    }

    return result;
}
