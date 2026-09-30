#ifndef GIOCO_H
#define GIOCO_H
#include "tavolo.h"

#define MAX_MANO 3
#define MAX_PRESE_GIOCATORE 40

typedef struct {
    Carta mano[MAX_MANO];
    int num_mano;
    Carta prese[MAX_PRESE_GIOCATORE];
    int num_prese;
    int scope;
    int denari;
    int settebello;
    Carta ultima_presa; // per assegnare carte rimaste a fine mano
    int ha_preso_ultimo;
} Giocatore;

typedef struct {
    Mazzo mazzo;
    Tavolo tavolo;
    Giocatore giocatori[2]; // 0 = umano, 1 = AI
    int turno; // 0 o 1
    int carte_date; // per gestire distribuzione
    Carta carte_rimanenti_tavolo_ultimo; // non serve, gestiamo con flag
    int gioco_finito;
} Partita;

void inizializza_partita(Partita *p);
int distribuisci_carte(Partita *p); // da 3 carte a testa se mano vuota e mazzo non vuoto, ritorna 1 se ha distribuito
int gioca_carta(Partita *p, int giocatore_idx, int indice_mano, int presa_scelta_idx); // ritorna 1 se scopa
void calcola_punteggio_finale(Partita *p, int punti[2]);

#endif
