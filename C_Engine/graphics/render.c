#include "render.h"
#include "raylib.h"
#include <stdio.h>
#include <string.h>

static Texture2D carte_textures[4][11]; // [seme][valore 1-10]
static int textures_caricate = 0;
static Texture2D retro_texture;

static const char* nome_file_seme(Seme s){
    switch(s){ case DENARI: return "denari"; case COPPE: return "coppe"; case BASTONI: return "bastoni"; case SPADE: return "spade"; default: return "denari"; }
}

void carica_texture_carte(void){
    if (textures_caricate) return;
    // prova a caricare da assets/cards/ - formato atteso: denari_1.png, denari_2.png ... bastoni_10.png
    // e anche retro.png per il retro
    for(int s=0;s<4;s++){
        for(int v=1;v<=10;v++){
            char path[128];
            // prova due path: relativo a exe (assets/cards/...) e ../assets/cards/... e assets/cards/...
            const char* seme_str = nome_file_seme((Seme)s);
            snprintf(path, sizeof(path), "assets/cards/%s_%d.png", seme_str, v);
            if (FileExists(path)){
                carte_textures[s][v] = LoadTexture(path);
            } else {
                snprintf(path, sizeof(path), "../assets/cards/%s_%d.png", seme_str, v);
                if (FileExists(path)) carte_textures[s][v] = LoadTexture(path);
                else {
                    snprintf(path, sizeof(path), "C_Engine/assets/cards/%s_%d.png", seme_str, v);
                    if (FileExists(path)) carte_textures[s][v] = LoadTexture(path);
                }
            }
        }
    }
    // retro per IA o mazzo
    if (FileExists("assets/cards/retro.png")) retro_texture = LoadTexture("assets/cards/retro.png");
    else if (FileExists("../assets/cards/retro.png")) retro_texture = LoadTexture("../assets/cards/retro.png");
    else if (FileExists("C_Engine/assets/cards/retro.png")) retro_texture = LoadTexture("C_Engine/assets/cards/retro.png");

    textures_caricate = 1;
    TraceLog(LOG_INFO, "Texture carte caricate (se presenti)");
}

void scarica_texture_carte(void){
    if (!textures_caricate) return;
    for(int s=0;s<4;s++) for(int v=1;v<=10;v++) if (carte_textures[s][v].id) UnloadTexture(carte_textures[s][v]);
    if (retro_texture.id) UnloadTexture(retro_texture);
    textures_caricate=0;
}

/*

case DENARI: return GOLD; 
      case COPPE: return SKYBLUE; 
      case BASTONI: return LIME; 
      case SPADE: return LIGHTGRAY; 

*/
static Color colore_seme(Seme s){
    switch(s){ case DENARI: return GOLD; case COPPE: return SKYBLUE; case BASTONI: return LIME; case SPADE: return LIGHTGRAY; default: return WHITE; }
}
static const char* simbolo_seme(Seme s){
    switch(s){ case DENARI: return "D"; case COPPE: return "C"; case BASTONI: return "B"; case SPADE: return "S"; default: return "?"; }
}

void disegna_carta(int x, int y, Carta c, int selezionata, int evidenziata){
    int w=80, h=120;
    // Se abbiamo texture, usala
    if (textures_caricate && c.valore>=1 && c.valore<=10 && carte_textures[c.seme][c.valore].id){
        Texture2D tex = carte_textures[c.seme][c.valore];
        Rectangle src = {0,0,(float)tex.width,(float)tex.height};
        Rectangle dst = {x,y,w,h};
        if (selezionata){ dst.y -= 15; DrawRectangleRounded((Rectangle){x-3,y-3-15,w+6,h+6},0.15f,4,YELLOW); }
        else if (evidenziata){ DrawRectangleRounded((Rectangle){x-3,y-3,w+6,h+6},0.15f,4,ORANGE); }
        DrawTexturePro(tex, src, dst, (Vector2){0,0}, 0, WHITE);
        DrawRectangleRoundedLines((Rectangle){dst.x,dst.y,w,h},0.15f,2, BLACK);
        return;
    }
    // Fallback rettangoli colorati (quello che hai ora)
    Color bg = WHITE;
    if (selezionata) bg = YELLOW;
    else if (evidenziata) bg = ORANGE;
    
    if (selezionata) y -= 15;
    DrawRectangleRounded((Rectangle){x,y,w,h}, 0.15f, 4, bg);
    DrawRectangleRoundedLines((Rectangle){x,y,w,h}, 0.15f, 4, BLACK);
    
    if (c.valore==0){ DrawText("?", x+30, y+45, 20, BLACK); return; }
    Color semeCol = (c.seme==DENARI || c.seme==COPPE) ? MAROON : DARKBLUE;
    DrawText(TextFormat("%d", c.valore), x+8, y+6, 18, BLACK);
    DrawText(simbolo_seme(c.seme), x+8, y+26, 18, semeCol);
    DrawText(TextFormat("%d", c.valore), x+30, y+50, 28, BLACK);
    DrawCircle(x+w/2, y+85, 12, colore_seme(c.seme));
}

void disegna_tavolo_grafico(Partita *p, int* selezione_tavolo, int num_sel){
    DrawText("TAVOLO", 50, 160, 22, RAYWHITE);
    for(int i=0;i<p->tavolo.num_carte;i++){
        int ev=0;
        for(int s=0;s<num_sel;s++) if(selezione_tavolo[s]==i) ev=1;
        disegna_carta(50 + i*95, 190, p->tavolo.carte[i], 0, ev);
    }
    if (p->tavolo.num_carte==0){
        DrawText("SCOPA! Tavolo vuoto", 300, 220, 20, YELLOW);
    }
    // disegna mazzo coperto
    if (retro_texture.id){
        DrawTextureEx(retro_texture, (Vector2){750,190},0,80.0f/retro_texture.width, WHITE);
    } else {
        DrawRectangleRounded((Rectangle){750,190,80,120},0.15f,4,DARKBLUE);
        DrawText("MAZZO",755,240,14,WHITE);
    }
    DrawText(TextFormat("%d", p->mazzo.num_rimaste), 770, 320, 18, WHITE);
}

void disegna_mano_grafica(Partita *p, int giocatore, int carta_sel){
    Giocatore *g=&p->giocatori[giocatore];
    const char* label = (giocatore==0)? "LA TUA MANO (clicca una carta)" : "MANO IA";
    DrawText(label, 50, 360, 18, RAYWHITE);
    for(int i=0;i<g->num_mano;i++){
        disegna_carta(50 + i*110, 390, g->mano[i], (i==carta_sel), 0);
    }
    DrawText(TextFormat("Tue prese: %d | Scope: %d", p->giocatori[0].num_prese, p->giocatori[0].scope), 50, 540, 18, RAYWHITE);
    DrawText(TextFormat("IA prese: %d | Scope: %d", p->giocatori[1].num_prese, p->giocatori[1].scope), 50, 560, 18, RAYWHITE);
}

void disegna_hud(Partita *p){
    DrawText(TextFormat("Turno: %s", p->turno==0? "TUO":"IA"), 50, 20, 20, p->turno==0? GREEN: RED);
    if (p->gioco_finito){
        int punti[2]; calcola_punteggio_finale(p, punti);
        DrawRectangle(0,0,900,650, (Color){0,0,0,180});
        DrawText("PARTITA FINITA!", 280, 250, 30, YELLOW);
        DrawText(TextFormat("Tu: %d punti  vs  IA: %d punti", punti[0], punti[1]), 220, 300, 20, WHITE);
        DrawText("Premi R per ricominciare", 280, 340, 18, LIGHTGRAY);
    }
}
