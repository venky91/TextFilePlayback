#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <unistd.h>
#include <ncurses.h>
#include <ctype.h>

bool quit = false; // outer condition for each thread. when quit = true, the threads finish executing
bool stopPlaying = false; //condition for the player to stop playing the text file
bool fileIsLoaded = false; // condition to check if the file has been loaded into the player
bool fileIsPlaying = false; // condition to check if the player is playing the file
const char* newFileName;
pthread_mutex_t a_mutex = PTHREAD_MUTEX_INITIALIZER;

void* update_player( void* data );
void* update_cntrl1( void* data );
void* update_cntrl2( void* data );
void* update_cntrl3( void* data );

void play_back(); // plays the text file
void load_file( const char* filename ); //loads a text file
void stop(); // stops the playing of the text file
void start(); // starts the playing of the text file
void rewind(); // sets the index of the text file string array to the first position
void seek_to( int byteOffset ); // goes to a certain index of the text file string array and plays from that position
void set_rate( float rate ); // sets the playback rate for the playing of the text file
void quit_program(); // cleanup function, sets stopPlaying, fileIsLoaded, and fileisPlaying to false, and quit to true

int offset; // variable that holds the position the user wants to go to in the text file, and plays from that position
int begin, end, lengthOfFile; // begin holds the index to play the text file forward, end holds the last index to play the text file backwords, lengthOfFile holds the length of the text file string array
float newPlayRate = 1; // initial playback rate
float playRate = abs ((1 / newPlayRate) * 1000000); // holds the playback rate
std::string fileArray[800];


int main( int argc, char* argv[] ) {
    
    int playerThread, controllerThread1, controllerThread2, controllerThread3; 
    pthread_t player, cntrl1, cntrl2, cntrl3; //pthread structures

    // thread id's
    int playerId = 1;
    int control1id = 2;
    int control2id = 3;
    int control3id = 4;

    // thread that plays the text file
    playerThread = pthread_create( &player, NULL, update_player, (void*)&playerId);
   
    // thread that takes in commands from the user; can load, play, stop, start, rewind, seek, set rate, or quit
    controllerThread1 = pthread_create( &cntrl1, NULL, update_cntrl1, (void*)&control1id );
    
    // thread randomly starts playing a loaded text file every 5-35 seconds
    controllerThread2 = pthread_create( &cntrl2, NULL, update_cntrl2, (void*)&control2id );
    
    // thread randomly stops playing a loaded text file every 5-35 seconds
    controllerThread3 = pthread_create( &cntrl3, NULL, update_cntrl3, (void*)&control3id );

    pthread_join( player, NULL );
    pthread_join( cntrl1, NULL );
    pthread_join( cntrl2, NULL );
    pthread_join( cntrl3, NULL );

    return 0;
} 

void* update_player( void* data ) {
   
    while ( quit == false ) {
        
        while ( fileIsPlaying == true && fileIsLoaded == true ) { // if the file is loaded and ready to play
            
            play_back(); // play the file

        }

    }

}

void* update_cntrl1( void* data ) {

    std::string i;
    int time;
    float rate;
    while ( quit == false ) {
       
        std::getline( std::cin, i ); // get a user command

        if ( i == " " ) {
   
            if ( fileIsPlaying == true ) { // if the file is already playing
                
                stop(); // stop the player

            }        
            
            else {

                start(); //else, start the player

            }
        
        }

        else if ( i == "r" ) {

            rewind(); // set the index of the the string array to the beginning

        } 

        else if ( i == "l" ) {

            std::string file;
            const char* fileName;
            bool q = false;
            while ( q == false ) { // while loop isn't necessary, disregard

                std::cout << "Enter the name of a file: ";
                std::getline( std::cin, file ); //get a file name
                fileName = file.c_str(); // convert to const char* 
                int mc = pthread_mutex_lock( &a_mutex );
                load_file( fileName ); // load the file
                mc = pthread_mutex_unlock( &a_mutex );
                q = true;
            }

        }

        else if ( i == "s" ) {
                
            std::cout << "Enter a time you wish to go to: ";
            std::cin >> time; // get time
            seek_to( time ); // go to the specified position in the string array

        } 

        else if ( i == "t" ) {

            std::cout << "Enter a playback rate: ";
            std::cin >> rate; // get a playback rate
            set_rate( rate ); // set the playback rate

        }

        else if ( i == "q" ) {

            quit_program(); // kill all threads
            
        }

    }
}

void* update_cntrl2( void* data ) {

    srand( time( NULL ) );
    while ( quit == false ) {

        usleep( (rand() % 30 + 5  ) * 1000000); // randomly get a waiting time
        start(); // start the player

    }
}

