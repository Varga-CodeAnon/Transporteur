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

// Côté récepteur :
// au lieu de jeter les paquets corrects qui sont reçus hors séquence il faut les stocker en attendant de recevoir le premier paquet manquant.
// Il vous faut donc un tableau de paquet comme côté émetteur et utiliser la fonction dans_fenetre
// Le protocole est bien plus efficace que le go back n.

/* =============================== */
/* Programme principal - récepteur */
/* =============================== */
int main(int argc, char *argv[])
{
    unsigned char message[MAX_INFO]; /* message pour l'application */
    paquet_t paquet;                 /* paquet utilisé par le protocole */
    paquet_t reponse;
    int fin = 0; /* condition d'arrêt */
    init_reseau(RECEPTION);
    int verif_num = 0;
    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* tant que le récepteur reçoit des données */
    while (!fin)
    {

        // attendre(); /* optionnel ici car de_reseau() fct bloquante */

        de_reseau(&paquet);
        if (verifier_controle(paquet))
        {
            if (paquet.num_seq == verif_num)
            { /* On drop le paquet, mais on acquitte */
                reponse.type = ACK;
                verif_num = (verif_num + 1) % SEQ_NUM_SIZE;

                /* extraction des donnees du paquet recu */
                for (int i = 0; i < paquet.lg_info; i++)
                {
                    message[i] = paquet.info[i];
                    
                }
                                /* remise des données à la couche application */
                fin = vers_application(message, paquet.lg_info);
            }
            reponse.type = ACK;
            reponse.lg_info = 0;
            reponse.num_seq = (verif_num == 0 ? SEQ_NUM_SIZE - 1 : verif_num - 1);
            reponse.somme_ctrl = generer_controle(reponse);
            vers_reseau(&reponse);
        }
    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}
