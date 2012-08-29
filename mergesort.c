#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include "mergesort.h"

int main(int argc, char **argv) {
    int rank, numNodes, numChunks;
    char hostname[1024];
	int dataSize=10; //MB random-Daten
	int chunkSize=2; //MB Ints (xMio-Int-Zahlen)
	int test[10]={5, 3, 7, 4, 9, 7, 0, 1, 3, 2};
	int *p;
	p=test;
	int *pEnd = dataSize+test-1;
	int state=1;
	int i;
	struct buffer chunk;
    MPI_Init (&argc, &argv);

    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &numNodes); //liefert Anzahl
	int ready[numNodes];
	ready[0]=0;
	for(i=1; i<3; i++)
		ready[i]=1;	//init aller Clients mit 1

	MPI_Request r;
    gethostname(hostname, sizeof (hostname));

    if(rank==0)
	{
		MPI_Barrier(MPI_COMM_WORLD);
		do
		{
			//numChunks=dataSize/chunkSize;
			//iteriere über alle Nodes und sende Chunk, wenn Node bereit
			int cl_state=0;
			for(i=1; i<numNodes; i++)
			{
				if(ready[i]==1)
				{
					printf("Adressen werden geprueft.\n");			
					if(p>pEnd)		//DataSet abgearbeitet --> Abbruch
						break;
					printf("Master will senden.\n");			
					MPI_Send((void*) p, chunkSize, MPI_INT, i, 1, MPI_COMM_WORLD);
					printf("Master: Erfolgreich gesendet. \n");
					p+=chunkSize;
					ready[i]=0;
					MPI_Irecv(&cl_state, 1, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &r);
					if(cl_state==1)
						ready[i]==1;
					printf("Master: Erfolgreich empfangen. \n");
				}
			}
//			printf("\nIch bin Master.\n");
		}while(p<=pEnd);

   		//Masterblock
	  	/*teilen der Gesamtdaten in Chunks
		for schleife über alle chunks
		for schleife zur Verteilung auf die Nodes (Abfrage der Nodes ob verfügbar??)
		*/
	}

    else
	{
		MPI_Barrier(MPI_COMM_WORLD); //Sync Master/Slave
      	//Slaveblock#############################################
		//Beim Master melden, dass Client bereit		
		MPI_Recv(&chunk.data, 2, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		printf("Client: Erfolgreich empfangen. \n");
		for(i=0; i<chunkSize; i++)
			printf(" %d", chunk.data[i]);
		printf("Client will senden. \n");		
		state=1;	
		MPI_Send(&state, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
		printf("Client: Erfolgreich gesendet. \n");	
		//if(Chunks empfangen)
			// Quicksort von Chunks
		  	/*
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
	}

    MPI_Finalize();
	
    return EXIT_SUCCESS;
}	
