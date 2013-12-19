#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "constants.h"
#include "error.h"
#include "message.h"
#include "print_demon.h"

static char *receiving_tube = NULL;
static char *config_file = NULL;
static int fd_t = -1;

static printers_list p_list = {.length = 0, .head = NULL, .tail = NULL};
static int print_id = 1;

void handleSigint(int signo)
{
    printf("\n_sig %d_\n", signo);
    printf("Fermeture du serveur d'impression en cours...\n");
    close(fd_t);
    unlink(receiving_tube);
    exit(EXIT_SUCCESS);
}

void closeEachPrinter(void)
{
    node p, w, tmp_printer, tmp_waiting;
    struct printer *c_printer;
    struct waiting *w_printer;

    p = p_list.tail;
    while(p != NULL)
    {
        c_printer = (struct printer *)(p->data);
        
        printf("Fermeture de l'imprimante `%s` en cours...\n", c_printer->name);


        close(c_printer->fd_printer);
        
        if (c_printer->fd_current_file != -2)
            close(c_printer->fd_current_file);

        if (unlink(c_printer->tube_path) == -1)
            perror("unlink");

        free(c_printer->name);
        free(c_printer->tube_path);
        free(c_printer->filename);

        w = (c_printer->wl).tail;
        while (w != NULL)
        {
            w_printer = (struct waiting *)(w->data);
            
            free(w_printer->filename);
            free(w_printer);
            
            tmp_waiting = w->prev;
            free(w);
            w = tmp_waiting;
        }

        free(c_printer);
        
        tmp_printer = p->prev;
        free(p);
        p = tmp_printer;
    }
}

    
void
add_printer(const char *name, const char *tube)
{
    int fd;
    char *buffer_name, *buffer_tube;
    size_t name_size;
    struct printer *p;

    fd = open(tube, O_WRONLY | O_NONBLOCK);
    if (fd == -1)
        ERROR_EXIT(456789);
    
    name_size = strlen(name);

    p = malloc(sizeof(struct printer));
    if (p == NULL)
        ERROR_EXIT(456789);

    printf("PRINTER WITH NAME `%s`\n", name);

    buffer_name = malloc(name_size + 1);
    strcpy(buffer_name, name);

    buffer_tube = malloc(strlen(tube) + 1);
    strcpy(buffer_tube, tube);

    p->stopped      = 1;
    p->fd_printer   = fd;
    p->name         = buffer_name;
    p->tube_path    = buffer_tube;
   
    p->fd_current_file  = -2;
    p->id_print         = -1;
    p->uid_user         = -1;
    p->filename         = NULL;

    (p->wl).length  = 0;
    (p->wl).head    = NULL;
    (p->wl).tail    = NULL;

    add_in_printer_list(&p_list, p);
}

int
send_to_printer(const char *printer_name, const char *filename, uid_t uid_user)
{
    struct waiting *w;
    node p = p_list.head;

    while(p != NULL)
    {
        struct printer *cur_printer = (struct printer *)(p->data);
        if (strcmp(cur_printer->name, printer_name) == 0)
        {
            w = malloc(sizeof(struct waiting));
            
            w->filename = malloc(strlen(filename) + 1);
            strcpy(w->filename, filename);
            
            w->uid_user = uid_user;
            w->id = print_id;

            add_in_waiting_list(&(cur_printer->wl), w);
            
            return print_id++;
        }

        p = p->next;
    }

    return UNKNOWN_PRINTER_NAME;
}

int
init_config_file(void)
{
    FILE *cfg;
    char buffer[128];
    char *tok;

    cfg = fopen(config_file, "r");
    if (cfg == NULL)
        ERROR_MSG(10, "Fichier de configuration n'existe pas\n%s", "");

    while(fgets(buffer, 128, cfg) != NULL)
    {
        tok = strchr(buffer, ' ');
        if (tok == NULL) 
        {
            fprintf(stderr, "Attention, ligne incorrect\n");
            continue;
        }


        *(tok) = '\0';
        ++tok;
        tok[strlen(tok)-1] = '\0';

        add_printer(buffer, tok);
    }

    fclose(cfg);

    return 0;
}

int
try_rights_on_file(uid_t uid, gid_t gid, const char *filename)
{
    struct stat s;

    if (stat(filename, &s) == -1)
        ERROR_EXIT(456789);

    if ((s.st_mode & S_IROTH) 
            || ((s.st_mode & S_IRGRP) && (s.st_gid == gid))
            || ((s.st_mode & S_IRUSR) && (s.st_uid == uid)))
        return 0;

    return 1;
}

