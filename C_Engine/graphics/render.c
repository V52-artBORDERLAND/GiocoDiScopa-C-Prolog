#include "render.h"
#include "raylib.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static Texture2D carte_textures[4][11];
static int textures_caricate = 0;
static Texture2D retro_texture;

static const char* nome_file_seme(Seme s){
    switch(s){ case DENARI: return "denari"; case COPPE: return "coppe"; case BASTONI: return "bastoni"; case SPADE: return "spade"; default: return "denari"; }
}

static void to_lower(char* s){
    for(int i=0;s[i];i++) s[i]=tolower((unsigned char)s[i]);
}

static int try_load(const char* dir, const char* filename, Texture2D* out){
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", dir, filename);
    if (FileExists(path)){
        *out = LoadTexture(path);
        TraceLog(LOG_INFO, "Caricata %s", path);
        return 1;
    }
    return 0;
}

void carica_texture_carte(void){
    if (textures_caricate) return;

    // Tutte le cartelle possibili, con slash e con trattino come nel tuo explorer
    const char* base_dirs[] = {
        "graphics/assets-cards",                // TUO CASO ATTUALE
        "graphics/assets/cards",                // vecchio caso
        "assets-cards",
        "assets/cards",
        "C_Engine/graphics/assets-cards",
        "C_Engine/graphics/assets/cards",
        "C_Engine/assets/cards",
        "../C_Engine/graphics/assets-cards",
        NULL
    };

    const char* valori_it[] = {"", "asso", "due", "tre", "quattro", "cinque", "sei", "sette", "otto", "nove", "dieci"};

    for(int s=0;s<4;s++){
        for(int v=1;v<=10;v++){
            if(carte_textures[s][v].id) continue;
            const char* seme = nome_file_seme((Seme)s);
            const char* val_it = valori_it[v];
            int caricata=0;

            for(int b=0; base_dirs[b]!=NULL && !caricata; b++){
                char nome[256];
                // pattern che hai tu: bastoni_asso.png, Due_di_bastoni.png, Cinque_di_bastoni.png
                snprintf(nome, sizeof(nome), "%s_%s.png", seme, val_it);
                if(try_load(base_dirs[b], nome, &carte_textures[s][v])){ caricata=1; continue; }

                snprintf(nome, sizeof(nome), "%s_%d.png", seme, v);
                if(try_load(base_dirs[b], nome, &carte_textures[s][v])){ caricata=1; continue; }

                snprintf(nome, sizeof(nome), "%s_di_%s.png", val_it, seme);
                if(try_load(base_dirs[b], nome, &carte_textures[s][v])){ caricata=1; continue; }

                // Con maiuscola: Due_di_bastoni, Cinque_di_bastoni, Dieci_di_Bastoni
                char val_cap[32]; strcpy(val_cap, val_it); val_cap[0]=toupper(val_cap[0]);
                char seme_cap[32]; strcpy(seme_cap, seme); seme_cap[0]=toupper(seme_cap[0]);

                snprintf(nome, sizeof(nome), "%s_di_%s.png", val_cap, seme);
                if(try_load(base_dirs[b], nome, &carte_textures[s][v])){ caricata=1; continue; }

                snprintf(nome, sizeof(nome), "%s_di_%s.png", val_cap, seme_cap);
                if(try_load(base_dirs[b], nome, &carte_textures[s][v])){ caricata=1; continue; }

                // prova lower della versione con maiuscola
                char lower[256]; strcpy(lower, nome); to_lower(lower);
                if(try_load(base_dirs[b], lower, &carte_textures[s][v])){ caricata=1; continue; }
            }
        }
    }

    // Scan brutale: apri ogni cartella e cerca file che contengono seme+valore
    for(int b=0; base_dirs[b]!=NULL; b++){
        if (!DirectoryExists(base_dirs[b])) continue;
        FilePathList list = LoadDirectoryFiles(base_dirs[b]);
        for(int i=0;i<(int)list.count;i++){
            const char* filepath = list.paths[i];
            const char* filename = GetFileName(filepath);
            char lower[256]; strncpy(lower, filename, 255); lower[255]=0; to_lower(lower);
            if(strstr(lower, ".png")==NULL && strstr(lower, ".jpg")==NULL) continue;

            for(int s=0;s<4;s++){
                for(int v=1;v<=10;v++){
                    if(carte_textures[s][v].id) continue;
                    const char* seme = nome_file_seme((Seme)s);
                    const char* val_it = valori_it[v];
                    if(strstr(lower, seme) && strstr(lower, val_it)){
                        carte_textures[s][v] = LoadTexture(filepath);
                        TraceLog(LOG_INFO, "Caricata (scan) %s -> %s %d", filepath, seme, v);
                    }
                }
            }
        }
        UnloadDirectoryFiles(list);
    }

    // retro
    const char* retro_names[] = {"retro.png", "dorso.png", "back.png", "retro.jpg", NULL};
    for(int b=0; base_dirs[b]!=NULL; b++){
        for(int r=0; retro_names[r]!=NULL; r++){
            char path[512]; snprintf(path,sizeof(path),"%s/%s", base_dirs[b], retro_names[r]);
            if(FileExists(path)){
                retro_texture = LoadTexture(path);
                TraceLog(LOG_INFO, "Retro %s", path);
                break;
            }
        }
        if(retro_texture.id) break;
    }

    textures_caricate = 1;
    int count=0; for(int s=0;s<4;s++) for(int v=1;v<=10;v++) if(carte_textures[s][v].id) count++;
    TraceLog(LOG_INFO, "Texture finali: %d/40", count);
    if(count==0) TraceLog(LOG_WARNING, "Nessuna texture trovata! Controlla che le png siano in graphics/assets-cards/");
}

