#ifndef TAVOLO_H
#define TAVOLO_H

#include "mazzo.h"
#define MAX_CARTE_TAVOLO 20
#define MAX_CARTE_PRESA 20

typedef struct {
    Carta carte[MAX_CARTE_TAVOLO];
    int num_carte;
} Tavolo;

typedef struct {
    int indici[MAX_CARTE_PRESA];
    int num_carte;
    int somma;
} Presa;

void inizializza_tavolo(Tavolo *t);
void aggiungi_carta_tavolo(Tavolo *t, Carta c);
void rimuovi_carta_tavolo(Tavolo *t, int indice);
void rimuovi_presa_tavolo(Tavolo *t, Presa *p);

// Trova tutte le prese possibili per un valore
int trova_prese_per_valore(Tavolo *t, int valore, Presa prese[], int max_prese);

#endif
