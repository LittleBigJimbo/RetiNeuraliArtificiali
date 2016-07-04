# RetiNeuraliArtificiali
Lavoro per la tesina di maturità incentrato sulla creazione di una rete neurale in grado di imparare a riconoscere i caratteri del database MNIST

Descrizione dei sei programmi presenti in questa repository:

01. <b>Percettrone</b>:  Percettrone non in grado di apprendere ed i cui pesi vanno impostati a mano 
02. <b>ApprendimentoPercy</b>: Percettrone che con numero sufficiente di esempi è in grado di apprendere cosa fare usando la regola delta
03. <b>Sigmoide</b>: Percettrone la cui funzione di trasferimento è la funzione sigmoidea. La regola delta cambia.
04. <b>ReteMultistrato</b>: La prima rete neurale nel vero senso della parola. Algoritmo di retropropagazione dell'errore
05. <b>MNIST</b>: Qualche leggera modifica per ottimizzare la rete sopra per il riconoscimento dei caratteri
06. <b>OpenCL</b>: Usando una minilibreria fatta ad hoc (molto close to metal), utilizzeremo la GPU per velocizzare l'addestramento.