void scarica_texture_carte(void){
    if (!textures_caricate) return;
    for(int s=0;s<4;s++) for(int v=1;v<=10;v++) if (carte_textures[s][v].id) UnloadTexture(carte_textures[s][v]);
    if (retro_texture.id) UnloadTexture(retro_texture);
    textures_caricate=0;
}

static Color colore_seme(Seme s){
    switch(s){ case DENARI: return GOLD; case COPPE: return SKYBLUE; case BASTONI: return LIME; case SPADE: return LIGHTGRAY; default: return WHITE; }
}
static const char* simbolo_seme(Seme s){
    switch(s){ case DENARI: return "D"; case COPPE: return "C"; case BASTONI: return "B"; case SPADE: return "S"; default: return "?"; }
}

void disegna_carta(int x, int y, Carta c, int selezionata, int evidenziata){
    int w=80, h=120;
    if (textures_caricate && c.valore>=1 && c.valore<=10 && carte_textures[c.seme][c.valore].id){
        Texture2D tex = carte_textures[c.seme][c.valore];
        Rectangle src = {0,0,(float)tex.width,(float)tex.height};
        Rectangle dst = {(float)x,(float)y,(float)w,(float)h};
        if (selezionata){ dst.y -= 15; DrawRectangleRounded((Rectangle){x-3,y-3-15,w+6,h+6},0.15f,4,YELLOW); }
        else if (evidenziata){ DrawRectangleRounded((Rectangle){x-3,y-3,w+6,h+6},0.15f,4,ORANGE); }
        DrawTexturePro(tex, src, dst, (Vector2){0,0}, 0, WHITE);
        DrawRectangleRoundedLines((Rectangle){dst.x,dst.y,(float)w,(float)h},0.15f,2, BLACK);
        return;
    }
    Color bg = WHITE; if (selezionata) bg = YELLOW; else if (evidenziata) bg = ORANGE;
    if (selezionata) y -= 15;
    DrawRectangleRounded((Rectangle){(float)x,(float)y,(float)w,(float)h}, 0.15f, 4, bg);
    DrawRectangleRoundedLines((Rectangle){(float)x,(float)y,(float)w,(float)h}, 0.15f, 4, BLACK);
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
        int ev=0; for(int s=0;s<num_sel;s++) if(selezione_tavolo[s]==i) ev=1;
        disegna_carta(50 + i*95, 190, p->tavolo.carte[i], 0, ev);
    }
    if (p->tavolo.num_carte==0) DrawText("SCOPA! Tavolo vuoto", 300, 220, 20, YELLOW);
    if (retro_texture.id) DrawTextureEx(retro_texture, (Vector2){750,190},0,80.0f/(float)retro_texture.width, WHITE);
    else { DrawRectangleRounded((Rectangle){750,190,80,120},0.15f,4,DARKBLUE); DrawText("MAZZO",755,240,14,WHITE); }
    DrawText(TextFormat("%d", p->mazzo.num_rimaste), 770, 320, 18, WHITE);
}

void disegna_mano_grafica(Partita *p, int giocatore, int carta_sel){
    Giocatore *g=&p->giocatori[giocatore];
    const char* label = (giocatore==0)? "LA TUA MANO (clicca una carta)" : "MANO IA";
    DrawText(label, 50, 360, 18, RAYWHITE);
    for(int i=0;i<g->num_mano;i++) disegna_carta(50 + i*110, 390, g->mano[i], (i==carta_sel), 0);
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
