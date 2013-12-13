#ifndef MESSAGE_H
#define MESSAGE_H

#include <sys/types.h>

struct sending_message
{
    char type;
    uid_t uid;
    gid_t gid;
    char *answering_tube;
    size_t buf_size;
    void *buf;
};

struct answering_message
{
    unsigned int length;
    char type;
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

