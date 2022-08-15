#include <print.h>

void resetPrint() {
    FILE *f = fopen("ux0:data/a.txt", "w");
    fclose(f);
}

void print(const char *format, ...) {
    va_list args;
    va_start(args, format);

    FILE *f = fopen("ux0:data/a.txt", "a");
    vfprintf(f, format, args);
    fclose(f);

    va_end(args);
}