int
check_if_id_exist(int id, uid_t uid)
{
    /*
     * si id > à l'id courant, alors pas la peine de continuer.
     */
    node p_node, w_node;
    struct printer *c_printer;
    struct waiting *w;

    for (p_node = p_list.head; p_node != NULL; p_node = p_node->next)
    {
        printf("FOR EACH PRINTER\n");
        c_printer = (struct printer *)(p_node->data); 
        if (c_printer->id_print == id)
        {
            printf("\tCANCELING CURRENT PRINTING\n");
            if (c_printer->uid_user != uid)
                return DONT_HAVE_ACCESS;

            c_printer->stopped = 1;
            return REMOVED;
        }

        for(w_node = (c_printer->wl).head; w_node != NULL; w_node = w_node->next)
        {
            w = (struct waiting *)(w_node->data);

            printf("\tFOR EACH WAITING LIST\n");

            if (w->id == id)
            {
                printf("\t\tSAME ID\n");
                if (w->uid_user != uid)
                    return DONT_HAVE_ACCESS;

                if(remove_node(&(c_printer->wl), w_node) == NULL)
                    return UNKNOWN_ID;

                free(w->filename);
                free(w);

                return REMOVED;
            }
        }
    }

    return UNKNOWN_ID;
}

void process_msg(unsigned int length, char *buf)
{
    char type;
    uid_t uid;
    gid_t gid;
    size_t pos;
    char answering_tube[ANSWERING_TUBE_SIZE];
    
    pos = 0;
    type = *buf;
    pos += sizeof(char);

    memcpy(&uid, buf + pos, sizeof(uid_t));
    pos += sizeof(uid_t);

    memcpy(&gid, buf + pos, sizeof(gid_t));
    pos += sizeof(gid_t);

    memcpy(answering_tube, buf + pos, ANSWERING_TUBE_SIZE);
    pos += ANSWERING_TUBE_SIZE;

    if (type == 'i')
    {
        char *printer_name, *filename;
        int answer;
        size_t printer_name_length, filename_length; 

        printer_name_length = strlen(buf + pos);
        printer_name = malloc(printer_name_length + 1);
        memcpy(printer_name, buf + pos, printer_name_length + 1);
        pos += printer_name_length + 1;

        filename_length = strlen(buf + pos);
        filename = malloc(filename_length + 1);
        memcpy(filename, buf+pos, filename_length + 1);
        pos +=filename_length + 1;

        if ((answer = try_rights_on_file(uid, gid, filename)) == DONT_HAVE_RIGHTS)
            write_answer(answering_tube, &answer, sizeof(int)); 
        else
        {
            if ((answer = send_to_printer(printer_name, filename, uid)) == UNKNOWN_PRINTER_NAME)
                write_answer(answering_tube, &answer, sizeof(int));
            else
                write_answer(answering_tube, &answer, sizeof(int));
        }

        free(printer_name);
        free(filename);
    }
    else if (type == 'c')
    {
        int id, answer;
        
        memcpy(&id, buf+pos, sizeof(int));
        answer = check_if_id_exist(id, uid);

        write_answer(answering_tube, &answer, sizeof(int));
    }
    else if (type == 'l')
    {
        size_t name_length;
        char *name;

        if (pos == length)
            write_list(answering_tube, NULL);
        else
        {
            name_length = strlen(buf + pos);
            name =  malloc(name_length * sizeof(char));
            memcpy(name, buf + pos, name_length + 1);
            write_list(answering_tube, name);

            free(name);
        }
    }
    else
        printf("NOT NOW\n");
}

void
write_answer(const char *tube, void *answer, size_t size)
{
    int fd;

    fd = open(tube, O_WRONLY);
    if (fd == -1)
        ERROR_EXIT(45678);

    write(fd, answer, size);

    close(fd);
}

void
write_list(const char *tube, const char *name)
{
    FILE *f;
    node p_node, w_node;
    struct printer *p;
    struct waiting *w;

    f = fopen(tube, "a");
    if (f == NULL)
        unlink(tube);

    for (p_node = p_list.head; p_node != NULL; p_node = p_node->next) 
    {
        p = (struct printer *)(p_node->data);

        if (name == NULL || strcmp(name, p->name) == 0)
        {
            fprintf(f, "=======\nImprimante `%s` :\n", p->name);
            
            if (p->fd_current_file != -2)
            {
                fprintf(f, "\nEn cours d'impression =>" 
                        "\n[%3d] %s ~ UID : %lu\n",
                        p->id_print, p->filename, 
                        (long unsigned int)p->uid_user);
            }

            fprintf(f, "\nListe d'attente = >");
            for (w_node = (p->wl).head; w_node != NULL; w_node = w_node->next)
            {
                w = (struct waiting *)(w_node->data);
                
                fprintf(f, "\n[%3d] %20s ~ UID : %lu\n", 
                        w->id, w->filename, (long unsigned int)w->uid_user);
            }

            fprintf(f, "======\n\n\n");
        }
    }

    fclose(f);
}

