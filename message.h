#ifndef MESSAGE_H
#define MESSAGE_H

#include <sys/types.h>

#define ANSWERING_TUBE_SIZE 64
#define CONSTANT_SIZE_REQUEST   \
    (sizeof(char) + sizeof(uid_t) + sizeof(gid_t) + ANSWERING_TUBE_SIZE)

struct sending_message
{
    char type;
    uid_t uid;
    gid_t gid;
    char answering_tube[ANSWERING_TUBE_SIZE];
    size_t buf_size;
    void *buf;
};

void create_random_tube_name(char tube[ANSWERING_TUBE_SIZE], char *seed);

int create_tube(const char *name);

void *create_message(struct sending_message m);

int send_message(const char * const tube, const void* const message);

int get_answer(const char * const anser_tube);

void print_answer(char[ANSWERING_TUBE_SIZE]);

#endif /* end of include guard: MESSAGE_H */

