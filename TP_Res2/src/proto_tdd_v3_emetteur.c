/*************************************************************
* proto_tdd_v0 -  émetteur                                   *
* TRANSFERT DE DONNEES  v0                                   *
*                                                            *
* Protocole sans contrôle de flux, sans reprise sur erreurs  *
*                                                            *
* E. Lavinal - Univ. de Toulouse III - Paul Sabatier         *
**************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

/* =============================== */
/* Programme principal - émetteur  */
/* =============================== */
int main(int argc, char *argv[])
{
    unsigned char message[MAX_INFO]; /* message de l'application */
    int taille_msg;                  /* taille du message */
    int code_retour;                 /* code de retour de la fonction attendre */
    paquet_t reponse;
    paquet_t tableau_de_paquet[SEQ_NUM_SIZE]; /* fenêtre de 8 paquets */
    int borne_inf;
    unsigned int curseur;
    int valide;
    init_reseau(EMISSION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* lecture de donnees provenant de la couche application */
    borne_inf = 0;
    curseur = 0;
    de_application(message, &taille_msg);
    /* Tant que l'émetteur a des données à envoyer, on envoie... */
    while (taille_msg != 0 || borne_inf != curseur)
    {
        if (dans_fenetre(borne_inf, curseur, TAILLE_FEN) && taille_msg)
        {
            /* construction paquet */
            for (int i = 0; i < taille_msg; i++)
            {
                tableau_de_paquet[curseur].info[i] = message[i];
            }
            tableau_de_paquet[curseur].lg_info = taille_msg;
            tableau_de_paquet[curseur].type = DATA;
            tableau_de_paquet[curseur].num_seq = curseur;
            tableau_de_paquet[curseur].somme_ctrl = generer_controle(tableau_de_paquet[curseur]);

            vers_reseau(&tableau_de_paquet[curseur]);

            if (borne_inf == curseur)
            { /* On démarre le temporisateur pour la fenêtre d'emission */
                depart_temporisateur(1, 50);
            }
            curseur = (curseur + 1) % SEQ_NUM_SIZE;
            de_application(message, &taille_msg);
        }
        /* ... Puis une fois tout envoyé et qu'on est sorti de la fenêtre d'emission, on attend */
        else
        {
            code_retour = attendre();
            if (code_retour == PAQUET_RECU)
            {
                de_reseau(&reponse);
                if (verifier_controle(reponse) && dans_fenetre(borne_inf, reponse.num_seq, TAILLE_FEN))
                {   /* Si l'aquittement reçu est conforme et celui attendu ... */
                    /* ... on décale la fenêtre... */
                    borne_inf = (reponse.num_seq + 1) % SEQ_NUM_SIZE;
                    arreter_temporisateur(1);
                    if (borne_inf != curseur)
                    {
                        /* ... et s'il reste des paquets non acquittés, on attend... */
                        depart_temporisateur(1, 50);
                    }
                }
                /* ... Sinon on le drop */
            }
            else
            {
                /* Si l'on a rien reçu, on est dans le cas du timeout : réemission de tout jusqu'au curseur (go-back-n) */
                int i = borne_inf;
                depart_temporisateur(1, 50);
                while (i != curseur)
                {
                    vers_reseau(&tableau_de_paquet[i]);
                    i = (i + 1) % SEQ_NUM_SIZE;
                }
            }
        }
    }
    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}
