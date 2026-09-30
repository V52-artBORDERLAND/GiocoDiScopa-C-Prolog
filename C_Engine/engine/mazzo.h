#ifndef MAZZO_H
#define MAZZO_H

typedef enum { DENARI = 0, COPPE, BASTONI, SPADE } Seme;

typedef struct {
    int valore; // 1-10 (1 Asso, 8 Fante, 9 Cavallo, 10 Re)
    Seme seme;
} Carta;

typedef struct {
    Carta carte[40];
    int testa;
    int num_rimaste;
} Mazzo;

void inizializza_mazzo(Mazzo *m);
void mescola_mazzo(Mazzo *m);
Carta pesca_carta(Mazzo *m);
int mazzo_vuoto(Mazzo *m);
const char* nome_seme(Seme s);
const char* nome_valore(int v);

#endif
