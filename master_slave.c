#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include <string.h>
#include "tools.h"

//SlaveMessages
#define WORK_TAG 1
#define END_TAG 2
#define DO_RECV_TAG 3
#define DO_SEND_TAG 4

//MB Ints (xMio-Int-Zahlen)
#define VOLUMESIZE 100000
#define CHUNKSIZE 100

//SlaveStates
#define SLAVE_NOT_READY -2
#define SLAVE_DEAD -1
#define SLAVE_BUSY 0
//SLAVE_READY = Größe des SlaveStacks




int max_array(int * a, int numElements);

//min nur nach max aufrufen!!
int min_array(int * a, int numElements, int max);

int slavesReady(int * a, int num);

int moreThanTwoSlaves(int * a, int num);




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
//		struct buffer preResultL;
//		struct buffer preResultR;
		int i;
		int *pWorker;
		int *pEnd;
		int *pWorkerL;
		int *pWorkerR;
		int *slaveState;

		//Volume mit Zufallszahlen initialisieren, sonst: Programmabbruch
		if(allocbuf(&volume,VOLUMESIZE))
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
			//printf("Master: Bereitschaft empfangen. \n");

			MPI_Send((void*) pWorker, CHUNKSIZE, MPI_INT, status.MPI_SOURCE, WORK_TAG, MPI_COMM_WORLD);
			//printf("Master: Chunk erfolgreich gesendet. \n");
			pWorker += CHUNKSIZE;
		}

		
		//////////////////////////////////////////////////////////////////////////////////////
		//END_TAG senden und Merge-Partner vermitteln/////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////
		slaveState = malloc(sizeof(int) * numNodes);
			// -2 - slave sortiert lokal
			// -1 - slave ist raus
			//  0 - slave merged mit anderem slave
			// >0 - slave ist bereit (mit Daten)
		for(i=0; i<numNodes; i++)
			slaveState[i]=SLAVE_NOT_READY;
		int x, y=0;
		int minNode;
		int maxNode;
		//init von runSize, da bei ersten Durchlauf im Slave keine Daten verschickt werden
		int runSize=0;
		while(moreThanTwoSlaves(slaveState, numNodes)) // >= 3 Slaves noch nicht tot
		{
			MPI_Recv(&runSize, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

			if(runSize > 0)
				slaveState[status.MPI_SOURCE] = runSize;
			else if(runSize == 0)
			{
				printf("Master: Sende END_TAG an Node %d.\n", status.MPI_SOURCE);			
				MPI_Send(0, 0, MPI_INT, status.MPI_SOURCE, END_TAG, MPI_COMM_WORLD);
			}
			// >= 2 Slaves haben runSize an Master gesendet und warten auf Partnervermittlung
			if(slavesReady(slaveState, numNodes))
			{
				y++;
				if(y==50)
					break;
				////Workaround MPI_Send mit Konstante
				//int * do_recv;
				//*do_recv=DO_RECV;
				//Senderichtung bestimmen
				for(x=0; x<numNodes; x++)
				{
					printf("Node %d: %d\n", x, slaveState[x]);
				}
				maxNode = max_array(slaveState, numNodes);
				minNode = min_array(slaveState, numNodes, maxNode);
				//printf("maxNode: %d\n", maxNode);
				//printf("minNode: %d\n", minNode);
				//Senden der Merging-Infos an die Slaves
				MPI_Send(&slaveState[minNode], 1, MPI_INT, maxNode, DO_RECV_TAG, MPI_COMM_WORLD);
				slaveState[maxNode] = SLAVE_BUSY;
				MPI_Send(&maxNode, 1, MPI_INT, minNode, DO_SEND_TAG, MPI_COMM_WORLD);
				slaveState[minNode] = SLAVE_DEAD;
				for(x=0; x<numNodes; x++)
				{
					printf("Node %d: %d\n", x, slaveState[x]);
				}
				printf("\n");
			}

		}
		

		//////////////////////////////////////////////////////////////////////////////////////
		//Empfangen der letzten beiden Ergebnisse/////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////
		//pWorkerL = preResultL.data;
		//pWorkerR = preResultR.data;

/*
		printf("Master: Warten auf Ergebnis 1 von 2. \n");
		MPI_Recv((void*) pWorkerL, preResultL.size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		printf("Master: Ergebnis 1 v_on 2 erfolgreich empfangen. \n");		

		printf("Master: Warten auf Ergebnis 2 von 2. \n");		
		MPI_Recv((void*) pWorkerR, preResultR.si//SlaveMessages
#define DO_RECV_TAG -5
#define DO_SEND_TAG -6ze, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
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
		struct buffer run;
		int runSize;
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
		printf("Slave %d: Beginne mit Verarbeiten der Chunks.\n", rank);
		while(1)
		{
			allocbuf(&chunk, CHUNKSIZE);
			//Beim Master melden, dass Slave bereit
			MPI_Send(0, 0, MPI_INT, 0, 0, MPI_COMM_WORLD);
			//printf("Slave %d: Sende bereit. \n", rank);

			//Chunk vom Master empfangen
			MPI_Recv(chunk.data, chunk.size, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			//printf("Slave %d: Empfang von Chunk erfolgreich. \n", rank);

			if(status.MPI_TAG == END_TAG)
			{
				printf("Slave %d: END_TAG empfangen.\n", rank);
				break;
			}

			//neuen Chunk in den bestehenden Run einmergen
			quicksort(&chunk);
			tmpBuf.data = slaveResult.data;
			tmpBuf.size = slaveResult.size;
			allocbuf(&slaveResult, tmpBuf.size+CHUNKSIZE);
			merge(&tmpBuf, &chunk, &slaveResult);
			freebuf(&tmpBuf);
			freebuf(&chunk);
		}
		
		//Alle Chunks erhalten: Warten auf InterSlaveMerge-Befehle		
		while(1)
		{
			MPI_Send(&slaveResult.size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
			MPI_Recv(&runSize, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			if(status.MPI_TAG == DO_RECV_TAG)
			{
				allocbuf(&run, runSize);
				MPI_Recv(run.data, run.size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

				//eingehenden Run in slaveResult mergen
				tmpBuf.data = slaveResult.data;
				tmpBuf.size = slaveResult.size;
				allocbuf(&slaveResult, tmpBuf.size+runSize);
				merge(&tmpBuf, &chunk, &slaveResult);
				freebuf(&tmpBuf);
 				freebuf(&run);
			}
			else if (status.MPI_TAG == DO_SEND_TAG)
			{
				//&runSize = Empfänger
				MPI_Send(slaveResult.data, slaveResult.size, MPI_INT, runSize, 0, MPI_COMM_WORLD);
				freebuf(&slaveResult);
				break;
			}
			else
				printf("\n\n\nTritt nie auf!!!!!\n\n\n");
		}
		//prtbhead(&slaveResult,6);
		//printf("\n");
		//prtbtail(&slaveResult,6);
		//printf("\n");
	}

	//Ab hier wieder für alle
	MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();	
    return EXIT_SUCCESS;
}



int max_array(int * a, int num)
{
	int i, position=1, max=0;
	for (i=1; i<num; i++)
	{
        if (a[i]>max)
		{
            max=a[i];
			position=i;
		}
	}
	printf("max: %d\n", max);
	return(position);
}


int min_array(int * a, int num, int maxPos)
{
	
   	int i, position=1, min=a[maxPos];
   	for (i=1; i<num; i++)
	{
         if ((a[i]<=min) && (a[i]>0))
		{
            min=a[i];
			position=i;
		}
	}
	printf("min: %d\n", min);
 	return(position);
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


int moreThanTwoSlaves(int * a, int num)
{
	int rest=num, i;
	for(i=1; i<num; i++)
	{
		if(a[i] == SLAVE_DEAD)
			num--;
	}
	if(num == 2)
		return 0;
	else
		return 1;
}
