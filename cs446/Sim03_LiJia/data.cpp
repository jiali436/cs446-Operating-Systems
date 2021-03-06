// Program Information ////////////////////////////////////////////////////////
/**
 * @file data.cpp
 *
 * @brief Implementation file that read configuration/metadata inputs 
 *        and produce useful operating system metrics using timer and threads. 
 * 
 * @author Jia Li
 * 
 * @details Implements all methods neccessary to read/print 
 *          useful operating system metrics using timer and threads.
 *          Use mutex to handle each process and semaphore to handle
 *          each thread.
 *
 * @version 1.00
 *          Jia Li (10 March 2017)
 *          Original Code
 *
 * @Note None
 */


// Header files ///////////////////////////////////////////////////////////////

#include <iostream> 
#include <iomanip>
#include <fstream> 
#include <string>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <pthread.h>
#include "MemoryFunction.h"
#include <ctime>
#include <sys/time.h>
#include <semaphore.h>
using namespace std; 

// Global Constants //////////////////////////////////////////////////////

const int START = 1; 
const int READY = 2; 
const int RUNNING = 3; 
const int WAITING = 4; 
const int EXIT = 5;


sem_t mutex;

// Structures //////////////////////////////////////////////////////

struct threadData
{
	struct timeval t1;
   struct timeval t2;
   float cycleTime; 
};

struct PCB 
{
   int processState; 
};

struct cycleTime 
{
   string componentName; 
   int time; 
};

struct logInfo
{
   string logCriteria; 
   char logFilePath[30]; 
};

struct configData
{
   char filePath[30]; 
   float versionNum;
   cycleTime cycleData[8];
   logInfo logData; 
   int printerQuantity; 
   int hardDriveQuantity; 
   int systemMemorySize;
   int blockMemorySize; 
};

struct metaData
{
   char code;
   char description[30]; 
   int cycles; 
};

// Function definitions //////////////////////////////////////////////////////

void readConfigData( configData &fileData, char fileName[], bool &readFlag );

void readMetaData( vector<metaData> &metaDataStream, configData &fileData, char filePath[] );

void printMetrics( vector<metaData> &metaDataStream, configData &fileData, PCB &processObj ); 
 
void outputToFileMetrics( vector<metaData> &metaDataStream, configData &fileData, PCB &processObj );

double findTime( struct timeval t1, struct timeval t2 ); 

void wait( struct timeval t1, struct timeval t2, float cycleTime ); 

// Function implementations  //////////////////////////////////////////////////////

/**
 * @brief main function.
 *
 * @details call other functions to read/print/output to file. 
 *          
 * @param in: argv, argv[]
 *
 * @note Checks for the number of command-line arguments.  
 */
int main( int argc, char* argv[] )
{
   configData fileData; 
   vector<metaData> metaDataStream; 
   bool readFlag = true;
   int size = 0;
   int index = 0;
   PCB processObj; 
   
   if( ( argc < 2 ) || ( argc > 2 ) )
      cout << "You either have too few command line arguments or too much. Abort.\n"; 
   else
   {
      readConfigData( fileData, argv[1], readFlag );
      if( readFlag == true )
      {
         readMetaData( metaDataStream, fileData, fileData.filePath );
         if( fileData.logData.logCriteria.compare("Both") == 0 )
         { 
            printMetrics( metaDataStream, fileData, processObj );
            outputToFileMetrics( metaDataStream, fileData, processObj );
         }
         else if( fileData.logData.logCriteria.compare("File") == 0 )
         {
            outputToFileMetrics( metaDataStream, fileData, processObj );
         }
         else if( fileData.logData.logCriteria.compare("Monitor") == 0 )
         {
            printMetrics( metaDataStream, fileData, processObj );
         }
         
      }  

   }
 
   return 0; 
}

/**
 * @brief readConfigData function.
 *
 * @details reads in config data from the config file. 
 *          
 * @param in: fileData, fileName, readFlag
 *
 * @note Accepts file name from the parameter. Then parse the string
 *       with the stream ignore function to read in the components of 
 *       operating system management and their cycle values. 
 *       Checks for empty file and incorrect
 *       filenames. In addition, the function reports if there is any 
 *       missing data in the cycle times. 
 */
