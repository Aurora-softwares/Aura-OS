#ifndef STDBOOL_H
#define STDBOOL_H

#ifndef __cplusplus
typedef unsigned char bool;
#define true 1
#define false 0
#define __bool_true_false_are_defined 1
#else
#include <stdbool.h>
#endif

#endif // STDBOOL_H
