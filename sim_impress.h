#ifndef SIM_IMPRESS_H
#define SIM_IMPRESS_H

#define EOF_SIZE 10
#define BUFFER_SIZE 64

#ifndef USAGE
#define USAGE(name) \
    do { \
        printf("%s -t printer_pipe -n printer_name", name); \
    } while (0)
#endif

#ifndef USAGE_ERROR
#define USAGE_ERROR(name, id, ...) \
    do { \
        ERROR(id, __VA_ARGS__); \
        USAGE(name); \
    } while (0)
#endif


struct imprimante_info
{
    const char *tube_name;          /* Tube recevant les données. */
    const char *imprimante_name;    /* Nom de l'imprimante. */
};


/*
 *  Fonction créant le tube de l'imprimante.
 */
void create_tube(void);

/*
 *  Fonction qui vérifie si la fin de l'envoie a été reçu.
 */
int has_eof(char *buffer, size_t buffer_size);

/*
 *  Fonction qui effectue la lecture des fichier envoyé par le serveur.
 */
void work(void);

#endif /* end of include guard: SIM_IMPRESS_H */

