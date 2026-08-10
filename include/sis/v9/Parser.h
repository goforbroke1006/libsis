//
// Created by goforbroke on 8/9/26.
//

#ifndef SYMBIAN_DEVELOPMENT_SISX_PARSER_H
#define SYMBIAN_DEVELOPMENT_SISX_PARSER_H

#include <utility>

#include "../BinaryReader.h"
#include "file.h"

namespace sis::v9 {
    class Parser {
    public:
        explicit Parser(BinaryReader reader) : reader_(std::move(reader)) {
        };

        sis::v9::File parse();

    private:
        BinaryReader reader_;

        sis::v9::FieldHeader read_field_header();

        sis::v9::Contents read_contents();

        static sis::v9::Controller read_controller(BinaryReader &reader);

        static sis::v9::Info read_info(BinaryReader &reader);

        static sis::v9::SupportedOptions read_supported_options(BinaryReader &reader);

        static sis::v9::SupportedLanguages read_supported_languages(BinaryReader &reader);

        static sis::v9::Prerequisites read_prerequisites(BinaryReader &reader);

        static sis::v9::Properties read_properties(BinaryReader &reader);

        static std::optional<sis::v9::Logo> read_optional_logo(BinaryReader &reader);

        static sis::v9::InstallBlock read_install_block(BinaryReader &reader);

        static sis::v9::SignatureCertificateChain read_signature_certificate_chain(
            BinaryReader &reader
        );

        /*static SISX::SignatureCertificateChain read_signature_certificate_chain_payload(
            BinaryReader &reader,
            const SISX::FieldHeader &header
        );*/

        static sis::v9::DataIndex read_data_index(BinaryReader &reader);

        static sis::v9::Uid read_uid(BinaryReader &reader);

        static sis::v9::String read_string(BinaryReader &reader);

        static sis::v9::String read_string_payload(BinaryReader &reader, const sis::v9::FieldHeader &header);

        static sis::v9::Version read_version(BinaryReader &reader);

        static sis::v9::DateTime read_datetime(BinaryReader &reader);

        static sis::v9::Date read_date(BinaryReader &reader);

        static sis::v9::Time read_time(BinaryReader &reader);

        static std::vector<sis::v9::String>
        read_array_of_string(BinaryReader &reader);

        /*static std::vector<SISX::SignatureCertificateChain>
        read_array_of_signature_certificate_chain(BinaryReader &reader);*/

        template<typename T, typename PayloadReader>
        static std::vector<T> read_array(
            BinaryReader &reader,
            sis::v9::Type expectedElementType,
            PayloadReader &&readPayload);

        static sis::v9::FieldHeader read_array_element_header(
            BinaryReader &reader,
            sis::v9::Type elementType);

        static void skip_padding(BinaryReader &reader, const sis::v9::FieldHeader &lastHeader);
    };
} //sis::v9


#endif //SYMBIAN_DEVELOPMENT_SISX_PARSER_H
