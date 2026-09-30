#include "gioco.h"
#include <stdio.h>

void inizializza_partita(Partita *p){
    inizializza_mazzo(&p->mazzo);
    mescola_mazzo(&p->mazzo);
    inizializza_tavolo(&p->tavolo);
    for(int g=0;g<2;g++){
        p->giocatori[g].num_mano=0;
        p->giocatori[g].num_prese=0;
        p->giocatori[g].scope=0;
        p->giocatori[g].denari=0;
        p->giocatori[g].settebello=0;
        p->giocatori[g].ha_preso_ultimo=0;
    }
    // 4 carte sul tavolo
    for(int i=0;i<4;i++) aggiungi_carta_tavolo(&p->tavolo, pesca_carta(&p->mazzo));
    p->turno=0;
    p->gioco_finito=0;
    distribuisci_carte(p);
}

int distribuisci_carte(Partita *p){
    if (p->giocatori[0].num_mano!=0 || p->giocatori[1].num_mano!=0) return 0;
    if (mazzo_vuoto(&p->mazzo)) {
        // controlla fine gioco
        if (p->tavolo.num_carte>0){
            // assegna carte rimaste a chi ha preso ultimo
            int ultimo = -1;
            for(int g=0;g<2;g++) if(p->giocatori[g].ha_preso_ultimo) ultimo=g;
            if (ultimo==-1) ultimo=0;
            for(int i=0;i<p->tavolo.num_carte;i++){
                Giocatore *gg=&p->giocatori[ultimo];
                gg->prese[gg->num_prese++]=p->tavolo.carte[i];
                if (p->tavolo.carte[i].seme==DENARI) gg->denari++;
                if (p->tavolo.carte[i].valore==7 && p->tavolo.carte[i].seme==DENARI) gg->settebello=1;
            }
            p->tavolo.num_carte=0;
        }
        p->gioco_finito=1;
        return 0;
    }
    for(int g=0;g<2;g++){
        for(int i=0;i<3;i++){
            if (mazzo_vuoto(&p->mazzo)) break;
            p->giocatori[g].mano[i]=pesca_carta(&p->mazzo);
        }
        p->giocatori[g].num_mano=3;
    }
    return 1;
}

int gioca_carta(Partita *p, int giocatore_idx, int indice_mano, int presa_scelta_idx){
    Giocatore *g = &p->giocatori[giocatore_idx];
    if (indice_mano<0 || indice_mano>=g->num_mano) return 0;
    Carta giocata = g->mano[indice_mano];
    // rimuovi dalla mano
    for(int i=indice_mano;i<g->num_mano-1;i++) g->mano[i]=g->mano[i+1];
    g->num_mano--;

    Presa prese[32];
    int n_prese = trova_prese_per_valore(&p->tavolo, giocata.valore, prese, 32);
    
    int scopa = 0;
    if (n_prese==0){
        aggiungi_carta_tavolo(&p->tavolo, giocata);
        g->ha_preso_ultimo=0;
        for(int i=0;i<2;i++) if(i!=giocatore_idx) p->giocatori[i].ha_preso_ultimo=0; // reset altri? in realta' solo ultimo che prende
        // Corregge logica ultimo: solo chi prende setta flag
        // quindi se non prendi, non sei ultimo
    } else {
        int scelta = presa_scelta_idx;
        if (scelta<0 || scelta>=n_prese) scelta=0;
        Presa *pr = &prese[scelta];
        // prendi carte
        g->prese[g->num_prese++] = giocata;
        if (giocata.seme==DENARI) g->denari++;
        if (giocata.valore==7 && giocata.seme==DENARI) g->settebello=1;

        // prendi dal tavolo (da indice alto a basso gia' ordinato)
        // dobbiamo copiare carte prima di rimuovere
        Carta prese_tavolo[MAX_CARTE_PRESA];
        for(int k=0;k<pr->num_carte;k++) prese_tavolo[k]=p->tavolo.carte[pr->indici[k]];

        rimuovi_presa_tavolo(&p->tavolo, pr);

        for(int k=0;k<pr->num_carte;k++){
            g->prese[g->num_prese++] = prese_tavolo[k];
            if (prese_tavolo[k].seme==DENARI) g->denari++;
            if (prese_tavolo[k].valore==7 && prese_tavolo[k].seme==DENARI) g->settebello=1;
        }

        // scopa?
        if (p->tavolo.num_carte==0 && !mazzo_vuoto(&p->mazzo) ) {
            g->scope++;
            scopa=1;
        }
        // aggiorna ultimo che ha preso
        for(int i=0;i<2;i++) p->giocatori[i].ha_preso_ultimo=0;
        g->ha_preso_ultimo=1;
        g->ultima_presa=giocata;
    }

    // cambia turno
    p->turno = 1 - giocatore_idx;
    
    // se mani vuote, ridistribuisci
    if (p->giocatori[0].num_mano==0 && p->giocatori[1].num_mano==0){
        distribuisci_carte(p);
    }

    return scopa;
}

void calcola_punteggio_finale(Partita *p, int punti[2]){
    punti[0]=p->giocatori[0].scope;
    punti[1]=p->giocatori[1].scope;

    if (p->giocatori[0].num_prese > p->giocatori[1].num_prese) punti[0]++;
    else if (p->giocatori[1].num_prese > p->giocatori[0].num_prese) punti[1]++;

    if (p->giocatori[0].denari > p->giocatori[1].denari) punti[0]++;
    else if (p->giocatori[1].denari > p->giocatori[0].denari) punti[1]++;

    if (p->giocatori[0].settebello) punti[0]++;
    if (p->giocatori[1].settebello) punti[1]++;

    // primiera: calcolo semplificato valori primiera (7=21,6=18,1=16,5=15,4=14,3=13,2=12, resto 10)
    int tabella[11]={0,16,12,13,14,15,18,21,10,10,10};
    int primiera[2]={0,0};
    for(int g=0;g<2;g++){
        int best_per_seme[4]={0,0,0,0};
        for(int i=0;i<p->giocatori[g].num_prese;i++){
            Carta c=p->giocatori[g].prese[i];
            int val=tabella[c.valore];
            if (val>best_per_seme[c.seme]) best_per_seme[c.seme]=val;
        }
        primiera[g]=best_per_seme[0]+best_per_seme[1]+best_per_seme[2]+best_per_seme[3];
    }
    if (primiera[0]>primiera[1]) punti[0]++;
    else if (primiera[1]>primiera[0]) punti[1]++;
}
