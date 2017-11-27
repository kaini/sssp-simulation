#pragma once
#include <boost/assert.hpp>
#include <boost/preprocessor.hpp>
#include <iostream>
#include <string>

#define STRINGY_ENUM_ITEM(R, DATA, ELEM) ELEM,
#define STRINGY_ENUM_OUT_CASE(R, DATA, ELEM)                                                                           \
    case DATA::ELEM:                                                                                                   \
        out << BOOST_PP_STRINGIZE(ELEM);                                                                               \
        break;
#define STRINGY_ENUM_IN_ELSEIF(R, DATA, ELEM)                                                                          \
    else if (str == BOOST_PP_STRINGIZE(ELEM)) {                                                                        \
        v = DATA::ELEM;                                                                                                \
    }
#define STRINGY_ENUM(ENUM, ...)                                                                                        \
    enum class ENUM : ::size_t { BOOST_PP_SEQ_FOR_EACH(STRINGY_ENUM_ITEM, 0, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) }; \
    inline ::std::ostream& operator<<(::std::ostream& out, ENUM v) {                                                   \
        switch (v) {                                                                                                   \
            BOOST_PP_SEQ_FOR_EACH(STRINGY_ENUM_OUT_CASE, ENUM, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))                  \
            default:                                                                                                   \
                BOOST_ASSERT(false);                                                                                   \
        }                                                                                                              \
        return out;                                                                                                    \
    }                                                                                                                  \
    inline ::std::istream& operator>>(::std::istream& in, ENUM& v) {                                                   \
        ::std::string str;                                                                                             \
        in >> str;                                                                                                     \
        if (!in.fail() && !in.bad()) {                                                                                 \
            if (false) {                                                                                               \
            }                                                                                                          \
            BOOST_PP_SEQ_FOR_EACH(STRINGY_ENUM_IN_ELSEIF, ENUM, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))                 \
            else {                                                                                                     \
                in.setstate(::std::ios_base::failbit);                                                                 \
            }                                                                                                          \
        }                                                                                                              \
        return in;                                                                                                     \
    }
