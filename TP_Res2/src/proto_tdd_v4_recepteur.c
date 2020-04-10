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

// Côté récepteur, il faut stocker les paquets corrects reçus dans le 
// désordre en attendant de recevoir le premier paquet manquant.
//
// => On stockera ces paquets dans un tableau comme côté émetteur
//    et on utilisera la fonction dans_fenetre() pour éviter les
//    débordements tampons

/* =============================== */
/* Programme principal - récepteur */
/* =============================== */

// FIXME: Programme non fonctionnel

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
    paquet_t buffer[TAILLE_FEN];
    int taille_buffer = 0;

    /* tant que le récepteur reçoit des données */
    while (!fin)
    {
        de_reseau(&paquet);
        if (verifier_controle(paquet))
        {
            if (paquet.num_seq == verif_num)
            {
                reponse.type = ACK;
                verif_num = (verif_num + 1) % SEQ_NUM_SIZE;

                /* Extraction des donnees du paquet recu */
                for (int i = 0; i < paquet.lg_info; i++)
                {
                    message[i] = paquet.info[i];
                }
                /* Remise des données à la couche application */
                fin = vers_application(message, paquet.lg_info);

                if (taille_buffer != 0)
                { /* Si des paquets ont été stocké, on les transmets à la couche application */
                    do
                    {
                        paquet = buffer[taille_buffer - 1];
                        reponse.type = ACK;
                        verif_num = (verif_num + 1) % SEQ_NUM_SIZE;
                        for (int i = 0; i < paquet.lg_info; i++)
                        {
                            message[i] = paquet.info[i];
                        }
                        fin = vers_application(message, paquet.lg_info);
                        taille_buffer--;
                    } while (taille_buffer > 0);
                }
            }
            else  /* Si on reçoit un paquet non attendu ... */
            { /* ... On stocke le paquet dans un buffer */
                buffer[taille_buffer] = paquet;
                taille_buffer++;
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
