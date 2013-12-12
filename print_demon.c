#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "error.h"
#include "print_demon.h"

static char receiving_tube[512];
static char config_file[512];


int main(int argc, char **argv)
{
    int tube_flag, cfg_flag;
    int optchar;

    tube_flag = 0;
    cfg_flag = 0;

    opterr = 0;

    while((optchar = getopt(argc, argv, "t:c:")) != -1)
    {
        switch(optchar)
        {
            case 't':
                if (tube_flag == 1)
                    ERROR_OPT(MORE_THAN_ONCE, optchar);
                tube_flag = 1;
                if (strlen(optarg) < 512)
                    strcpy(receiving_tube, optarg);
                else
                    ERROR_MSG(1, "Nom de tube trop long\n", "");
                break;
            case 'c':
                if (cfg_flag == 1)
                    ERROR_MSG(1, "Plus d'un argument -c...\n", "");
                cfg_flag = 1;
                if (strlen(optarg) < 512)
                    strcpy(config_file, optarg);
                else
                    ERROR_MSG(1, "Nom de tube trop long\n", "");
                break;
            case '?':
                if (optopt == 't' || optopt == 'c')
                    ERROR_MSG(1, "L'option -%c attend un argument\n", optopt);
                else if (isprint(optopt))
                    ERROR_MSG(1, "Option inconnu `-%c`\n", optopt);
                else 
                    ERROR_MSG(1, "Caractère inconnu `-\\x%x`\n", optopt);
                break;
            default:
                exit(EXIT_FAILURE);
        }
    }

    if (tube_flag == 0) 
        ERROR_MSG(1, "Argument -t manquant\n", "");

    if (cfg_flag == 0)
        ERROR_MSG(1, "Argument -c manquant\n", "");

    printf("%s\n%s\n", receiving_tube, config_file);


    return EXIT_SUCCESS;
}
