#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include <string.h>
#include "tools.h"
#include "helper.h"

//SlaveMessages
#define WORK_TAG 1
#define END_TAG 2
#define DO_RECV_TAG 3
#define DO_SEND_TAG 4

//MB Ints (xMio-Int-Zahlen)
// jetzt in cfg.*
//#define VOLUMESIZE 100
//#define CHUNKSIZE 10

//SlaveStates
#define SLAVE_NOT_READY -2
#define SLAVE_DEAD -1
#define SLAVE_BUSY 0
//SLAVE_READY = Größe des SlaveStacks

//MINIMUM 2 SLAVES (3 NODES) BEI PROGRAMMSTART


int main(int argc, char **argv) {
    int rank, numNodes, numChunks;
    char hostname[1024];

    // Argumente parsen
    struct config cfg;
    argparse(argc, argv, &cfg);

    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);		//liefert Node-Nr.
    MPI_Comm_size (MPI_COMM_WORLD, &numNodes);	//liefert Anzahl

	MPI_Status status;
    gethostname(hostname, sizeof (hostname));

	/* ###################################################################### */
	/* # Masterblock ######################################################## */
	/* ###################################################################### */
    if(rank==0)
	{
	  /* //////////////////////////////////////////////////////////////////// */
	  /* // Master Init  //////////////////////////////////////////////////// */
	  /* //////////////////////////////////////////////////////////////////// */
		struct buffer volume;
		struct buffer preResultL;
		struct buffer preResultR;
		int i;
		int *pWorker;
		int *pEnd;
		int *slaveState;

		//Volume mit Zufallszahlen initialisieren, sonst: Programmabbruch
		if(allocbuf(&volume, cfg.volumesize))
		{
//			randbuf(&volume);
			for (i=0; i<cfg.volumesize; i++)
			{
				volume.data[i] = cfg.volumesize - i;
			}
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

		/* ////////////////////////////////////////////////////////////// */
		/* //Chunks an Nodes senden////////////////////////////////////// */
		/* ////////////////////////////////////////////////////////////// */
		printf("Master: Beginne mit Senden der Chunks.\n");
		//Solange Chunk an verfügbaren Node senden, wie noch Chunks vorhanden
		while(pWorker<=pEnd)
		{
			MPI_Recv(0, 0, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);         
			//printf("Master: Bereitschaft empfangen. \n");

			MPI_Send((void*) pWorker, cfg.chunksize, MPI_INT, status.MPI_SOURCE, WORK_TAG, MPI_COMM_WORLD);
			//printf("Master: Chunk erfolgreich gesendet. \n");
			pWorker += cfg.chunksize;
		}

		/* ////////////////////////////////////////////////////////////// */
		/* //END_TAG senden und Merge-Partner vermitteln///////////////// */
		/* ////////////////////////////////////////////////////////////// */
		slaveState = malloc(sizeof(int) * numNodes);
			// -2 - slave sortiert lokal
			// -1 - slave ist raus
			//  0 - slave merged mit anderem slave
			// >0 - slave ist bereit (mit Daten)
		for(i=0; i<numNodes; i++)
			slaveState[i]=SLAVE_NOT_READY;
		int x;
		int minNode;
		int maxNode;
		int cntEndtag=0;
		int runSize;
		while(moreThanTwoSlaves(slaveState, numNodes) || (cntEndtag < numNodes-1)
				|| (slaveBusy(slaveState,numNodes) && (!(moreThanTwoSlaves(slaveState, numNodes) || (cntEndtag < numNodes-1)))))
		{
			runSize=0;
			MPI_Recv(&runSize, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

			if(runSize > 0 /*&& (slaveState[status.MPI_SOURCE] == 0)*/)
			{
				slaveState[status.MPI_SOURCE] = runSize;
			}
			else if(runSize == 0)
			{
				printf("Master: Sende END_TAG an Node %d.\n", status.MPI_SOURCE);		
				MPI_Send(0, 0, MPI_INT, status.MPI_SOURCE, END_TAG, MPI_COMM_WORLD);
				cntEndtag++;
			}

			//Wenn mindestens 3 Slaves leben und 2 der 3 eine RunSize gemeldet haben
			if(slavesReady(slaveState, numNodes) && (moreThanTwoSlaves(slaveState, numNodes)))
			{
				////Workaround MPI_Send mit Konstante
				//int * do_recv;
				//*do_recv		int master=0;=DO_RECV;
				//Senderichtung bestimmen
				printf("\nNode:");
				for(x=0; x<numNodes; x++)
				{
					printf("\t%d", x);
				}
				printf("\nState:");
				for(x=0; x<numNodes; x++)
				{
					printf("\t%d", slaveState[x]);
				}
				printf("\n");
				maxNode = max_array(slaveState, numNodes);
				minNode = min_array(slaveState, numNodes, maxNode);
				if(minNode == maxNode)
				{
					printf("\n\nFehler!!!! minNode == maxNode!!!!\n\n");
					break;
				}
				//printf("maxNode: %d\n", maxNode);
				//printf("minNode: %d\n", minNode);
				//Senden der Merging-Infos an die Slaves
				MPI_Send(&slaveState[minNode], 1, MPI_INT, maxNode, DO_RECV_TAG, MPI_COMM_WORLD);
				printf("-maxNode: %d\n",maxNode);
				slaveState[maxNode] = SLAVE_BUSY;
				MPI_Send(&maxNode, 1, MPI_INT, minNode, DO_SEND_TAG, MPI_COMM_WORLD);
				printf(";minNode: %d\n",minNode);
				slaveState[minNode] = SLAVE_DEAD;
			}
			//printf(".");

		}
		/*for(x=0; x<numNodes; x++)
		{
			printf("\t0	0	%d", x);
		}
		printf("\nState:");
		for(x=0; x<numNodes; x++)
		{
			printf("\t%d", slaveState[x]);
		}*/
		printf("\n\n\nJetzt ist der Master draußen\n\n\n");


		/* ////////////////////////////////////////////////////////////// */
		/* //Empfangen der letzten beiden Ergebnisse///////////////////// */
		/* ////////////////////////////////////////////////////////////// */

		int master=0;
		int slaveL;
		int slaveR;

		slaveL=getReadySlave(slaveState, numNodes);
		printf("SlaveL: %d\n", slaveL);
		MPI_Send(&master, 1, MPI_INT, slaveL, DO_SEND_TAG, MPI_COMM_WORLD);
		printf("slaveStateL: %d\n", slaveState[slaveL]);
		allocbuf(&preResultL,slaveState[slaveL]);
		slaveState[slaveL]=SLAVE_DEAD;

		slaveR=getReadySlave(slaveState, numNodes);
		printf("SlaveR: %d\n", slaveR);
		MPI_Send(&master, 1, MPI_INT, slaveR, DO_SEND_TAG, MPI_COMM_WORLD);
		printf("slaveStateR: %d\n", slaveState[slaveR]);
		allocbuf(&preResultR,slaveState[slaveR]);
		slaveState[slaveR]=SLAVE_DEAD;


		printf("Master: Warten auf Ergebnis 1 von 2. \n");
		MPI_Recv(preResultL.data, preResultL.size, MPI_INT, slaveL, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		printf("Master: Ergebnis 1 von 2 erfolgreich empfangen. \n");		

		printf("Master: Warten auf Ergebnis 2 von 2. \n");
		MPI_Recv(preResultR.data, preResultR.size, MPI_INT, slaveR, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		printf("Master: Ergebnis 2 von 2 erfolgreich empfangen. \n");		


		printf("Master: Beginne mit finalem Merge.\n");
		merge(&preResultL, &preResultR, &volume);
		printf("Master: Finales Merging abgeschlossen!\n\n\n");

		prtbhead(&volume,volume.size);
		printf("\n");
//		prtbtail(&volume,6);
//		printf("\n");
	}



    /* ########################################################################## */
    /* # Slaveblock ############################################################# */
    /* ########################################################################## */
    else
	{
	   /* ////////////////////////////////////////////////////////////////// */
	   /* //Slave Init////////////////////////////////////////////////////// */
	   /* ////////////////////////////////////////////////////////////////// */
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


		/* ////////////////////////////////////////////////////////////// */
		/* //Chunks verarbeiten////////////////////////////////////////// */
		/* ////////////////////////////////////////////////////////////// */
		printf("Slave %d: Beginne mit Verarbeiten der Chunks.\n", rank);
		while(1)
		{
			allocbuf(&chunk, cfg.chunksize);
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
			allocbuf(&slaveResult, tmpBuf.size+chunk.size);
			merge(&tmpBuf, &chunk, &slaveResult);
			freebuf(&tmpBuf);
			freebuf(&chunk);
		}
		printf("Slave %d - SlaveResult: %d\n",rank,slaveResult.size);
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
				prtbhead(&slaveResult,slaveResult.size);
				printf("\n");
				MPI_Send(slaveResult.data, slaveResult.size, MPI_INT, runSize, 0, MPI_COMM_WORLD);
				printf("Datasize von Node %d: %d\n",rank, slaveResult.size);
				freebuf(&slaveResult);
				break;
			}
			else
				printf("\n\n\nUngueltiges TAG empfangen!!!!!\nTag: %d\n\n", status.MPI_TAG);
		}

		//prtbhead(&slaveResult,6);
		//printf("\n");
		//prtbtail(&slaveResult,6);
		//printf("\n");
		printf("\nNode %d ist tot und rutscht ins Barrier.\n\n", rank);
	}

	//Ab hier wieder für alle
	MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return EXIT_SUCCESS;
}
