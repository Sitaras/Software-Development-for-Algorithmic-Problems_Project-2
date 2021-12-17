#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include "Vector/vector.h"
#include "./hashTable/hashTable.h"
#include "LSH/lsh.h"
#include "./parsing/parsingLSH.h"
#include "./hashTable/hashTableList/hashTableList.h"

#define W_DIVIDER 40


int w;
int k_LSH;
int hashTableSize;


static int wValueCalculation(int dim){
  if(dim>850){
    return 400;
  }else if(dim>700){
    return 200;
  }else if(dim>500){
    return 100;
  }else if(dim>300){
    return 50;
  }else if(dim>150){
    return 20;
  }else{
    return 6;
  }
}


void vectorTimeSeriesLSH(char* arg_inputFile,char* arg_queryFile,int arg_k_LSH,int arg_L,char* arg_outputFile,int distanceTrueOff)  {
  // for this case every timeseries represented as vector in R^d
  // (not time representation needed,same implementation with the previous project)

  char inputFile[100];
  strcpy(inputFile,arg_inputFile);
  char queryFile[100];
  strcpy(queryFile,arg_queryFile);
  char outputFile[100];
  strcpy(outputFile,arg_outputFile);
  int l=arg_L;
  k_LSH = arg_k_LSH;


  hashTableSize = 1000;



  srand(time(NULL));

  LSH lsh;
  List list;
  clock_t begin = clock();

  int dim = findDimLSH(inputFile);
  printf("DIMENSION = %d\n",dim);

  list = initializeList();

  int numberOfVectorsInFile = 0;
  readFileLSH(inputFile,&list,&numberOfVectorsInFile,0,NULL,dim);

  clock_t end = clock();

  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  printf("Parsed input file in : %f seconds\n",time_spent);
  printf("Number of vectors in input file: %d\n",numberOfVectorsInFile);
  hashTableSize=numberOfVectorsInFile/16;

  printf("Finding optimal value of w based on the input file\n");
  w = wValueCalculation(dim);
  printf("Found value of w = %d\n",w );

  begin = clock();
  lsh = initializeLSH(l,dim);
  insertFromListToLSH(list,lsh);
  end = clock();

  time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  printf("Created LSH in : %f seconds\n",time_spent);

  readQueryFileLSH(queryFile,outputFile,lsh,list,dim,distanceTrueOff);

  destroyLSH(lsh);
  listDelete(list,0);
}


void vectorTimeSeriesLSHFrechetDiscrete(char* arg_inputFile,char* arg_queryFile,int arg_k_LSH,int arg_L,char* arg_outputFile,double arg_delta,int distanceTrueOff){
  // for this case every timeseries represented as curve in R^2
  // in this implementation (compared with the previous one) snapping and padding added (before a timeseries being inserted or searched in LSH)
  // and the distance between 2 curves now calculated with the Discrete Frechet metric

  char inputFile[100];
  strcpy(inputFile,arg_inputFile);
  char queryFile[100];
  strcpy(queryFile,arg_queryFile);
  char outputFile[100];
  strcpy(outputFile,arg_outputFile);
  int l=arg_L;
  double delta=arg_delta;
  k_LSH = 4;
  hashTableSize = 1000;



  srand(time(NULL));



  LSH lsh;
  List list;
  clock_t begin = clock();
  int dim = findDimLSH(inputFile);
  double sum=0.0;
  double time[dim];
  for(int i=0;i<dim;i++){
    time[i]=sum;
    sum+=1.0;
  }
  printf("DIMENSION = %d\n",dim);
  list = initializeList();
  int numberOfVectorsInFile = 0;
  readFileLSH(inputFile,&list,&numberOfVectorsInFile,1,time,dim);
  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  printf("Parsed input file in : %f seconds\n",time_spent);
  printf("Number of vectors in input file: %d\n",numberOfVectorsInFile);
  hashTableSize=numberOfVectorsInFile/16;

  printf("Finding optimal value of w based on the input file\n");
  w = wValueCalculation(dim);
  printf("Found value of w = %d\n",w );

  begin = clock();

  // the vector that will result from snapping that used each time to compute the value of the correspoding g function,
  // it has twice the initial dimension of the timeseries
  // so initialize the LSH with the needed dimension (2*dim)
  lsh = initializeLSH(l,2*dim);

  Grids grids = initializeGrids(delta,l,2); //  initialize the corresponding t for the 2 dimensions (x,y coordinates) that use at the snapping

  insertTimeSeriesFromListToLSH(list,lsh,grids,delta);
  end = clock();
  time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  printf("Created LSH in : %f seconds\n",time_spent);

  readQueryFileLSH_DiscreteFrechet(queryFile,outputFile,lsh,list,grids,delta,time,dim,distanceTrueOff);
  deleteGrids(grids,2);
  destroyLSH(lsh);
  listDelete(list,0);
}


void vectorTimeSeriesLSHFrechetContinuous(char* arg_inputFile,char* arg_queryFile,int arg_k_LSH,char* arg_outputFile,double arg_delta,double epsilon,int distanceTrueOff){
  // for this case every timeseries represented as curve in the line R.
  // in this implementation (compared with the previous one) added:
  // filtering, snapping, minima n' maxima, padding
  // (before a timeseries being inserted or searched in LSH)
  // and the distance between 2 curves now calculated with the Continuous Frechet metric from the corresponding library
  // (Fred-master, an interface function has been created to link this library with our code).
  // Also, LSH structure for this case has only one hash table, not L hash tables.


  char inputFile[100];
  strcpy(inputFile,arg_inputFile);
  char queryFile[100];
  strcpy(queryFile,arg_queryFile);
  char outputFile[100];
  strcpy(outputFile,arg_outputFile);
  int l=1;
  double delta=arg_delta;
  k_LSH = arg_k_LSH;
  k_LSH = 1;
  hashTableSize = 1000;

  srand(time(NULL));

  LSH lsh;
  List list;
  clock_t begin = clock();
  int dim = findDimLSH(inputFile);
  double sum=0.0;
  double time[dim];
  for(int i=0;i<dim;i++){
    time[i]=sum;
    sum+=1.0;
  }
  printf("DIMENSION = %d\n",dim);
  list = initializeList();
  int numberOfVectorsInFile = 0;
  readFileLSH(inputFile,&list,&numberOfVectorsInFile,1,time,dim);
  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  printf("Parsed input file in : %f seconds\n",time_spent);
  printf("Number of vectors in input file: %d\n",numberOfVectorsInFile);
  hashTableSize=numberOfVectorsInFile/16;

  printf("Finding optimal value of w based on the input file\n");
  w = wValueCalculation(dim);
  printf("Found value of w = %d\n",w );

  begin = clock();
  lsh = initializeLSH(l,dim);
  Grids grid = initializeGrids(delta,l,1); // (l=1) -> only one t as we only have one hash table / one grid
  insertContinuousTimeSeriesFromListToLSH(list,lsh,delta,epsilon,grid);
  end = clock();
  time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  printf("Created LSH in : %f seconds\n",time_spent);
  readQueryFileLSH_ContinuousFrechet(queryFile,outputFile,lsh,list,delta,epsilon,time,dim,grid,distanceTrueOff);

  deleteGrids(grid,1);
  destroyLSH(lsh);
  listDelete(list,0);
}
