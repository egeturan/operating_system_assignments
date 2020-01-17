// C program to illustrate process
#include<stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <fcntl.h> 
#include <sys/time.h>
#include <time.h>

// A function to implement bubble sort  
void bubbleSort(int arr[], int n)  
{  
    int i, j;  
    for (i = 0; i < n-1; i++)  {    
      
    // Last i elements are already in place  
    for (j = 0; j < n-i-1; j++)  {
        if (arr[j] > arr[j+1]) {
            int temp = arr[j];
            arr[j] = arr[j + 1];
            arr[j + 1] = temp;
        }
    }
    }
}  


void sendKth(int * values, char * newString, int valueCount, int kthLarge){
    char temp[10];   
    
    for (int i = valueCount - 1; i >= valueCount - kthLarge; i--)
    {
       sprintf(temp, "%d", values[i]);
        strcat (newString, temp);
        strcat (newString, "\n");
    }

}


int main(int argc,char* argv[]) 
{ 
    int childPid;
    int fd , size, kthlargest, processCountN;
    char *buffer = (char*) malloc(100000000 * sizeof(char));
    int *values = (int*) malloc(100000000 * sizeof(char));
    int childProcesses[5];
    int currentPid;
    int currentChildCount = 0; 
    char *inputs[2];
    int inputCount = 0;
    //Timer
    clock_t t;

    
    //printf("Program Name Is: %s\n",argv[0]); 

    if(argc == 1){ 
        printf("\nKth large, number of child processes, input files and output file is not specifed\n");
    } 
    else if(argc == 2){
        printf("\nNumber of child processes, input files and output file is not specifed ( N ) is \n"); 
    }
    else if(argc == 3){
        printf("\nInput files and output file is not specifed\n"); 
    }
    else if(argc == 4){
        printf("\nOutput file is not specifed\n"); 
    }
    else if(argc > 9){
        printf("N cannot be bigger than 5 thus there cannot be more than 5 input file");
        return 0;
    }
    else if(argc >= 5) //If validation pass first stages
    { 
        

        //Validation Control Starts for second stage
        printf("k: %s \nN: %s \n", argv[1], argv[2]);
        
            printf("Input files are: ");
            for (size_t i = 3; i < argc - 1; i++)
            {
                printf("%s ", argv[i]);
                inputCount++;
            }
            printf("\n");

        printf("Output file : %s\n", argv[argc - 1]);

        kthlargest = atoi(argv[1]);

        processCountN = atoi(argv[2]);

        if(processCountN != inputCount){
            printf("Process count and input file counts need to be equal****\nPlease try again\n");
            return 0;
        }

        if(processCountN > 5){
            printf("N cannot be bigger than 5");
            return 0;
        }

        if(kthlargest < 1 || kthlargest > 1000){
            printf("K need to be between 1 and 1000");
            return 0;
        }

        //Last validations are completed
        //Computations are starting

        printf("Given child process count is %d \n", processCountN);
   
    //create processes

    //start timer
    t = clock(); 
    
    int j = 0;
    
    do{
        inputs[0] = argv[3 + currentChildCount];
        childPid = fork();
        if(childPid == 0){
            break;
        }
        printf("Child %d created\n", childPid);
        childProcesses[currentChildCount] = childPid;
        currentChildCount++;
        j++;
    }while(j < processCountN && childPid != 0);
  

    if(childPid == 0){
         
    }else {
        for (int i = 0; i < currentChildCount; i++)
        {       
            wait(NULL);
        }
  
    }

    if(childPid == 0){ // Child processes works after this process IMPORTANT

    //printf("-------------------------inputs[0] is : %s----------------------\n", inputs[0]);
    //opening file depending on the inputs

    fd = open(inputs[0], O_RDONLY); 

    if(fd < 0){
        printf("Error while readin the file, Error is: %d\n", fd);
    }

    size = read(fd, buffer, 100000000); 
 

     //I can print the integers in the buffer
    /* for(int i = 0; i < size; ++i){
         if(buffer[i] == '\n'){
            printf("*\n");
         }else{
            printf("%c", buffer[i]);
         }
     }*/
    

    int valueCount = 0;
    int charOrder = 0;
    char *charCarrier = (char*) malloc(sizeof(char) * 100000000);
 /*   strcpy(charCarrier, "i"); //possibly I solve the concatenation
    printf("\n%s\n", charCarrier);
*/
    //Create integer array with these data
    for(int i = 0; i < size; ++i){
        if(buffer[i] == '\n'|| buffer[i] == '\t' || buffer[i] == ' '){
           //printf("%d\n", atoi(charCarrier));
           values[valueCount] = atoi(charCarrier);
           memset(charCarrier, ' ', 10 * sizeof(char)); //make it empty the filled areas of the array
           valueCount++;
           charOrder = 0;
        }else {
            charCarrier[charOrder] = buffer[i];
            charOrder++;
        }
    }

    free(charCarrier);

    bubbleSort(values, valueCount); //data is sorted

    int sizeWrite; 
    int intermediateCount = 1;

    char temp1[6];
    currentPid = getpid();
    sprintf(temp1, "%d", currentPid);

    int fileProne = open(temp1, O_WRONLY | O_CREAT | O_TRUNC, 0666); 
    if (fileProne < 0) 
    { 
        perror("r1"); 
        exit(1); 
    } 
    
    char *newString = (char*) malloc(sizeof(char) * 1000000);

    sendKth(values, newString, valueCount, kthlargest);

    sizeWrite = write(fileProne, newString, strlen(newString));

    free(newString);
    
    close(fileProne);

    free(buffer);

    printf("Child is terminated\n");

    return 0;

    }  
    }

    
    //printf("\nCreated child count of the parent %d", currentChildCount);
    int valueCount;
    int charOrder, sizeWrite;
    char *charCarrier;
    char temp1[6];
   

    //Read files from the parent
    char *outputValues[5]; //to have output values

    char *writeText = (char*) malloc(sizeof(char) * 100000000); //string that will be written into the output file


    //open output file
    int fileProne = open(argv[argc - 1], O_WRONLY | O_CREAT | O_TRUNC, 0666); 
    if (fileProne < 0) 
    { 
        perror("r1"); 
        exit(1); 
    }

    int valsFinal[6000];

    int indexForWriting = 0;

    for(int i = 0; i < currentChildCount; ++i){ //work for intermediate files IMPORTANT

    buffer = (char*) malloc(100000000 * sizeof(char)); // reallocate the buffer
    sprintf(temp1, "%d", childProcesses[i]);

    fd = open(temp1, O_RDONLY); 
    //printf("Fd is: %d \n", fd); //control for file read
    

    size = read(fd, buffer, 10000000); //file is read

    //printf("Fd is: %d and size: %d\n", fd , size);

    //Create integer array with these data

    valueCount = 0;
    charOrder = 0;
    charCarrier = (char*) malloc(10000000 * sizeof(char));

    //Create integer array with these data
    for(int i = 0; i < strlen(buffer); ++i){
        if(buffer[i] == '\n'){
           //printf("%d\n", atoi(charCarrier));
           values[valueCount] = atoi(charCarrier);
           memset(charCarrier, ' ', 10 * sizeof(char)); //make it empty the filled areas of the array
           valueCount++;
           charOrder = 0;
        }else {
            charCarrier[charOrder] = buffer[i];
            charOrder++;
        }
    }

    free(charCarrier);
    
    int writeTo = indexForWriting + kthlargest;
    int index1 = 0;

    for (int i = indexForWriting; i < writeTo; i++)
    {
        valsFinal[i] = values[index1];
        index1++;
    }
    

    int deleteStatus;

    //deletefile
    deleteStatus = remove(temp1);

    if (deleteStatus == 0)
        printf("%s File is deleted successfully.\n", temp1);
    else
    {
        printf("File deletion error\n");
    }

    /*
    for (int i = 0; i < valueCount; i++)
    {   
        printf("%d\n", values[i]);
    }  
    */

    indexForWriting = indexForWriting + kthlargest;


    }// reading completed

    int totalValueCount =  kthlargest * inputCount;

    bubbleSort(valsFinal, totalValueCount);
    char temp4[20];

    printf("%s", writeText);
    
    for (int j = totalValueCount - 1; j >= totalValueCount - kthlargest; j--)
    {
        sprintf(temp4, "%d", valsFinal[j]);
        strcat(writeText , temp4);
        strcat(writeText , "\n");
    }
    

    printf("Values are written.\n");
    sizeWrite = write(fileProne, writeText, strlen(writeText));
    close(fileProne);

    t = clock() - t; 
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds 
  
    printf("Execution took %f seconds to execute \n", time_taken); 
    
    free(buffer);
    free(writeText);

    printf("\a");

    return 0; 
} 