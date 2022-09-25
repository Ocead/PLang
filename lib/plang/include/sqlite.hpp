//
// Created by Johannes on 17.08.2022.
//

#ifndef LIBPLANG_SQLITE_HPP
#define LIBPLANG_SQLITE_HPP

#include <sqlite3.h>

#define EXTERN(name)                                                                                                   \
    extern unsigned char name[];                                                                                       \
    extern unsigned int name##_len;                                                                                    \
    static_assert(true)

#endif//LIBPLANG_SQLITE_HPP
