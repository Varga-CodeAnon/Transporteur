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

// Côté émetteur, il faut désormais un temporisateur par paquet. TODO: OK!

// Lors de l'expiration d'un temporisateur on ne renvoi que le
// paquet correspondant et donc pas toute la fenêtre /!\ TODO:

// Quand on reçoit un ACK, il ne vaut que pour ce paquet.
// Donc il y a deux solutions :
// - si ce n'est pas le premier paquet en attente d'ack, on met à
//   vrai un booléen (TODO:) dans un tableau ack_recu (TODO: OK! ).
// - si c'est le premier paquet, on fait avancer la fenêtre tant
//   qu'on le peut. C'est à dire pour ce paquet et pour tous ceux 
//   précédemment reçus (connus grâce au tableau ack_recu) TODO:

/* =============================== */
/* Programme principal - émetteur  */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message de l'application */
    int taille_msg; /* taille du message */
    int code_retour; /* code de retour de la fonction attendre */
    paquet_t reponse;
    paquet_t tableau_de_paquet[SEQ_NUM_SIZE]; /* fenêtre de 16 paquets */
    int borne_inf;
    unsigned int curseur;
    int valide;
    init_reseau(EMISSION);
    int tab_ack_recu[SEQ_NUM_SIZE];
    int ack_attendu = 0;
    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* lecture de donnees provenant de la couche application */
    borne_inf = 0;
    curseur = 0;
    /* tant que l'émetteur a des données à envoyer */
    de_application(message, &taille_msg);
    while ( taille_msg != 0 || borne_inf != curseur) {
        
        
        if (dans_fenetre(borne_inf,curseur,TAILLE_FEN) && taille_msg)
        {   
            /* construction paquet */
            for (int i=0; i<taille_msg; i++) {
                tableau_de_paquet[curseur].info[i] = message[i];
            }
            tableau_de_paquet[curseur].lg_info = taille_msg;
            tableau_de_paquet[curseur].type = DATA;
            tableau_de_paquet[curseur].num_seq = curseur;
            tableau_de_paquet[curseur].somme_ctrl = generer_controle(tableau_de_paquet[curseur]);

            vers_reseau(&tableau_de_paquet[curseur]);
            
            depart_temporisateur(curseur,50);  // on démarre un temporisateur par paquet.
            
            curseur = (curseur + 1)%SEQ_NUM_SIZE;
            de_application(message, &taille_msg);
        }
        else {  /* Dès que curseur sort de la fenêtre, on envoi tout sans attendre, puis une fois tout envoyé, on attend */
            code_retour = attendre();
            if (code_retour == PAQUET_RECU){  /* Si on reçoit un ACK... */
                de_reseau(&reponse);
                if (verifier_controle(reponse) && dans_fenetre(borne_inf,reponse.num_seq,TAILLE_FEN)){  /* ... et qu'il est conforme */
                    if (ack_attendu == reponse.num_seq){  /* S'il correspond au paquet attendu */
                        tab_ack_recu[reponse.num_seq] = 1;
                        while (tab_ack_recu[ack_attendu]==1){ // tant que les acquittements attendu ont été reçu, on décale la fenêtre
                            /* décalage de la fenêtre */
                            arreter_temporisateur(ack_attendu);
                            borne_inf = ack_attendu;
                            ack_attendu = (ack_attendu+1) % SEQ_NUM_SIZE;
                        }  
                    }
                    else {  // Sinon on met à vrai le booléen dans tab_ack_recu
                        arreter_temporisateur(reponse.num_seq);
                        tab_ack_recu[reponse.num_seq] = 1;
                    }
                }
            }
            else {
                /* Cas du timeout : réemission du paquet manquant (selective repeat) */
                int i = borne_inf;
                depart_temporisateur(borne_inf,50);
                vers_reseau(&tableau_de_paquet[borne_inf]);
                i = (i+1) % SEQ_NUM_SIZE;
            }
        }
    }
    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}
