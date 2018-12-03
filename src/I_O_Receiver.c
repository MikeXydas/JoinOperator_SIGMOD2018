#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "I_O_structs.h"
#include "queryStructs.h"
#include "pipeI_O.h"
#include "queryManip.h"
#include "checksumInterface.h"
#include "jointPerformer.h"
#include "interListInterface.h"

int main(void) {

    
    FILE * stdoutFile = fopen("stdoutFile", "w");  //checking the outpout

    if(stdoutFile == NULL) {
        perror("Failed to open file\n");
        return -1;
    }
    fprintf(stdoutFile, "start\n");
    fflush(stdoutFile);
    

    Input * input = InitializeInput();

    char * inputStr = readFromPipe("Done");     //function for reading from pipe until "Done" is received

    fprintf(stdoutFile, "%s\n", inputStr);
    fflush(stdoutFile);

    ConstructInput(input, inputStr);            //Construct linked list of input filenames
    
    free(inputStr);


    fprintf(stdoutFile, "Building relArray...\n");
    fflush(stdoutFile);
    
    
    relationsheepArray relArray =  InitializeRelSheepArray(input->numNodes);
    FillRelArray(&relArray, input);
    FreeInput(input);
    
    fprintf(stdoutFile, "Writing Ready...\n");
    fflush(stdoutFile);
    write(STDOUT_FILENO, "Ready\n\0", strlen("Ready\n\0")); //Send signal to harness in order to start sending queries 
    
    fprintf(stdoutFile, "Ready\n");
    fflush(stdoutFile);

    
    while(1) {


        inputStr = readFromPipe("F");
        
        if( strlen(inputStr) == 0) {        //no more query batches
            break;
        }

        int end = 0, start = 0;
        myint_t i, rels = 1, joins = 0, sums = 1, filters = 0;
        myint_t slice = 0;
        for(i = 0; i < strlen(inputStr); i++) {

                if (inputStr[i] == ' ') {
                
                    if ( slice == 0)
                        rels++;
                    else if ( slice == 2)
                        sums++;
                    else {
                        perror("Wrong Input Format0");
                        return -1;
                    }
                
                }else if (inputStr[i] == '|') {
                    slice++;               
                }

                if (inputStr[i] == '=') {

                    if ( slice == 0) {
                        perror("Wrong Input Format1");
                        return -1;
                    }
                    else if ( slice == 1){
                        if(inputStr[(i + 2) ] == '.' || inputStr[(i + 3)] == '.')
                            joins++;
                        else
                            filters++;
                    }
                    else {
                        perror("Wrong Input Format2");
                        return -1;
                    }

                }

                if (inputStr[i] == '>' || inputStr[i] == '<') {

                    if ( slice == 0) {
                        perror("Wrong Input Format3");
                        return -1;
                    }
                    else if ( slice == 1)
                        filters++;
                    else {
                        perror("Wrong Input Format4");
                        return -1;
                    }

                }
                if( inputStr[i] == '\n') {
                    start = end;
                    end = i + 1;

                    char * queryStr = (char *) malloc((i - start + 1) * sizeof(char));
                    memset(queryStr, '\0', (i - start + 1) );
                    strncpy(queryStr, inputStr + start, i - start);
                    strcat(queryStr, "\0");

                    
                    query * newQuery = ConstructQuery(stdoutFile, queryStr, rels, joins, sums, filters, relArray);

                    headInter * headInt = initialiseHead();
                    //Perform filters
                    for(myint_t whichFilter = 0; whichFilter < newQuery->numOfFilters; whichFilter++) {
                        workerF(&(newQuery->filters[whichFilter]), headInt);
                    }

                    //Perform joins
                    for(myint_t whichJoin = 0; whichJoin < newQuery->numOfJoins; whichJoin++) {

                        //fprintf(stderr, "Inters: %ld\n", headInt->numOfIntermediates);

                        //fprintf(stderr, "Inters1: %ld\n", headInt->numOfIntermediates);

                        workerJ(&(newQuery->joins[whichJoin]), headInt);
                        //fprintf(stderr, "Inters2: %ld\n", headInt->numOfIntermediates);

                    }

                    //Perform checksums
                    checksum * cs = performChecksums(newQuery->sums, newQuery->numOfSums, headInt);
                    // for(int whichCs = 0; whichCs < cs->numbOfChecksums; whichCs++) {
                    //    fprintf(stderr, "CS = %ld\n", cs->checksums[whichCs]);

                    // }


                    writePipe(cs);

                    //Perform checksums

                    //fprintf(stderr, "FINISHED ONE: Inters: %ld | Rows = %ld\n", headInt->numOfIntermediates, headInt->start->data->numbOfRows);

                    fprintf(stderr, "FINISHED ONE: Inters: %ld | Rows = %ld\n", headInt->numOfIntermediates, headInt->start->data->numbOfRows);

                    //fprintf(stderr, "%ld | %ld\n", headInt->start->data->rowIds[0][0], headInt->start->data->rowIds[1][0]);

                    //perror("aaaaaaaaaa\n");
                    //Perform joins


                    //cheksum * cSum = ValFunction(query, ...);

                    char * debug = malloc(20);
                    sprintf(debug, "%ld", newQuery->numOfJoins );
                    fprintf(stdoutFile, "%s\n---------\n", debug );
                    fflush(stdoutFile);
                    free(debug);

                    debug = malloc(20);
                    sprintf(debug, "%ld", newQuery->numOfFilters );
                    fprintf(stdoutFile, "%s\n---------\n", debug );
                    fflush(stdoutFile);
                    free(debug);

                    debug = malloc(20);
                    sprintf(debug, "%ld", newQuery->numOfSums );
                    fprintf(stdoutFile, "%s\n---------\n", debug );
                    fflush(stdoutFile);
                    free(debug);

                    fprintf(stdoutFile, "@@@@@@@@@\n");
                    fflush(stdoutFile);
                    
                    
                    for(int g = 0; g < newQuery->numOfRels; g++ ) {
                        char * debug = malloc(20);
                        sprintf(debug, "%ld", newQuery->rels[g]);
                        fprintf(stdoutFile, "%s\n", debug );
                        fflush(stdoutFile);
                        free(debug);
                    }


                    fprintf(stdoutFile, "joins\n" );
                    fflush(stdoutFile);

                    for(int g = 0; g < newQuery->numOfJoins; g++ ) {
                        char * debug = malloc(20);
                        sprintf(debug, "%ld", newQuery->joins[g].participant1.rel);
                        fprintf(stdoutFile, "%s\n", debug );
                        fflush(stdoutFile);
                        free(debug);
                        debug = malloc(20);
                        sprintf(debug, "%ld", newQuery->joins[g].participant1.rows);
                        fprintf(stdoutFile, "%s\n", debug );
                        fflush(stdoutFile);
                        free(debug);
                        debug = malloc(20);
                        sprintf(debug, "%ld", newQuery->joins[g].participant2.rel);
                        fprintf(stdoutFile, "%s\n", debug );
                        fflush(stdoutFile);
                        free(debug);
                        debug = malloc(20);
                        sprintf(debug, "%ld", newQuery->joins[g].participant2.rows);
                        fprintf(stdoutFile, "%s\n", debug );
                        fflush(stdoutFile);
                        free(debug);
                    }

                    fprintf(stdoutFile, "filters\n" );
                    fflush(stdoutFile);

                    for(int g = 0; g < newQuery->numOfFilters; g++ ) {
                        char * debug = malloc(20);
                        sprintf(debug, "%ld", newQuery->filters[g].participant.rel);
                        fprintf(stdoutFile, "%s\n", debug );
                        fflush(stdoutFile);
                        free(debug);
                        debug = malloc(20);
                        sprintf(debug, "%ld", newQuery->filters[g].participant.rows);
                        fprintf(stdoutFile, "%s\n", debug );
                        fflush(stdoutFile);
                        free(debug);
                        debug = malloc(20);
                        sprintf(debug, "%d", newQuery->filters[g].op);
                        fprintf(stdoutFile, "%s\n", debug );
                        fflush(stdoutFile);
                        free(debug);
                        debug = malloc(20);
                        sprintf(debug, "%ld", newQuery->filters[g].value);
                        fprintf(stdoutFile, "%s\n", debug );
                        fflush(stdoutFile);
                        free(debug);
                    }

                    fprintf(stdoutFile, "sums\n" );
                    fflush(stdoutFile);

                    for(int g = 0; g < newQuery->numOfSums; g++ ) {
                        char * debug = malloc(20);
                        sprintf(debug, "%ld", newQuery->sums[g].rel);
                        fprintf(stdoutFile, "%s\n", debug );
                        fflush(stdoutFile);
                        free(debug);
                        debug = malloc(20);
                        sprintf(debug, "%ld", newQuery->sums[g].rows);
                        fprintf(stdoutFile, "%s\n", debug );
                        fflush(stdoutFile);
                        free(debug);
                        
                    }


                    fprintf(stdoutFile, "-------------\n" );
                    fflush(stdoutFile);

                    FreeQuery(newQuery);
                    // fprintf(stdoutFile, "QueryAFTER:%s\n", queryStr);                   
                    // fflush(stdoutFile);
                    
                    free(queryStr);
                    
                    rels = 1;
                    joins = 0;
                    sums = 1;
                    filters = 0;
                    slice = 0;
                }
        }

    free(inputStr);
        
    }
    

    if( fclose(stdoutFile) ) {
        perror("Failed to close file");
        return -1;
    }
    while(1) {

    }
	
	return 0;

}