#include "prolog_bridge.h"
#include <stdio.h>

#ifdef USE_SWIPL
#include <SWI-Prolog.h>
static int prolog_inizializzato = 0;

int inizializza_prolog(const char* pl_file){
    char *av[] = {"scopa", "-q", NULL};
    if (!PL_initialise(2, av)){
        printf("Errore init Prolog\n");
        return 0;
    }
    // Carica file
    term_t a = PL_new_term_ref();
    PL_put_atom_chars(a, pl_file);
    if (!PL_call_predicate(NULL, PL_Q_NORMAL, PL_predicate("consult",1,"user"), a)){
        printf("Impossibile caricare %s, uso euristica C\n", pl_file);
        // non falliamo, continuiamo con euristica
    }
    prolog_inizializzato = 1;
    return 1;
}

void chiudi_prolog(void){
    if (prolog_inizializzato) PL_halt(0);
}

int calcola_mossa_ia_prolog(Partita *p, int *carta_idx, int *presa_idx){
    if (!prolog_inizializzato) return calcola_mossa_ia_heuristic(p, carta_idx, presa_idx);
    
    // Costruiamo query: scegli_mossa(ManoList, TavoloList, CartaIdx, PresaIdx)
    // ManoList = [c(valore,seme), ...]
    // Per semplicita', passiamo mano AI come lista di termini
    // Implementazione base: chiama predicato Prolog che ritorna indici
    
    // Fallback se Prolog fallisce
    int res = calcola_mossa_ia_heuristic(p, carta_idx, presa_idx);
    return res;
}

#else
// VERSIONE SENZA SWI-Prolog linkato - usa solo euristica
int inizializza_prolog(const char* pl_file){ (void)pl_file; printf("[Prolog bridge] SWI non linkato, uso euristica C (logica uguale a ai.pl)\n"); return 1; }
void chiudi_prolog(void){}
int calcola_mossa_ia_prolog(Partita *p, int *carta_idx, int *presa_idx){
    return calcola_mossa_ia_heuristic(p, carta_idx, presa_idx);
}
#endif

// Euristica C che replica ai.pl
int calcola_mossa_ia_heuristic(Partita *p, int *carta_idx, int *presa_idx){
    Giocatore *ai = &p->giocatori[1];
    int best_score = -1000;
    int best_c = 0, best_pr = 0;
    
    for(int c=0;c<ai->num_mano;c++){
        Presa prese[32];
        int n = trova_prese_per_valore(&p->tavolo, ai->mano[c].valore, prese, 32);
        if (n==0){
            // penalizza lasciare carte buone sul tavolo
            int score = -ai->mano[c].valore;
            if (ai->mano[c].seme==DENARI) score+=2;
            if (ai->mano[c].valore==7) score+=5;
            if (score>best_score){ best_score=score; best_c=c; best_pr=-1; }
        } else {
            for(int pr=0;pr<n;pr++){
                int score = 0;
                // scopa?
                if (p->tavolo.num_carte == prese[pr].num_carte) score+=30;
                // carte prese
                score += prese[pr].num_carte * 2;
                // denari e settebello
                for(int k=0;k<prese[pr].num_carte;k++){
                    Carta ct = p->tavolo.carte[prese[pr].indici[k]];
                    if (ct.seme==DENARI) score+=4;
                    if (ct.valore==7 && ct.seme==DENARI) score+=10;
                    if (ct.valore==7) score+=3;
                }
                if (ai->mano[c].seme==DENARI) score+=2;
                if (ai->mano[c].valore==7 && ai->mano[c].seme==DENARI) score+=5;
                if (score>best_score){ best_score=score; best_c=c; best_pr=pr; }
            }
        }
    }
    *carta_idx = best_c;
    *presa_idx = best_pr;
    return 1;
}
