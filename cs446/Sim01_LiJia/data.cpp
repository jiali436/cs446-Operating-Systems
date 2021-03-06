// Program Information ////////////////////////////////////////////////////////
/**
 * @file data.cpp
 *
 * @brief Implementation file that read configuration/metadata inputs 
 *        and produce useful operating system metrics. 
 * 
 * @author Jia Li
 * 
 * @details Implements all methods neccessary to read/print the 
 *          config/metadata inputs. 
 *
 * @version 1.00
 *          Jia Li (3 Februrary 2017)
 *          Original Code
 *
 * @Note None
 */


// Header files ///////////////////////////////////////////////////////////////

#include <iostream> 
#include <fstream> 
#include <string>
#include <cstring>
#include <vector>

using namespace std; 

// Structures //////////////////////////////////////////////////////

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

void printMetrics( vector<metaData> &metaDataStream, configData &fileData ); 
 
void outputToFileMetrics( vector<metaData> &metaDataStream, configData &fileData );

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
            outputToFileMetrics( metaDataStream, fileData );
            printMetrics( metaDataStream, fileData );
         }
         else if( fileData.logData.logCriteria.compare("File") == 0 )
         {
            outputToFileMetrics( metaDataStream, fileData );
         }
         else if( fileData.logData.logCriteria.compare("Monitor") == 0 )
         {
            printMetrics( metaDataStream, fileData );
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
   string temp; 
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
            fileData.cycleData[index].componentName = temp; 
            fin >> temp; 

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
 * @brief printMetrics function. 
 *
 * @details output all the meta-data metrics to the monitor. 
 *          
 * @param in: metaDataStream, fileData
 *
 * @note None
 */
void printMetrics( vector<metaData> &metaDataStream, configData &fileData )
{
   int index = 0;
   int indexTwo = 0;
   int length = 0;

   for( index = 0; index < 8; index++ )
   {
      cout << fileData.cycleData[index].componentName;
      cout << " = ";
      cout << fileData.cycleData[index].time << " ms/cycle\n"; 
   } 
   
   if( fileData.logData.logCriteria.compare("Both") == 0 )
   {
      cout << "Logged to: monitor and " << fileData.logData.logFilePath << endl; 
   }
   else if( fileData.logData.logCriteria.compare("File") == 0 )
   {
      cout << "Logged to: " << fileData.logData.logFilePath << endl; 
   }
   else if( fileData.logData.logCriteria.compare("Monitor") == 0 )
   {
      cout << "Logged to: monitor" << endl;
   }
   cout << endl;

   cout << "Meta-Data Metrics" << endl; 

   for( index = 0; index < metaDataStream.size( ); index++ ) 
   {
      if( metaDataStream[index].code == 'P' )
      {
         for( indexTwo = 0; indexTwo < 8; indexTwo++ )
         {
            if( fileData.cycleData[indexTwo].componentName.compare("Processor") == 0 )
            {
               cout << metaDataStream[index].code << "(";
               cout << metaDataStream[index].description << ")" << metaDataStream[index].cycles; 
               cout << " - "; 
               cout << metaDataStream[index].cycles * fileData.cycleData[indexTwo].time << " ms" << endl;
            }
         }
      }
      else if( metaDataStream[index].code == 'M' )
      {
         for( indexTwo = 0; indexTwo < 8; indexTwo++ )
         {
            if( fileData.cycleData[indexTwo].componentName.compare("Memory") == 0 )
            {
               cout << metaDataStream[index].code << "(";
               cout << metaDataStream[index].description << ")" << metaDataStream[index].cycles; 
               cout << " - "; 
               cout << metaDataStream[index].cycles * fileData.cycleData[indexTwo].time << " ms" << endl;
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
               cout << metaDataStream[index].code << "(";
               cout << metaDataStream[index].description << ")" << metaDataStream[index].cycles; 
               cout << " - "; 
               cout << metaDataStream[index].cycles * fileData.cycleData[indexTwo].time << " ms" << endl;
            }
         }
      }
   }
   
}

/**
 * @brief outputToFileMetrics function. 
 *
 * @details output all the meta-data metrics to a specific file location. 
 *          
 * @param in: metaDataStream, fileData
 *
 * @note None
 */
void outputToFileMetrics( vector<metaData> &metaDataStream, configData &fileData )
{
   int index = 0;
   int indexTwo = 0;
   int length = 0;
   ofstream fout; 

   fout.open( fileData.logData.logFilePath );

   for( index = 0; index < 8; index++ )
   {
      fout << fileData.cycleData[index].componentName;
      fout << " = ";
      fout << fileData.cycleData[index].time << " ms/cycle\n"; 
   } 
   
   if( fileData.logData.logCriteria.compare("Both") == 0 )
   {
      fout << "Logged to: monitor and " << fileData.logData.logFilePath << endl; 
   }
   else if( fileData.logData.logCriteria.compare("File") == 0 )
   {
      fout << "Logged to: " << fileData.logData.logFilePath << endl; 
   }
   else if( fileData.logData.logCriteria.compare("Monitor") == 0 )
   {
      fout << "Logged to: monitor" << endl;
   }
   fout << endl;

   fout << "Meta-Data Metrics" << endl; 

   for( index = 0; index < metaDataStream.size( ); index++ ) 
   {
      if( metaDataStream[index].code == 'P' )
      {
         for( indexTwo = 0; indexTwo < 8; indexTwo++ )
         {
            if( fileData.cycleData[indexTwo].componentName.compare("Processor") == 0 )
            {
               fout << metaDataStream[index].code << "(";
               fout << metaDataStream[index].description << ")" << metaDataStream[index].cycles; 
               fout << " - "; 
               fout << metaDataStream[index].cycles * fileData.cycleData[indexTwo].time << " ms" << endl;
            }
         }
      }
      else if( metaDataStream[index].code == 'M' )
      {
         for( indexTwo = 0; indexTwo < 8; indexTwo++ )
         {
            if( fileData.cycleData[indexTwo].componentName.compare("Memory") == 0 )
            {
               fout << metaDataStream[index].code << "(";
               fout << metaDataStream[index].description << ")" << metaDataStream[index].cycles; 
               fout << " - "; 
               fout << metaDataStream[index].cycles * fileData.cycleData[indexTwo].time << " ms" << endl;
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
               fout << metaDataStream[index].code << "(";
               fout << metaDataStream[index].description << ")" << metaDataStream[index].cycles; 
               fout << " - "; 
               fout << metaDataStream[index].cycles * fileData.cycleData[indexTwo].time << " ms" << endl;
            }
         }
      }
   }

   fout.close( ); 
}
