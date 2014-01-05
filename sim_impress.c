#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>

#include "error.h"
#include "sim_impress.h"

/*
 *  Contient toutes les informations vitales de l'imprimante.
 */
struct imprimante_info infos;

void
create_tube(void)
{
    if (mkfifo(infos.tube_name, S_IRWXU | S_IRWXG | S_IRWXO) == -1)
        ERROR_E(30, "Erreur lors de la création du tube de récéption.\n");
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
        ERROR_E(10, "Erreur lors de l'ouverture du tube de lecture `%s`.\n",
                infos.tube_name);

    fd_writing = open("/dev/null", O_WRONLY);
    if (fd_writing == -1)
        ERROR_E(10, "Erreur lors de l'ouverture de /dev/null.");

    total_read = 0;
    while((bytes_read = read(fd_reading, buffer, BUFFER_SIZE)) > 0)
    {
        if (has_eof(buffer, bytes_read) == 0)
        {
            write(fd_writing, buffer, (bytes_read - 10));
            total_read += (bytes_read - 10);
            sleep(bytes_read);
            total_read = 0;
        }
        else
        {
            write(fd_writing, buffer, bytes_read);
            total_read += bytes_read;
        }
    }

    close(fd_reading);
    close(fd_writing);
}

int 
main(int argc, const char **argv)
{
    int tube_set, name_set, cpt;

    if (argc != 5)
        USAGE_ERROR(argv[0], 10, "Nombre d'arguments invalide...\n");

    tube_set = 0;
    name_set = 0;
    cpt = 1;
    while (cpt < argc)
    {
        if (strcmp(argv[cpt], "-t") == 0)
        {
            if (tube_set == 1)
                USAGE_ERROR(argv[0], 10, "Argument -t déjà saisi.\n");

            tube_set = 1;
            infos.tube_name = argv[++cpt];
        }
        else if (strcmp(argv[cpt], "-n") == 0)
        {
            if (name_set == 1)
                USAGE_ERROR(argv[0], 10, "Argument -n déjà saisi.\n");

            name_set = 1;
            infos.tube_name = argv[++cpt];
        }
        else
            USAGE_ERROR(argv[0], 10, "Argument inconnu.\n");

        ++cpt;
    }

    create_tube();
    work();

    return EXIT_SUCCESS;
}
