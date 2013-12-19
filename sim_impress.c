#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>

#include "error.h"
#include "constants.h"
#include "sim_impress.h"

struct imprimante_info infos;

void
create_tube(void)
{
    if (mkfifo(infos.tube_name, S_IRWXU | S_IRWXG | S_IRWXO) == -1)
        ERROR_EXIT(12455);
}

int 
has_eof(char *buffer, size_t buffer_size)
{
    int cpt;

    if (buffer_size < EOF_SIZE)
        return 1;

    cpt = buffer_size - 1;
    while(cpt > (buffer_size - EOF_SIZE - 1))
    {
        if (buffer[cpt--] != '\0')
            return 2;
    }

    return 0;
}

void
work(void)
{
    int fd_reading, fd_writing;
    size_t bytes_read, total_read;
    char buffer[BUFFER_SIZE];

    fd_reading = open(infos.tube_name, O_RDONLY);
    if (fd_reading == -1)
        ERROR_EXIT(124);

    fd_writing = open("/dev/null", O_WRONLY);
    if (fd_writing == -1)
        ERROR_EXIT(4567);

    total_read = 0;
    while((bytes_read = read(fd_reading, buffer, BUFFER_SIZE)) > 0)
    {
        if (has_eof(buffer, bytes_read) == 0)
        {
            write(fd_writing, buffer, (bytes_read - 10));
            total_read += (bytes_read - 10);
            sleep((total_read/10000));
        }
        else
        {
            write(fd_writing, buffer, bytes_read);
            total_read += bytes_read;
        }
    }

    if (bytes_read == -1)
        ERROR_EXIT(56789);

    close(fd_reading);
    close(fd_writing);
}

int 
main(int argc, const char **argv)
{
    int tube_set, name_set, cpt;

    if (argc != 5)
        ERROR_MSG(10, "Nombre d'arguments invalide...\n%s", "");

    cpt = 1;
    while (cpt < argc)
    {
        if (strlen(argv[cpt]) == 2 && argv[cpt][0] == '-' && (cpt + 1) < argc) 
        {
            if (argv[cpt][1] == 't')
            {
               if (tube_set == 1)
                   ERROR_MSG(123, "Arg -t déjà saisie...\n%s", "");
               tube_set = 1;
               
               infos.tube_name = argv[++cpt];
            }
            else if (argv[cpt][1] == 'n')
            {
                if (name_set == 1)
                    ERROR_MSG(124, "Arg -n déjà saisie...\n%s", "");
                name_set = 1;

                infos.imprimante_name = argv[++cpt];
            }
            else
                ERROR_MSG(1234, "Argument inconnu...\n%s", "");
        }
        else
            ERROR_MSG(1323, "Argument invalide...\n%s", "");

        ++cpt;
    }

    create_tube();
    work();

    return EXIT_SUCCESS;
}
