#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include "engine/mazzo.h"
#include "engine/tavolo.h"
#include "engine/gioco.h"
#include "prolog_bridge.h"
#include "graphics/render.h"

int main(void){
    srand(time(NULL));
    InitWindow(900, 650, "Scopa - C + Prolog + Raylib");
    SetTargetFPS(60);

    Partita partita;
    inizializza_partita(&partita);
    carica_texture_carte();
    inizializza_prolog("../Prolog_AI/ai.pl");

    int carta_selezionata = -1;
    int presa_indici_sel[20];
    int num_presa_sel = 0;
    float timer_ia = 0;

    while(!WindowShouldClose()){
        // --- UPDATE ---
        if (!partita.gioco_finito){
            if (partita.turno==0){ // UMANO
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                    Vector2 m = GetMousePosition();
                    // click mano
                    for(int i=0;i<partita.giocatori[0].num_mano;i++){
                        Rectangle r = {50 + i*110, 390, 80, 120};
                        if (CheckCollisionPointRec(m, r)){
                            carta_selezionata = i;
                            num_presa_sel = 0;
                        }
                    }
                    // click tavolo per scegliere presa multipla
                    if (carta_selezionata!=-1){
                        Carta giocata = partita.giocatori[0].mano[carta_selezionata];
                        Presa prese[32];
                        int n = trova_prese_per_valore(&partita.tavolo, giocata.valore, prese, 32);
                        if (n>0){
                            // se ci sono prese singole, gioco automatico
                            if (partita.tavolo.carte[prese[0].indici[0]].valore == giocata.valore){
                                // gioca subito
                                int scopa = gioca_carta(&partita, 0, carta_selezionata, 0);
                                carta_selezionata=-1;
                                num_presa_sel=0;
                            } else {
                                // per somme multiple, clicca carte tavolo per comporre somma
                                for(int i=0;i<partita.tavolo.num_carte;i++){
                                    Rectangle r = {50 + i*95, 190, 80, 120};
                                    if (CheckCollisionPointRec(m, r)){
                                        // toggle selezione
                                        int found=-1;
                                        for(int s=0;s<num_presa_sel;s++) if(presa_indici_sel[s]==i) found=s;
                                        if (found!=-1){
                                            for(int s=found;s<num_presa_sel-1;s++) presa_indici_sel[s]=presa_indici_sel[s+1];
                                            num_presa_sel--;
                                        } else {
                                            presa_indici_sel[num_presa_sel++]=i;
                                        }
                                    }
                                }
                                // check se somma corrisponde e premi SPAZIO per confermare
                            }
                        } else {
                            // nessuna presa, gioca automaticamente sul tavolo
                            if (IsKeyPressed(KEY_SPACE) || true){ // auto
                                // per UX: se nessuna presa, cliccare spazio o ricliccare carta
                                // qui facciamo auto dopo secondo click
                            }
                        }
                    }
                }
                // Conferma mossa umana
                if (carta_selezionata!=-1){
                    Carta giocata = partita.giocatori[0].mano[carta_selezionata];
                    Presa prese[32];
                    int n = trova_prese_per_valore(&partita.tavolo, giocata.valore, prese, 32);
                    if (n==0 && IsKeyPressed(KEY_SPACE)){
                        gioca_carta(&partita, 0, carta_selezionata, -1);
                        carta_selezionata=-1;
                    } else if (n>0){
                        // se selezione manuale somma
                        int somma=0;
                        for(int s=0;s<num_presa_sel;s++) somma+=partita.tavolo.carte[presa_indici_sel[s]].valore;
                        if (somma==giocata.valore && num_presa_sel>0){
                            // trova presa corrispondente
                            for(int pr=0;pr<n;pr++){
                                if (prese[pr].num_carte!=num_presa_sel) continue;
                                // confronta set indici (non ordinati)
                                int ok=1;
                                for(int k=0;k<prese[pr].num_carte;k++){
                                    int found=0;
                                    for(int s=0;s<num_presa_sel;s++) if(prese[pr].indici[k]==presa_indici_sel[s]) found=1;
                                    if (!found) ok=0;
                                }
                                if (ok){
                                    gioca_carta(&partita, 0, carta_selezionata, pr);
                                    carta_selezionata=-1;
                                    num_presa_sel=0;
                                    break;
                                }
                            }
                        }
                        if (IsKeyPressed(KEY_SPACE) && n>0 && partita.tavolo.carte[prese[0].indici[0]].valore!=giocata.valore){
                            // se utente preme spazio senza selezione corretta, prendi prima disponibile
                            // (facilita)
                        }
                    }
                }
                // Auto-play se nessuna presa: premi SPACE
                if (carta_selezionata!=-1){
                    Carta g = partita.giocatori[0].mano[carta_selezionata];
                    Presa tmp[32];
                    if (trova_prese_per_valore(&partita.tavolo, g.valore, tmp, 32)==0 && IsKeyPressed(KEY_SPACE)){
                        gioca_carta(&partita, 0, carta_selezionata, -1);
                        carta_selezionata=-1;
                        num_presa_sel=0;
                    }
                }

            } else { // IA
                timer_ia += GetFrameTime();
                if (timer_ia>1.0f){ // 1 sec di "pensiero"
                    int c_idx, pr_idx;
                    calcola_mossa_ia_prolog(&partita, &c_idx, &pr_idx);
                    int scopa = gioca_carta(&partita, 1, c_idx, pr_idx);
                    timer_ia=0;
                }
            }
        } else {
            if (IsKeyPressed(KEY_R)){
                inizializza_partita(&partita);
                carta_selezionata=-1;
                num_presa_sel=0;
            }
        }

        // --- DRAW ---
        BeginDrawing();
            ClearBackground((Color){20,80,40,255});
            disegna_tavolo_grafico(&partita, presa_indici_sel, num_presa_sel);
            disegna_mano_grafica(&partita, 0, carta_selezionata);
            
            // istruzioni
            if (!partita.gioco_finito && partita.turno==0){
                if (carta_selezionata==-1) DrawText("Seleziona una carta dalla tua mano", 50, 330, 16, LIGHTGRAY);
                else {
                    Carta g = partita.giocatori[0].mano[carta_selezionata];
                    Presa pr[32]; int n = trova_prese_per_valore(&partita.tavolo, g.valore, pr, 32);
                    if (n==0) DrawText("Nessuna presa -> premi SPAZIO per mettere sul tavolo", 50, 330, 16, YELLOW);
                    else if (partita.tavolo.carte[pr[0].indici[0]].valore==g.valore) DrawText("Presa singola disponibile -> presa automatica al click", 50, 330, 16, GREEN);
                    else DrawText("Seleziona carte che sommano al valore giocato, poi SPACE se non automatico", 50, 330, 16, YELLOW);
                }
            }

            disegna_hud(&partita);
        EndDrawing();
    }

    scarica_texture_carte();
    chiudi_prolog();
    CloseWindow();
    return 0;
}