void readConfigData( configData &fileData, char fileName[], bool &readFlag )
{
   ifstream fin;
   const int logStreamSize = 2;  
   string temp, tempTwo;
   int index = 0; 
   int indexTwo = 0;

   fin.open(fileName); 
   readFlag = true; 
   
   if( fin.peek() == std::ifstream::traits_type::eof() )
   {
      cout << fileName << " is either an empty config file or an invalid filename\n";
      readFlag = false; 
   }
   else 
   {
      while( !fin.eof( ) ) 
      {
         fin >> temp; 
         if( ( temp.compare("Start") == 0 ) || ( temp.compare("End") == 0 ) )
         {
            fin.ignore( 1000, '\n' ); 
         }
         else if( temp.compare("Log:") == 0 )
         {
            while( indexTwo < logStreamSize )
            {
               fin >> temp; 
               indexTwo++;
            }
            fin >> fileData.logData.logCriteria; 
            indexTwo = 0; 
         }
         else if( temp.compare("Log") == 0 )
         {
            fin.ignore( 1000, ':' );
            fin >> fileData.logData.logFilePath; 
         }
         else if( temp.compare("Version/Phase:") == 0 )
         {
            fin >> fileData.versionNum;
         }
         else if( temp.compare("File") == 0 ) 
         {
            fin.ignore( 1000, ':' );
            fin >> fileData.filePath; 
         }
         else
         {
            tempTwo = temp; 
            fin >> temp; 

            if( tempTwo.compare("System") == 0 && temp.compare("memory") == 0 )
            {
            	fin.ignore( 1000, ':' );
               fin >> fileData.systemMemorySize;
            }
            else if( tempTwo.compare("Memory") == 0 && temp.compare("block") == 0 )
            {
            	fin.ignore( 1000, ':' );
               fin >> fileData.blockMemorySize;
            }
            else if( (tempTwo.compare("Printer") == 0) && (index > 7) )
            {
               fin >> fileData.printerQuantity;
            }
            else if( (tempTwo.compare("Hard") == 0) && (index > 7) )
            {
            	fin.ignore( 1000, ':' );
               fin >> fileData.hardDriveQuantity;
            }
            else 
            {
            	fileData.cycleData[index].componentName = tempTwo;

            	if( temp.compare("cycle") != 0 && temp.compare("display") != 0 ) 
            	{
               	fileData.cycleData[index].componentName = fileData.cycleData[index].componentName + " " + temp;
            	}

            	fin.ignore( 1000, ':' );

            	if( fin.peek() == '\n' )
            	{
               	cout << "You're missing one or more cycle time in the config file!\n";
               	readFlag = false;
               	fin.ignore( 1000, char_traits<char>::eof() );
            	}
            	else
               	fin >> fileData.cycleData[index].time;

            	index++; 
            }
         }
         
      }
   }
   
   fin.close( ); 
}

/**
 * @brief readMetaData function
 *
 * @details reads in meta-data from the meta-data file regarding to processes
 *          and cycle numbers.
 *          
 * @param in: metaDataStream (vector data structure), filePath
 *
 * @note Parse the input stream in the file base on white space 
 *       characters and special characters. Reject these characters 
 *       they are not being used as special delimiters. The algorithm 
 *       checks for incorrect/missing data in its inputs. In addition,
 *       the algorithm checks for typos in the file.
 */
