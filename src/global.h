#ifndef GLOBAL
#define GLOBAL 1
#define MAX_LEN 260
#define FALSE 0
#define TRUE 1
extern int VERBOSE;
typedef u_int8_t uint8_t;

static const char *byte_to_binary(int x) {
    static char b[9];
    b[0] = '\0';

    int z;
    for (z = 128; z > 0; z >>= 1) {
        strcat(b, ((x & z) == z) ? "1" : "0");
    }

    return b;
}

#endif
