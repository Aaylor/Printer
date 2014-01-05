#ifndef MPR_H
#define MPR_H

#include <stdio.h>

#include "error.h"

#ifndef USAGE
#define USAGE(name) \
    do { \
        printf("%s -P imprimante fichier\n%s -L [imprimante]\n%s -C id", \
            name, name, name); \
    } while (0)
#endif

#ifndef USAGE_ERROR
#define USAGE_ERROR(name, id, ...) \
    do { \
        ERROR(id, __VA_ARGS__); \
        USAGE(name); \
    } while (0)
#endif


int is_number(const char *src);

char *to_abs_ref(char *reference);

void get_base_message(struct sending_message *msg);

void *get_printing_message(const char *const imp_name, 
        const char *const filename); 

void *get_canceling_message(int id);

void *get_listing_message(char *name);

void handle_answer(void);


#endif /* end of include guard: MPR_H */

