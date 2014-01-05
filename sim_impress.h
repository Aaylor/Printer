#ifndef SIM_IMPRESS_H
#define SIM_IMPRESS_H

#define EOF_SIZE 10
#define BUFFER_SIZE 64

#ifndef USAGE
#define USAGE(name) \
    do { \
        printf("%s -t printer_pipe -n printer_name", name); \
    } while (0)
#endif

#ifndef USAGE_ERROR
#define USAGE_ERROR(name, id, ...) \
    do { \
        ERROR(id, __VA_ARGS__); \
        USAGE(name); \
    } while (0)
#endif

struct imprimante_info
{
    const char *tube_name;
    const char *imprimante_name;
};

void create_tube(void);

int has_eof(char *buffer, size_t buffer_size);

void work(void);

#endif /* end of include guard: SIM_IMPRESS_H */