void readMetaData( vector<metaData> &metaDataStream, configData &fileData, char filePath[] )
{
   ifstream fin; 
   metaData temp; 
   char tempChar = '\0';
   char tempCharTwo = '\0';
   int tempNum = 0; 
   int tempNumTwo = 0; 
   bool flag = false;
   char tempStr[30]; 
   int index = 0;
   int indexTwo = 0;
   int length = 0;

   fin.open( filePath ); 
   
   if( fin.peek() == std::ifstream::traits_type::eof() )
   {
      cout << filePath << " is either an empty metadata file or an invalid filename\n"; 
   }
   else
   {
      fin.ignore( 1000, '\n' ); 
      
      while( !fin.eof( ) ) 
      {
         temp.code = fin.get( ); 
         while( temp.code == '\n' )
            temp.code = fin.get( );

         while( temp.code == ' ' )
            temp.code = fin.get( );
         
         if( temp.code == 'E' )
         {
            fin.ignore( 1000, char_traits<char>::eof() ); 
         }
         else if( ( temp.code == 'S' ) ||
                  ( temp.code == 'A' ) ||
                  ( temp.code == 'M' ) ||
                  ( temp.code == 'P' ) ||
                  ( temp.code == 'I' ) ||
                  ( temp.code == 'O' ) )
         {
            fin.ignore( 1000, '(' ); 
            fin.getline( temp.description, 1000, ')' );
            
            if( ( temp.code == 'S' ) || ( temp.code == 'A' ) )
            {
               if( ( strcmp("start", temp.description) == 0 ) ||
                   ( strcmp("end", temp.description) == 0 ) )
               {
                  flag = true; 
               } 
               else 
               {
                  flag = false; 
                  fin.ignore( 1000, char_traits<char>::eof() );
                  cout << "Sorry, either you made a typo or you forgot to enter the description\n"; 
               }
            } 
            else if( temp.code == 'P' )
            {
               if( strcmp("run", temp.description) == 0 )
               {
                  flag = true; 
               } 
               else 
               {
                  flag = false; 
                  fin.ignore( 1000, char_traits<char>::eof() );
                  cout << "Sorry, either you made a typo or you forgot to enter the description\n";  
               }
            }
            else if( temp.code == 'M' )
            {
               if( ( strcmp("allocate", temp.description) == 0 ) ||
                   ( strcmp("block", temp.description) == 0 ) )
               {
                  flag = true; 
               } 
               else 
               {
                  flag = false; 
                  fin.ignore( 1000, char_traits<char>::eof() );
                  cout << "Sorry, either you made a typo or you forgot to enter the description\n"; 
               }
            }
            else if( ( temp.code == 'I' ) || ( temp.code == 'O' ) )
            {
               flag = false; 

               for( indexTwo = 0; indexTwo < 8; indexTwo++ )
               {
                  length = strlen( temp.description ); 

                  if( fileData.cycleData[indexTwo].componentName.compare(1, length, temp.description, 1, length) == 0 )
                  {
                     flag = true;   
                  }
               }
 
               if( flag == false )
               {
                  fin.ignore( 1000, char_traits<char>::eof() );
                  cout << "Sorry, either you made a typo or you forgot to enter the description\n";
               }
            }
            
            if( flag == true )
            {
               tempChar = fin.get( );

               while( tempChar == ' ' )
                  tempChar = fin.get( );

               if( tempChar == '-' )
               {
                  cout << "Invalid negative cycle values in meta-data file.\n";
                  fin.ignore( 1000, char_traits<char>::eof() );
               }
               else if( ( tempChar != '0' ) && 
                   ( tempChar != '1' ) && 
                   ( tempChar != '2' ) &&
                   ( tempChar != '3' ) &&
                   ( tempChar != '4' ) &&
                   ( tempChar != '5' ) &&
                   ( tempChar != '6' ) && 
                   ( tempChar != '7' ) && 
                   ( tempChar != '8' ) && 
                   ( tempChar != '9' ) )
               {
                  cout << "You're missing a cycle value in the meta-data file.\n";
                  fin.ignore( 1000, char_traits<char>::eof() );
               }
               else
               {
                  tempNum = tempChar - '0'; 
                  tempCharTwo = fin.get( ); 
                  temp.cycles = tempNum;
                  while( ( tempCharTwo == '0' ) || 
                      ( tempCharTwo == '1' ) || 
                      ( tempCharTwo == '2' ) ||
                      ( tempCharTwo == '3' ) ||
                      ( tempCharTwo == '4' ) ||
                      ( tempCharTwo == '5' ) ||
                      ( tempCharTwo == '6' ) || 
                      ( tempCharTwo == '7' ) || 
                      ( tempCharTwo == '8' ) || 
                      ( tempCharTwo == '9' ) )
                  {
                     tempNum = tempNum * 10; 
                     tempNumTwo = tempCharTwo - '0'; 
                     temp.cycles = tempNum + tempNumTwo;
                     tempCharTwo = fin.get( );
                  }

                  metaDataStream.push_back( temp );

                  tempChar = fin.peek( ); 
            
                  if( tempChar == ' ' )
                  {
                     fin.ignore( 1000, ' ' );
                     tempChar = fin.peek( ); 
                     if( tempChar == '\n' )
                        fin.ignore( 1000, '\n' ); 
                  }
                  index++;
               } 
            }
         }
         else
         {
            cout << "In the metadata file, you either did not enter a metadata code or the code is invalid.\n"; 
            fin.ignore( 1000, char_traits<char>::eof() );  
         }
      }
   }
   fin.close( ); 

}

