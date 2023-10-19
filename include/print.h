#ifndef PRINT_H
#define PRINT_H

enum printType {
	PRINT_STDOUT,
	PRINT_FILE
};

void setPrintType(enum printType type, const char *file);
void print(const char *format, ...);

#endif
