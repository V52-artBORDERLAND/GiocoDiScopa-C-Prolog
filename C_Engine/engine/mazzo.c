#include "mazzo.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void inizializza_mazzo(Mazzo *m) {
    int i = 0;
    for (int s = 0; s < 4; s++) {
        for (int v = 1; v <= 10; v++) {
            m->carte[i].valore = v;
            m->carte[i].seme = (Seme)s;
            i++;
        }
    }
    m->testa = 0;
    m->num_rimaste = 40;
}

void mescola_mazzo(Mazzo *m) {
    // srand va chiamato una volta sola nel main, ma lo teniamo sicuro
    for (int i = 39; i > 0; i--) {
        int j = rand() % (i + 1);
        Carta tmp = m->carte[i];
        m->carte[i] = m->carte[j];
        m->carte[j] = tmp;
    }
    m->testa = 0;
    m->num_rimaste = 40;
}

Carta pesca_carta(Mazzo *m) {
    if (m->testa < 40) {
        m->num_rimaste--;
        return m->carte[m->testa++];
    }
    Carta vuota = {0, DENARI};
    return vuota;
}

int mazzo_vuoto(Mazzo *m) { return m->testa >= 40; }

const char* nome_seme(Seme s) {
    switch(s){ case DENARI: return "Denari"; case COPPE: return "Coppe"; case BASTONI: return "Bastoni"; case SPADE: return "Spade"; default: return "?"; }
}
const char* nome_valore(int v) {
    switch(v){ case 1: return "A"; case 8: return "F"; case 9: return "C"; case 10: return "R"; default: { static char buf[4]; snprintf(buf,4,"%d",v); return buf; } }
}
