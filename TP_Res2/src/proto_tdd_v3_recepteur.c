/*************************************************************
* proto_tdd_v0 -  récepteur                                  *
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


/* =============================== */
/* Programme principal - récepteur */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message pour l'application */
    paquet_t paquet; /* paquet utilisé par le protocole */
    paquet_t reponse;
    int fin = 0; /* condition d'arrêt */
    init_reseau(RECEPTION);
    int verif_num = 0;
    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* tant que le récepteur reçoit des données */
    while ( !fin ) {

        // attendre(); /* optionnel ici car de_reseau() fct bloquante */

        de_reseau(&paquet);

        if (!verifier_controle(paquet)){
            printf("[!] ERREUR\n");
        }
        else {
            if (paquet.num_seq != verif_num){
            printf("[!] DOUBLON\nverif_num %d   num_seq %d",verif_num,paquet.num_seq);
            reponse.type = ACK;
            /* On drop le paquet, mais on acquitte */
            }
            else {
                reponse.type = ACK;
                /* remise des données à la couche application */
                fin = vers_application(message, paquet.lg_info);
                verif_num = (verif_num + 1) % NUMEROTATION;
                
                /* extraction des donnees du paquet recu */
                for (int i=0; i<paquet.lg_info; i++) {
                    message[i] = paquet.info[i];
                    // printf("COUCOU%d\n",i);
                }
            }
            vers_reseau(&reponse);
        }
    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}