void work()
{
    char *buffer;
    size_t bytes_read;
    unsigned int msg_length;
    struct node *p;

    fd_t = open(receiving_tube, O_RDONLY | O_NONBLOCK);
    if (fd_t == -1)
        ERROR_EXIT(567890);

    printf("Le serveur est prêt à l'utilisation.\n");
    while(1)
    {
        bytes_read = read(fd_t, &msg_length, sizeof(unsigned int));
        if (bytes_read > 0 && msg_length > 0)
        {
            buffer = malloc(msg_length);
            read(fd_t, buffer, msg_length);
            process_msg(msg_length, buffer);
            free(buffer);
            msg_length = 0;
        }

        for (p = p_list.head; p != NULL; p = p->next)
        {
            size_t bytes_read_in_file = 0;
            struct printer *current_printer;
            char print_buffer[BUFFER_SIZE];

            current_printer = (struct printer *)(p->data);
           
            if (current_printer->fd_current_file != -2)
            {
                printf("fd print : %d\n", current_printer->fd_printer);
                printf("fd cf : %d\n", current_printer->fd_current_file);
                bytes_read_in_file = read(current_printer->fd_current_file, print_buffer, BUFFER_SIZE);
                if (bytes_read_in_file == 0 || current_printer->stopped == 1)
                {
                    write(current_printer->fd_printer, END_OF_PRINT, 10);
                    close(current_printer->fd_current_file);
                    current_printer->uid_user = -1;
                    current_printer->fd_current_file = -2;
                    current_printer->id_print = 0;
                    current_printer->stopped = 0;
                }
                else
                    write(current_printer->fd_printer, print_buffer, bytes_read_in_file);
            }
            else
            {
                if ((current_printer->wl).length > 0)
                {
                    node w;
                    struct waiting *current_data;

                    w = pop(&(current_printer->wl));
                    current_data = (struct waiting *)(w->data);

                    current_printer->stopped = 0;
                    current_printer->uid_user = current_data->uid_user;
                    current_printer->id_print = current_data->id;
                    current_printer->fd_current_file = open(current_data->filename, O_RDONLY);
                    if (current_printer->fd_current_file == -1)
                    {
                        perror("opening...\n");
                        current_printer->fd_current_file = -2;
                    }
                    
                    free(current_printer->filename);
                    current_printer->filename = current_data->filename;

                    free(current_data);
                    free(w);
                }
            }
        }
        
        sleep(1);
    }
}


int 
main(int argc, char **argv)
{
    int receiving_tube_set, config_file_set, cpt, return_value;
    pid_t pid;
    char *init_sim_impress[4];

    if (argc != 5)
        ERROR_MSG(100, "Nombre d'arguments incorrect...\n%s", "");

    cpt = 1;
    config_file_set = 0;
    receiving_tube_set = 0;
    while(cpt < argc)
    {
        if (strlen(argv[cpt]) == 2 && argv[cpt][0] == '-')
        {
            if (argv[cpt][1] == 't')
            {
                if (receiving_tube_set == 1)
                    ERROR_OPT(MORE_THAN_ONCE, argv[cpt][1]);

                receiving_tube_set = 1;
                receiving_tube = argv[++cpt];
            }
            else if (argv[cpt][1] == 'c')
            {
                if (config_file_set == 1)
                    ERROR_OPT(MORE_THAN_ONCE, argv[cpt][1]);

                config_file_set = 1;
                config_file = argv[++cpt];
            }
            else
                ERROR_OPT(UNKNOWN_OPTION, argv[cpt][1]);
        }
        else
            ERROR_MSG(100, "Arg incorrect\n %s\n", argv[cpt]);

        ++cpt;
    }
   
    signal(SIGINT, handleSigint);
    signal(SIGSEGV, handleSigint);
    atexit(closeEachPrinter);

    init_sim_impress[0] = "./init_simulateurs";
    init_sim_impress[1] = "-c";
    init_sim_impress[2] = config_file;
    init_sim_impress[3] = NULL;

    pid = fork();
    if (pid == 0)
    {
        if (execvp(init_sim_impress[0], init_sim_impress) == -1)
        {
            perror("execvp");
            _exit(EXIT_FAILURE);
        }
        _exit(EXIT_SUCCESS);
    }
    
    waitpid(pid, &return_value, 0);
    if (WEXITSTATUS(return_value) != EXIT_SUCCESS)
    {
        fprintf(stderr, "STATUS % d\nErreur dans l'initialisation des imprimantes."
                "\nLe serveur va fermer.\n", WEXITSTATUS(return_value));
        return EXIT_FAILURE;
    }

    if (create_tube(receiving_tube) == -1)
        ERROR_EXIT(34567890);

    sleep(1);
        
    init_config_file();
    work();


    return EXIT_SUCCESS;
}
