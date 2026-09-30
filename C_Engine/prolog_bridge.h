#ifndef PROLOG_BRIDGE_H
#define PROLOG_BRIDGE_H
#include "gioco.h"

int inizializza_prolog(const char* pl_file);
void chiudi_prolog(void);
int calcola_mossa_ia_prolog(Partita *p, int *carta_idx, int *presa_idx); // usa Prolog se disponibile
int calcola_mossa_ia_heuristic(Partita *p, int *carta_idx, int *presa_idx); // fallback C

#endif
