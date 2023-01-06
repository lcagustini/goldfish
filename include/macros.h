#ifndef MACROS_H
#define MACROS_H

#define STRINGIFY(c) #c
#define VARIADIC_COUNT(...) ((int)(sizeof((int[]){__VA_ARGS__})/sizeof(int)))
#define UNPARENTHESES(...) __VA_ARGS__

#define membersize(type, member) sizeof(((type *)0)->member)

#endif
