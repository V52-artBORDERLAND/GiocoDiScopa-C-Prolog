#include "tavolo.h"
#include <string.h>

void inizializza_tavolo(Tavolo *t){ t->num_carte = 0; }

void aggiungi_carta_tavolo(Tavolo *t, Carta c){
    if (t->num_carte < MAX_CARTE_TAVOLO) t->carte[t->num_carte++] = c;
}

void rimuovi_carta_tavolo(Tavolo *t, int indice){
    if (indice < 0 || indice >= t->num_carte) return;
    for (int i = indice; i < t->num_carte - 1; i++) t->carte[i] = t->carte[i+1];
    t->num_carte--;
}

void rimuovi_presa_tavolo(Tavolo *t, Presa *p){
    // Rimuove dal piu' alto al piu' basso per non sballare indici
    for (int k = p->num_carte - 1; k >= 0; k--){
        rimuovi_carta_tavolo(t, p->indici[k]);
    }
}

// Backtracking per trovare combinazioni che sommano a valore
static void cerca_combinazioni(Tavolo *t, int valore, int start, int somma_attuale, int usati[], int n_usati, Presa prese[], int *count, int max_prese){
    if (*count >= max_prese) return;
    if (somma_attuale == valore && n_usati > 0){
        Presa *pr = &prese[*count];
        pr->num_carte = n_usati;
        pr->somma = valore;
        for(int i=0;i<n_usati;i++) pr->indici[i]=usati[i];
        // ordina indici decrescente per rimozione facile
        for(int i=0;i<n_usati;i++) for(int j=i+1;j<n_usati;j++) if(pr->indici[i]<pr->indici[j]){int tmp=pr->indici[i]; pr->indici[i]=pr->indici[j]; pr->indici[j]=tmp;}
        (*count)++;
        return;
    }
    if (somma_attuale > valore) return;
    for (int i = start; i < t->num_carte; i++){
        usati[n_usati] = i;
        cerca_combinazioni(t, valore, i+1, somma_attuale + t->carte[i].valore, usati, n_usati+1, prese, count, max_prese);
    }
}

int trova_prese_per_valore(Tavolo *t, int valore, Presa prese[], int max_prese){
    int count = 0;
    // Prima cerca prese singole (priorita' regola Scopa)
    for (int i = 0; i < t->num_carte; i++){
        if (t->carte[i].valore == valore){
            if (count < max_prese){
                prese[count].num_carte = 1;
                prese[count].indici[0] = i;
                prese[count].somma = valore;
                count++;
            }
        }
    }
    if (count > 0) return count; // Se c'e' presa singola, per regola si DEVE prendere quella

    int usati[MAX_CARTE_TAVOLO];
    cerca_combinazioni(t, valore, 0, 0, usati, 0, prese, &count, max_prese);
    return count;
}
