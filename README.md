# DESCRIZIONE  
Implementazione di una biglietteria che gestisce i client in maniera concorrrente.  
La biglietteria dispone di un totale prestabilito di biglietti, i client possono connettersi e richiederne una certa quantità da acquistare, fino ad esaurimento scorte.  
L'acquisto da parte di un client è gestito e moderato dal server (biglietteria).  
Più client possono connettersi contemporaneamente per effettuare l'acquisto, nel caso in cui sia rimasta una certa quantità di biglietti e sonopiù client a richiederla allora il primo ad effettuare l'acquisto si aggiudicherà i biglietti.  

# UTILIZZO  
Clona la repository attraverso:   
`git clone <url_repository>`  

Spostati all'interno della cartella del progetto:  
`cd </path/alla/cartella/>`  

All'interno della cartella compila `server_b.c` e `client.c` con:  
`make all`  

Esegui il server su un terminale:    
`./server`  

Esegui i/il client su un'altro terminale:  
`./client`  
