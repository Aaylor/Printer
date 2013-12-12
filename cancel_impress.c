#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char **argv)
{
    printf("CANCEL IMPRESS\n");

#ifdef _POSIX_C_SOURCE
    printf("POSIX\n");
#endif

    return EXIT_SUCCESS;
}