/**
 * @brief findTime function.
 *
 * @details find the time in ms between two intervals. 
 *          
 * @param in: t1, t2
 *
 * @note N/A.  
 */
double findTime( struct timeval t1, struct timeval t2 )
{
   double elapsedTime = ( t2.tv_sec - t1.tv_sec ) * 1000.0;
   elapsedTime += ( t2.tv_usec - t1.tv_usec ) / 1000.0;
   elapsedTime = elapsedTime / 1000.0;  
}

/**
 * @brief wait function.
 *
 * @details determine the time in ms between two intervals. 
 *          
 * @param in: t1, t2, cycleTime
 *
 * @note N/A.  
 */
void wait( struct timeval t1, struct timeval t2, double cycleTime )
{
	double elapsedTime = findTime( t1, t2 ); 
 
   cycleTime = elapsedTime + cycleTime;
   
   while( elapsedTime < cycleTime )
   {
   	gettimeofday(&t2, NULL);
      elapsedTime = findTime( t1, t2 ); 
   }
}

/**
 * @brief pWait function.
 *
 * @details calculate and wait for the elipsed time in ms between two intervals. 
 *          
 * @param in: threadArg
 *
 * @note N/A.  
 */
void *pWait(void *threadArg)
{
	
	sem_wait(&mutex); 
   struct threadData *IOData = (struct threadData *) threadArg;
   
   double elapsedTime = findTime( IOData->t1, IOData->t2 ); 

   IOData->cycleTime = IOData->cycleTime + elapsedTime;
   
   while( elapsedTime < IOData->cycleTime )
   {
   	gettimeofday(&IOData->t2, NULL); 
      elapsedTime = findTime( IOData->t1, IOData->t2 );
   }
	sem_post(&mutex);
}

/**
 * @brief printMetrics function. 
 *
 * @details output all the meta-data time and thread metrics to the monitor. Utilize
 *          mutex to handle processes and semaphore to handle threads.
 *          
 * @param in: metaDataStream, fileData, processObj
 *
 * @note None
 */
