L'obiettivo di questo Homework è quello di produrre un codice che effettui una semplice tecnica di codifica basata sulla DCT.

Lo studente è libero di scegliere il linguaggio di programmazione che preferisce tra Matlab, Python, C, C++, Java. Altri linguaggi possono essere usati su richiesta giustificata.

### Consegna

Si deve produrre: il codice che esegue le operazioni descritte nel seguito; le indicazioni di compilazione / esecuzione ove necessario; un report come specificato nel seguito.

### Specifiche del codice

Il programma prodotto deve eseguire le operazioni seguenti:

1. Caricare un'immagine a colori RGB in formato BMP e/o JPG e/o PGM (uno dei tre formati è sufficiente)
2. Effettuare il cambio di spazio dei colori da RGB a YCbCr
3. Per ognuna delle componenti Y, Cb e Cr, dato un numero R tra 1 e 100:
   1. Effettua la DCT bidimensionale con blocchi di dimensione parametrizzabile N della component
   2. Mette a zero una frazione pari a R% dei coefficienti DCT dell'intera componente
   3. Effettua la DCT inversa sui blocchi dopo sogliaggio, ottenendo la versione "compressa" della componente
   4. Calcola l'MSE tra la componente originale e la componente "compressa"
4. Calcola l'MSE pesato:
   $$
   {MSE}_P = \frac3 4 {MSE}_Y + \frac 1 8 {MSE}_{Cb} + \frac 1 8 {MSE}_{Cr}
   $$
5. Calcola il PSNR pesato come
   $$
   10\log_{10} \left(\frac{255^2}{{MSE}_P}\right)
   $$
6. Ripete i passi da 3 a 5 per diversi valori di R: da 10 a 100 a passi di 10
7. Traccia la curva del PSNR in funzione di *R*

### Consegna del report

Il report deve contenere:

1. Scelte di progetto del codice: linguaggio di programmazione, librerie, etc (max 1/4 di pagina)
2. Istruzioni di esecuzione del codice (max 1 pagina)
3. Esempi di codifica: per un'immagine fissate, tracciare la curve tasso distorsione per N=8, N=16 e N=64 e commentare i risultati

**Consegnare un PDF per il report, ed uno ZIP con il codice**
