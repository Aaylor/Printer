#ifndef MPR_H
#define MPR_H

#include <stdio.h>

#include "error.h"

#ifndef USAGE
#define USAGE(name) \
    do { \
        printf("%s -P imprimante fichier\n%s -L [imprimante]\n%s -C id", \
            name, name, name); \
    } while (0)
#endif

#ifndef USAGE_ERROR
#define USAGE_ERROR(name, id, ...) \
    do { \
        ERROR(id, __VA_ARGS__); \
        USAGE(name); \
    } while (0)
#endif



/*
 *  Vérifie si la chaîne de caractère donnée est un nombre.
 *  Renvoie 1 si c'est un nombre, 0 sinon.
 */
int is_number(const char *src);

/*
 *  Transforme la référence en référence absolue.
 */
char *to_abs_ref(char *reference);

/*
 *  Remplie msg à l'aide des données communes à chacun des types de messages
 *  possible.
 *  C'est à dire l'uid, le gid et le tube de réponse.
 */
void get_base_message(struct sending_message *msg);

/*
 *  Renvoie un buffer contenant le message lié au type d'impression.
 */
void *get_printing_message(const char *const imp_name, 
        const char *const filename); 

/*
 *  Renvoie un buffer contenant le message lié au type d'annulation 
 *  d'impression.
 */
void *get_canceling_message(int id);

/*
 *  Renvoie un buffer contenant le message lié au type d'affiche de la liste
 *  des impressions.
 */
void *get_listing_message(char *name);

/*
 *  Fonction qui gère les réponses du serveur selon le type du message envoyé.
 */
void handle_answer(void);


#endif /* end of include guard: MPR_H */

