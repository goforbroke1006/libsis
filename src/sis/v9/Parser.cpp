//
// Created by goforbroke on 8/9/26.
//

#include "sis/v9/Parser.h"

#include <iostream>
#include <zlib.h>

namespace {
    std::vector<uint8_t> inflate_impl(const std::vector<uint8_t> &input, uint64_t uncompressedSize, int windowBits) {
        if (uncompressedSize > SIZE_MAX)
            throw std::runtime_error("Output too large");

        if (input.size() > UINT_MAX)
            throw std::runtime_error("Input too large");

        if (uncompressedSize > UINT_MAX)
            throw std::runtime_error(
                "Output too large for this one-shot inflater"
            );

        std::vector<uint8_t> output(
            static_cast<size_t>(uncompressedSize)
        );

        z_stream stream{};

        stream.next_in =
                const_cast<Bytef *>(
                    reinterpret_cast<const Bytef *>(input.data())
                );

        stream.avail_in =
                static_cast<uInt>(input.size());

        stream.next_out =
                reinterpret_cast<Bytef *>(output.data());

        stream.avail_out =
                static_cast<uInt>(output.size());

        int rc = inflateInit2(&stream, windowBits);

        if (rc != Z_OK)
            throw std::runtime_error("inflateInit2 failed");

        rc = ::inflate(&stream, Z_FINISH);

        const auto totalIn = stream.total_in;
        const auto totalOut = stream.total_out;

        std::string msg =
                stream.msg ? stream.msg : "";

        inflateEnd(&stream);

        if (rc != Z_STREAM_END) {
            throw std::runtime_error(
                "inflate failed: rc=" + std::to_string(rc) +
                ", total_in=" + std::to_string(totalIn) +
                ", total_out=" + std::to_string(totalOut) +
                ", msg=" + msg
            );
        }

        if (totalOut != uncompressedSize) {
            throw std::runtime_error(
                "SISX decompressed size mismatch: expected=" +
                std::to_string(uncompressedSize) +
                ", actual=" +
                std::to_string(totalOut)
            );
        }

        return output;
    }

    std::vector<uint8_t> zlib_inflate(const std::vector<uint8_t> &input, uint64_t uncompressedSize) {
        try {
            // Raw RFC 1951 DEFLATE
            return inflate_impl(
                input,
                uncompressedSize,
                -MAX_WBITS
            );
        } catch (const std::exception &rawError) {
            std::cerr
                    << "raw DEFLATE failed: "
                    << rawError.what()
                    << '\n';

            // Diagnostic fallback: RFC 1950 zlib wrapper
            return inflate_impl(
                input,
                uncompressedSize,
                MAX_WBITS
            );
        }
    }

    sis::v9::FieldHeader read_field_header(BinaryReader &reader) {
        const uint32_t raw_type = reader.read_u32_le();
        const uint32_t len = reader.read_u32_le();

        uint64_t length;
        if (len & 0x80000000u) {
            const uint32_t high = reader.read_u32_le();

            length = (static_cast<uint64_t>(high) << 31) | (len & 0x7FFFFFFFu);
        } else {
            length = len;
        }

        return sis::v9::FieldHeader::from_raw(raw_type, length);
    }
};

namespace {
    void assert_field_type(const sis::v9::Type gotType, const sis::v9::Type wantType) {
        if (gotType != wantType)
            throw std::runtime_error(
                std::string()
                + "unexpected type: "
                + "want: " + sis::v9::type_to_string(wantType)
                + ", "
                + "got: " + sis::v9::type_to_string(gotType)
            );
    }
}

sis::v9::File sis::v9::Parser::parse() {
    // 0x00  UID1 = 0x10201A7A
    // 0x04  UID2
    // 0x08  UID3
    // 0x0C  UID checksum
    sis::v9::Header header{};
    header.uid1 = reader_.read_u32_le();
    header.uid2 = reader_.read_u32_le();
    header.uid3 = reader_.read_u32_le();
    header.checksum = reader_.read_u32_le();

    if (reader_.position() != 16)
        throw std::runtime_error("SISXParser::parse(): expected 16 bytes offset");

    sis::v9::File result;

    result.header = header;

    // 0x10  SISContents
    result.contents = read_contents();

    return result;
}

sis::v9::FieldHeader sis::v9::Parser::read_field_header() {
    const uint32_t raw_type = reader_.read_u32_le();
    const uint32_t len = reader_.read_u32_le();

    uint64_t length;
    if (len & 0x80000000u) {
        const uint32_t high = reader_.read_u32_le();

        length = (static_cast<uint64_t>(high) << 31) | (len & 0x7FFFFFFFu);
    } else {
        length = len;
    }

    return sis::v9::FieldHeader::from_raw(raw_type, length);
}

