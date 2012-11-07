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
#define DEAD_TAG 5

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
		while(moreThanTwoSlaves(slaveState, numNodes) || (cntEndtag < numNodes)
				|| (slaveBusy(slaveState,numNodes) && (!(moreThanTwoSlaves(slaveState, numNodes)
				|| (cntEndtag < numNodes)))))
		{
			runSize=0;
			MPI_Recv(&runSize, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

			if(runSize > 0)
			{
				slaveState[status.MPI_SOURCE] = runSize;
			}
			else if(runSize == 0)
			{
				//printf("Master: Sende END_TAG an Slave %d.\n", status.MPI_SOURCE);
				MPI_Send(0, 0, MPI_INT, status.MPI_SOURCE, END_TAG, MPI_COMM_WORLD);
				cntEndtag++;
				if(cntEndtag == numNodes-1)
					cntEndtag++;
				printf("\nSlave:");
				for(x=0; x<numNodes; x++)
				{
					printf(" %d", x);
				}
				printf("\nState:");
				for(x=0; x<numNodes; x++)
				{
					printf(" %d", slaveState[x]);
				}
				printf("\n");
			}
			else if(runSize == -1)
			{
				slaveState[status.MPI_SOURCE]=-1;
				MPI_Send(0, 0, MPI_INT, status.MPI_SOURCE, DEAD_TAG, MPI_COMM_WORLD);
			}

			//Wenn mindestens 3 Slaves leben und 2 der 3 eine RunSize gemeldet haben
			if(slavesReady(slaveState, numNodes) && (moreThanTwoSlaves(slaveState, numNodes)))
			{
				////Workaround MPI_Send mit Konstante
				//int * do_recv;
				//*do_recv		int master=0;=DO_RECV;
				//Senderichtung bestimmen
				printf("\nSlave:");
				for(x=0; x<numNodes; x++)
				{
					printf(" %d", x);
				}
				printf("\nState:");
				for(x=0; x<numNodes; x++)
				{
					printf(" %d", slaveState[x]);
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
				//printf("Sende ISM-Daten (1) an Slave %d",maxNode);
				MPI_Send(&slaveState[minNode], 1, MPI_INT, maxNode, DO_RECV_TAG, MPI_COMM_WORLD);
				printf("-maxNode: %d\n",maxNode);
				slaveState[maxNode] = SLAVE_BUSY;
				//printf("Sende ISM-Daten (2) an Slave %d",minNode);
				MPI_Send(&maxNode, 1, MPI_INT, minNode, DO_SEND_TAG, MPI_COMM_WORLD);
				printf(";minNode: %d\n",minNode);
				slaveState[minNode] = SLAVE_DEAD;
			}
			//printf(".");

		}

		printf("\nSlave:");
		for(x=0; x<numNodes; x++)
		{
			printf("\t %d", x);
		}
		printf("\nState:");
		for(x=0; x<numNodes; x++)
		{
			printf("\t%d", slaveState[x]);
		}
		//printf("\n\n\nJetzt ist der Master draußen\n\n\n");


		/* ////////////////////////////////////////////////////////////// */
		/* //Empfangen der letzten Ergebnisse//////////////////////////// */
		/* ////////////////////////////////////////////////////////////// */


		int master=0;
		int slaveL;
		int slaveR;

		//Wenn ein Slave alle Chunks bekommen hat
		if (singleSlave(slaveState, numNodes, cfg.volumesize))
		{
			slaveL=getReadySlave(slaveState, numNodes);
			MPI_Send(&master, 1, MPI_INT, slaveL, DO_SEND_TAG, MPI_COMM_WORLD);
			MPI_Recv(volume.data, volume.size, MPI_INT, slaveL, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			slaveState[slaveL]=SLAVE_DEAD;
		}
		else
		{
			slaveL=getReadySlave(slaveState, numNodes);
			//printf("SlaveL: %d\n", slaveL);
			MPI_Send(&master, 1, MPI_INT, slaveL, DO_SEND_TAG, MPI_COMM_WORLD);
			//printf("slaveStateL: %d\n", slaveState[slaveL]);
			allocbuf(&preResultL,slaveState[slaveL]);
			slaveState[slaveL]=SLAVE_DEAD;

			if(waitingForResults(slaveState, numNodes))
			{
				MPI_Recv(&runSize, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
				slaveState[status.MPI_SOURCE] = runSize;
			}
		
			slaveR=getReadySlave(slaveState, numNodes);
			//printf("SlaveR: %d\n", slaveR);
			MPI_Send(&master, 1, MPI_INT, slaveR, DO_SEND_TAG, MPI_COMM_WORLD);
			//printf("slaveStateR: %d\n", slaveState[slaveR]);
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
		}


		//Aufräumen der nutzlosen Slaves (nie einen Chunk bekommen)
		for(i=1; i<numNodes; i++)
		{
	
			if(slaveState[i] == SLAVE_NOT_READY)
			{
				//END_TAG senden, da bisher null Kommunikation
				MPI_Recv(&runSize, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
				MPI_Send(0, 0, MPI_INT, status.MPI_SOURCE, END_TAG, MPI_COMM_WORLD);
				//Slave töten
				MPI_Recv(&runSize, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
				MPI_Send(0, 0, MPI_INT, status.MPI_SOURCE, DEAD_TAG, MPI_COMM_WORLD);
				slaveState[i] = SLAVE_DEAD;
			}

		}


//		prtbhead(&volume,volume.size);
		printf("\n\n\n");
		prtbtail(&volume,6);
		printf("\n");
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

		printf("Slave %d: Init erfolgreich.\n", rank);
		//Master/Slave-Sync
		MPI_Barrier(MPI_COMM_WORLD);


		/* ////////////////////////////////////////////////////////////// */
		/* //Chunks verarbeiten////////////////////////////////////////// */
		/* ////////////////////////////////////////////////////////////// */
		printf("Slave %d: Beginne mit Verarbeiten der Chunks.\n", rank);
		int anyData=0;
		while(1)
		{
			allocbuf(&chunk, cfg.chunksize);
			//Beim Master melden, dass Slave bereit
			MPI_Send(0, 0, MPI_INT, 0, 0, MPI_COMM_WORLD);
			//printf("Slave %d: Sende bereit. \n", rank);
			printf("SlaveResult Node %d:", rank);
			prtbtail(&slaveResult,slaveResult.size);
			printf("\n\n");
			//Chunk vom Master empfangen
			MPI_Recv(chunk.data, chunk.size, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			//printf("Slave %d: Empfang von Chunk erfolgreich. \n", rank);
			printf("Chunkdata Node %d:", rank);
			prtbtail(&chunk,chunk.size);
			printf("\n\n");
			if(status.MPI_TAG == END_TAG)
			{
				printf("Slave %d: END_TAG empfangen.\n", rank);
				freebuf(&chunk);
				break;
			}
			else
			{
				anyData=1;
				//neuen Chunk in den bestehenden Run einmergen
				quicksort(&chunk);
				tmpBuf.data = slaveResult.data;
				tmpBuf.size = slaveResult.size;
				allocbuf(&slaveResult, tmpBuf.size+chunk.size);
				merge(&tmpBuf, &chunk, &slaveResult);
				freebuf(&tmpBuf);
				freebuf(&chunk);
			}

			
		}
		//printf("Slave %d - SlaveResult: %d\n",rank,slaveResult.size);

		//////////////////////////////////////////////////////////////////////////////
		//Alle Chunks erhalten: Warten auf InterSlaveMerge-Befehle////////////////////
		//////////////////////////////////////////////////////////////////////////////
		while(1)
		{
			if(!(anyData))
				slaveResult.size=-1;
			printf("Node %d: Hier kommt das Problem. Meine SlaveResult.size ist %d\n", rank, slaveResult.size);
			MPI_Send(&slaveResult.size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
			MPI_Recv(&runSize, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			printf("Node %d: Bis hier schaffens nur die Guten!\n", rank);
			if(status.MPI_TAG == DO_RECV_TAG)
			{
				allocbuf(&run, runSize);
				MPI_Recv(run.data, run.size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
				printf("E - SlaveResult Node %d:", rank);
				prtbtail(&slaveResult,slaveResult.size);
				printf("\n\n");
				//eingehenden Run in slaveResult mergen
				tmpBuf.data = slaveResult.data;
				tmpBuf.size = slaveResult.size;
				allocbuf(&slaveResult, tmpBuf.size+runSize);
				merge(&tmpBuf, &run, &slaveResult);
				freebuf(&tmpBuf);
				freebuf(&run);
			}
			else if (status.MPI_TAG == DO_SEND_TAG)
			{
				//&runSize = Empfänger
				//prtbhead(&slaveResult,slaveResult.size);
				//printf("\n");
				printf("S - SlaveResult Node %d:", rank);
				prtbtail(&slaveResult,slaveResult.size);
				printf("\n\n");
				MPI_Send(slaveResult.data, slaveResult.size, MPI_INT, runSize, 0, MPI_COMM_WORLD);
				//printf("Datasize von Node %d: %d\n",rank, slaveResult.size);
				freebuf(&slaveResult);
				break;
			}
			else if (status.MPI_TAG == DEAD_TAG)
			{
				printf("Slave %d: Ich arme Sau habe nie Daten bekommen!\n",rank);
				break;
			}
			else
			{
				printf("\n\nUngueltiges TAG empfangen!!!!! Tag: %d\n", status.MPI_TAG);
				printf("slaveResult Slave %d: %d", rank, slaveResult.size);
			}
		}

		//prtbhead(&slaveResult,6);
		//printf("\n");
		//prtbtail(&slaveResult,6);
		//printf("\n");
		//printf("\nNode %d ist tot und rutscht ins Barrier.\n\n", rank);
	}



	/********************************************************/
	//Ab hier wieder für alle
	/********************************************************/
	MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return EXIT_SUCCESS;
}

// nur ein Slave hat alle Daten bekommen
// gibt true zurück wenn anzahl slaves mit volumesize>0 == 1
int singleSlave(int * a, int num, int volumesize)
{
  int i;
  for(i=1; i<num; i++)
    {
		if(a[i] == volumesize)
			return 1;
    }
  return 0;		
}

