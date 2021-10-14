/*
  Author: Yaroslav Ilin

  This is a single-header C/C++ library that defines short names 
  for types with clear vision of size of those types.

  This library doesn't work just by including it in your program.
  You have to specify the way you want to use this library. 
  There're 2 ways of defining the types: either using preprocessor,
  which will just replace all the matches found in your code with
  specified type, or using the typedef operator, which actually declares
  a type for a compiler to address to.

  You, the library users, probably have different opinions and use cases,
  so you have an ability to choose the way the types would be declared. 



  Quick review:

  Types are named according to this naming self-claimed kind-of-convention:

    [s/u/f/float][8/16/32/64][l/f]

    Which translates as:
      [s/u/f/float] - kind of type: [s]igned / [u]nsigned / [f]loat / float.
      [8/16/32/64] - amount of bits occupied: 8 bits / 16 bits / 32 bits / 64 bits.
      [l/f] - optional suffix, which stands for [l]east / [f]ast.
      Least - variable, which should have at least N amount of bits occupied.
      Fast - variable, which should have at least N amount of bits occupied
             and can be expanded for faster calculations.

    Some examples:
      s8   - signed integer type with 8 bits occupied (1 bit for sign, 7 bits available).
      u16  - unsigned integer type with 16 bits occupied (No bits for sign, 16 bits available).
      s32l - signed integer type with at least 32 bits occupied (1 bit for sign, 15 (or more) bits available)
      u64f - unsigned integer type with at least 64 bits occupied (No bits for sign, 64 (or more) bits available)
      f32  - floating-point type with 32 bits occupied (IEEE-754 32-bit Single precision).
      f64  - floating-point type with 64 bits occupied (IEEE-754 64-bit Double precision).



  Use instruction:

  1. Before including this header file, you should specify the way
     types will be declared. Possible directives:
  
    #define YPL_TYPES_BY_DEFINE  - uses a preprocessor to replace short names
                                   with actual types before compiling your program.

    #define YPL_TYPES_BY_TYPEDEF - uses typedef operator, which will actually declare
                                   new types compiler can address to.

  2. After specifying the way types will be declared, you have to specify the types
     that would be referenced to. This only applies to integer types, because
     floating-point types are consistent with size. Possible directives:

    #define TYPES_USING_MICROSOFT - uses Microsoft __intN data types. These types have fixed width.
    Reference: https://docs.microsoft.com/en-us/cpp/cpp/data-type-ranges?view=msvc-160
    WARNING: Microsoft types will work only in Microsoft MSVC compiler,
             so be aware of that, if you're using clang, gcc, or other.

    #define TYPES_USING_CPP_STANDART - uses default C++ data types. These types have variable width
                                       depending on OS data model. (See link below)
    Reference: https://en.cppreference.com/w/cpp/language/types

    (Next directives are using C library "stdint.h", which is compatible with C++)
    (Next description quotes are from site below)
    Reference: https://www.cplusplus.com/reference/cstdint/

    #define TYPES_USING_EXACT - uses intN_t and uintN_t types.
      "Integer type with a width of exactly 8, 16, 32, or 64 bits.
       For signed types, negative values are represented using 2's complement.
       No padding bits.
       Optional: These typedefs are not defined if no types with such characteristics exist.*"

    Types sNl (s16l, s32l, s64l) and uNl (u16l, u32l, u64l) are included by default.
    They map to int_leastN_t and uint_leastN_t types respectively.
      "Integer type with a minimum of 8, 16, 32, or 64 bits.
       No other integer type exists with lesser size and at least the specified width."

    Types sNf (s16f, s32f, s64f) and uNl (u16f, u32f, u64f) are included by default.
    They map to int_fastN_t and uint_fastN_t types respectively.
      "Integer type with a minimum of 8, 16, 32, or 64 bits.
       At least as fast as any other integer type with at least the specified width."

    "Some of these typedefs may denote the same types.
     Therefore, function overloads should not rely on these being different."
    
     * Notice that some types are optional (and thus, with no portability guarantees).
     A particular library implementation may also define additional types with other
     widths supported by its system. In any case, if either the signed or the unsigned version
     is defined, both the signed and unsigned versions are defined."

  3. Additional options:
  
     In case you don't want to use short float type names (fN and floatN), you can use directive
     #define TYPES_NO_FLOATS, which will disable those typedefs/defines.

     Same for "least" and "fast" types:
     #define TYPES_NO_LEAST - excludes sNl (int_leastN_t) and uNl (uint_leastN_t) types.

     #define TYPES_NO_FAST - excludes sNf (int_fastN_t) and uNf (uint_fastN_t) types.

     #define TYPES_USE_NAMEPSACE - enables namespace.

  Example program:

    ----------------------
    #define YPL_TYPES_BY_TYPEDEF
    #define YPL_TYPES_USING_EXACT
    #include "types.h"
    #include <stdio.h>

    int main() {
      s64 num_exact_s64   = S64_MAX;
      u64 num_exact_u64   = U64_MAX;
      s16 num_exact_s16   = S16_MAX;
      s16l num_least_s16  = S16L_MAX;
      s16f num_fast_s16   = S16F_MAX;
      float64 num_float64 = 3.14159265359;
      float32 num_float32 = 3.14159265359;

      printf("num_exact_s64: (%llu bits) %lld\n",  sizeof(num_exact_s64) * 8, num_exact_s64);
      printf("num_exact_u64: (%llu bits) %llu\n",  sizeof(num_exact_u64) * 8, num_exact_u64);
      printf("num_exact_s16: (%llu bits) %d\n",    sizeof(num_exact_s16) * 8, num_exact_s16);
      printf("num_least_s16: (%llu bits) %d\n",    sizeof(num_least_s16) * 8, num_least_s16);
      printf("num_fast_s16:  (%llu bits) %d\n",    sizeof(num_fast_s16)  * 8, num_fast_s16);
      printf("num_float64:   (%llu bits) %.11f\n", sizeof(num_float64)   * 8, num_float64);
      printf("num_float32:   (%llu bits) %.11f\n", sizeof(num_float32)   * 8, num_float32);
      return 0;
    }
    ----------------------

*/