sis::v9::Contents sis::v9::Parser::read_contents() {
    sis::v9::Contents contents{};

    const auto contentsHeader = read_field_header();

    if (contentsHeader.type != sis::v9::Type::Contents)
        throw std::runtime_error("Expected SISContents, got "
                                 + std::to_string(contentsHeader.type_num()) + " "
                                 + sis::v9::type_to_string(contentsHeader.type));

    while (!reader_.empty()) {
        const sis::v9::FieldHeader fieldHeader = read_field_header();

        switch (static_cast<sis::v9::Type>(fieldHeader.type)) {
            case sis::v9::Type::ControllerChecksum:
                contents.controllerChecksum = reader_.read_u16_le();
                break;
            case sis::v9::Type::DataChecksum:
                reader_.read_bytes(fieldHeader.length);
                // TODO:
                std::cerr << "fake reading for [DataChecksum]: " << fieldHeader.length << " bytes" << std::endl;
                break;

            // case SISX::Type::Controller:
            //     reader_.read_bytes(fieldHeader.length);
            //     contents.controller = {}; // TODO:
            //     std::cerr << "fake reading for [Controller]: " << fieldHeader.length << " bytes" << std::endl;
            //     break;

            case sis::v9::Type::Compressed:
                contents.compressed = {
                    .algorithm = reader_.read_u32_le(),
                    .uncompressedSize = reader_.read_u64_le(),
                };

                {
                    if (contents.compressed.algorithm == sis::v9::COMP_ALG_NONE) {
                        // parse controller children...
                        // TODO:
                        // reader_.read_bytes(ctrlHeader.length);
                    } else if (contents.compressed.algorithm == sis::v9::COMP_ALG_DEFLATE) {
                        if (fieldHeader.length < 12)
                            throw std::runtime_error("Invalid SISCompressed length");

                        auto compressedData = reader_.read_bytes(fieldHeader.length - 12);
                        auto inflate = zlib_inflate(
                            compressedData,
                            contents.compressed.uncompressedSize
                        );

                        BinaryReader inflateReader(inflate);
                        contents.compressed.controller = read_controller(inflateReader);
                    }
                }

                break;

            case sis::v9::Type::Data:
                reader_.read_bytes(fieldHeader.length);
                contents.data = {}; // TODO:
                std::cerr << "fake reading for [Data]: " << fieldHeader.length << " bits" << std::endl;
                break;

            default:
                // skip data block
                reader_.read_bytes(fieldHeader.length);
                std::cerr << "skip reading for UNKNOWN:"
                        << " type: " << fieldHeader.type_num() << ", "
                        << " type: " << sis::v9::type_to_string(fieldHeader.type) << ", "
                        << " length: " << fieldHeader.length << " bits" << ", "
                        << std::endl;
                break;
        }

        // SIS fields are aligned to 4-byte boundaries.
        const uint64_t padding = (4 - (fieldHeader.length % 4)) % 4;
        reader_.read_bytes(padding);
    }

    return contents;
}

sis::v9::Controller sis::v9::Parser::read_controller(BinaryReader &reader) {
    const auto ctrlFH = ::read_field_header(reader);
    assert_field_type(ctrlFH.type, sis::v9::Type::Controller);

    sis::v9::Controller result;


    //
    // Required fields, in order
    //
    result.info = read_info(reader);
    result.supportedOptions = read_supported_options(reader);
    result.supportedLanguages = read_supported_languages(reader);
    result.prerequisites = read_prerequisites(reader);
    result.properties = read_properties(reader);
    result.logo = read_optional_logo(reader);
    result.installBlock = read_install_block(reader);
    result.certChain = read_signature_certificate_chain(reader);
    result.dataIndex = read_data_index(reader);
    // TODO: read other properties

    return result;
}

sis::v9::Info sis::v9::Parser::read_info(BinaryReader &reader) {
    const auto fh = ::read_field_header(reader);
    assert_field_type(fh.type, sis::v9::Type::Info);

    BinaryReader inReader(reader.read_bytes(fh.length));

    sis::v9::Info info;

    info.uid = read_uid(inReader);
    info.vendorUniqueName = read_string(inReader);
    info.names = read_array_of_string(inReader);
    info.vendorNames = read_array_of_string(inReader);
    info.version = read_version(inReader);
    info.creationTime = read_datetime(inReader);
    info.installType = inReader.read_u8();
    info.installFlags = inReader.read_u8();

    skip_padding(reader, fh);

    return info;
}

