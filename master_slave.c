#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
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
#define NO_ISM_ENTERED -2
#define SLAVE_DEAD -1
#define SLAVE_BUSY 0
//SLAVE_READY = Größe des SlaveStacks

//MINIMUM 1 SLAVES (2 NODES) BEI PROGRAMMSTART

// Zeitdifferenz in ns
uint64_t timediff(struct timespec* start, struct timespec* end)
{
  return 1000000000*(end->tv_sec - start->tv_sec) + end->tv_nsec - start->tv_nsec;
}



int main(int argc, char **argv) {
  int rank, numNodes, numChunks;
  char hostname[1024];

  // Zeiten
  struct timespec t_0, t_1, t_2, t_3, t_4;

  // Argumente parsen / Default-config
  struct config cfg;
  cfg.dbg = 0;
  cfg.prtopt = 0;
  cfg.qsort = 1;
  cfg.msort = 0;
  cfg.cachesize = 10000;

  if (! argparse(argc, argv, &cfg) )
    {
      exit;
    }

  MPI_Init (&argc, &argv);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);	//liefert Node-Nr.
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

      // Startzeit
      clock_gettime(CLOCK_MONOTONIC, &t_0);

      if(cfg.prtopt)
	printf("vol: %d, chunk: %d, cache: %d, qsort: %d, msort: %d, dbg: %d\n",
	       cfg.volumesize, cfg.chunksize, cfg.cachesize, cfg.qsort,  cfg.msort,  cfg.dbg);

      //Volume mit Zufallszahlen initialisieren, sonst: Programmabbruch
      if(allocbuf(&volume, cfg.volumesize))
	{
	  randbuf(&volume);
	  /* for (i=0; i<cfg.volumesize; i++) */
	  /* { */
	  /* 	volume.data[i] = cfg.volumesize - i; */
	  /* } */
	  pWorker = volume.data;
	  pEnd = volume.data + volume.size - 1;
	}
      else
	{
	  if(cfg.dbg) printf("Master: Erstellen des Volumes fehlgeschlagen!\n");
	  MPI_Finalize();
	  exit;
	}
      if(cfg.dbg) printf("Master: Init erfolgreich.\n");

      MPI_Barrier(MPI_COMM_WORLD);

      // Zeit zur Zufallszahlengenerierung
      clock_gettime(CLOCK_MONOTONIC, &t_1);

      /* ////////////////////////////////////////////////////////////// */
      /* //Chunks an Nodes senden////////////////////////////////////// */
      /* ////////////////////////////////////////////////////////////// */
      if(cfg.dbg) printf("Master: Beginne mit Senden der Chunks.\n");
      //Solange Chunk an verfügbaren Node senden, wie noch Chunks vorhanden
      while(pWorker<=pEnd)
	{
	  MPI_Recv(0, 0, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	  MPI_Send((void*) pWorker, cfg.chunksize, MPI_INT, status.MPI_SOURCE, WORK_TAG, MPI_COMM_WORLD);
	  pWorker += cfg.chunksize;
	}

      // Zeit zum Chunks verteilen
      clock_gettime(CLOCK_MONOTONIC, &t_2);

      //END_TAG an alle senden	
      if(cfg.dbg) printf("\nMaster: Beginne mit Senden der END_TAGs.\n");
      for(i=1; i<numNodes; i++)
	{
	  MPI_Recv(0, 0, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	  MPI_Send(0, 0, MPI_INT, i, END_TAG, MPI_COMM_WORLD);
	}
      MPI_Barrier(MPI_COMM_WORLD);

      // Zeit bis alle End-Tags verteilt
      clock_gettime(CLOCK_MONOTONIC, &t_3);

      /* ////////////////////////////////////////////////////////////// */
      /* //Merge-Partner vermitteln//////////////////////////////////// */
      /* ////////////////////////////////////////////////////////////// */
      slaveState = malloc(sizeof(int) * numNodes);
      // -2 - slave sortiert lokal
      // -1 - slave ist raus
      //  0 - slave merged mit anderem slave
      // >0 - slave ist bereit (mit Daten)
      for(i=0; i<numNodes; i++)
	slaveState[i]=NO_ISM_ENTERED;

      int x;
      int j=0;
      int minNode;
      int maxNode;
      int runSize;

      while((moreThanTwoSlaves(slaveState, numNodes) || (slaveBusy(slaveState,numNodes)))
	    && (!(singleSlave(slaveState, numNodes, cfg.volumesize))))
	{
	  j++;

	  runSize=0;
	  MPI_Recv(&runSize, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

	  if(runSize > 0)
	    {
	      slaveState[status.MPI_SOURCE] = runSize;
	    }
	  else if(runSize == 0)
	    {
	      if(cfg.dbg) prtSlaveState(slaveState, numNodes);
	    }
	  else if(runSize == -1)
	    {
	      slaveState[status.MPI_SOURCE]=-1;
	      MPI_Send(0, 0, MPI_INT, status.MPI_SOURCE, DEAD_TAG, MPI_COMM_WORLD);
	    }

	  //Wenn mindestens 3 Slaves leben und 2 der 3 eine RunSize gemeldet haben
	  if(slavesReady(slaveState, numNodes) && (moreThanTwoSlaves(slaveState, numNodes)))
	    {
	      //Senderichtung bestimmen
	      if(cfg.dbg) prtSlaveState(slaveState, numNodes);
	      maxNode = max_array(slaveState, numNodes);
	      minNode = min_array(slaveState, numNodes, maxNode);
	      if(minNode == maxNode)
		{
		  if(cfg.dbg) printf("\n\nFehler!!!! minNode == maxNode!!!!\n\n");
		  break;
		}
	      //Senden der Merging-Infos an die Slaves
	      MPI_Send(&slaveState[minNode], 1, MPI_INT, maxNode, DO_RECV_TAG, MPI_COMM_WORLD);
	      slaveState[maxNode] = SLAVE_BUSY;

	      MPI_Send(&maxNode, 1, MPI_INT, minNode, DO_SEND_TAG, MPI_COMM_WORLD);
	      slaveState[minNode] = SLAVE_DEAD;
	    }
	}

      if(cfg.dbg) prtSlaveState(slaveState, numNodes);

      /* ////////////////////////////////////////////////////////////// */
      /* //Empfangen der letzten Ergebnisse//////////////////////////// */
      /* ////////////////////////////////////////////////////////////// */

      int master=0;
      int slaveL;
      int slaveR;
      int count=0;

      // Daten aller Slaves melden, die aus (ungeklärten) Gründen
      // noch nicht zum Zug gekommen sind
      if(cfg.dbg) prtSlaveState(slaveState, numNodes);
      for(i=1; i<numNodes; i++)
	{
	  if(slaveState[i] == NO_ISM_ENTERED)
	    {
	      MPI_Recv(&runSize, 1, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	      if(runSize == SLAVE_DEAD)
		MPI_Send(0, 0, MPI_INT, status.MPI_SOURCE, DEAD_TAG, MPI_COMM_WORLD);
	      slaveState[i] = runSize;
	    }
	}
      if(cfg.dbg) prtSlaveState(slaveState, numNodes);

      //Wenn ein Slave alle Chunks bekommen hat
      if (singleSlave(slaveState, numNodes, cfg.volumesize))
	{
	  slaveL=getReadySlave(slaveState, numNodes);
	  MPI_Send(&master, 1, MPI_INT, slaveL, DO_SEND_TAG, MPI_COMM_WORLD);
	  MPI_Recv(volume.data, volume.size, MPI_INT, slaveL, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	  slaveState[slaveL]=SLAVE_DEAD;
	}
      //Wenn mindestens zwei Slaves mit Daten existieren
      else
	{
	  slaveL=getReadySlave(slaveState, numNodes);
	  MPI_Send(&master, 1, MPI_INT, slaveL, DO_SEND_TAG, MPI_COMM_WORLD);
	  allocbuf(&preResultL,slaveState[slaveL]);
	  slaveState[slaveL]=SLAVE_DEAD;

	  if(waitingForResults(slaveState, numNodes))
	    {
	      MPI_Recv(&runSize, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	      slaveState[status.MPI_SOURCE] = runSize;
	    }
		
	  slaveR=getReadySlave(slaveState, numNodes);
	  MPI_Send(&master, 1, MPI_INT, slaveR, DO_SEND_TAG, MPI_COMM_WORLD);
	  allocbuf(&preResultR,slaveState[slaveR]);
	  slaveState[slaveR]=SLAVE_DEAD;

	  if(cfg.dbg) printf("Master: Warten auf Ergebnis von Node %d. \n", slaveL);

	  MPI_Recv(preResultL.data, preResultL.size, MPI_INT, slaveL, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	  if(cfg.dbg) printf("Master: Ergebnis erfolgreich empfangen. \n");
	  if(cfg.dbg) printf("Master: Warten auf Ergebnis von Node %d. \n", slaveR);

	  MPI_Recv(preResultR.data, preResultR.size, MPI_INT, slaveR, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

	  if(cfg.dbg) printf("Master: Ergebnis erfolgreich empfangen. \n");
	  if(cfg.dbg) printf("Master: Beginne mit finalem Merge.\n");

	  merge(&preResultL, &preResultR, &volume);

	  if(cfg.dbg) printf("Master: Finales Merging abgeschlossen!\n");
	}

      if(cfg.dbg)
	{
	  printf("\n");
	  prtbhead(&volume,5);
	  printf("\n");
	  prtbtail(&volume,5);
	  printf("\n");
	}
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

      if(cfg.dbg) printf("Slave %d: Init erfolgreich.\n", rank);
      //Master/Slave-Sync
      MPI_Barrier(MPI_COMM_WORLD);

      // bei Mergesort einmal Tmp-Buffer allokieren
      struct buffer msortTmpBuf;
      if(cfg.msort) allocbuf(&msortTmpBuf, cfg.chunksize);

      /* ////////////////////////////////////////////////////////////// */
      /* //Chunks verarbeiten////////////////////////////////////////// */
      /* ////////////////////////////////////////////////////////////// */
      if(cfg.dbg) printf("Slave %d: Beginne mit Verarbeiten der Chunks.\n", rank);
      int anyData=0;
      while(1)
	{
	  allocbuf(&chunk, cfg.chunksize);

	  //Beim Master melden, dass Slave bereit
	  MPI_Send(0, 0, MPI_INT, 0, 0, MPI_COMM_WORLD);
	  MPI_Recv(chunk.data, chunk.size, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

	  if(status.MPI_TAG == END_TAG)
	    {
	      if(cfg.dbg) printf("Slave %d: END_TAG empfangen.\n", rank);
	      freebuf(&chunk);
	      break;
	    }
	  else
	    {
	      anyData=1;
	      //neuen Chunk in den bestehenden Run einmergen

	      if(cfg.qsort) quicksort(&chunk);
	      else if (cfg.msort) mergesort(&chunk, &msortTmpBuf, cfg.cachesize);

	      tmpBuf.data = slaveResult.data;
	      tmpBuf.size = slaveResult.size;
	      allocbuf(&slaveResult, tmpBuf.size+chunk.size);
	      merge(&tmpBuf, &chunk, &slaveResult);
	      freebuf(&tmpBuf);
	      freebuf(&chunk);
	    }
			
	}

      // bei Mergesort einmal Tmp-Buffer freigeben
      if(cfg.msort)
	freebuf(&msortTmpBuf);

      MPI_Barrier(MPI_COMM_WORLD);

      //////////////////////////////////////////////////////////////////////////////
      //Alle Chunks erhalten: Warten auf InterSlaveMerge-Befehle////////////////////
      //////////////////////////////////////////////////////////////////////////////
      while(1)
	{
	  if(!(anyData))
	    slaveResult.size=-1;

	  MPI_Send(&slaveResult.size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	  MPI_Recv(&runSize, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

	  if(status.MPI_TAG == DO_RECV_TAG)
	    {
	      allocbuf(&run, runSize);
	      MPI_Recv(run.data, run.size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

	      tmpBuf.data = slaveResult.data;
	      tmpBuf.size = slaveResult.size;
	      allocbuf(&slaveResult, tmpBuf.size+runSize);
	      merge(&tmpBuf, &run, &slaveResult);
	      freebuf(&tmpBuf);
	      freebuf(&run);
	    }
	  else if (status.MPI_TAG == DO_SEND_TAG)
	    {
	      MPI_Send(slaveResult.data, slaveResult.size, MPI_INT, runSize, 0, MPI_COMM_WORLD);

	      freebuf(&slaveResult);
	      break;
	    }
	  else if (status.MPI_TAG == DEAD_TAG)
	    {
	      if(cfg.dbg) printf("Slave %d: Ich arme Sau habe nie Daten bekommen!\n",rank);
	      break;
	    }
	  else
	    {
	      if(cfg.dbg) printf("\n\nUngueltiges TAG empfangen!!!!! Tag: %d\n", status.MPI_TAG);
	      if(cfg.dbg) printf("slaveResult Slave %d: %d", rank, slaveResult.size);
	    }
	}
    }



  /********************************************************/
  //Ab hier wieder für alle
  /********************************************************/
  MPI_Barrier(MPI_COMM_WORLD);

  if (0 == rank)
    {
      // Zeit bis alle Merges abgeschlossen
      clock_gettime(CLOCK_MONOTONIC, &t_4);

      printf ("%llu\t%llu\t%llu\t%llu\n", 
	      timediff(&t_0, &t_1), timediff(&t_0, &t_2),
	      timediff(&t_0, &t_3), timediff(&t_0, &t_4));
    }

  MPI_Finalize();
  return EXIT_SUCCESS;
}



