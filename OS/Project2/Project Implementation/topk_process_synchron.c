// C program to illustrate process
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h> 
#include <fcntl.h> 
#include <sys/shm.h> 
#include <sys/stat.h> 
#include <sys/mman.h>
#include <semaphore.h>

int main(int argc, char *argv[])
{
    int childPid;
    int fd, size, kthlargest, processCountN;

    //Dynamic allocations
    int childProcesses[20];
    int currentPid;
    int currentChildCount = 0;
    char *inputs[20];
    int inputCount = 0;
    int currentProcessId;
    clock_t t; 
    //Variables for shared_memory
    /* name of the shared memory object */
    const char* name = "shared_memE";
     /* the size (in bytes) of shared memory object */
    const int SIZE = 1000000; 
    int shm_fd;

    sem_t mutex;

    


    //printf("Program Name Is: %s\n",argv[0]);

    if (argc == 1)
    {
        printf("\nKth large, number of child processes, input files and output file is not specifed\n");
    }
    else if (argc == 2)
    {
        printf("\nNumber of child processes, input files and output file is not specifed ( N ) is \n");
    }
    else if (argc == 3)
    {
        printf("\nInput files and output file is not specifed\n");
    }
    else if (argc == 4)
    {
        printf("\nOutput file is not specifed\n");
    }
    else if (argc > 13)
    {
        printf("N cannot be bigger than 5 thus there cannot be more than 5 input file");
        return 0;
    }
    else if (argc >= 5) //If validation pass first stages
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

        if (processCountN != inputCount)
        {
            printf("Process count and input file counts need to be equal\nPlease try again\n");
            return 0;
        }

        if (processCountN > 10)
        {
            printf("N cannot be bigger than 5");
            return 0;
        }

        if (kthlargest < 100 || kthlargest > 10000)
        {
            printf("K need to be between 100 and 10000");
            return 0;
        }

        //Last validations are completed
        //Computations are starting

        //create processes

        int j = 0;
        t = clock(); 

        do
        {
            sem_init(&mutex, 0, 1);
            inputs[0] = argv[3 + currentChildCount];
            currentProcessId = j;
            childPid = fork();
            if (childPid == 0)
            {
                break;
            }
            childProcesses[currentChildCount] = childPid;
            currentChildCount++;
            j++;
        } while (j < processCountN && childPid != 0);

         if (childPid == 0)
        {
        }
        else
        {
            for (size_t i = 0; i < currentChildCount; i++)
            {
                wait(NULL);
            }
        }


         if (childPid == 0)
            { // Child processes works after this process IMPORTANT
            printf("Child is started\n");
            printf("--------Process is: %d --------------- inputs[0] is : %s----------------------\n", currentProcessId ,inputs[0]);
            //opening file depending on the inputs

            FILE * filePtr = fopen(inputs[0], "r"); 

            if (filePtr == NULL) 
            { 
                printf("File read is unsuccessfull\n"); 
                return 0; 
            } 

            int *values = (int *) malloc(100000000 * sizeof(int));
            int arrSize = 0;
            int currentValue;

            while( fscanf(filePtr, "%d", &currentValue) == 1){
                values[arrSize] = currentValue;
                arrSize++;
            }   

            /*printf("VALUES ARE\n Array size is: %d\n", arrSize);
            for (int a = 0; a < arrSize; a++)
            {
               printf("%d  ", values[a]);
            }

            printf("\n");
            */

            //bubbleSort(values, arrSize);

            int *startingPoint;

            /* pointer to shared memory obect */
            int * shared_ptr, * endOfSharedMemoryP; 
            int sharedMemoryArraySize;
            
            //Critical Section
            shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666); /* create the shared memory object */
        
            ftruncate(shm_fd, SIZE); /* configure the size of the shared memory object */

            /* memory map the shared memory object */
            shared_ptr = (int *) mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0); 

            //pointer acquisition
            startingPoint = shared_ptr;
            shared_ptr += (kthlargest * 2);
            endOfSharedMemoryP = shared_ptr;

            int i = 0; //our array index
            
            printf("We filled this amount of the shared memory (process id: %d) so far %d\n", currentProcessId ,endOfSharedMemoryP[0]);

            sem_wait(&mutex);
          
            while(endOfSharedMemoryP[0] < kthlargest){//we didn't fill shared memory yet
                
                printf("++ %d ", endOfSharedMemoryP[0]);
                printf("current process id is: %d\n", currentProcessId);
               
                //now sorth the section
                int boladd = 0;
                for (int k = 0; k <= endOfSharedMemoryP[0]; k++)
                {
                    if(values[i] > startingPoint[k]){
                        for (int e = endOfSharedMemoryP[0]; e > k; e--)
                        {
                            startingPoint[e] = startingPoint[e - 1];
                        }
                        
                        startingPoint[k] = values[i];
                        boladd = 1;
                        break;
                    }
                }
                if(boladd == 0){
                    startingPoint[endOfSharedMemoryP[0]] = values[i];
                }

                i++;
                int val = endOfSharedMemoryP[0];
                val++;
                endOfSharedMemoryP[0] = val;
                sem_post(&mutex); 
                //will this continue again???
                sem_wait(&mutex);
                
            }
            sem_post(&mutex); 
            sem_wait(&mutex);

            while (i < arrSize)
            {
                 for (int j = 0; j < endOfSharedMemoryP[0]; j++)
                    {
                        if(values[i] > startingPoint[j]){
                            for (int e = endOfSharedMemoryP[0]; e > j; e--)
                            {
                                startingPoint[e] = startingPoint[e - 1];
                            }
                            startingPoint[j] = values[i];
                            break;
                        } 
                    }
                i++;
                sem_post(&mutex); 

                sem_wait(&mutex);   
            }

            /*
            for (int i = 0; i < kthlargest; i++)
            {
                printf("Value: %d\n", startingPoint[i]);
            }*/


