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

static char type;
static char answer_tube[64];

int
is_number(const char *src)
{
    while(*src)
    {
        if (!isdigit(*src))
            return 0;
        ++src;
    }

    return 1;
}

char *
to_abs_ref(char *reference)
{
    if (*reference == '/')
        return reference;
    return reference;
}

void *
get_printing_message(const char * const imp_name, const char * const filename)
{
    struct sending_message msg;
    size_t imp_length, filename_length, pos, buf_size;
    char *buf, *returned_message;

    imp_length = strlen(imp_name) + 1;
    filename_length = strlen(filename) + 1;
    buf_size = (imp_length + filename_length);

    buf = malloc(buf_size);
    if (buf == NULL)
        ERROR_EXIT(11234);

    pos = 0;
    memcpy(buf + pos, imp_name, imp_length);
    pos += imp_length;

    memcpy(buf + pos, filename, filename_length);
    pos += filename_length;

    msg.type = type;
    msg.uid = getuid();
    msg.gid = getgid();
    strcpy(msg.answering_tube, answer_tube);
    msg.buf_size = buf_size;
    msg.buf = buf;

    returned_message = create_message(msg); 

    free(buf);

    return returned_message;
}

void *
get_canceling_message(int id)
{
    struct sending_message msg;
    char *returned_message;

    msg.type = type;
    msg.uid = getuid();
    msg.gid = getgid();
    strcpy(msg.answering_tube, answer_tube);
    msg.buf_size = sizeof(int);
    msg.buf = &id;

    returned_message = create_message(msg);
    return returned_message;
}

void *
get_listing_message(char *name)
{
    struct sending_message msg;
    char *buf, *returned_message;
    size_t name_length;

    if (name == NULL)
    {
        buf = NULL;
        name_length = 0;
    }
    else
    {
        name_length = strlen(name) + 1;
        buf = malloc(name_length * sizeof(char));
        strcpy(buf, name);
    }

    msg.type = type;
    msg.uid = getuid();
    msg.gid = getgid();
    strcpy(msg.answering_tube, answer_tube);
    msg.buf_size = name_length;
    msg.buf = buf;

    returned_message = create_message(msg);
    return returned_message;
}

void 
handle_answer(void)
{
    int answer;
    
    if (type == 'i')
    {
        switch((answer = get_answer(answer_tube)))
        {
            case DONT_HAVE_RIGHTS:
                fprintf(stderr, "Vous n'avez pas les droits requis.\n");
                break;
            case UNKNOWN_PRINTER_NAME:
                fprintf(stderr, "L'imprimante demandée n'existe pas.\n");
                break;
            default:
                printf("La demande a été acceptée.\n"
                        "ID impression : %d\n", answer);
                break;
        }
    }
    else if (type == 'c')
    {
        switch((answer = get_answer(answer_tube)))
        {
            case REMOVED:
                printf("L'impression a bien été supprimée.\n");
                break;
            case UNKNOWN_ID:
                fprintf(stderr, "ID d'impression inconnue.\n");
                break;
            case DONT_HAVE_ACCESS:
                fprintf(stderr, "Vous n'avez pas les accès requis pour annuler"
                        " cette impression.\n");
                break;
            default:
                fprintf(stderr, "Erreur inconnue.\n");
                break;
        }
    }
    else if (type == 'l')
        print_answer(answer_tube);
    else
        fprintf(stderr, "Type de demande inconnue...\n");
}

int 
main(int argc, char **argv)
{
    int cpt, imp_set, file_set;
    char *imprimante, *file = NULL, *server_tube;
    char *name;
    void *message;
    struct stat s;

    srand(time(NULL));

    if (argc == 1 || argc > 4)
        ERROR_MSG(56789, "Trop d'arguments...\n%s", "");

    server_tube = getenv("IMP_PATH");
    if (server_tube == NULL)
        ERROR_MSG(12, "IMP_PATH n'existe pas...%s\n", "");

    create_random_tube_name(answer_tube, file);
    if (mkfifo(answer_tube, S_IRWXU | S_IRWXG | S_IRWXO) == -1)
        ERROR_EXIT(11);
   
    if (strcmp(argv[1], "-P") == 0)
    {
        if (argc != 4)
            ERROR_MSG(4678, "Bad args...\n%s", "");
       
        imprimante = argv[2];
        file = argv[3];

        if (stat(file, &s) == -1)
            ERROR_EXIT(10);
        if (!S_ISREG(s.st_mode))
            ERROR_MSG(11, "Le fichier n'est pas régulier...\n%s", "");
        
        type = 'i';
        message = get_printing_message(imprimante, file);
    }
    else if (strcmp(argv[1], "-C") == 0)
    {
        if (argc != 3)
            ERROR_MSG(5678, "Bad args...\n%s", "");

        if (!is_number(argv[2]))
            ERROR_MSG(56789, "Argument non entier...\n%s", "");

        type = 'c';
        message = get_canceling_message(atoi(argv[2]));
    }
    else if (strcmp(argv[1], "-L") == 0)
    {
        if (argc == 2)
            name = argv[2];
        else
            name = NULL;

        type = 'l';
        message = get_listing_message(name);
    }
    else
        ERROR_MSG(1234, "ARGUMENT INCONNU.\n%s", "");

    send_message(server_tube, message);
    handle_answer();

    return EXIT_SUCCESS;
}
