//
// Created by goforbroke on 8/9/26.
//

#ifndef SYMBIAN_DEVELOPMENT_SIS_PARSER_H
#define SYMBIAN_DEVELOPMENT_SIS_PARSER_H

#include "../BinaryReader.h"
#include "types.h"

namespace sis::legacy {
    class Parser {
    public:
        explicit Parser(const BinaryReader &reader) : reader_(reader) {
        };

        SisFile parse();

    private:
        BinaryReader reader_;

        SisHeader read_header();

        SisField read_field();

        std::vector<SISFileDescription> read_content();
    };
} //sis::legacy


#endif //SYMBIAN_DEVELOPMENT_SIS_PARSER_H
