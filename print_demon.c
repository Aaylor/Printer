#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "constants.h"
#include "error.h"
#include "print_demon.h"

static const char *receiving_tube;
static const char *config_file;

static printers_list p_list;
static int print_id = 1;

void
add_printer(const char *name, const char *tube)
{
    int fd;
    char *buffer_name;
    size_t name_size;
    struct printer *p;

    name_size = strlen(name);

    p = malloc(sizeof(struct printer));
    if (p == NULL)
        ERROR_EXIT(456789);

    buffer_name = malloc(name_size + 1);
    strcpy(buffer_name, name);

    fd = open(tube, O_WRONLY | O_NONBLOCK);
    if (fd == -1)
        ERROR_EXIT(456789);

    p->name = buffer_name;
    p->fd_printer = fd;
    p->fd_current_file = -2;

    add_in_printer_list(&p_list, p);
}

int
send_to_printer(const char *printer_name, const char *filename)
{
    struct waiting *w;
    printer p = p_list.head;

    while(p != NULL)
    {
        struct printer *cur_printer = (struct printer *)(p->data);
        if (strcmp(cur_printer->name, printer_name) == 0)
        {
            w = malloc(sizeof(struct waiting));
            w->filename = malloc(strlen(filename) + 1);
            strcpy(w->filename, filename);
            w->id = print_id;

            add_in_waiting_list(&(cur_printer->wl), w);
            return print_id++;
        }

        p = p->next;
    }

    return 0;
}

int
init_config_file(void)
{
    int fd;
    struct printer *p;

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

        (tok++)[0] = '\0';
        tok[strlen(tok) - 1] = '\0';

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

void process_msg(char *buf)
{
    int id;
    char type;
    uid_t uid;
    gid_t gid;
    size_t length_answer_tube, pos;
    char *answering_tube;
    
    pos = 0;
    memcpy(&type, buf + pos, sizeof(char));
    pos += sizeof(char);

    memcpy(&uid, buf + pos, sizeof(uid_t));
    pos += sizeof(uid_t);

    memcpy(&gid, buf + pos, sizeof(gid_t));
    pos += sizeof(gid_t);

    memcpy(&length_answer_tube, buf + pos, sizeof(size_t));
    pos += sizeof(size_t);

    answering_tube = malloc(length_answer_tube + 1);
    if (answering_tube == NULL)
        ERROR_EXIT(56789);
    memcpy(answering_tube, buf + pos, length_answer_tube);
    answering_tube[length_answer_tube] = '\0';
    pos += length_answer_tube;

    if (type == 'i')
    {
        char *printer_name, *filename;
        size_t length_printer_name, length_filename;

        memcpy(&length_printer_name, buf + pos, sizeof(size_t));
        pos += sizeof(size_t);

        printer_name = malloc(length_printer_name + 1);
        memcpy(printer_name, buf + pos, length_printer_name);
        printer_name[length_printer_name] = '\0';
        pos += length_printer_name;

        memcpy(&length_filename, buf + pos, sizeof(size_t));
        pos += sizeof(size_t);

        filename = malloc(length_filename + 1);
        memcpy(filename, buf+pos, length_filename);
        filename[length_filename] = '\0';
        pos +=length_filename;

        if (try_rights_on_file(uid, gid, filename) != 0)
            write_answer(answering_tube, NO_RIGHTS); 
        else
        {
            if ((id = send_to_printer(printer_name, filename)) == 0)
                write_answer(answering_tube, UNKNOWN_PRINTER_NAME);
            else
                write_answer(answering_tube, id);
        }

    }
    else
        printf("NOT NOW\n");
}

void
write_answer(const char *tube, int answer)
{
    int fd;

    fd = open(tube, O_WRONLY);
    if (fd == -1)
        ERROR_EXIT(45678);

    write(fd, &answer, sizeof(int));

    close(fd);
}

void work()
{
    int fd;
    char *buffer;
    size_t bytes_read;
    unsigned int msg_length;
    struct node *p;

    fd = open(receiving_tube, O_RDONLY | O_NONBLOCK);
    if (fd == -1)
        ERROR_EXIT(567890);

    while(1)
    {
        bytes_read = read(fd, &msg_length, sizeof(unsigned int));
        if (bytes_read > 0 && msg_length > 0)
        {
            buffer = malloc(msg_length);
            read(fd, buffer, msg_length);
            process_msg(buffer);
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
                if (bytes_read_in_file == 0)
                {
                    write(current_printer->fd_printer, END_OF_PRINT, 10);
                    close(current_printer->fd_current_file);
                    current_printer->fd_current_file = -2;

                }
                else
                    write(current_printer->fd_printer, print_buffer, bytes_read_in_file);
            }
            else
            {
                if ((current_printer->wl).length > 0)
                {
                    struct waiting *current_data;
                    current_data = (struct waiting *)(pop(&(current_printer->wl))->data);

                    current_printer->fd_current_file = open(current_data->filename, O_RDONLY);
                    if (current_printer->fd_current_file == -1)
                    {
                        perror("opening...\n");
                        current_printer->fd_current_file = -2;
                    }
                }
            }
        }

        sleep(1);
    }
}


int 
main(int argc, char **argv)
{
    int receiving_tube_set, config_file_set, cpt;

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

    

    printf("TUBE : %s\nCONF : %s\n", receiving_tube, config_file);
    init_config_file();
    work();


    return EXIT_SUCCESS;
}
