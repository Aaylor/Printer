#ifndef PRINT_DEMON_H
#define PRINT_DEMON_H

#include "error.h"

enum __error_type_print_demon{
    MORE_THAN_ONCE,
    ARGUMENT_TOO_LONG,
    MISSING_ARGUMENT,
    UNKNOWN_OPTION,
    UNKNOWN_CHARACTER
};
typedef enum __error_type_print_demon daemon_error_t;

#define __ERROR_MSG(__error_t) \
    ((__error_t) == MORE_THAN_ONCE ? "L'argument `%c` existe plus d'une fois.\n" :\
     ((__error_t) == ARGUMENT_TOO_LONG ? "L'option `%c` possède un argument trop longue.\n" :\
      ((__error_t) == MISSING_ARGUMENT ? "L'option `%c` manque.\n" : \
       ((__error_t) == UNKNOWN_OPTION ? "L'option `%c` est inconnue.\n" :\
        "Caractère d'option non reconnu : `\\x%x`.\n"))))

#define ERROR_OPT(__error_t, c) \
    ERROR_MSG((__error_t), __ERROR_MSG((__error_t)), (c))


#endif /* endif PRINT_DEMON_H */

