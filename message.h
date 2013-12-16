#ifndef MESSAGE_H
#define MESSAGE_H

#include <sys/types.h>

#define ANSWERING_TUBE_SIZE     64
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

void
create_random_tube_name(char[64], char *);

void *
create_message(struct sending_message);

void
send_message(const char * const, const void* const);

int
get_answer(const char * const);

#endif /* end of include guard: MESSAGE_H */

