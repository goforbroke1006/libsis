//
// Created by goforbroke on 8/9/26.
//

#include "sis/v9/Parser.h"
#include "sis/v9/zlib_inflate.h"

#include <iostream>

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

sis::v9::File sis::v9::Parser::parse(BinaryReader &reader) {
    // 0x00  UID1 = 0x10201A7A
    // 0x04  UID2
    // 0x08  UID3
    // 0x0C  UID checksum
    sis::v9::Header header{};
    header.uid1 = FileUID32(reader.read_u32_le());
    header.uid2 = FileUID32(reader.read_u32_le());
    header.uid3 = FileUID32(reader.read_u32_le());
    header.checksum = FileUID32(reader.read_u32_le());

    if (reader.position() != 16)
        throw std::runtime_error("SISXParser::parse(): expected 16 bytes offset");

    sis::v9::File result;

    result.header = header;

    // 0x10  SISContents
    result.contents = read_contents(reader);

    return result;
}

sis::v9::FieldHeader sis::v9::Parser::read_field_header(BinaryReader &reader) {
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

sis::v9::Contents sis::v9::Parser::read_contents(BinaryReader &reader) {
    sis::v9::Contents contents{};

    const auto contentsHeader = read_field_header(reader);

    if (contentsHeader.type != sis::v9::Type::Contents)
        throw std::runtime_error("Expected SISContents, got "
                                 + std::to_string(contentsHeader.type_num()) + " "
                                 + sis::v9::type_to_string(contentsHeader.type));

    while (!reader.empty()) {
        const sis::v9::FieldHeader fieldHeader = read_field_header(reader);

        switch (static_cast<sis::v9::Type>(fieldHeader.type)) {
            case sis::v9::Type::ControllerChecksum:
                contents.controllerChecksum = reader.read_u16_le();
                skip_padding(reader, fieldHeader);
                break;
            case sis::v9::Type::DataChecksum:
                reader.read_bytes(fieldHeader.length);
                // TODO:
                std::cerr << "fake reading for [DataChecksum]: " << fieldHeader.length << " bytes" << std::endl;
                skip_padding(reader, fieldHeader);
                break;

            case sis::v9::Type::Compressed:
                contents.compressed = read_compressed_payload(reader, fieldHeader);

                {
                    if (contents.compressed.algorithm == sis::v9::CompressedAlgorithm::COMP_ALG_NONE) {
                        // parse controller children...
                        // TODO:
                        // reader.read_bytes(ctrlHeader.length);
                    } else if (contents.compressed.algorithm == sis::v9::CompressedAlgorithm::COMP_ALG_DEFLATE) {
                        if (fieldHeader.length < 12)
                            throw std::runtime_error("Invalid SISCompressed length");


                        auto inflate = zlib_inflate(
                            contents.compressed.compressedData,
                            contents.compressed.uncompressedSize
                        );

                        BinaryReader inflateReader(inflate);
                        contents.controller = read_controller(inflateReader);
                    }
                }

                break;

            case sis::v9::Type::Data:
                contents.data = read_data_payload(reader, fieldHeader);
                skip_padding(reader, fieldHeader);
                break;

            default:
                // skip data block
                reader.read_bytes(fieldHeader.length);
                std::cerr << "skip reading for UNKNOWN:"
                        << " type: " << fieldHeader.type_num() << ", "
                        << " type: " << sis::v9::type_to_string(fieldHeader.type) << ", "
                        << " length: " << fieldHeader.length << " bits" << ", "
                        << std::endl;
                skip_padding(reader, fieldHeader);
                break;
        }
    }

    return contents;
}

sis::v9::Compressed sis::v9::Parser::read_compressed(BinaryReader &reader) {
    const auto fh = read_field_header(reader);
    assert_field_type(fh.type, sis::v9::Type::Compressed);

    const auto payload = read_compressed_payload(reader, fh);

    return payload;
}

sis::v9::Compressed sis::v9::Parser::read_compressed_payload(BinaryReader &reader, const FieldHeader &header) {
    sis::v9::Compressed result;

    result.algorithmRaw = reader.read_u32_le();
    result.algorithm = static_cast<sis::v9::CompressedAlgorithm>(result.algorithmRaw);
    result.uncompressedSize = reader.read_u64_le();

    if (header.length < 12)
        throw std::runtime_error("Invalid SISCompressed length");
    result.compressedData = reader.read_bytes(header.length - 12);

    return result;
}

sis::v9::Controller sis::v9::Parser::read_controller(BinaryReader &reader) {
    const auto fh = read_field_header(reader);
    assert_field_type(fh.type, sis::v9::Type::Controller);

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

    skip_padding(reader, fh);

    return result;
}

sis::v9::Data sis::v9::Parser::read_data_payload(BinaryReader &reader, const FieldHeader &header) {
    sis::v9::Data result;

    result.units = read_array_of_data_unit(reader);

    return result;
}

sis::v9::Info sis::v9::Parser::read_info(BinaryReader &reader) {
    const auto fh = read_field_header(reader);
    assert_field_type(fh.type, sis::v9::Type::Info);

    BinaryReader inReader(reader.read_bytes(fh.length));

    sis::v9::Info result;

    result.uid = read_uid(inReader);
    result.vendorUniqueName = read_string(inReader);
    result.names = read_array_of_string(inReader);
    result.vendorNames = read_array_of_string(inReader);
    result.version = read_version(inReader);
    result.creationTime = read_datetime(inReader);
    result.installType = inReader.read_u8();
    result.installFlags = inReader.read_u8();

    skip_padding(reader, fh);

    return result;
}

sis::v9::SupportedOptions sis::v9::Parser::read_supported_options(BinaryReader &reader) {
    const auto fh = read_field_header(reader);
    assert_field_type(fh.type, sis::v9::Type::SupportedOptions);

    sis::v9::SupportedOptions supOpts;

    reader.read_bytes(fh.length); // TODO: write a real reading

    skip_padding(reader, fh);

    return supOpts;
}

sis::v9::SupportedLanguages sis::v9::Parser::read_supported_languages(BinaryReader &reader) {
    const auto fh = read_field_header(reader);
    assert_field_type(fh.type, sis::v9::Type::SupportedLanguages);


    sis::v9::SupportedLanguages supLangs;

    reader.read_bytes(fh.length); // TODO: write a real reading

    skip_padding(reader, fh);

    return supLangs;
}

sis::v9::Prerequisites sis::v9::Parser::read_prerequisites(BinaryReader &reader) {
    const auto fh = read_field_header(reader);
    assert_field_type(fh.type, sis::v9::Type::Prerequisites);


    sis::v9::Prerequisites pre;

    reader.read_bytes(fh.length); // TODO: write a real reading

    skip_padding(reader, fh);

    return pre;
}

sis::v9::Properties sis::v9::Parser::read_properties(BinaryReader &reader) {
    const auto fh = read_field_header(reader);
    assert_field_type(fh.type, sis::v9::Type::Properties);


    sis::v9::Properties props;

    reader.read_bytes(fh.length); // TODO: write a real reading

    skip_padding(reader, fh);

    return props;
}

std::optional<sis::v9::Logo> sis::v9::Parser::read_optional_logo(BinaryReader &reader) {
    const auto posBeforeReading = reader.position();
    const auto fh = read_field_header(reader);
    if (fh.type != sis::v9::Type::Logo) {
        reader.seek(posBeforeReading); // revert position back
        return std::optional<sis::v9::Logo>();
    }


    sis::v9::Logo logo{
        .fileDescription = read_file_description(reader),
    };

    skip_padding(reader, fh);

    return std::optional<sis::v9::Logo>(logo);
}

sis::v9::InstallBlock sis::v9::Parser::read_install_block(BinaryReader &reader) {
    const auto fh = read_field_header(reader);
    assert_field_type(fh.type, sis::v9::Type::InstallBlock);

    sis::v9::InstallBlock insBlock;
    insBlock.files = read_array_of_file_description(reader);
    insBlock.embeddedControllers = read_array_of_controller(reader);
    insBlock.ifBlocks = read_array_of_if(reader);

    skip_padding(reader, fh);

    return insBlock;
}

sis::v9::FileDescription sis::v9::Parser::read_file_description(BinaryReader &reader) {
    const auto fh = read_field_header(reader);
    assert_field_type(fh.type, sis::v9::Type::FileDescription);

    const auto payload = read_file_description_payload(reader, fh);

    skip_padding(reader, fh);

    return payload;
}

sis::v9::SignatureCertificateChain sis::v9::Parser::read_signature_certificate_chain(
    BinaryReader &reader
) {
    const auto fh = read_field_header(reader);
    assert_field_type(fh.type, sis::v9::Type::SignatureCertChain);

    sis::v9::SignatureCertificateChain certChain;

    reader.read_bytes(fh.length); // TODO: write a real reading

    skip_padding(reader, fh);

    return certChain;
}

sis::v9::DataIndex sis::v9::Parser::read_data_index(BinaryReader &reader) {
    const auto fh = read_field_header(reader);
    assert_field_type(fh.type, sis::v9::Type::DataIndex);

    sis::v9::DataIndex dataIdx{};

    reader.read_bytes(fh.length); // TODO: write a real reading

    skip_padding(reader, fh);

    return dataIdx;
}


sis::v9::Uid sis::v9::Parser::read_uid(BinaryReader &reader) {
    const auto fh = read_field_header(reader);
    assert_field_type(fh.type, sis::v9::Type::Uid);

    return {
        .value = reader.read_u32_le(),
    };
}

sis::v9::String sis::v9::Parser::read_string(BinaryReader &reader) {
    const auto fh = read_field_header(reader);
    assert_field_type(fh.type, sis::v9::Type::String);

    const auto result = read_string_payload(reader, fh);

    /*// UTF-16 = 2 bytes per code unit.
    if ((fh.length % 2) != 0) {
        throw std::runtime_error("Invalid SISString length");
    }

    const uint64_t charCount = fh.length / 2;

    result.value.reserve(
        static_cast<std::size_t>(charCount)
    );

    for (uint64_t i = 0; i < charCount; ++i) {
        const uint16_t ch = reader.read_u16_le();

        result.value.push_back(
            static_cast<char16_t>(ch)
        );
    }*/

    skip_padding(reader, fh);

    return result;
}

sis::v9::String
sis::v9::Parser::read_string_payload(BinaryReader &reader, const sis::v9::FieldHeader &header) {
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

    result.str = std::string(result.value.begin(), result.value.end());

    return result;
}

sis::v9::FileDescription
sis::v9::Parser::read_file_description_payload(BinaryReader &reader, const sis::v9::FieldHeader &header) {
    sis::v9::FileDescription result;

    result.target = read_string(reader);
    result.mimeType = read_string(reader);
    result.capabilities = read_opt_capabilities(reader);
    result.hash = read_hash(reader);
    result.operation = reader.read_u32_le();
    result.operationOptions = reader.read_u32_le();
    result.length = reader.read_u64_le();
    result.uncompressedLength = reader.read_u64_le();
    result.dataIndex = reader.read_u32_le();

    return result;
}

sis::v9::Controller
sis::v9::Parser::read_controller_payload(BinaryReader &reader, const sis::v9::FieldHeader &header) {
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

    return result;
}

sis::v9::If sis::v9::Parser::read_if_payload(BinaryReader &reader, const sis::v9::FieldHeader &header) {
    sis::v9::If result;
    result.expression = read_expression(reader);
    result.installBlock = std::make_unique<sis::v9::InstallBlock>(read_install_block(reader));
    result.elseIfs = read_array_of_elseif(reader);

    return result;
}

sis::v9::ElseIf sis::v9::Parser::read_elseif_payload(BinaryReader &reader, const sis::v9::FieldHeader &header) {
    sis::v9::ElseIf result;
    result.expression = read_expression(reader);
    result.installBlock = std::make_unique<sis::v9::InstallBlock>(read_install_block(reader));

    return result;
}

sis::v9::DataUnit
sis::v9::Parser::read_data_unit_payload(BinaryReader &reader, const sis::v9::FieldHeader &header) {
    sis::v9::DataUnit result;
    result.files = read_array_of_file_data(reader);

    return result;
}

sis::v9::FileData
sis::v9::Parser::read_file_data_payload(BinaryReader &reader, const sis::v9::FieldHeader &header) {
    const auto posBefore = reader.position();

    sis::v9::FileData result;
    result.compressed = read_compressed(reader);

    reader.seek(posBefore + header.length);

    return result;
}


std::optional<sis::v9::Capabilities>
sis::v9::Parser::read_opt_capabilities(BinaryReader &reader) {
    const auto posBefore = reader.position();

    const auto fh = read_field_header(reader);
    if (fh.type != sis::v9::Type::Capabilities) {
        reader.seek(posBefore); // revert offset
        return std::optional<sis::v9::Capabilities>{};
    }

    sis::v9::Capabilities result;

    reader.read_bytes(fh.length); // TODO: add a real reading

    skip_padding(reader, fh);

    return result;
}

sis::v9::Expression sis::v9::Parser::read_expression(BinaryReader &reader) {
    const auto fh = read_field_header(reader);
    assert_field_type(fh.type, sis::v9::Type::Expression);

    const auto blockEnd = reader.position() + fh.length;

    sis::v9::Expression result;
    result.operatorTypeRaw = reader.read_u32_le();
    result.operatorType = static_cast<sis::v9::ExpressionOperator>(result.operatorTypeRaw);

    switch (result.operatorType) {
        case sis::v9::ExpressionOperator::Equal:
        case sis::v9::ExpressionOperator::NotEqual:
        case sis::v9::ExpressionOperator::GreaterThan:
        case sis::v9::ExpressionOperator::LessThan:
        case sis::v9::ExpressionOperator::GreaterOrEqual:
        case sis::v9::ExpressionOperator::LessOrEqual:
            result.left = std::make_unique<Expression>(read_expression(reader));
            result.right = std::make_unique<Expression>(read_expression(reader));
            break;
        default:
            result.left = nullptr;
            result.right = nullptr;
            break;
    }

    switch (result.operatorType) {
        case sis::v9::ExpressionOperator::Number:
        case sis::v9::ExpressionOperator::Option:
        case sis::v9::ExpressionOperator::Variable:
            result.integerValue = static_cast<int32_t>(reader.read_u32_le());
            break;

        case sis::v9::ExpressionOperator::String:
            result.stringValue = read_string(reader);
            break;

        default:
            break;
    }

    reader.seek(blockEnd); // TODO: implement correctly

    skip_padding(reader, fh);

    return result;
}

sis::v9::Hash sis::v9::Parser::read_hash(BinaryReader &reader) {
    const auto fh = read_field_header(reader);
    assert_field_type(fh.type, sis::v9::Type::Hash);

    sis::v9::Hash result;

    reader.read_bytes(fh.length); // TODO: add a real reading

    skip_padding(reader, fh);

    return result;
}

sis::v9::Version sis::v9::Parser::read_version(BinaryReader &reader) {
    const auto fh = read_field_header(reader);
    assert_field_type(fh.type, sis::v9::Type::Version);

    sis::v9::Version result{
        .major = reader.read_u32_le(),
        .minor = reader.read_u32_le(),
        .build = reader.read_u32_le(),
    };

    skip_padding(reader, fh);

    return result;
}

sis::v9::DateTime sis::v9::Parser::read_datetime(BinaryReader &reader) {
    const auto fh = read_field_header(reader);
    assert_field_type(fh.type, sis::v9::Type::DateTime);

    sis::v9::DateTime result{
        .date = sis::v9::Parser::read_date(reader),
        .time = sis::v9::Parser::read_time(reader),
    };

    skip_padding(reader, fh);

    return result;
}

sis::v9::Date sis::v9::Parser::read_date(BinaryReader &reader) {
    const auto fh = read_field_header(reader);
    assert_field_type(fh.type, sis::v9::Type::Date);

    sis::v9::Date result{
        .year = reader.read_u16_le(),
        .month = reader.read_u8(),
        .day = reader.read_u8(),
    };

    skip_padding(reader, fh);

    return result;
}

sis::v9::Time sis::v9::Parser::read_time(BinaryReader &reader) {
    const auto fh = read_field_header(reader);
    assert_field_type(fh.type, sis::v9::Type::Time);

    sis::v9::Time result{
        .hours = reader.read_u8(),
        .minutes = reader.read_u8(),
        .seconds = reader.read_u8(),
    };

    skip_padding(reader, fh);

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

std::vector<sis::v9::FileDescription>
sis::v9::Parser::read_array_of_file_description(BinaryReader &reader) {
    return read_array<sis::v9::FileDescription>(
        reader,
        sis::v9::Type::FileDescription,
        sis::v9::Parser::read_file_description_payload
    );
}

std::vector<sis::v9::Controller>
sis::v9::Parser::read_array_of_controller(BinaryReader &reader) {
    return read_array<sis::v9::Controller>(
        reader,
        sis::v9::Type::Controller,
        sis::v9::Parser::read_controller_payload
    );
}

std::vector<sis::v9::If>
sis::v9::Parser::read_array_of_if(BinaryReader &reader) {
    return read_array<sis::v9::If>(
        reader,
        sis::v9::Type::If,
        sis::v9::Parser::read_if_payload
    );
}

std::vector<sis::v9::ElseIf> sis::v9::Parser::read_array_of_elseif(BinaryReader &reader) {
    return read_array<sis::v9::ElseIf>(
        reader,
        sis::v9::Type::ElseIf,
        sis::v9::Parser::read_elseif_payload
    );
}

std::vector<sis::v9::DataUnit> sis::v9::Parser::read_array_of_data_unit(BinaryReader &reader) {
    return read_array<sis::v9::DataUnit>(
        reader,
        sis::v9::Type::DataUnit,
        sis::v9::Parser::read_data_unit_payload
    );
}

std::vector<sis::v9::FileData> sis::v9::Parser::read_array_of_file_data(BinaryReader &reader) {
    return read_array<sis::v9::FileData>(
        reader,
        sis::v9::Type::FileData,
        sis::v9::Parser::read_file_data_payload
    );
}

template<typename T, typename PayloadReader>
std::vector<T>
sis::v9::Parser::read_array(
    BinaryReader &reader,
    sis::v9::Type expectedElementType,
    PayloadReader &&readPayload
) {
    const auto fh = read_field_header(reader);

    assert_field_type(fh.type, sis::v9::Type::Array);

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

    //skip_padding(reader, fh);

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