sis::v9::SupportedOptions sis::v9::Parser::read_supported_options(BinaryReader &reader) {
    const auto fh = ::read_field_header(reader);
    assert_field_type(fh.type, sis::v9::Type::SupportedOptions);

    sis::v9::SupportedOptions supOpts;

    reader.read_bytes(fh.length); // TODO: write a real reading

    return supOpts;
}

sis::v9::SupportedLanguages sis::v9::Parser::read_supported_languages(BinaryReader &reader) {
    const auto fh = ::read_field_header(reader);
    assert_field_type (fh.type , sis::v9::Type::SupportedLanguages);


    sis::v9::SupportedLanguages supLangs;

    reader.read_bytes(fh.length); // TODO: write a real reading

    return supLangs;
}

sis::v9::Prerequisites sis::v9::Parser::read_prerequisites(BinaryReader &reader) {
    const auto fh = ::read_field_header(reader);
    assert_field_type (fh.type , sis::v9::Type::Prerequisites);


    sis::v9::Prerequisites pre;

    reader.read_bytes(fh.length); // TODO: write a real reading

    return pre;
}

sis::v9::Properties sis::v9::Parser::read_properties(BinaryReader &reader) {
    const auto fh = ::read_field_header(reader);
    assert_field_type (fh.type , sis::v9::Type::Properties);


    sis::v9::Properties props;

    reader.read_bytes(fh.length); // TODO: write a real reading

    return props;
}

std::optional<sis::v9::Logo> sis::v9::Parser::read_optional_logo(BinaryReader &reader) {
    const auto posBeforeReading = reader.position();
    const auto fh = ::read_field_header(reader);
    if (fh.type != sis::v9::Type::Logo) {
        reader.seek(posBeforeReading); // revert position back
        return std::optional<sis::v9::Logo>();
    }


    sis::v9::Logo logo;

    reader.read_bytes(fh.length); // TODO: write a real reading

    return std::optional<sis::v9::Logo>(logo);
}

sis::v9::InstallBlock sis::v9::Parser::read_install_block(BinaryReader &reader) {
    const auto fh = ::read_field_header(reader);
    assert_field_type (fh.type , sis::v9::Type::InstallBlock);


    sis::v9::InstallBlock insBlock;

    reader.read_bytes(fh.length); // TODO: write a real reading

    return insBlock;
}

sis::v9::SignatureCertificateChain sis::v9::Parser::read_signature_certificate_chain(
    BinaryReader &reader
) {
    const auto fh = ::read_field_header(reader);
    assert_field_type (fh.type , sis::v9::Type::SignatureCertChain);


    sis::v9::SignatureCertificateChain certChain;

    reader.read_bytes(fh.length); // TODO: write a real reading

    return certChain;
}

/*SISX::SignatureCertificateChain SISXParser::read_signature_certificate_chain_payload(
    BinaryReader &reader,
    const SISX::FieldHeader &header
) {
    SISX::SignatureCertificateChain result;

    reader.read_bytes(header.length); // TODO: write a real reading

    return result;
}*/

sis::v9::DataIndex sis::v9::Parser::read_data_index(BinaryReader &reader) {
    const auto fh = ::read_field_header(reader);
    assert_field_type (fh.type , sis::v9::Type::DataIndex);


    sis::v9::DataIndex dataIdx;

    reader.read_bytes(fh.length); // TODO: write a real reading

    return dataIdx;
}


sis::v9::Uid sis::v9::Parser::read_uid(BinaryReader &reader) {
    const auto fh = ::read_field_header(reader);
    assert_field_type (fh.type , sis::v9::Type::Uid);

    return {
        .value = reader.read_u32_le(),
    };
}

sis::v9::String sis::v9::Parser::read_string(BinaryReader &reader) {
    const auto strFH = ::read_field_header(reader);
    assert_field_type(strFH.type, sis::v9::Type::String);

    sis::v9::String result;

    // UTF-16 = 2 bytes per code unit.
    if ((strFH.length % 2) != 0) {
        throw std::runtime_error("Invalid SISString length");
    }

    const uint64_t charCount = strFH.length / 2;

    result.value.reserve(
        static_cast<std::size_t>(charCount)
    );

    for (uint64_t i = 0; i < charCount; ++i) {
        const uint16_t ch = reader.read_u16_le();

        result.value.push_back(
            static_cast<char16_t>(ch)
        );
    }

    return result;
}

