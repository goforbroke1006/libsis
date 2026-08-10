//
// Created by goforbroke on 8/9/26.
//

#ifndef LIBSIS_SIS_V9_FIELD_H
#define LIBSIS_SIS_V9_FIELD_H

#include <string>

namespace sis::v9 {
    enum class Type : uint32_t {
        String = 1,
        Array = 2,
        Compressed = 3,
        Version = 4,
        VersionRange = 5,
        Date = 6,
        Time = 7,
        DateTime = 8,
        Uid = 9,
        Language = 11,
        Contents = 12,
        Controller = 13,
        Info = 14,
        SupportedLanguages = 15,
        SupportedOptions = 16,
        Prerequisites = 17,
        Dependency = 18,
        Properties = 19,
        Property = 20,
        Signatures = 21,
        CertificateChain = 22,
        Logo = 23,
        FileDescription = 24,
        Hash = 25,
        If = 26,
        ElseIf = 27,
        InstallBlock = 28,
        Expression = 29,
        Data = 30,
        DataUnit = 31,
        FileData = 32,
        SupportedOption = 33,
        ControllerChecksum = 34,
        DataChecksum = 35,
        Signature = 36,
        Blob = 37,
        SignatureAlgorithm = 38,
        SignatureCertChain = 39,
        DataIndex = 40,
        Capabilities = 41,
    };


    inline std::string type_to_string(Type type) {
        switch (type) {
            case Type::String:
                return "String";
            case Type::Array:
                return "Array";
            case Type::Compressed:
                return "Compressed";
            case Type::Version:
                return "Version";
            case Type::VersionRange:
                return "VersionRange";
            case Type::Date:
                return "Date";
            case Type::Time:
                return "Time";
            case Type::DateTime:
                return "DateTime";
            case Type::Uid:
                return "Uid";
            case Type::Language:
                return "Language";
            case Type::Contents:
                return "Contents";
            case Type::Controller:
                return "Controller";
            case Type::Info:
                return "Info";
            case Type::SupportedLanguages:
                return "SupportedLanguages";
            case Type::SupportedOptions:
                return "SupportedOptions";
            case Type::Prerequisites:
                return "Prerequisites";
            case Type::Dependency:
                return "Dependency";
            case Type::Properties:
                return "Properties";
            case Type::Property:
                return "Property";
            case Type::Signatures:
                return "Signatures";
            case Type::CertificateChain:
                return "CertificateChain";
            case Type::Logo:
                return "Logo";
            case Type::FileDescription:
                return "FileDescription";
            case Type::Hash:
                return "Hash";
            case Type::If:
                return "If";
            case Type::ElseIf:
                return "ElseIf";
            case Type::InstallBlock:
                return "InstallBlock";
            case Type::Expression:
                return "Expression";
            case Type::Data:
                return "Data";
            case Type::DataUnit:
                return "DataUnit";
            case Type::FileData:
                return "FileData";
            case Type::SupportedOption:
                return "SupportedOption";
            case Type::ControllerChecksum:
                return "ControllerChecksum";
            case Type::DataChecksum:
                return "DataChecksum";
            case Type::Signature:
                return "Signature";
            case Type::Blob:
                return "Blob";
            case Type::SignatureAlgorithm:
                return "SignatureAlgorithm";
            case Type::SignatureCertChain:
                return "SignatureCertChain";
            case Type::DataIndex:
                return "DataIndex";
            case Type::Capabilities:
                return "Capabilities";
            default:
                return "<Unknown>";
        }
    }


    struct FieldHeader {
        Type type;
        uint64_t length;

        static FieldHeader from_raw(uint32_t rawType, uint64_t length) {
            return {
                .type = static_cast<Type>(rawType),
                .length = length,
            };
        }

        [[nodiscard]] uint32_t type_num() const {
            return static_cast<uint32_t>(type);
        }
    };
} //sis::v9

#endif //LIBSIS_SIS_V9_FIELD_H
