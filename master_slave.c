#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include <string.h>
#include "mergesort.h"
#include "tools.h"

#define WORKTAG 1
#define ENDTAG 2

//MB Ints (xMio-Int-Zahlen)
#define VOLUMESIZE 1000
#define CHUNKSIZE 10

//SlaveStates
#define SLAVE_NOT_READY -2
#define SLAVE_DEAD -1
#define SLAVE_BUSY 0
//SLAVE_READY = Größe des SlaveStacks

//SlaveMessages
#define DO_RECV -1
//DO_SEND = rank vom Slaveempfänger


int max_array(int * a, int numElements);

//min nur nach max aufrufen!!
int min_array(int * a, int numElements, int max);

int slavesReady(int * a, int num);

int enoughSlaves(int * a, int num);




//MINIMUM 2 SLAVES (3 NODES) BEI PROGRAMMSTART

int main(int argc, char **argv) {
    int rank, numNodes, numChunks;
    char hostname[1024];
	
    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);		//liefert Node-Nr.
    MPI_Comm_size (MPI_COMM_WORLD, &numNodes);	//liefert Anzahl
	
	MPI_Status status;
    gethostname(hostname, sizeof (hostname));

	//########################################################################################
	//Masterblock#############################################################################
	//########################################################################################
    if(rank==0)
	{   //////////////////////////////////////////////////////////////////////////////////////
		//Master Init/////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////
		struct buffer volume;
		struct buffer preResultL;
		struct buffer preResultR;
		int i;
		int *pWorker;
		int *pEnd;
		int *pWorkerL;
		int *pWorkerR;
		int *slaveState;

		//Volume mit Zufallszahlen initialisieren, sonst: Programmabbruch
		if(allocbuf(&volume,VOLUMESIZE) && allocbuf(&preResultL,VOLUMESIZE) && allocbuf(&preResultR,VOLUMESIZE))
		{
			randbuf(&volume);
			pWorker = volume.data;
			pEnd = volume.data + volume.size - 1;
		}
		else
		{
			printf("Master: Erstellen des Volumes fehlgeschlagen!\n");
		    MPI_Finalize();
			exit;
		}		
		printf("Master: Init erfolgreich.\n");
		MPI_Barrier(MPI_COMM_WORLD);


		//////////////////////////////////////////////////////////////////////////////////////
		//Chunks an Nodes senden//////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////
		printf("Master: Beginne mit Senden der Chunks.\n");
		//Solange Chunk an verfügbaren Node senden, wie noch Chunks vorhanden
		while(pWorker<=pEnd)
		{
			MPI_Recv(0, 0, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);         
			//printf("Master: Erfolgreich empfangen. \n");

			MPI_Send((void*) pWorker, CHUNKSIZE, MPI_INT, status.MPI_SOURCE, WORKTAG, MPI_COMM_WORLD);
			//printf("Master: Erfolgreich gesendet. \n");
			pWorker += CHUNKSIZE;
		}


		//////////////////////////////////////////////////////////////////////////////////////
		//ENDTAG senden und Mergen vermitteln/////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////
		slaveState = malloc(sizeof(int) * numNodes);
			// -2 - slave sortiert lokal
			// -1 - slave ist raus
			//  0 - slave merged mit anderem slave
			// >0 - slave ist bereit (mit Daten)
		for(i=0; i<numNodes; i++)
			slaveState[i]=-2;

		int minNode, maxNode, runSize;
		while(enoughSlaves(slaveState, numNodes)) //ändern
		{
			MPI_Recv(&runSize, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

			if(runSize)
				slaveState[status.MPI_SOURCE] = runSize;
			else
			{
				printf("Master: Sende ENDTAG an Node %d.\n", status.MPI_SOURCE);			
				MPI_Send(0, 0, MPI_INT, status.MPI_SOURCE, ENDTAG, MPI_COMM_WORLD);
			}

			if(slavesReady(slaveState, numNodes))
			{
				//Workaround MPI_Send mit Konstante
				int * do_recv;
				*do_recv=DO_RECV;
				//Senderichtung bestimmen
				maxNode = max_array(slaveState, numNodes);
				minNode = min_array(slaveState, numNodes, maxNode);
				//Speicherung für den Master
				slaveState[maxNode] = SLAVE_NOT_READY;
				slaveState[minNode] = SLAVE_DEAD;
				//Senden der Merging-Infos an die Slaves
				MPI_Send(do_recv, 1, MPI_INT, maxNode, 0, MPI_COMM_WORLD);
				MPI_Send(&maxNode, 1, MPI_INT, minNode, 0, MPI_COMM_WORLD);				       
			}
		}
		

		//////////////////////////////////////////////////////////////////////////////////////
		//Empfangen der letzten beiden Ergebnisse/////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////
		pWorkerL = preResultL.data;
		pWorkerR = preResultR.data;

/*
		printf("Master: Warten auf Ergebnis 1 von 2. \n");
		MPI_Recv((void*) pWorkerL, preResultL.size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		printf("Master: Ergebnis 1 von 2 erfolgreich empfangen. \n");		

		printf("Master: Warten auf Ergebnis 2 von 2. \n");		
		MPI_Recv((void*) pWorkerR, preResultR.size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		printf("Master: Ergebnis 2 von 2 erfolgreich empfangen. \n");		


		printf("Master: Beginne mit finalem Merge.\n");
		//...
		printf("Master: Finales Merging abgeschlossen!\n");
*/
	}



	//########################################################################################
	//Slaveblock##############################################################################
	//########################################################################################
    else
	{	//////////////////////////////////////////////////////////////////////////////////////
		//Slave Init//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////
		struct buffer chunk;
		struct buffer tmpBuf; //Puffer zum Mergen
		struct buffer slaveResult; //Ergebnis fürs Mergen
		allocbuf(&slaveResult, 0);

		//Volume mit Zufallszahlen initialisieren, sonst: Programmabbruch
		//ENTFÄLLT, weil beides nur noch CHUNKSIZE groß
		/*if(allocbuf(&chunk, CHUNKSIZE) && allocbuf(&tmpMergBuf, CHUNKSIZE))
		{
			printf("Slave: Erstellen beider Puffer erfolgreich!\n");
		}
		else
		{
			printf("Slave: Erstellen eines Puffers fehlgeschlagen!\n");
		    MPI_Finalize();
			exit;
		}*/

		printf("Slave %d: Init erfolgreich.\n", rank);
		//Master/Slave-Sync
		MPI_Barrier(MPI_COMM_WORLD);
		

		//////////////////////////////////////////////////////////////////////////////////////
		//Chunks verarbeiten//////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////
		printf("Slave %i: Beginne mit Verarbeiten der Chunks.\n", rank);
		while(1)
		{
			allocbuf(&chunk, CHUNKSIZE);
			//Beim Master melden, dass Slave bereit
			MPI_Send(0, 0, MPI_INT, 0, 0, MPI_COMM_WORLD);
			//printf("Slave: Erfolgreich gesendet. \n");

			//Chunk vom Master empfangen
			MPI_Recv(chunk.data, chunk.size, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			//printf("Slave: Erfolgreich empfangen. \n");
			if(status.MPI_TAG == ENDTAG)
			{
				printf("Slave %i: ENDTAG empfangen.\n", rank);
				break;
			}


			quicksort(&chunk);
			tmpBuf.data = slaveResult.data;
			tmpBuf.size = slaveResult.size;
			allocbuf(&slaveResult, tmpBuf.size+CHUNKSIZE);
			merge(&tmpBuf, &chunk, &slaveResult);
			freebuf(&tmpBuf);			
			
		}

		
			prtbhead(&slaveResult,6);
			printf("\n");
			prtbtail(&slaveResult,6);
			printf("\n");



	}
	MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();	
    return EXIT_SUCCESS;
}



int max_array(int * a, int num)
{
   int i, max=0;
   for (i=1; i<num; i++)
         if (a[i]>max)
            max=a[i];
   return(i);
}


int min_array(int * a, int num, int max)
{
   int i, min=max;
   for (i=1; i<num; i++)
         if ((a[i]<min) && (a[i]>0))
            min=a[i];
   return(i);
}


int slavesReady(int * a, int num)
{
	int ret=0, i;
	for(i=1; i<num; i++)
	{
		if(a[i]>0)
			ret++;
	}
	if(ret>=2)
		return 1;
	else
		return 0;
}


int enoughSlaves(int * a, int num)
{
	int rest=num, i;
	for(i=1; i<num; i++)
	{
		if(a[i] == -1)
			num--;
	}
	if(num == 2)
		return 0;
	else
		return 1;
}