void* update_cntrl3( void* data ) {

    srand( time( NULL ) );
    while ( quit == false ) {
    
        usleep( (rand() % 30 + 5  ) * 1000000); // randomly get a waiting time
        stop(); // stop the player

    }
}

void load_file ( const char* filename ) {

    int i;
    std::string line;
    std::ifstream myfile( filename );

    if ( myfile.is_open() ) { // if the file is open

        while ( myfile.good() ) { // if the file is good

            myfile >> fileArray[i++]; // get next word in the text file and store it in the string array
            lengthOfFile = i; // store the length of the file
            begin = 0; // beginning is index 0
            end = i - 2; // end is index legnthOfFile - 2
            
            if ( myfile.eof() ) { // if the end of file has been reached
                    
                break; // exit the while loop

            }
        }

        myfile.close(); // close the file
        fileIsLoaded = true; // the file has been loaded
    }

    else
        std::cout << "Unable to open file" << std::endl; // the file was not able to loaded


}

void play_back() {

    if ( newPlayRate > 0 ) { // if the user has entered a newPlayRate greater than 0, play the file forwards
        
        while ( stopPlaying == false ) { // while the file is playing

            
            do {
                
                if ( stopPlaying == true ) { // if at any point during the execution the file stops playing

                    break; // stop playing the file

                }   

                int rc = pthread_mutex_lock( &a_mutex );
                std::cout << fileArray[begin++] << std::endl; // output content of the index starting from index 0 and going forward
                rc = pthread_mutex_unlock( &a_mutex );
                
                
                
                usleep( playRate ); // wait the time of the playRate before printing out the content of the next index
                
            } while ( begin < lengthOfFile );
        
            int mc = pthread_mutex_lock( &a_mutex );
            stopPlaying = true; // if the player outputted everything, stop the player
            mc = pthread_mutex_unlock( &a_mutex );

        }

    }

    else { // the user has inputted a newPlaybackRate less than 0, player will output the file backwards

        while ( stopPlaying == false ) { // while the player is playing
        
            do {
                
                if ( stopPlaying == true ) { // if at any point during the execution stopPlaying is set to false
                
                    break; // stop playing

                }

                int rc = pthread_mutex_lock( &a_mutex );
                std::cout << fileArray[end--] << std::endl; // output content of the index starting at the last index and going backwards
                rc = pthread_mutex_unlock( &a_mutex );        

                usleep( playRate ); // wait he time of the playRate before printing out the content of the next index

            } while ( end >= 0 );
            
            int mc = pthread_mutex_lock( &a_mutex );
            stopPlaying = true; // if the player outputted everything, stop the player
            mc = pthread_mutex_unlock( &a_mutex );

        }
            
    }

}
void rewind() {

    int rc = pthread_mutex_unlock( &a_mutex );
    begin = 0; // if the playRate > 0, the string array goes to index 0
    end = lengthOfFile; // if the playRate < 0, the string array goes to the last index
    rc = pthread_mutex_unlock( &a_mutex );

}

void seek_to( int byteOffset ) {
    int rc = pthread_mutex_unlock( &a_mutex );
    // set each index to the position of byteOffset
    begin = byteOffset;
    end = byteOffset;
    rc = pthread_mutex_unlock( &a_mutex );
    
}

void set_rate( float rate ) {

    int rc = pthread_mutex_lock( &a_mutex );
    if ( newPlayRate < 0 ) { 

        begin = end + 2; // get the position where the player should start from in case you want to go forwards instead of backwards
        newPlayRate = rate;
        playRate = abs(( 1/ newPlayRate ) * 1000000);

    }
    else {

        end = begin -2; // get the position where the player should start from in case you want to go backwards instead of forwards
        newPlayRate = rate;
        playRate = abs(( 1/ newPlayRate ) * 1000000);
    }
    rc = pthread_mutex_unlock( &a_mutex );

}

void stop() {
    
    int rc = pthread_mutex_lock( &a_mutex );
    stopPlaying = true;
    fileIsPlaying = false;
    rc = pthread_mutex_unlock( &a_mutex );

}

void start() {

    int rc = pthread_mutex_lock( &a_mutex );
    if ( begin ==  lengthOfFile ) {  

        begin = 0; // rewinds the file
        
    }

    if ( end == 0 ) { // way to automatic 

        end = lengthOfFile - 2; // rewinds the file
    }

    stopPlaying = false;
    fileIsPlaying = true;
    rc = pthread_mutex_unlock( &a_mutex );

}

void quit_program() {

    int rc = pthread_mutex_lock( &a_mutex );
    stopPlaying = true;
    quit = true;
    fileIsPlaying = false;
    fileIsLoaded = false;
    rc = pthread_mutex_unlock( &a_mutex );
}
