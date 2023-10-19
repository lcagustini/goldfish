#include <print.h>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static enum printType currentType;
static FILE *openFile;

void setPrintType(enum printType type, const char *file) {
    switch (type) {
		case PRINT_FILE:
			openFile = fopen(file, "w");
            fclose(openFile);
			openFile = fopen(file, "a");
			break;
		default:
			break;
    }

    currentType = type;
}

void print(const char *format, ...) {
    FILE *printTarget;
    switch (currentType) {
		case PRINT_FILE:
            printTarget = openFile;
            break;
		default:
            printTarget = stdout;
			break;
    }

    va_list args;
    va_start(args, format);

    vfprintf(printTarget, format, args);

    va_end(args);
}
