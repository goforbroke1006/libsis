//
// Created by goforbroke on 8/12/26.
//

#ifndef LIBSIS_SIS_V9_COUT_H
#define LIBSIS_SIS_V9_COUT_H

#include "types.h"

#include <ostream>
#include <iomanip>

inline
std::ostream &operator
<<(std::ostream &out, sis::v9::FileUID32 const &uid) {
    const auto flags = out.flags();
    const auto fill = out.fill();

    out << "0x" << std::setw(8) << std::setfill('0') << std::hex << uid.value; // print

    out.flags(flags);
    out.fill(fill);

    return out;
}

inline
std::ostream &operator
<<(std::ostream &out, sis::v9::String const &str) {
    out << std::string(str.value.begin(), str.value.end());
    return out;
}

inline
std::ostream &operator
<<(std::ostream &out, sis::v9::Uid const &uid) {
    out << uid.value;
    return out;
}

inline
std::ostream &operator
<<(std::ostream &out, sis::v9::Version const &version) {
    out << version.major << "." << version.minor;
    return out;
}

inline
std::ostream &operator
<<(std::ostream &out, sis::v9::Date const &date) {
    out
            << date.year
            << "-" << (date.month < 10 ? "0" : "") << static_cast<int>(date.month)
            << "-" << (date.day < 10 ? "0" : "") << static_cast<int>(date.day);
    return out;
}

inline
std::ostream &operator
<<(std::ostream &out, sis::v9::Time const &time) {
    out
            << (time.hours < 10 ? "0" : "") << std::to_string(time.hours)
            << ":" << (time.minutes < 10 ? "0" : "") << std::to_string(time.minutes)
            << ":" << (time.seconds < 10 ? "0" : "") << std::to_string(time.seconds);
    return out;
}

inline
std::ostream &operator
<<(std::ostream &out, sis::v9::DateTime const &dt) {
    out << dt.date << " " << dt.time;
    return out;
}

inline
std::ostream &operator
<<(std::ostream &out, sis::v9::CompressedAlgorithm const &cAlg) {
    switch (cAlg) {
        case sis::v9::CompressedAlgorithm::COMP_ALG_NONE:
            out << "NONE";
            break;
        case sis::v9::CompressedAlgorithm::COMP_ALG_DEFLATE:
            out << "DEFLATE";
            break;
        default:
            out << "UNKNOWN";
            break;
    }
    return out;
}

inline
std::ostream &operator
<<(std::ostream &out, sis::v9::Expression const &exp) {
    out
            << "("
            << sis::v9::exp_op_to_str(exp.operatorType);

    if (exp.left) {
        out << "," << *exp.left;
    }
    if (exp.right) {
        out << "," << *exp.right;
    }

    if (exp.integerValue.has_value()) {
        out
                << ","
                << exp.integerValue.value();
    }
    if (exp.stringValue.has_value()) {
        out
                << ","
                << "\"" << exp.stringValue.value() << "\"";
    }

    out << ")";
    return out;
}

#endif //LIBSIS_SIS_V9_COUT_H
