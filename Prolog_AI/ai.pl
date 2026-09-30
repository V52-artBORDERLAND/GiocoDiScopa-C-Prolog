% ai.pl - Intelligenza per Scopa
% Predicato principale chiamato da C (anche se usiamo euristica C, questa e' la logica di riferimento)
% scegli_mossa(+Mano, +Tavolo, -CartaIdx, -PresaIdx)

% Mano = [carta(Valore,Seme), ...]
% Tavolo = [carta(Valore,Seme), ...]

% Strategia in ordine di priorita':
% 1. Scopa se possibile
% 2. Prendi Settebello (7 denari)
% 3. Prendi piu' denari possibile
% 4. Prendi 7 se possibile
% 5. Prendi piu' carte possibile
% 6. Scarta carta meno utile (non denari, non 7)

% Helper per sommare valori tavolo
somma_lista([],0).
somma_lista([carta(V,_)|T], S):- somma_lista(T,S1), S is S1+V.

% Trova tutte le prese possibili per una carta
presa_possibile(Valore, Tavolo, Presa):-
    member(carta(Valore,_), Tavolo),
    % se c'e' presa singola, quella ha priorita'
    include(=(carta(Valore,_)), Tavolo, Singole),
    Singole \= [],
    member(C, Singole),
    Presa=[C].
presa_possibile(Valore, Tavolo, Presa):-
    % cerca combinazioni che sommano
    subset_sum(Tavolo, Valore, Presa),
    Presa \= [].

% subset_sum semplice (non ottimizzato)
subset_sum([],0,[]).
subset_sum([carta(V,S)|T], Sum, [carta(V,S)|R]):-
    Sum >= V,
    Sum1 is Sum-V,
    subset_sum(T, Sum1, R).
subset_sum([_|T], Sum, R):-
    subset_sum(T, Sum, R).

% valuta presa
valore_presa(Presa, Val):-
    length(Presa, N),
    include(denari, Presa, Den),
    length(Den, ND),
    (member(carta(7,denari), Presa) -> BSet=10 ; BSet=0),
    include(sette, Presa, Sette),
    length(Sette, NS),
    Val is N*2 + ND*4 + BSet + NS*3.

denari(carta(_,denari)).
sette(carta(7,_)).

% Scegli mossa migliore
scegli_mossa(Mano, Tavolo, CartaIdx, PresaIdx):-
    findall(score(S,C,P), (
        nth0(C, Mano, carta(Vc,Sc)),
        findall(Pr, presa_possibile(Vc, Tavolo, Pr), Prese),
        (Prese=[] -> S is -Vc, P = -1 ; member(P, Prese), valore_presa([carta(Vc,Sc)|P], S))
    ), Scores),
    max_score(Scores, score(_, CartaIdx, PresaIdx)).

max_score([X],X).
max_score([score(S1,_,_)|T], Max):-
    max_score(T, score(S2,C2,P2)),
    (S1>=S2 -> Max=score(S1,C2,P2) ; Max=score(S2,C2,P2)).

% Per compatibilita' con bridge C, esponiamo anche predicato semplice
% calcola_mossa(CartaValore, Tavolo, MigliorPresa)
