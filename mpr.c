#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "error.h"
#include "mpr.h"

static char type = 'i';
static char answer_tube[64];

void 
create_random_tube_name(char *seed)
{
    int left;
    size_t current_length;

    left = 54;
    strcpy(answer_tube, "/tmp/tAsw_");
    current_length = strlen(answer_tube);

    while (*seed && left > 23)
    {
        if (!isalnum(*seed))
        {
            seed += 1;
            continue;
        }

        if (rand() % 2 == 0)
            answer_tube[current_length++] = toupper(*seed);
        else
            answer_tube[current_length++] = tolower(*seed);
        seed += 3;
        answer_tube[current_length] = '\0';
        --left;
    }

    while (left-- > 1)
        answer_tube[current_length++] = (rand() % 10) + '0';
    answer_tube[current_length] = '\0';
}

char *
to_abs_ref(char *reference)
{
    if (*reference == '/')
        return reference;
}

char *
create_message(const char * const imp_name, const char * const filename)
{
    int length_imp, length_filename, length_answer_tube;
    char *message;
    unsigned int message_size;
    uid_t uid;
    gid_t gid;
    size_t pos;

    uid = getuid();
    gid = getgid();

    length_imp = strlen(imp_name);
    length_filename = strlen(filename);
    length_answer_tube = strlen(answer_tube);

    message_size = sizeof(unsigned int) + sizeof(char) + sizeof(uid_t) + sizeof(gid_t)
        + (3 * sizeof(int)) + length_imp + length_filename + length_answer_tube + 3;

    message = malloc(message_size);

    pos = 0;
    memcpy(message, &message_size, sizeof(unsigned int));

    pos += sizeof(unsigned int);
    memcpy(message + pos, &type, sizeof(char));

    pos += sizeof(char);
    memcpy(message + pos, &uid, sizeof(uid_t));

    pos += sizeof(uid_t);
    memcpy(message + pos, &gid, sizeof(gid_t));

    pos += sizeof(gid_t);
    memcpy(message + pos, &length_imp, sizeof(int));
    
    pos += sizeof(int);
    memcpy(message + pos, imp_name, length_imp);

    pos += length_imp;
    memcpy(message + pos, &length_filename, sizeof(int));

    pos += sizeof(int);
    memcpy(message + pos, filename, length_filename);

    pos += length_filename;
    memcpy(message + pos, &length_answer_tube, sizeof(int));
    
    pos += sizeof(int);
    memcpy(message + pos, answer_tube, length_answer_tube);
   
    return message;
}

void 
send_message(const char * const tube, const char * const message)
{
    int fd;
    unsigned int size;

    memcpy(&size, message, sizeof(unsigned int));

    fd = open(tube, O_WRONLY);
    if (fd == -1)
        ERROR_EXIT(10);   

    write(fd, message, size);
    unlink(tube);
    close(fd);
}

void 
get_answer(void)
{
    int fd, answer;
    size_t b_read;

    if (mkfifo(answer_tube, S_IRWXU | S_IRWXG | S_IRWXO) == -1)
        ERROR_EXIT(11);

    fd = open(answer_tube, O_RDONLY);
    if (fd == -1)
        ERROR_EXIT(10);

    while((b_read = read(fd, &answer, sizeof(int))) > 0);

    switch(answer)
    {
        case -1:
            printf("ERROR ON READ ANSWER....\n");
            break;
        default:
            printf("Id impression : %d\n", answer);
            break;
    }

    unlink(answer_tube);
    close(fd);
}



int 
main(int argc, const char **argv)
{
    int cpt, imp_set, file_set;
    char *imprimante, *file, *message, *server_tube;

    srand(time(NULL));

    if (argc != 4)
        ARG_ERROR_EXIT(2, USAGE);

    server_tube = getenv("IMP_PATH");
    if (server_tube == NULL)
        ERROR_MSG(12, "IMP_PATH n'existe pas...%s", "");

    imp_set = 0;
    file_set = 0;
    cpt = 1;
    while (cpt < argc)
    {
        if (strcmp(argv[cpt], "-p") == 0)
        {
            if ((cpt + 1 < argc) && (imp_set == 0))
            {
                if ((imprimante = malloc((strlen(argv[cpt + 1]) + 1) * sizeof(char))) == NULL)
                    ERROR_EXIT(1);
                strcpy(imprimante, argv[cpt + 1]);
                ++cpt;
            }
            else
                ARG_ERROR_EXIT(3, USAGE);
        }
        else
        {   
            if (file_set == 1)
                ARG_ERROR_EXIT(3, USAGE);

            if ((file = malloc((strlen(argv[cpt]) + 1) * sizeof(char))) == NULL)
                ERROR_EXIT(1);
            strcpy(file, argv[cpt]);
        }
        ++cpt;
    }

    create_random_tube_name(file);
    message = create_message(imprimante, file);
    send_message(server_tube, message);
    get_answer();

    return EXIT_SUCCESS;
}
