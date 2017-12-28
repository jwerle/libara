#ifndef ARA_COMMON_H
#define ARA_COMMON_H

/**
 * Much of this file has been inspired by `KHR/khrplatform.h` header file
 * provided by the hc Group
 */

#if defined(_WIN32)
#  define ARA_EXPORT __declspec(dllimport)
#elif defined(__GNUC__) && (__GNUC__ * 100 + __GNUC_MINOR) >= 303
#  define ARA_EXPORT __attribute__((visibility("default")))
#  define ARA_INLINE inline
#else
#  define ARA_EXPORT
#  define ARA_INLINE
#endif

#ifndef ARA_ALIGNMENT
#  define ARA_ALIGNMENT sizeof(unsigned long) /* platform word */
#endif

typedef int ARAint;
typedef int ARAsizei;
typedef void ARAvoid;
typedef char ARAchar;
typedef short ARAshort;

typedef unsigned int ARAuint;
typedef unsigned int ARAenum;

typedef unsigned int ARAbitfield;
typedef unsigned char ARAboolean;
typedef unsigned short ARAushort;

// C99
#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) || defined(__GNUC__)

#include <stdint.h>
typedef int32_t ara_int32_t;
typedef int64_t ara_int64_t;
typedef uint32_t ara_uint32_t;
typedef uint64_t ara_uint64_t;

#define ARA_SUPPORT_INT64 1
#define ARA_SUPPORT_FLOAT 1

#elif defined(_WIN32)

typedef __int32 ara_int32_t;
typedef __int64 ara_int64_t;
typedef unsigned __int32 ara_uint32_t;
typedef unsigned __int64 ara_uint64_t;

#define ARA_SUPPORT_INT64 1
#define ARA_SUPPORT_FLOAT 1

#else

#include <stdint.h>

typedef int32_t ara_int32_t;
typedef int64_t ara_int64_t;
typedef uint32_t ara_uint32_t;
typedef uint64_t ara_uint64_t;

#define ARA_SUPPORT_INT64 1
#define ARA_SUPPORT_FLOAT 1

#endif

typedef signed char ara_int8_t;
typedef unsigned char ara_uint8_t;
typedef signed short int ara_int16_t;
typedef unsigned short int ara_uint16_t;

#ifdef _WIN64
typedef signed long long int ara_intptr_t;
typedef signed long long int ara_ssize_t;
typedef unsigned long long int ara_uintptr_t;
typedef unsigned long long int ara_usize_t;
#else
typedef signed long int ara_intptr_t;
typedef signed long int ara_ssize_t;
typedef unsigned long int ara_uintptr_t;
typedef unsigned long int ara_usize_t;
#endif

#if ARA_SUPPORT_FLOAT
typedef float ara_float_t;
#endif

#if ARA_SUPPORT_INT64
typedef ara_uint64_t ara_utime_nanoseconds_t;
typedef ara_int64_t ara_stime_nanoseconds_t;
#endif

#ifndef ARA_MAX_ENUM
#define ARA_MAX_ENUM 0x7FFFFFFF
#endif

typedef enum {
    ARA_FALSE = 0,
    ARA_TRUE  = 1,
    ARA_BOOLEAN_ENUM_FORCE_SIZE = ARA_MAX_ENUM
} ara_boolean_enum_t;

/**
 * ARA typedefs
 */

typedef ara_uint8_t ARAubyte;
typedef ara_float_t ARAfloat;
typedef ara_ssize_t ARAsizeiptr;
typedef ara_intptr_t ARAintptr;

typedef ara_int64_t ARAint64;
typedef ara_uint64_t ARAuint64;

#endif