sis::v9::String sis::v9::Parser::read_string_payload(BinaryReader &reader, const sis::v9::FieldHeader &header) {
    sis::v9::String result;

    // UTF-16 = 2 bytes per code unit.
    if ((header.length % 2) != 0) {
        throw std::runtime_error("Invalid SISString length");
    }

    const uint64_t charCount = header.length / 2;

    result.value.reserve(
        static_cast<std::size_t>(charCount)
    );

    for (uint64_t i = 0; i < charCount; ++i) {
        const uint16_t ch = reader.read_u16_le();

        result.value.push_back(
            static_cast<char16_t>(ch)
        );
    }

    return result;
}


sis::v9::Version sis::v9::Parser::read_version(BinaryReader &reader) {
    const auto fh = ::read_field_header(reader);
    assert_field_type(fh.type, sis::v9::Type::Version);

    sis::v9::Version result{
        .major = reader.read_u32_le(),
        .minor = reader.read_u32_le(),
        .build = reader.read_u32_le(),
    };

    return result;
}

sis::v9::DateTime sis::v9::Parser::read_datetime(BinaryReader &reader) {
    const auto fh = ::read_field_header(reader);
    assert_field_type(fh.type, sis::v9::Type::DateTime);

    sis::v9::DateTime result{
        .date = sis::v9::Parser::read_date(reader),
        .time = sis::v9::Parser::read_time(reader),
    };

    return result;
}

sis::v9::Date sis::v9::Parser::read_date(BinaryReader &reader) {
    const auto fh = ::read_field_header(reader);
    assert_field_type(fh.type, sis::v9::Type::Date);

    sis::v9::Date result{
        .year = reader.read_u16_le(),
        .month = reader.read_u8(),
        .day = reader.read_u8(),
    };

    return result;
}

sis::v9::Time sis::v9::Parser::read_time(BinaryReader &reader) {
    const auto fh = ::read_field_header(reader);
    assert_field_type (fh.type , sis::v9::Type::Time);

    sis::v9::Time result{
        .hours = reader.read_u8(),
        .minutes = reader.read_u8(),
        .seconds = reader.read_u8(),
    };

    return result;
}

std::vector<sis::v9::String>
sis::v9::Parser::read_array_of_string(BinaryReader &reader) {
    return read_array<sis::v9::String>(
        reader,
        sis::v9::Type::String,
        sis::v9::Parser::read_string_payload
    );
}

/*std::vector<SISX::SignatureCertificateChain>
SISXParser::read_array_of_signature_certificate_chain(BinaryReader &reader) {
    return read_array<SISX::SignatureCertificateChain>(
        reader,
        SISX::Type::SignatureCertChain, SISXParser
        ::read_signature_certificate_chain_payload
    );
}*/

template<typename T, typename PayloadReader>
std::vector<T>
sis::v9::Parser::read_array(
    BinaryReader &reader,
    sis::v9::Type expectedElementType,
    PayloadReader &&readPayload
) {
    const auto fh = ::read_field_header(reader);

    assert_field_type (fh.type , sis::v9::Type::Array);

    const uint64_t arrayEnd = reader.position() + fh.length;

    //
    // SISArray specifies the element type once.
    //
    const uint32_t rawElementType = reader.read_u32_le();

    const auto elementType = static_cast<sis::v9::Type>(rawElementType);

    if (elementType != expectedElementType) {
        throw std::runtime_error(std::string("Unexpected SISArray element type -")
                                 + " want " + sis::v9::type_to_string(expectedElementType)
                                 + " got " + sis::v9::type_to_string(elementType)
        );
    }

    std::vector<T> result;

    while (reader.position() < arrayEnd) {
        //
        // Array element does NOT contain its type.
        // It starts with its length.
        //
        const auto elementHeader =
                read_array_element_header(
                    reader,
                    elementType
                );

        result.push_back(
            readPayload(
                reader,
                elementHeader
            )
        );
    }


    return result;
}

sis::v9::FieldHeader
sis::v9::Parser::read_array_element_header(BinaryReader &reader, sis::v9::Type elementType) {
    const uint32_t low =
            reader.read_u32_le();

    uint64_t length;

    if ((low & 0x80000000u) == 0) {
        length = low;
    } else {
        const uint32_t high =
                reader.read_u32_le();

        length =
                (static_cast<uint64_t>(high) << 31) |
                (low & 0x7FFFFFFFu);
    }

    return sis::v9::FieldHeader{
        .type = elementType,
        .length = length
    };
}

void sis::v9::Parser::skip_padding(BinaryReader &reader, const sis::v9::FieldHeader &lastHeader) {
    // SIS fields are aligned to 4-byte boundaries.
    const uint64_t padding = (4 - (lastHeader.length % 4)) % 4;
    reader.read_bytes(padding);
}
