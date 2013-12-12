#ifndef ERROR_H
#define ERROR_H 1

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef ERROR_EXIT
#define ERROR_EXIT(id) \
    do { \
        fprintf(stderr, "\nerror in file `%s`, line %d :\n\t%s\n", \
                __FILE__, __LINE__, strerror(errno)); \
        if ((id) > 0) exit((id)); \
    } while (0)
#endif /* endif ERROR_EXIT  */

#ifndef ARG_ERROR_EXIT
#define ARG_ERROR_EXIT(id, msg) \
    do { \
        fprintf(stderr, (msg)); \
        if ((id) > 0) exit((id)); \
    } while (0)
#endif /* endif ARG_ERROR_EXIT  */

#ifndef ERROR_MSG
#define ERROR_MSG(id, msg, ...) \
    do { \
        fprintf(stderr, (msg), __VA_ARGS__); \
        if ((id) > 0) exit((id)); \
    } while (0)
#endif /* endif ERROR_MSG  */

#endif /* endif ERROR_H */

