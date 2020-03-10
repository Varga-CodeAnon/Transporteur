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
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message de l'application */
    int taille_msg; /* taille du message */
    int code_retour; /* code de retour de la fonction attendre */
    paquet_t paquet; /* paquet utilisé par le protocole */
    paquet_t reponse;
    paquet_t tableau_de_paquet[NUMEROTATION]; /* fenêtre de 8 paquets */
    int borne_inf;
    unsigned int curseur;

    init_reseau(EMISSION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* lecture de donnees provenant de la couche application */
    de_application(message, &taille_msg);
    borne_inf = 0;
    curseur = 0;

    /* tant que l'émetteur a des données à envoyer */
    while ( taille_msg != 0 ) {
        if (dans_fenetre(borne_inf,curseur,TAILLE_FEN))
        {    
            /* construction paquet */
            for (int i=0; i<taille_msg; i++) {
                tableau_de_paquet[curseur].info[i] = message[i];
            }
            tableau_de_paquet[curseur].lg_info = taille_msg;
            tableau_de_paquet[curseur].type = DATA;
            tableau_de_paquet[curseur].num_seq = 0;
            tableau_de_paquet[curseur].somme_ctrl = generer_controle(tableau_de_paquet[curseur]);
            /* ------------------- */

            /* remise à la couche reseau */
            vers_reseau(&paquet);
            if (borne_inf == curseur){
                depart_temporisateur(1,100); 
            }
            curseur = (curseur + 1)%NUMEROTATION;
        }
        else {  /* On envoi tout sans attendre, puis une fois tout envoyé, on attend */
            code_retour = attendre();
            if (code_retour == PAQUET_RECU){
                de_reseau(&reponse);
                if (verifier_controle(reponse) && dans_fenetre(borne_inf,reponse.num_seq,TAILLE_FEN)){
                    /* décalage de la fenêtre */
                    borne_inf = (reponse.num_seq + 1) % NUMEROTATION;
                    arreter_temporisateur(1);
                    if (borne_inf != curseur){
                        /* Reste des paquets non acquittés, on relance le timer */
                        depart_temporisateur(1,100);
                    }
                }
            }
            else {
                /* Cas du timeout : réemission de tout jusqu'au curseur (go-back-n) */
                int i = borne_inf;
                depart_temporisateur(1,100);
                while (i != curseur){
                    vers_reseau(&tableau_de_paquet[i]);
                    i = (i+1) % NUMEROTATION;
                }
            }
        }
    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}
