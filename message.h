#ifndef MESSAGE_H
#define MESSAGE_H

#include <sys/types.h>

#define ANSWERING_TUBE_SIZE 64
#define CONSTANT_SIZE_REQUEST   \
    (sizeof(char) + sizeof(uid_t) + sizeof(gid_t) + ANSWERING_TUBE_SIZE)

/*
 *  Structure d'envoie de message.
 *
 *  La variable buf correspond aux données de chacun des types différents.
 */
struct sending_message
{
    char type;
    uid_t uid;
    gid_t gid;
    char answering_tube[ANSWERING_TUBE_SIZE];
    size_t buf_size;
    void *buf;
};

/*
 *  Créer un nom de tube aléatoire, dirigé vers le dossier /tmp/. 
 *  Le seed permet de placer un jeu de caractère correspond à un type
 *  particulier. Il est utilisé aléatoirement.
 */
void create_random_tube_name(char tube[ANSWERING_TUBE_SIZE], char *seed);

/*
 *  Créer le tube au nom donné.
 *  Renvoie -1 si le tube n'a pu être créé.
 */
int create_tube(const char *name);

/*
 *  Créer un message (c'est à dire un buffer contigüe) à partir de la structure
 *  message donnée en argument.
 *  Le message sera toujours donné dans l'ordre suivant :
 *      - Taille totale du message
 *      - Type du message
 *      - uid
 *      - gid
 *      - Nom du tube de réponse
 *      - Taille du buffer
 *      - Buffer
 */
void *create_message(struct sending_message m);

/*
 *  Envoie le message vers le tube donné par la variable globale IMP_PATH.
 *  Retourne 0 si l'envoi s'est bien passé.
 */
int send_message(const char * const tube, const void* const message);

/*
 *  Récupère la réponse envoyée par le serveur.
 *
 *  Renvoie la réponse du serveur.
 *  La serveur est ensuite interprétée selon le type du message envoyé.
 */
int get_answer(const char * const answer_tube);

/*
 *  Affiche la réponse envoyée par le serveur.
 *  (Ce cas ne sert que pour l'affichage de la liste des impression)
 */
void print_answer(char[ANSWERING_TUBE_SIZE]);

#endif /* end of include guard: MESSAGE_H */

