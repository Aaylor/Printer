#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/wait.h>

#include "error.h"

void
create_processus(const char *file)
{
    FILE *f;
    pid_t pid;
    char buffer[128];
    char *args[6];
    char *tok;

    f = fopen(file, "r");
    if (f == NULL)
        _exit(EXIT_FAILURE);

    args[0] = "./sim_impress";
    args[1] = "-n";
    args[3] = "-t";
    args[5] = NULL;

    while (fgets(buffer, 128, f) != NULL)
    {
        tok = strchr(buffer, ' ');
        if (tok == NULL)
            continue;

        (tok++)[0] = '\0';
        tok[strlen(tok) - 1] = '\0';

        args[2] = buffer;
        args[4] = tok;

        pid = fork();
        if (pid == 0)
        {
            if (fork())
            {
                if(execvp(args[0], args) == -1)
                {
                    perror("Error for execute sim_impress");
                    _exit(EXIT_FAILURE);
                }
                _exit(EXIT_SUCCESS);
            }
            _exit(EXIT_SUCCESS);
        }
    }
}


int 
main(int argc, const char **argv)
{
    if (argc != 3)
        ERROR_EXIT(2356534);

    if (strcmp(argv[1], "-c") != 0)
        ERROR_EXIT(456789);

    create_processus(argv[2]);
    
    return EXIT_SUCCESS;
}