// --- Optional namespace ---
#if defined YPL_TYPES_USE_NAMESPACE
namespace ypl {
#endif

// --- Preprocessor defines section ---
#if defined YPL_TYPES_BY_DEFINE
# if defined YPL_TYPES_USING_EXACT
#  if !defined _YPL_TYPES_STDINT_INCLUDED
#   define _YPL_TYPES_STDINT_INCLUDED
#   include <stdint.h>
#  endif
#  define s8  int8_t
#  define u8  uint8_t
#  define s16 int16_t
#  define u16 uint16_t
#  define s32 int32_t
#  define u32 uint32_t
#  define s64 int64_t
#  define u64 uint64_t
# elif defined YPL_TYPES_USING_MICROSOFT
#  define s8  __int8
#  define u8  unsigned __int8
#  define s16 __int16
#  define u16 unsigned __int16
#  define s32 __int32
#  define u32 unsigned __int32
#  define s64 __int64
#  define u64 unsigned __int64
# elif defined YPL_TYPES_USING_CPP_STANDART
#  define s8  signed char
#  define u8  unsigned char
#  define s16 short int
#  define u16 unsigned short int
#  define s32 int
#  define u32 unsigned int
#  define s64 long long int
#  define u64 unsigned long long int
# endif
# if !defined YPL_TYPES_NO_FAST
#  if !defined _YPL_TYPES_STDINT_INCLUDED
#   define _YPL_TYPES_STDINT_INCLUDED
#   include <stdint.h>
#  endif
#  define s8f  int_fast8_t
#  define u8f  uint_fast8_t
#  define s16f int_fast16_t
#  define u16f uint_fast16_t
#  define s32f int_fast32_t
#  define u32f uint_fast32_t
#  define s64f int_fast64_t
#  define u64f uint_fast64_t
# endif
# if !defined YPL_TYPES_NO_LEAST
#  if !defined _YPL_TYPES_STDINT_INCLUDED
#   define _YPL_TYPES_STDINT_INCLUDED
#   include <stdint.h>
#  endif
#  define s8l  int_least8_t
#  define u8l  uint_least8_t
#  define s16l int_least16_t
#  define u16l uint_least16_t
#  define s32l int_least32_t
#  define u32l uint_least32_t
#  define s64l int_least64_t
#  define u64l uint_least64_t
# endif

# if !defined YPL_TYPES_NO_FLOATS
#  include <float.h>
#  define f32 float
#  define float32 float
#  define f64 double
#  define float64 double
# endif
#endif

// --- Typedefs section ---
#if defined YPL_TYPES_BY_TYPEDEF
# if defined YPL_TYPES_USING_EXACT
#  if !defined _YPL_TYPES_STDINT_INCLUDED
#   define _YPL_TYPES_STDINT_INCLUDED
#   include <stdint.h>
#  endif
typedef int8_t   s8;
typedef uint8_t  u8;
typedef int16_t  s16;
typedef uint16_t u16;
typedef int32_t  s32;
typedef uint32_t u32;
typedef int64_t  s64;
typedef uint64_t u64;
# elif defined YPL_TYPES_USING_MICROSOFT
typedef __int8           s8;
typedef unsigned __int8  u8;
typedef __int16          s16;
typedef unsigned __int16 u16;
typedef __int32          s32;
typedef unsigned __int32 u32;
typedef __int64          s64;
typedef unsigned __int64 u64;
# elif defined YPL_TYPES_USING_CPP_STANDART
typedef char               s8;
typedef unsigned char      u8;
typedef short              s16;
typedef unsigned short     u16;
typedef int                s32;
typedef unsigned int       u32;
typedef long long          s64;
typedef unsigned long long u64;
# endif
# if !defined YPL_TYPES_NO_FAST
#  if !defined _YPL_TYPES_STDINT_INCLUDED
#   define _YPL_TYPES_STDINT_INCLUDED
#   include <stdint.h>
#  endif
typedef int_fast8_t   s8f;
typedef uint_fast8_t  u8f;
typedef int_fast16_t  s16f;
typedef uint_fast16_t u16f;
typedef int_fast32_t  s32f;
typedef uint_fast32_t u32f;
typedef int_fast64_t  s64f;
typedef uint_fast64_t u64f;
# endif
# if !defined YPL_TYPES_NO_LEAST
#  if !defined _YPL_TYPES_STDINT_INCLUDED
#  define _YPL_TYPES_STDINT_INCLUDED
#  include <stdint.h>
#  endif
typedef int_least8_t   s8l;
typedef uint_least8_t  u8l;
typedef int_least16_t  s16l;
typedef uint_least16_t u16l;
typedef int_least32_t  s32l;
typedef uint_least32_t u32l;
typedef int_least64_t  s64l;
typedef uint_least64_t u64l;
# endif

# if !defined YPL_TYPES_NO_FLOATS
typedef float  f32;
typedef float  float32;
typedef double f64;
typedef double float64;
# endif
#endif

// --- Limits section ---
#if defined YPL_TYPES_USING_EXACT
# define S8_MAX INT8_MAX
# define U8_MAX UINT8_MAX
# define S16_MAX INT16_MAX
# define U16_MAX UINT16_MAX
# define S32_MAX INT32_MAX
# define U32_MAX UINT32_MAX
# define S64_MAX INT64_MAX
# define U64_MAX UINT64_MAX
#elif defined YPL_TYPES_USING_MICROSOFT
# if !defiined _YPL_TYPES_LIMITS_INCLUDED
#  define _YPL_TYPES_LIMITS_INCLUDED
#  include <limits.h>
# endif
# define S8_MAX SCHAR_MAX
# define U8_MAX UCHAR_MAX
# define S16_MAX SHRT_MAX
# define U16_MAX USHRT_MAX
# define S32_MAX INT_MAX
# define U32_MAX UINT_MAX
# define S64_MAX LLONG_MAX
# define U64_MAX ULLONG_MAX
#elif defined YPL_TYPES_USING_CPP_STANDART
# if !defined _YPL_TYPES_LIMITS_INCLUDED
#  define _YPL_TYPES_LIMITS_INCLUDED
#  include <limits.h>
# endif
# define S8_MAX SCHAR_MAX
# define U8_MAX UCHAR_MAX
# define S16_MAX SHRT_MAX
# define U16_MAX USHRT_MAX
# define S32_MAX INT_MAX
# define U32_MAX UINT_MAX
# define S64_MAX LLONG_MAX
# define U64_MAX ULLONG_MAX
#endif

#if !defined YPL_TYPES_NO_FAST
# define S8F_MAX INT_FAST8_MAX
# define U8F_MAX UINT_FAST8_MAX
# define S16F_MAX INT_FAST16_MAX
# define U16F_MAX UINT_FAST16_MAX
# define S32F_MAX INT_FAST32_MAX
# define U32F_MAX UINT_FAST32_MAX
# define S64F_MAX INT_FAST64_MAX
# define U64F_MAX UINT_FAST64_MAX
#endif
#if !defined YPL_TYPES_NO_LEAST
# define S8L_MAX INT_LEAST8_MAX
# define U8L_MAX UINT_LEAST8_MAX
# define S16L_MAX INT_LEAST16_MAX
# define U16L_MAX UINT_LEAST16_MAX
# define S32L_MAX INT_LEAST32_MAX
# define U32L_MAX UINT_LEAST32_MAX
# define S64L_MAX INT_LEAST64_MAX
# define U64L_MAX UINT_LEAST64_MAX
#endif

#if !defined YPL_TYPES_NO_FLOATS
# define F32_MAX FLT_MAX
# define F32_MIN FLT_MIN
# define FLOAT32_MAX FLT_MAX
# define FLOAT32_MIN FLT_MIN
# define F64_MAX DBL_MAX
# define F64_MIN DBL_MIN
# define FLOAT64_MAX DBL_MAX
# define FLOAT64_MIN DBL_MIN
#endif

// --- Optional namespace ---
#if defined YPL_TYPES_USE_NAMESPACE
}
#endif