/*
            if(shared_ptr[0] == 0){


            for (int i = 0; i < kthlargest; i++)
            {
                shared_ptr[i] = values[i];
            }

            }else{
                 for (int i = 0; i < kthlargest; i++)
                {
                   for (int j = 0; j < kthlargest; j++)
                    {
                        if(values[i] > shared_ptr[j]){
                            for (int k = kthlargest - 2; k >= j; k--)
                            {
                                shared_ptr[j + 1] = shared_ptr[j]; 
                            }
                            shared_ptr[j] = values[i];
                             break;
                        } 
                    }
                }

            }

            sem_post(&mutex); 
            
*/

            exit(0);

        } //Child process finieshed

        //Parent active now
        //Open Shared Memory location

        printf("Parent Started...\n");

            /* open the shared memory object */
        shm_fd = shm_open(name, O_RDONLY, 0666); 

         int * shared_ptr; 
         int sharedArrSize = 0;
    
        /* memory map the shared memory object */
        shared_ptr = mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0); 

        //Parent has the kthvalues
        for (int i = 0; i < kthlargest; i++)
        {
            printf("Values: %d\n", shared_ptr[i]);
            sharedArrSize++;
        }
        
        printf("Shared array size: %d\n", sharedArrSize);


    char *output = malloc(100000000 * sizeof(char));
    char str[10];

    for (int i = 0; i < kthlargest; i++)
    {
        sprintf(str, "%d", shared_ptr[i]);
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
    
    t = clock() - t; 
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds 
  
    printf("Execution took %f seconds to execute \n", time_taken); 


    int sizeWrite = write(fileProne, output, strlen(output));
    close(fileProne);
    
    //printf("%s\nMain thread terminates\n", output);

    free(output);


    shm_unlink(name); //shared memory key needed to be deleted
    
            
    }

    return 0;
}