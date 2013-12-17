#ifndef PRINT_DEMON_H
#define PRINT_DEMON_H

#include "error.h"
#include "queue.h"

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

typedef struct queue printers_list;
typedef struct queue waiting_list;

typedef struct node *node;

struct printer
{
    char stopped;
    int fd_printer;
    const char *name;
    const char *tube_path;
    
    int fd_current_file;
    int id_print;
    uid_t uid_user;
    char *filename;
    
    waiting_list wl;
};

struct waiting
{
    char *filename;
    uid_t uid_user;
    int id;
};

#define add_in_printer_list(q, d) add_in_queue((q), (d))
#define add_in_waiting_list(q, d) add_in_queue((q), (d))

void
write_answer(const char *, void *, size_t);

void
write_list(const char *, const char *);

#endif /* endif PRINT_DEMON_H */

