#include <stdio.h>
#include "couche_transport.h"
#include "services_reseau.h"

/* ************************************** */
/* Fonctions utilitaires couche transport */
/* ************************************** */

/* Ajouter ici vos fonctions utilitaires */
uint8_t generer_controle(paquet_t p){
    uint8_t somme_ctrl = 0;
    somme_ctrl ^= p.type;
    somme_ctrl ^= p.num_seq;
    somme_ctrl ^= p.lg_info;
    for (int i=0; i<p.lg_info; i++){
        somme_ctrl ^= p.info[i];
    }
    return somme_ctrl;
}


int verifier_controle(paquet_t p){
    p.somme_ctrl ^= p.type;
    p.somme_ctrl ^= p.num_seq;
    p.somme_ctrl ^= p.lg_info;
    for (int i=0; i<p.lg_info; i++){
        p.somme_ctrl ^= p.info[i];
    }
    return (p.somme_ctrl == 0);
}


/* ======================================================================= */
/* =================== Fenêtre d'anticipation ============================ */
/* ======================================================================= */

/*--------------------------------------*/
/* Fonction d'inclusion dans la fenetre */
/*--------------------------------------*/
int dans_fenetre(unsigned int inf, unsigned int pointeur, int taille) {

    unsigned int sup = (inf+taille-1) % SEQ_NUM_SIZE;

    return
        /* inf <= pointeur <= sup */
        ( inf <= sup && pointeur >= inf && pointeur <= sup ) ||
        /* sup < inf <= pointeur */
        ( sup < inf && pointeur >= inf) ||
        /* pointeur <= sup < inf */
        ( sup < inf && pointeur <= sup);
}
