:- module(scopa_rules, [
    presa_valida/3,
    migliore_mossa/3
]).

% --- UTILITY: Somma del valore delle carte ---
% Ogni carta ha la forma: carta(Valore, Seme)
somma_carte([], 0).
somma_carte([carta(V, _)|Resto], Somma) :-
    somma_carte(Resto, SommaResto),
    Somma is V + SommaResto.

% --- REGLA 1: Presa Valida ---
% Presa con carta singola (Regola d'oro della Scopa: se c'è una carta dello stesso valore, va presa quella)
presa_valida(carta(V, S), Tavolo, [carta(V, STavolo)]) :-
    member(carta(V, STavolo), Tavolo), !.

% Presa con combinazione di carte (se non esiste la carta singola uguale)
presa_valida(carta(V, _), Tavolo, Prese) :-
    subseq(Tavolo, Prese),
    Prese \= [],
    somma_carte(Prese, V).

% Genera sottoinsiemi di una lista (per trovare tutte le combinazioni di prese)
subseq([], []).
subseq([Head|Tail], [Head|Sub]) :- subseq(Tail, Sub).
subseq([_|Tail], Sub) :- subseq(Tail, Sub).

% --- REGLA 2: Valutazione Euristica della Mossa (Per l'IA) ---
% Calcola un punteggio per la presa effettuata
valuta_mossa(CartaGiocata, Tavolo, CartePrese, Punteggio) :-
    % 1. Punti base per le carte prese
    length(CartePrese, NumCarte),
    
    % 2. Bonus Scopa (se la presa svuota il tavolo)
    subtract(Tavolo, CartePrese, TavoloRimanente),
    (TavoloRimanente == [] -> BonusScopa = 100 ; BonusScopa = 0),

    % 3. Bonus Settebello (7 di Denari)
    (member(carta(7, denari), CartePrese) -> BonusSettebello = 50 ; BonusSettebello = 0),
    (CartaGiocata = carta(7, denari) -> MalusSettebello = -20 ; MalusSettebello = 0),

    % 4. Bonus Carte di Denari
    include(is_denari, CartePrese, DenariPrese),
    length(DenariPrese, NumDenari),

    Punteggio is NumCarte + BonusScopa + BonusSettebello + MalusSettebello + (NumDenari * 10).

is_denari(carta(_, denari)).

% --- REGLA 3: Trovare la Mossa Migliore dell'IA ---
% L'IA valuta tutte le carte che ha in mano e sceglie la carta e la presa con punteggio più alto
migliore_mossa(ManoIA, Tavolo, mossa(CartaMigliore, PreseMigliori)) :-
    findall(
        punteggio(Punti, Carta, Prese),
        (
            member(Carta, ManoIA),
            (
                presa_valida(Carta, Tavolo, Prese)
            ;   
                % Se la carta non fa prese, l'IA la scarta (Prese = [])
                \+ presa_valida(Carta, Tavolo, _),
                Prese = [],
                Punti = -5 % Scartare ha un punteggio leggermente negativo
            ),
            (Prese \= [] -> valuta_mossa(Carta, Tavolo, Prese, Punti) ; true)
        ),
        Mosse
    ),
    % Ordina le mosse per punteggio decrescente e prende la migliore
    sort(1, @>=, Mosse, [punteggio(_, CartaMigliore, PreseMigliori)|_]).