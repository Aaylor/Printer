#ifndef INIT_SIMULATEURS_H
#define INIT_SIMULATEURS_H

/*
 *  Fonction s'occupant de lire le fichier de configuration et de lancer chacun
 *  des processus d'imprimante.
 *  Si une ligne ne correspond pas au format demandé, alors il passe
 *  automatiquement à la suivante sans tenter d'ouvrir de processus.
 */
void create_processus(const char *file);

#endif /* end of include guard: INIT_SIMULATEURS_H */

