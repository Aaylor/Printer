#include <ctype.h>
#include <errno.h>
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
    size_t size;
    char *cwd, *abs_ref, *tmp;

    size = 256;
    cwd = malloc(size * sizeof(char));
    if (cwd == NULL)
    {
        perror("Erreur lors de l'allocation de cwd ");
        return NULL;
    }

    errno = 0;
    while(getcwd(cwd, size) == NULL && errno == ERANGE)
    {
        size *= 2;
        tmp = realloc(cwd, size);
        if (tmp == NULL)
        {
            free(cwd);
            perror("Erreur lors de la réallocation de cwd ");
            return NULL;
        }

        cwd = tmp;
    }

    abs_ref = malloc(strlen(reference) + strlen(cwd) + 2);
    if (abs_ref == NULL)
    {
        perror("Erreur lors de l'allocation du chemin absolu ");
        return NULL;
    }

    sprintf(abs_ref, "%s/%s", cwd, reference);
    
    free(cwd);

    return abs_ref;
}

void
get_base_message(struct sending_message *msg)
{
    msg->type = type;
    msg->uid = getuid();
    msg->gid = getgid();
    strcpy(msg->answering_tube, answer_tube);
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
        ERROR_E(10, "Erreur dans l'allocation de mémoire lors de la récupération du message\n");

    pos = 0;
    memcpy(buf + pos, imp_name, imp_length);
    pos += imp_length;

    memcpy(buf + pos, filename, filename_length);
    pos += filename_length;

    get_base_message(&msg);
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

    get_base_message(&msg);
    msg.buf_size = sizeof(int);
    msg.buf = &id;

    returned_message = create_message(msg);
    
    return returned_message;
}

void *
get_listing_message(char *name)
{
    struct sending_message msg;
    char *returned_message;
    size_t name_length;

    if (name == NULL)
        name_length = 0;
    else
        name_length = strlen(name) + 1;

    get_base_message(&msg);
    msg.buf_size = name_length;
    msg.buf = name;

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
            case ERROR_IN_QUEUE:
                fprintf(stderr, "Erreur lors de l'ajout à la liste " 
                        "d'impression.\n");
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
    int to_free;
    char *imprimante, *file, *server_tube, *name;
    void *message;
    struct stat s;

    srand(time(NULL));
    to_free = 0;
    file = NULL;

    if (argc == 1 || argc > 4)
        USAGE_ERROR(argv[0], 1, "Nombre d'argument incompatible.\n");

    server_tube = getenv("IMP_PATH");
    if (server_tube == NULL)
        ERROR(2, "La variable globale IMP_PATH n'existe pas...\n");

    create_random_tube_name(answer_tube, file);
    if (mkfifo(answer_tube, S_IRWXU | S_IRWXG | S_IRWXO) == -1)
        ERROR_E(20, "Erreur dans la cration du tube de réponse.\n");


    if (strcmp(argv[1], "-P") == 0 || strcmp(argv[1], "-p") == 0)
    {
        if (argc != 4)
            USAGE_ERROR(argv[0], 3, "Nombre d'argument incompatible avec l'option -P.\n");
       
        imprimante = argv[2];
        if (*argv[3] == '/')
            file = argv[3];
        else
        {
            file = to_abs_ref(argv[3]);
            to_free = 1;
        }

        if (stat(file, &s) == -1)
            ERROR_E(12, "Problème lors de la lecture des stats du fichier.\n");
        if (!S_ISREG(s.st_mode))
            ERROR(30, "Le fichier n'est pas régulier, et ne peut dont être imprimé.\n");
       
        type = 'i';
        message = get_printing_message(imprimante, file);
        
        if (to_free)
            free(file);
    }
    else if (strcmp(argv[1], "-C") == 0 || strcmp(argv[1], "-c") == 0)
    {
        if (argc != 3)
            USAGE_ERROR(argv[0], 3, "Nombre d'argument incompatible avec l'option -C.\n");

        if (!is_number(argv[2]))
            ERROR(31, "L'argument entrée n'est pas un entier.");

        type = 'c';
        message = get_canceling_message(atoi(argv[2]));
    }
    else if (strcmp(argv[1], "-L") == 0 || strcmp(argv[1], "-l") == 0)
    {
        if (argc == 3)
            name = argv[2];
        else if (argc == 2)
            name = NULL;
        else
            USAGE_ERROR(argv[0], 3, "Nombre d'argument incompatible avec l'option -L.\n" );

        type = 'l';
        message = get_listing_message(name);
    }
    else
        USAGE_ERROR(argv[0], 5, "Argument inconnu.");
   
    if (send_message(server_tube, message) == 1)
    {
        free(message);
        fprintf(stderr, "Le message n'a pas pu être envoyé.\n");
    }
    free(message);

    handle_answer();
    
    
    return EXIT_SUCCESS;
}
