#ifndef ERROR_H
#define ERROR_H 1

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 
 *  Macro facilitant les messages d'erreurs.
 */

#ifndef __ERR
#define __ERR(...) \
    do { \
        fprintf(stderr, __VA_ARGS__); \
    } while (0)
#endif /* __ERR */

#ifndef ERROR
#define ERROR(id, ...) \
    do { \
        __ERR(__VA_ARGS__); \
        if ((id) > 0) exit((id)); \
    } while (0)
#endif /* ERROR */

#ifndef ERROR_E
#define ERROR_E(id, ...) \
    do { \
        __ERR(__VA_ARGS__); \
        fprintf(stderr, "%s\n", strerror(errno)); \
        if ((id) > 0) exit((id)); \
    } while (0)
#endif /* ERROR_E */


/*
 *  Enumeration comportant tout les types d'erreurs possible lors des réponses
 *  du serveur.
 */

enum printing
{
    DONT_HAVE_RIGHTS = -1,
    UNKNOWN_PRINTER_NAME = -2, 
    ERROR_IN_QUEUE = -3,
};

enum canceling
{
    REMOVED,
    UNKNOWN_ID,
    DONT_HAVE_ACCESS,
};


#endif /* endif ERROR_H */