void printMetrics( vector<metaData> &metaDataStream, configData &fileData, PCB &processObj )
{
   int index = 0;
   int indexTwo = 0;
   int length = 0;
   int processID = 0; 
   int threadSize = 1;
   unsigned int memoryNum = 0;
   struct timeval t1, t2; 
   double elapsedTime;
   double cycles; 
   pthread_t OSThreads;
   pthread_attr_t attr;
   void *status; 
   struct threadData td; 
   int rc; 
   int indexThree = 0;
   int indexFour = 0;
   int indexFive = 0;

   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
   gettimeofday(&t1, NULL);
  	gettimeofday(&t2, NULL); 
   elapsedTime = findTime( t1, t2 ); 
   
   cout << setprecision(6) << elapsedTime << " - " << "Simulator program starting\n";  

   for( index = 0; index < metaDataStream.size( ); index++ ) 
   {

		
		int pthread_mutex_lock(pthread_mutex_t *mutex);


      if( metaDataStream[index].code == 'P' )
      {
         for( indexTwo = 0; indexTwo < 8; indexTwo++ )
         {
            if( fileData.cycleData[indexTwo].componentName.compare("Processor") == 0 )
            {
               processObj.processState = WAITING; 
               cycles = ( metaDataStream[index].cycles * fileData.cycleData[indexTwo].time ) / 1000.0; 
  					gettimeofday(&t2, NULL);  
               elapsedTime = findTime( t1, t2 ); 
               cout << setprecision(6) << elapsedTime << " - " << "Process" << processID << ": start processing action"<< endl;
               wait( t1, t2, cycles );
               gettimeofday(&t2, NULL); 
               elapsedTime = findTime( t1, t2 ); 
               cout << setprecision(6) << elapsedTime << " - " << "Process" << processID << ": end processing action"<< endl;
            }
         }
      }
      else if( metaDataStream[index].code == 'A' )
      {
         if( strcmp( metaDataStream[index].description, "start" ) == 0 )
         {
            processObj.processState = START; 
            processID++;
            gettimeofday(&t2, NULL); 
            elapsedTime = findTime( t1, t2 ); 
            cout << setprecision(6) << elapsedTime << " - " << "OS: preparing process " << processID << endl;
            gettimeofday(&t2, NULL); 
            elapsedTime = findTime( t1, t2 ); 
            cout << setprecision(6) << elapsedTime << " - " << "OS: starting process " << processID << endl;
         }
         else if( strcmp( metaDataStream[index].description, "end" ) == 0 )
         {
            processObj.processState = EXIT; 
            gettimeofday(&t2, NULL); 
            elapsedTime = findTime( t1, t2 ); 
            cout << setprecision(6) << elapsedTime << " - " << "OS: removing process " << processID << endl;
         }
      }
      else if( metaDataStream[index].code == 'M' )
      {
         for( indexTwo = 0; indexTwo < 8; indexTwo++ )
         {
            if( fileData.cycleData[indexTwo].componentName.compare("Memory") == 0 )
            {
               if( strcmp( metaDataStream[index].description, "allocate" ) == 0 )
         		{
                  processObj.processState = RUNNING;;
            		cycles = ( metaDataStream[index].cycles * fileData.cycleData[indexTwo].time ) / 1000.0; 
  					   gettimeofday(&t2, NULL); 
                  wait( t1, t2, cycles ); 
                  elapsedTime = findTime( t1, t2 ); 
               	cout << setprecision(6) << elapsedTime << " - " << "Process" << processID << ": allocating memory"<< endl;
               	gettimeofday(&t2, NULL); 
                  elapsedTime = findTime( t1, t2 ); 
               	cout << setprecision(6) << elapsedTime << " - " << "Process" << processID << ": memory allocated at ";
                  cout << "0x" << setfill('0');
                  cout << setw(8) << hex << memoryNum << endl;
                  memoryNum = allocateMemory( memoryNum, fileData.blockMemorySize, fileData.systemMemorySize);
         		}
               else if( strcmp( metaDataStream[index].description, "block" ) == 0 )
         		{
                  processObj.processState = READY;
            		cycles = ( metaDataStream[index].cycles * fileData.cycleData[indexTwo].time ) / 1000.0; 
  					   gettimeofday(&t2, NULL); 
                  wait( t1, t2, cycles );  
                  elapsedTime = findTime( t1, t2 );  
               	cout << setprecision(6) << elapsedTime << " - " << "Process" << processID << ": start memory blocking"<< endl;
               	gettimeofday(&t2, NULL); 
                  elapsedTime = findTime( t1, t2 ); 
               	cout << setprecision(6) << elapsedTime << " - " << "Process" << processID << ": end memory blocking"<< endl;
         		}
            }
         }
      }
      else if( ( metaDataStream[index].code == 'I' ) || ( metaDataStream[index].code == 'O' ) )
      {
         for( indexTwo = 0; indexTwo < 8; indexTwo++ )
         {
            length = strlen( metaDataStream[index].description ); 

            if( fileData.cycleData[indexTwo].componentName.compare(1, length, metaDataStream[index].description, 1, length) == 0 )
            {
               processObj.processState = WAITING;
            	cycles = ( metaDataStream[index].cycles * fileData.cycleData[indexTwo].time ) / 1000.0; 
  					gettimeofday(&t2, NULL); 
               elapsedTime = findTime( t1, t2 ); 
               cout << setprecision(6) << elapsedTime << " - " << "Process" << processID << ": start ";
               cout << metaDataStream[index].description; 
               if( metaDataStream[index].code == 'I' )
               {
               	cout << " input\n"; 
               }
               if( metaDataStream[index].code == 'O' )
               {
               	cout << " output\n"; 
               } 
               td.t1 = t1; 
               td.t2 = t2; 
               td.cycleTime = cycles; 

					
					sem_init(&mutex, 0, 1);

               rc = pthread_create(&OSThreads, &attr, pWait, (void*) &td);
					if(rc)
 					{
						cout << "Error: cannot create thread " << rc << endl; 
 						exit(-1); 
					}
               processObj.processState = READY;
               rc = pthread_join(OSThreads, &status);
      
					sem_destroy(&mutex);

      			if( rc )
      			{
         			cout << "Error: can't join " << rc << endl;
         			exit(-1);
      			}

               gettimeofday(&t2, NULL);
               elapsedTime = findTime( t1, t2 ); 
               cout << setprecision(6) << elapsedTime << " - " << "Process" << processID << ": end ";
               cout << metaDataStream[index].description; 

               if( metaDataStream[index].code == 'I' )
               {
               	cout << " input"; 
               }
               if( metaDataStream[index].code == 'O' )
               {
               	cout << " output"; 
               }
               if( strcmp( metaDataStream[index].description, "hard drive" ) == 0 ) 
               {
                  cout << " on HDD " << indexFour;
                  indexFour++;
                  indexFour = indexFour % fileData.hardDriveQuantity;
               }
               if( strcmp( metaDataStream[index].description, "printer" ) == 0 ) 
               {
                  cout << " on PRNTR " << indexFive;
                  indexFive++;
                  indexFive = indexFive % fileData.printerQuantity;
               }
               cout << endl;
            }
         }
      }
		int pthread_mutex_unlock(pthread_mutex_t *mutex);
   }
}

