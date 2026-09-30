#ifndef RENDER_H
#define RENDER_H
#include "../engine/gioco.h"
#include "raylib.h"

void carica_texture_carte(void);
void scarica_texture_carte(void);
void disegna_carta(int x, int y, Carta c, int selezionata, int evidenziata);
void disegna_tavolo_grafico(Partita *p, int* selezione_tavolo, int num_sel);
void disegna_mano_grafica(Partita *p, int giocatore, int carta_sel);
void disegna_hud(Partita *p);

#endif
