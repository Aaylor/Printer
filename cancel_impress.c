#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "error.h"
#include "message.h"

static char type = 'a';
static char answering_tube[64];

int 
is_number(const char *s)
{
    while (*s)
    {
        if (!isdigit(*s))
            return 0;
        ++s;
    }

    return 1;
}

void *
write_message(int id)
{
    void *message;
    struct sending_message m;

    m.type = type;
    m.uid = getuid();
    m.gid = getgid();
    m.answering_tube = answering_tube;
    m.buf_size = sizeof(int);
    m.buf = &id;

    message = create_message(m);
    
    return message;
}

void
handle_answer(void)
{
    int answer;

    switch((answer = get_answer(answering_tube)))
    {
        case 1:
            printf("EVERYTHIN IS FINE\n");
            break;
        case 2:
            printf("UH OH\n");
            break;
        case 4:
            printf("AGLOUGLOU\n");
            break;
    }
}

int 
main(int argc, char **argv)
{
    int id, cpt;
    void *message;
    char *server_tube;
    
    if (argc != 2)
        ERROR_MSG(100, "Arguments invalide\n%s", "");

    server_tube = getenv("IMP_PATH");
    if (server_tube == NULL)
        ERROR_MSG(12, "IMP_PATH n'existe pas...\n%s", "");

    if (!is_number(argv[1]))
        ERROR_MSG(100, "L'argument n'est pas un entier\n%s", "");

    id = atoi(argv[1]);

    create_random_tube_name(answering_tube, "cAqzfsNCedLsidMpsRedqSsZ");
    message = write_message(id);
    send_message(server_tube, message);
    handle_answer();

    return EXIT_SUCCESS;
}
