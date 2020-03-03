/*************************************************************
* proto_tdd_v0 -  émetteur                                   *
* TRANSFERT DE DONNEES  v0                                   *
*                                                            *
* Protocole sans contrôle de flux, sans reprise sur erreurs  *
*                                                            *
* E. Lavinal - Univ. de Toulouse III - Paul Sabatier         *
**************************************************************/

#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

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

/* =============================== */
/* Programme principal - émetteur  */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message de l'application */
    int taille_msg; /* taille du message */
    int code_retour; /* code de retour de la fonction attendre */
    paquet_t paquet; /* paquet utilisé par le protocole */
    paquet_t reponse;

    init_reseau(EMISSION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* lecture de donnees provenant de la couche application */
    de_application(message, &taille_msg);
    paquet.num_seq = 0;
    /* tant que l'émetteur a des données à envoyer */
    while ( taille_msg != 0 ) {

        /* construction paquet */
        for (int i=0; i<taille_msg; i++) {
            paquet.info[i] = message[i];
        }
        paquet.lg_info = taille_msg;
        paquet.type = DATA;
        paquet.somme_ctrl = generer_controle(paquet);

        do{
            /* remise à la couche reseau */
            depart_temporisateur(1,100);
            vers_reseau(&paquet);
            code_retour = attendre();
        } while (code_retour != -1);  /* tant que timeout */

        arreter_temporisateur(1);
        // reponse.type = ACK;
        de_reseau(&reponse); // on vide le bufbuf

        /* lecture des donnees suivantes de la couche application */
        de_application(message, &taille_msg);
        paquet.num_seq = (paquet.num_seq + 1)%2;
    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}
