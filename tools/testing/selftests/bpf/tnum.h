// SPDX-License-Identifier: GPL-2.0-only
/* tnum.h: Header file for tnum utility functions */

#ifndef __TNUM_H__
#define __TNUM_H__

#include <stdint.h>

typedef uint64_t u64;
typedef int64_t s64;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint8_t u8;

struct tnum {
	u64 value;
	u64 mask;
};

#define TNUM(_v, _m)	(struct tnum){.value = (_v), .mask = (_m)}

/* Function prototypes */
struct tnum tnum_scast(struct tnum a, u8 size);

#endif /* __TNUM_H__ */

