#ifndef PRINT_DEMON_H
#define PRINT_DEMON_H

#include "error.h"
#include "queue.h"

#ifndef USAGE
#define USAGE(name) \
    do { \
        printf("%s -t tube_path -c config_file", name); \
    } while (0)
#endif

#ifndef USAGE_ERROR
#define USAGE_ERROR(name, id, ...) \
    do { \
        ERROR(id, __VA_ARGS__); \
        USAGE(name); \
    } while (0)
#endif


#define BUFFER_SIZE 64
#define END_OF_PRINT "\0\0\0\0\0\0\0\0\0\0"

#define add_in_printer_list(q, d) add_in_queue((q), (d))
#define add_in_waiting_list(q, d) add_in_queue((q), (d))

typedef struct queue printers_list;
typedef struct queue waiting_list;

typedef struct node *node;

struct printer
{
    char stopped;
    int fd_printer;
    char *name;
    char *tube_path;
    
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


void handle_sigint(int signo);

void close_each_printer(void);

void add_printer(const char *name, const char *tube);

int send_to_printer(const char *printer_name, const char *filename, 
        uid_t uid_user); 

int init_config_file(void);

int try_rights_on_file(uid_t uid, gid_t gid, const char *filename);

int check_if_id_exist(int id, uid_t uid);

void process_msg(unsigned int length, char *buf);

void write_answer(const char *, void *, size_t);

void write_list(const char *, const char *);

void work(void);

#endif /* endif PRINT_DEMON_H */

