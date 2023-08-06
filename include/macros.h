#ifndef MACROS_H
#define MACROS_H

#define STRINGIFY(c) #c
#define VARIADIC_COUNT(type, ...) ((int)(sizeof((type[]){__VA_ARGS__})/sizeof(type)))
#define UNPARENTHESES(...) __VA_ARGS__

#define membersize(type, member) sizeof(((type *)0)->member)

#endif
