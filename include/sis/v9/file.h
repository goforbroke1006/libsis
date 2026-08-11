//
// Created by goforbroke on 8/9/26.
//

// https://www.cryer.co.uk/file-types/s/sis/softwareinstallsis.pdf

#ifndef LIBSIS_SIS_V9_FILE_H
#define LIBSIS_SIS_V9_FILE_H

#include <cstdint>
#include <vector>
#include <optional>
#include <memory>

#include "primitive.h"
#include "field.h"

namespace sis::v9 {
    struct Header {
        uint32_t uid1;
        uint32_t uid2;
        uint32_t uid3;
        uint32_t uid4;
        uint32_t checksum;
    };

    enum TInstallType : uint8_t {
        EInstInstallation,
        EInstAugmentation,
        EInstPartialUpgrade,
        EInstPreInstalledApp,
        EInstPreInstalledPatch,
    };

    struct Info {
        Uid uid;

        String vendorUniqueName;

        // SISArray<SISString>
        std::vector<String> names;

        // SISArray<SISString>
        std::vector<String> vendorNames;

        Version version;
        DateTime creationTime;

        uint8_t installType;
        uint8_t installFlags;
    };

    struct SupportedOption {
        // SISArray<SISString>
        // One localized name per supported language.
        std::vector<String> names;
    };

    struct SupportedOptions {
        // SISArray<SISSupportedOption>
        std::vector<SupportedOption> options;
    };

    struct Language {
        uint32_t language;
    };

    struct SupportedLanguages {
        // SISArray<SISLanguage>
        std::vector<Language> languages;
    };

    struct VersionRange {
        FieldHeader header;

        Version from;

        // optional upper bound
        std::optional<Version> to;
    };

    struct Dependency {
        Uid uid;

        VersionRange versionRange;

        // SISArray<SISString>
        std::vector<String> dependencyNames;
    };

    struct Prerequisites {
        // SISArray<SISDependency>
        std::vector<Dependency> targetDevices;

        // SISArray<SISDependency>
        std::vector<Dependency> dependencies;
    };

    struct Property {
        int32_t key;
        int32_t value;
    };

    struct Properties {
        // SISArray<SISProperty>
        std::vector<Property> properties;
    };

    struct Capabilities {
        std::vector<uint8_t> data;
    };

    struct Hash {
        uint32_t algorithm;
        std::vector<uint8_t> data;
    };

    enum class FileOperation : uint32_t {
        Install = 1,
        Run = 2,
        Text = 4,
        Null = 8
    };

    struct FileDescription {
        String target;
        String mimeType;

        std::optional<Capabilities> capabilities;
        Hash hash;

        uint32_t operation;
        uint32_t operationOptions;

        uint64_t length;
        uint64_t uncompressedLength;

        uint32_t dataIndex;
    };

    struct Logo {
        FileDescription fileDescription;
    };

    enum class ExpressionOperator : uint32_t {
        // Binary
        Equal = 1,
        NotEqual = 2,
        GreaterThan = 3,
        LessThan = 4,
        GreaterOrEqual = 5,
        LessOrEqual = 6,

        // Logical
        And = 7,
        Or = 8,

        // Unary
        Not = 9,

        // Functions
        Exists = 10,
        AppProperties = 11,
        DevProperties = 12, // PACKAGE()

        // Primitives
        String = 13,
        Option = 14,
        Variable = 15,
        Number = 16,
    };

    inline
    std::string exp_op_to_str(const ExpressionOperator &op) {
        switch (op) {
            case ExpressionOperator::Equal: return "Equal";
            case ExpressionOperator::NotEqual: return "NotEqual";
            case ExpressionOperator::GreaterThan: return "GreaterThan";
            case ExpressionOperator::LessThan: return "LessThan";
            case ExpressionOperator::GreaterOrEqual: return "GreaterOrEqual";
            case ExpressionOperator::LessOrEqual: return "LessOrEqual";

            case ExpressionOperator::And: return "And";
            case ExpressionOperator::Or: return "Or";

            case ExpressionOperator::Not: return "Not";

            case ExpressionOperator::Exists: return "Exists";
            case ExpressionOperator::AppProperties: return "AppProperties";
            case ExpressionOperator::DevProperties: return "DevProperties";

            case ExpressionOperator::String: return "String";
            case ExpressionOperator::Option: return "Option";
            case ExpressionOperator::Variable: return "Variable";
            case ExpressionOperator::Number: return "Number";

            default:
                return "<unknown>";
        }
    }

    struct Expression {
        uint32_t operatorTypeRaw;
        ExpressionOperator operatorType;

        std::unique_ptr<Expression> left;
        std::unique_ptr<Expression> right;

        std::optional<int32_t> integerValue;
        std::optional<String> stringValue;
    };

    struct Controller;


    struct ElseIf;
    struct InstallBlock;

    struct If {
        Expression expression;
        std::unique_ptr<InstallBlock> installBlock;

        std::vector<ElseIf> elseIfs;
    };

    struct ElseIf {
        Expression expression;
        std::unique_ptr<InstallBlock> installBlock;
    };

    struct InstallBlock {
        // SISArray<SISFileDescription>
        std::vector<FileDescription> files;

        // SISArray<SISEmbeddedController>
        std::vector<Controller> embeddedControllers;

        // SISArray<SISIf>
        std::vector<If> ifBlocks;


        InstallBlock() = default;

        InstallBlock(InstallBlock &&) noexcept = default;

        InstallBlock &operator=(InstallBlock &&) noexcept = default;

        InstallBlock(const InstallBlock &) = delete;

        InstallBlock &operator=(const InstallBlock &) = delete;
    };

    struct SignatureAlgorithm {
        std::string algorithmIdentifier;
    };

    struct Signature {
        SignatureAlgorithm algorithm;

        std::vector<uint8_t> signature;
    };

    struct CertificateChain {
        std::vector<uint8_t> certificates;
    };

    struct SignatureCertificateChain {
        // SISArray<SISSignature>
        std::vector<Signature> signatures;

        CertificateChain certificateChain;
    };

    struct DataIndex {
        uint32_t index;
    };

    struct Controller {
        Info info;
        SupportedOptions supportedOptions;
        SupportedLanguages supportedLanguages;
        Prerequisites prerequisites;
        Properties properties;

        std::optional<Logo> logo;

        InstallBlock installBlock;

        SignatureCertificateChain certChain;

        DataIndex dataIndex;
    };

    constexpr uint32_t COMP_ALG_NONE = 0;
    constexpr uint32_t COMP_ALG_DEFLATE = 1;

    struct Compressed {
        uint32_t algorithm;
        uint64_t uncompressedSize;
        Controller controller;
    };

    struct FileData {
        // TODO: compressed file data
    };

    struct DataUnit {
        std::vector<FileData> data;
    };

    struct Data {
        std::vector<DataUnit> units;
    };

    struct Contents {
        uint16_t controllerChecksum;
        uint16_t dataChecksum;
        Compressed compressed;
        Data data;
    };

    struct File {
        Header header;
        Contents contents;

        File() = default;

        File(const File &) = delete;

        File &operator=(const File &) = delete;

        File(File &&) noexcept = default;

        File &operator=(File &&) noexcept = default;
    };
} //sis::v9

#endif //LIBSIS_SIS_V9_FILE_H
