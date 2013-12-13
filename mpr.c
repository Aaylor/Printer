#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "error.h"
#include "message.h"
#include "mpr.h"

static char type = 'i';
static char answer_tube[64];

char *
to_abs_ref(char *reference)
{
    if (*reference == '/')
        return reference;
    return reference;
}

void *
write_message(const char * const imp_name, const char * const filename)
{
    struct sending_message msg;
    size_t imp_length, filename_length, pos, buf_size;
    char *buf, *returned_message;

    imp_length = strlen(imp_name);
    filename_length = strlen(filename);
    buf_size = (imp_length + filename_length) + (2 * sizeof(size_t));

    buf = malloc(buf_size);
    if (buf == NULL)
        ERROR_EXIT(11234);

    pos = 0;

    memcpy(buf, &imp_length, sizeof(size_t));
    pos += sizeof(size_t);

    memcpy(buf + pos, imp_name, imp_length);
    pos += imp_length;

    memcpy(buf + pos, &filename_length, sizeof(size_t));
    pos += sizeof(size_t);

    memcpy(buf + pos, filename, filename_length);
    pos += filename_length;

    msg.type = type;
    msg.uid = getuid();
    msg.gid = getgid();
    msg.answering_tube = answer_tube;
    msg.buf_size = buf_size;
    msg.buf = buf;

    returned_message = create_message(msg); 

    free(buf);

    return returned_message;
}

void 
handle_answer(void)
{
    int answer;

    switch((answer = get_answer(answer_tube)))
    {
        case -1 :
            printf("Error.");
            break;
        default:
            printf("ID : %d\n", answer);
            break;
    }
}

int 
main(int argc, char **argv)
{
    int cpt, imp_set, file_set;
    char *imprimante, *file, *server_tube;
    void *message;
    struct stat s;

    srand(time(NULL));

    if (argc != 4)
        ARG_ERROR_EXIT(2, USAGE);

    server_tube = getenv("IMP_PATH");
    if (server_tube == NULL)
        ERROR_MSG(12, "IMP_PATH n'existe pas...%s\n", "");

    imp_set = 0;
    file_set = 0;
    cpt = 1;
    while (cpt < argc)
    {
        if (strcmp(argv[cpt], "-p") == 0)
        {
            if ((cpt + 1 < argc) && (imp_set == 0))
            {
                ++cpt;
                imprimante = argv[cpt];
                imp_set = 1;
            }
            else
                ARG_ERROR_EXIT(3, USAGE);
        }
        else
        {   
            if (file_set == 1)
                ARG_ERROR_EXIT(3, USAGE);

            file = argv[cpt];
            file_set = 1;
        }
        ++cpt;
    }

    if (stat(file, &s) == -1)
        ERROR_EXIT(10);
    if (!S_ISREG(s.st_mode))
        ERROR_MSG(11, "Le fichier n'est pas régulier...\n%s", "");

    create_random_tube_name(answer_tube, file);
    message = write_message(imprimante, file);
    send_message(server_tube, message);
    handle_answer();

    return EXIT_SUCCESS;
}
