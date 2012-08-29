#include <iostream>
#include <stdlib.h>
#include <math.h>


int main(int argc, char **argv) {
  int rank, size;
  char hostname[1024];
  int dataSize=32*1024*1024;
  int chunkSize=1024*1024; //4MB Ints (1Mio-Int-Zahlen)

  MPI_Init (&argc, &argv);

  MPI_Comm_rank (MPI_COMM_WORLD, &rank);
  MPI_Comm_size (MPI_COMM_WORLD, &size);
  gethostname(hostname, sizeof (hostname));

  if(rank==0)
    numChunks=dataSize/chunkSize;

  //Masterblock
  /*teilen der Gesamtdaten in Chunks
    for schleife über alle chunks
    for schleife zur Verteilung auf die Nodes (Abfrage der Nodes ob verfügbar??)


  */
  else
    //Slaveblock#############################################

    // Quicksort von Chunks
    /*
      while(Chunk verfügbar?)
      - Speicher allokieren
      - Chunk von Master empfangen
      - Chunk sortieren
    */
    // Internes Mergen
    /*
      for...
      Meldung an Master, welche chunks vorhanden sind
    */

    //kollaboratives Mergen
    /*
      Nachricht vom Master: Sender?,
      if(Sender)
      - Ja, Chunk senden
    */
    MPI_Finalize();

  return EXIT_SUCCESS;
}
