#ifndef PRINT_DEMON_H
#define PRINT_DEMON_H

#include "error.h"
#include "queue.h"

#ifndef USAGE
#define USAGE(name) \
    do { \
        printf("%s -t tube_path -c config_file", name); \
    } while (0)
#endif

#ifndef USAGE_ERROR
#define USAGE_ERROR(name, id, ...) \
    do { \
        ERROR(id, __VA_ARGS__); \
        USAGE(name); \
    } while (0)
#endif


#define BUFFER_SIZE 64
#define END_OF_PRINT "\0\0\0\0\0\0\0\0\0\0"

/*
 *  Deux définition permettant de différencier l'ajout dans la liste
 *  d'impression et l'ajout dans la liste d'attente.
 */
#define add_in_printer_list(q, d) add_in_queue((q), (d))
#define add_in_waiting_list(q, d) add_in_queue((q), (d))

/* 
 *  Deux définitions de types permettant de différencier la liste d'impression
 *  et la liste d'imprimantes.
 */
typedef struct queue printers_list;
typedef struct queue waiting_list;

typedef struct node *node;

struct printer
{
    char stopped;           /* Détermine si l'impression a été arrêté en cours. */
    int fd_printer;         /* Descripteur de l'imprimante. */
    char *name;             /* Nom de l'imprimante. */
    char *tube_path;        /* Nom du tube de l'imprimante/ */
    
    int fd_current_file;    /* Descripteur du fichier courant.  */
    int id_print;           /* Id de l'impression courant. */
    uid_t uid_user;         /* uid du propriétaire de l'impression. */
    char *filename;         /* Nom de fichier */
    
    waiting_list wl;        /* File d'attente. */
};

struct waiting
{
    char *filename; /* Nom du fichier en attente. */
    uid_t uid_user; /* uid du propriétaire de l'impression. */
    int id;         /* id de l'impression. */
};


/*
 *  Gère le signal d'interruption, et permet la fermeture complète de chacun
 *  des serveurs d'impression.
 */
void handle_sigint(int signo);

/*
 *  Fonction appelé suite au signal d'interruption, ou à l'arrêt du serveur.
 *  Permet d'unlinker tout les tubes, et de permettre leurs suppressions.
 */
void close_each_printer(void);

/*
 *  Ajoute une imprimante à la liste des imprimantes.
 */
void add_printer(const char *name, const char *tube);

/*
 *  Met dans la liste d'attente de l'imprimante la demande d'impression.
 *  Retourne    UNKNOWN_PRINTER_NAME si le nom d'imprimante n'existe pas,
 *              ERROR_IN_QUEUE si l'ajout dans la queue n'a pas été possible 
 *              l'id d'impression si tout s'est bien passé.
 */
int send_to_printer(const char *printer_name, const char *filename, 
        uid_t uid_user); 

/*
 *  Initialise le serveur a partir du fichier de configuration.
 */
int init_config_file(void);

/*
 *  Test les droits d'accès sur le fichier.
 *  Retourne 0 si l'utilisateur a les bons droits, 1 sinon.
 */
int try_rights_on_file(uid_t uid, gid_t gid, const char *filename);

/*
 *  Vérifie si l'id de l'impression existe et tente de le supprimer.
 *  Retourne    DONT_HAVE_ACCESS si l'utilisateur ne correspond pas
 *              UNKNOWN_ID si l'id n'existe pas
 *              REMOVED si l'impression a bien été supprimée.
 */
int check_if_id_exist(int id, uid_t uid);

/*
 *  Fini de lire le message et le traite, selon le type de message selectionné.
 */
void process_msg(unsigned int length, char *buf);

/*
 *  Ecrit la réponse vers le tube de retour.
 */
void write_answer(const char *tube, void *answer, size_t size);

/*
 *  Ecrit la liste des impressions vers le serveur de retour.
 */
void write_list(const char *, const char *);

/*
 *  Fonction qui effectue le travail du serveur.
 *  Il contient la boucle principale, qui travaille constament entre la lecture
 *  des messages qu'il reçoit, et l'envoie des impressions.
 */
void work(void);

#endif /* endif PRINT_DEMON_H */