/**
 * @brief outputToFileMetrics function. 
 *
 * @details output all the meta-data time and thread metrics to the monitor. Utilize
 *          mutex to handle processes and semaphore to handle threads.  
 *          
 * @param in: metaDataStream, fileData, processObj
 *
 * @note None
 */
void outputToFileMetrics( vector<metaData> &metaDataStream, configData &fileData, PCB &processObj )
{
   int index = 0;
   int indexTwo = 0;
   int length = 0;
   int processID = 0; 
   int threadSize = 1;
   unsigned int memoryNum = 00000000;
   struct timeval t1, t2; 
   double elapsedTime;
   double cycles; 
   pthread_t OSThreads;
   pthread_attr_t attr;
   void *status; 
   struct threadData td; 
   int rc; 
   int indexThree = 0;
   int indexFour = 0;
   int indexFive = 0;
   ofstream fout; 

   fout.open( fileData.logData.logFilePath );
   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
   gettimeofday(&t1, NULL);
  	gettimeofday(&t2, NULL); 
   elapsedTime = findTime( t1, t2 ); 
   
   fout << setprecision(6) << elapsedTime << " - " << "Simulator program starting\n";  

   for( index = 0; index < metaDataStream.size( ); index++ ) 
   {
      int pthread_mutex_lock(pthread_mutex_t *mutex);

      if( metaDataStream[index].code == 'P' )
      {
         for( indexTwo = 0; indexTwo < 8; indexTwo++ )
         {
            if( fileData.cycleData[indexTwo].componentName.compare("Processor") == 0 )
            {
               processObj.processState = WAITING; 
               cycles = ( metaDataStream[index].cycles * fileData.cycleData[indexTwo].time ) / 1000.0; 
  					gettimeofday(&t2, NULL);  
               elapsedTime = findTime( t1, t2 ); 
               fout << setprecision(6) << elapsedTime << " - " << "Process" << processID << ": start processing action"<< endl;
               wait( t1, t2, cycles );
               gettimeofday(&t2, NULL); 
               elapsedTime = findTime( t1, t2 ); 
               fout << setprecision(6) << elapsedTime << " - " << "Process" << processID << ": end processing action"<< endl;
            }
         }
      }
      else if( metaDataStream[index].code == 'A' )
      {
         if( strcmp( metaDataStream[index].description, "start" ) == 0 )
         {
            processObj.processState = START; 
            processID++;
            gettimeofday(&t2, NULL); 
            elapsedTime = findTime( t1, t2 ); 
            fout << setprecision(6) << elapsedTime << " - " << "OS: preparing process " << processID << endl;
            gettimeofday(&t2, NULL); 
            elapsedTime = findTime( t1, t2 ); 
            fout << setprecision(6) << elapsedTime << " - " << "OS: starting process " << processID << endl;
         }
         else if( strcmp( metaDataStream[index].description, "end" ) == 0 )
         {
            processObj.processState = EXIT; 
            gettimeofday(&t2, NULL); 
            elapsedTime = findTime( t1, t2 ); 
            fout << setprecision(6) << elapsedTime << " - " << "OS: removing process " << processID << endl;
         }
      }
      else if( metaDataStream[index].code == 'M' )
      {
         for( indexTwo = 0; indexTwo < 8; indexTwo++ )
         {
            if( fileData.cycleData[indexTwo].componentName.compare("Memory") == 0 )
            {
               if( strcmp( metaDataStream[index].description, "allocate" ) == 0 )
         		{
                  processObj.processState = RUNNING;;
            		cycles = ( metaDataStream[index].cycles * fileData.cycleData[indexTwo].time ) / 1000.0; 
  					   gettimeofday(&t2, NULL); 
                  wait( t1, t2, cycles ); 
                  elapsedTime = findTime( t1, t2 );
               	fout << setprecision(6) << elapsedTime << " - " << "Process" << processID << ": allocating memory"<< endl; 
               	gettimeofday(&t2, NULL); 
                  elapsedTime = findTime( t1, t2 ); 
               	fout << setprecision(6) << elapsedTime << " - " << "Process" << processID << ": memory allocated at ";
                  fout << "0x" << setfill('0');
                  fout << setw(8) << hex << memoryNum << endl;
                  memoryNum = allocateMemory( memoryNum, fileData.blockMemorySize, fileData.systemMemorySize);
         		}
               else if( strcmp( metaDataStream[index].description, "block" ) == 0 )
         		{
                  processObj.processState = READY;
            		cycles = ( metaDataStream[index].cycles * fileData.cycleData[indexTwo].time ) / 1000.0; 
  					   gettimeofday(&t2, NULL); 
                  wait( t1, t2, cycles );  
                  elapsedTime = findTime( t1, t2 );  
               	fout << setprecision(6) << elapsedTime << " - " << "Process" << processID << ": start memory blocking"<< endl;
               	gettimeofday(&t2, NULL); 
                  elapsedTime = findTime( t1, t2 ); 
               	fout << setprecision(6) << elapsedTime << " - " << "Process" << processID << ": end memory blocking"<< endl;
         		}
            }
         }
      }
      else if( ( metaDataStream[index].code == 'I' ) || ( metaDataStream[index].code == 'O' ) )
      {
         for( indexTwo = 0; indexTwo < 8; indexTwo++ )
         {
            length = strlen( metaDataStream[index].description ); 

            if( fileData.cycleData[indexTwo].componentName.compare(1, length, metaDataStream[index].description, 1, length) == 0 )
            {
               processObj.processState = WAITING;
            	cycles = ( metaDataStream[index].cycles * fileData.cycleData[indexTwo].time ) / 1000.0; 
  					gettimeofday(&t2, NULL); 
               elapsedTime = findTime( t1, t2 ); 
               fout << setprecision(6) << elapsedTime << " - " << "Process" << processID << ": start ";
               fout << metaDataStream[index].description; 
               if( metaDataStream[index].code == 'I' )
               {
               	fout << " input\n"; 
               }
               if( metaDataStream[index].code == 'O' )
               {
               	fout << " output\n"; 
               } 
               td.t1 = t1; 
               td.t2 = t2; 
               td.cycleTime = cycles; 
               sem_init(&mutex, 0, 1);
               rc = pthread_create(&OSThreads, &attr, pWait, (void*) &td);
					if(rc)
 					{
						fout << "Error: cannot create thread " << rc << endl; 
 						exit(-1); 
					}
               processObj.processState = READY;
               rc = pthread_join(OSThreads, &status);
               sem_init(&mutex, 0, 1);
      
      			if( rc )
      			{
         			fout << "Error: can't join " << rc << endl;
         			exit(-1);
      			}

               gettimeofday(&t2, NULL);
               elapsedTime = findTime( t1, t2 ); 
               fout << setprecision(6) << elapsedTime << " - " << "Process" << processID << ": end ";
               fout << metaDataStream[index].description; 
               if( metaDataStream[index].code == 'I' )
               {
               	fout << " input"; 
               }
               if( metaDataStream[index].code == 'O' )
               {
               	fout << " output"; 
               }
               
               if( strcmp( metaDataStream[index].description, "hard drive" ) == 0 ) 
               {
                  fout << " on HDD " << indexFour;
                  indexFour++;
                  indexFour = indexFour % fileData.hardDriveQuantity;
               }
               if( strcmp( metaDataStream[index].description, "printer" ) == 0 ) 
               {
                  fout << " on PRNTR " << indexFive;
                  indexFive++;
                  indexFive = indexFive % fileData.printerQuantity;
               }
               fout << endl;
            }
         }
      }
      int pthread_mutex_unlock(pthread_mutex_t *mutex);
   }

   pthread_exit(NULL);
}

