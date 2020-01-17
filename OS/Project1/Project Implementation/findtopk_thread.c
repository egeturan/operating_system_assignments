// C program 
#include<stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h> 
#include <pthread.h> 
#include <time.h>
#include <sys/time.h>

//Global variables
int kthLargest;
int kthValues[6000];
int threadCount;

typedef struct DataPass {
    int threadId;
    char *inputD;
} thdata;

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

void *threadFunction(void *param) 
{ 
    int size, fd;
    char *buffer = (char *)  malloc(100000000 * sizeof(char));
    //char *inputs = (char*) param;
    int *values = (int *) malloc(100000000 * sizeof(int)); 
    thdata *data1; 
    data1 = (thdata *) param;  /* type cast to a pointer to thdata */
    char *inputs = data1->inputD;
    int threadIdN = data1->threadId;


    //printf("input file is : %s----------------------\n", inputs);
    //opening file depending on the inputs

    fd = open(inputs, O_RDONLY); 

    if(fd < 0){
        printf("Error while readin the file, Error is: %d\n", fd);
    }

    size = read(fd, buffer, 100000000); 

    //printf("Fd is: %d and size: %d\n", fd , size);

    int valueCount = 0;
    int charOrder = 0;
    char *charCarrier = (char *) malloc(100000000 * sizeof(char));

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

    printf("Thread ıd is: %d", threadIdN);

    int index = threadIdN * kthLargest;
    int valIndex = valueCount - 1;
    printf("-------");

    for (int i = index; i < index + kthLargest; i++)
    {
        //printf("Index is: %d\n", i);
        //printf("%d\n", values[valIndex]);
        kthValues[i] = values[valIndex];
        valIndex--;
    }
    

    free(buffer);
    free(values);

    printf("Thread is completed\n");

    return 0;
}

int main(int argc,char* argv[]) 
{ 
    int inputCount = 0;
    char *buffer = (char *) malloc(100000000 * sizeof(char));
    pthread_t tidA[5]; /* id of the created thread */
    pthread_t tid;
	pthread_attr_t attr;  /* set of thread attributes */
    //struct DataPass *datapass;
    thdata data[5];
    //timer variables
    clock_t t; 
    
    
    //printf("Program Name Is: %s\n",argv[0]); 

    if(argc == 1){ 
        printf("\nKth large, number of threads, input files and output file is not specifed\n");
    } 
    else if(argc == 2){
        printf("\nNumber of threads, input files and output file is not specifed ( N ) is \n"); 
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

        kthLargest = atoi(argv[1]);

        threadCount = atoi(argv[2]);

        if(threadCount != inputCount){
            printf("Thread count and input file counts need to be equal****\nPlease try again\n");
            return 0;
        }

        if(threadCount > 5){
            printf("N cannot be bigger than 5");
            return 0;
        }

        if(kthLargest < 1 || kthLargest > 1000){
            printf("K need to be between 1 and 1000");
            return 0;
        }

        //Last validations are completed
        //Computations are starting

        //printf("\nGiven thread count is %d \n", threadCount);
    

    //Create Threads

    int j = 0;
    char *temp; 

    //timer starts
    clock_t t;     

    do{
        printf("Thread %d is created\n", j + 1);
        temp = argv[3 + j];
        data[j].inputD = temp;
        data[j].threadId = j;
      /*  (*datapass).threadId = 1;
        (*datapass).inputD = temp;*/
       // printf("Data is: %d\n", data[j].threadId );
       // printf("InputD is: %s\n", data[j].inputD );
        //thread created
        pthread_attr_init (&attr); 	
	    pthread_create (&tid, &attr, threadFunction, (void *) &data[j]); 
        tidA[j] = tid; 
        j++;
    }while(j < threadCount);
    

    for (int i = 0; i < threadCount; i++)
    {
        pthread_join (tidA[i], NULL); //wait thread to finish its job.
    }

    int totalSizeOfArr = threadCount * kthLargest;

    char *output = (char *) malloc(1000000 * sizeof(char));
    char str[10];


    bubbleSort(kthValues, totalSizeOfArr);
    
    for (int i = totalSizeOfArr - 1; i >= totalSizeOfArr - kthLargest; i--)
    {       
             sprintf(str, "%d", kthValues[i]);
             if(strlen(output) == 0){
                strcpy(output, str);
                strcat(output, "\n");  
             }else{
                 strcat(output, str);
                strcat(output, "\n");  
             }
         
    }

    //open output file
    int fileProne = open(argv[argc - 1], O_WRONLY | O_CREAT | O_TRUNC, 0666); 
    if (fileProne < 0) 
    { 
        perror("r1"); 
        exit(1); 
    }


    int sizeWrite = write(fileProne, output, strlen(output));
    close(fileProne);

    printf("Writing is completed\n");

    t = clock() - t; 

    double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds 
  
    printf("Execution took %f seconds to execute \n", time_taken); 

    printf("\a");

    free(output);

    return 0;
    }
}