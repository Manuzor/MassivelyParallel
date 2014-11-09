#pragma once

using mpInt8   = signed char;    static_assert(sizeof((mpInt8)0)   == 1, "Incorrect size.");
using mpInt16  = signed short;   static_assert(sizeof((mpInt16)0)  == 2, "Incorrect size.");
using mpInt32  = signed int;     static_assert(sizeof((mpInt32)0)  == 4, "Incorrect size.");
using mpInt64  = signed __int64; static_assert(sizeof((mpInt64)0)  == 8, "Incorrect size.");

using mpUInt8  = unsigned char;    static_assert(sizeof((mpUInt8)0)  == 1, "Incorrect size.");
using mpUInt16 = unsigned short;   static_assert(sizeof((mpUInt16)0) == 2, "Incorrect size.");
using mpUInt32 = unsigned int;     static_assert(sizeof((mpUInt32)0) == 4, "Incorrect size.");
using mpUInt64 = unsigned __int64; static_assert(sizeof((mpUInt64)0) == 8, "Incorrect size.");
