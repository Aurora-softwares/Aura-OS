#ifndef _UCLIBC_CONFIG_H
#define _UCLIBC_CONFIG_H

// Define your target architecture and processor here
#define TARGET_ARCH "x86_64"
#define TARGET_SUBARCH "generic"

// Define the endianness of your target (e.g., BIG_ENDIAN or LITTLE_ENDIAN)
#define ENDIANNESS "LITTLE_ENDIAN"

// Define the default C library features
#define UCLIBC_HAS_FLOATS 0
#define UCLIBC_HAS_IPV6 0
#define UCLIBC_HAS_THREADS_NATIVE 0

// Customize any other configuration options you need

#endif /* _UCLIBC_CONFIG_H */
