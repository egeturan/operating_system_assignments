// C program to illustrate process
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <mqueue.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h> 

#include "queuePosix.h"

// A function to implement bubble sort
void bubbleSort(int arr[], int n)
{
    int i, j;
    for (i = 0; i < n - 1; i++)
    {

        // Last i elements are already in place
        for (j = 0; j < n - i - 1; j++)
        {
            if (arr[j] > arr[j + 1])
            {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

void kthreturner(int * data, char *arr, int sizeChar, int kthLarge, int counter)
{
    int kth = 1;
    int valueIndex = counter * kthLarge;
    char charcarry[200];
    int charIndex = 0;

    for (int i = 0; i < strlen(arr); i++)
    {
      if (arr[i] == '\n')
        {
            data[valueIndex] = atoi(charcarry);
            charIndex = 0;
            valueIndex++;
            memset(charcarry, 0, sizeof charcarry);
        }
        else{
            charcarry[charIndex] = arr[i];
            charIndex++;
        } 
    }
    
}

int main(int argc, char *argv[])
{
    int childPid;
    int fd, size, kthlargest, processCountN;

    //Dynamic allocations
    char *buffer = (char *) malloc(100000000 * sizeof(char));
    int *values = (int *) malloc(100000000 * sizeof(int));
    int childProcesses[5];
    int currentPid;
    int currentChildCount = 0;
    char *inputs[2];
    int inputCount = 0;
    char *currentMQname;
    char *mqNameStr[5];
     clock_t t; 

    struct mq_attr attr;

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
    else if (argc > 9)
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

        if (processCountN > 5)
        {
            printf("N cannot be bigger than 5");
            return 0;
        }

        if (kthlargest < 1 && kthlargest > 1000)
        {
            printf("K need to be between 1 and 1000");
            return 0;
        }

        // printf("   ** * ** *    %s", mqNameStr[2]);
        mqNameStr[0] = "/justaname";
        mqNameStr[1] = "/justaname2";
        mqNameStr[2] = "/justaname3";
        mqNameStr[3] = "/justaname4";
        mqNameStr[4] = "/justaname5";

        //Last validations are completed
        //Computations are starting

        //create processes

        int j = 0;
        t = clock(); 

        do
        {
            currentMQname = mqNameStr[j];
            inputs[0] = argv[3 + currentChildCount];
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
            //printf("-------------------------inputs[0] is : %s----------------------\n", inputs[0]);
            //opening file depending on the inputs

            fd = open(inputs[0], O_RDONLY);

            if (fd < 0)
            {
                printf("Error while readin the file, Error is: %d\n", fd);
            }

            size = read(fd, buffer, 100000000);

            //printf("Fd is: %d and size: %d\n", fd , size);

            int valueCount = 0;
            int charOrder = 0;
            char *charCarrier = (char *) malloc(2000 * sizeof(char));
            //Create integer array with these data
            for (int i = 0; i < size; ++i)
            {
                if (buffer[i] == '\n' || buffer[i] == '\t' || buffer[i] == ' ')
                {
                    //printf("%d\n", atoi(charCarrier));
                    values[valueCount] = atoi(charCarrier);
                    memset(charCarrier, ' ', 10 * sizeof(char)); //make it empty the filled areas of the array
                    valueCount++;
                    charOrder = 0;
                }
                else
                {
                    charCarrier[charOrder] = buffer[i];
                    charOrder++;
                }
            }

            free(charCarrier);

            bubbleSort(values, valueCount); //data is sorted

            //Write to the intermediate file
            int sizeWrite;

            char temp1[6];

            char *newString = (char *)malloc(10000);
            int totaSize =  kthlargest;

            for (int i = valueCount - 1; i >= valueCount - kthlargest; i--)
            {
                sprintf(temp1, "%d", values[i]);
                strcat(newString, temp1);
                strcat(newString, "\n");
            }
            
       


            //printf( "****%ld\n", strlen(newString));


     /*       for (int i = valueCount - 1; i >= 0; --i)
            {
                char temp[6];
                sprintf(temp, "%d", values[i]);
                strcat(newString, temp);
                strcat(newString, "\n");
            }*/

            //now only send not write into file
            mqd_t mq;
            struct item item;
            int n;
            int validation = 0;
            printf("Child creates memory queue\n");
            mq = mq_open(currentMQname, O_RDWR | O_CREAT, 0664, 0);
            if (mq == -1)
            {
                perror("mq_open failed\n");
                exit(1);
            }
            int i = 0;
            while (validation < 1)
            {
                item.id = i;
                strcpy(item.astr, newString);
                n = mq_send(mq, (char *)&item, sizeof(struct item), 0);
                if (n == -1)
                {
                    perror("mq_send failed\n");
                    exit(1);
                }
                i++;
                sleep(1);
                validation++;
            }
            mq_close(mq);

            //free
            free(newString);
            free(buffer);
            free(values);

            printf("Child is terminated\n");
            return 0;
        }
    }

    printf("Parent is started\n");
    //printf("\nCreated child count of the parent %d", currentChildCount);
    int valueCount;
    int charOrder, sizeWrite;
    char *charCarrier;
    char temp1[6];

    mqd_t mq;
    struct mq_attr mq_attr;
    struct item *itemptr;
    int n, buflen;
    char *bufptr;
    int validation = processCountN;
    int counter = 0;
    char *please;
    int data[6000];

    while (counter < validation)
    {
        char astr[5000];

        mq = mq_open(mqNameStr[counter], O_RDWR, 0666, NULL);
        mq_getattr(mq, &mq_attr);
        //printf("mq maximum msgsize = %d\n", (int) mq_attr.mq_msgsize);
        /* allocate large enough space for the buffer */
        buflen = mq_attr.mq_msgsize;
        bufptr = (char *) malloc(buflen);

        //printf("Process value message %d is: ", counter);
        n = mq_receive(mq, (char *)bufptr, buflen, NULL);
        //error control
        if (n == -1)
        {
            perror("mq_receive failed\n");
            exit(1);
        }
        //printf("mq_receive success, message size=%d\n", n);
        itemptr = (struct item *)bufptr;
        //printf("item->id = %d\n", itemptr->id);
        //printf("item->astr = %s\n", itemptr->astr); //taken corrently
        
        kthreturner(data ,itemptr->astr, buflen, kthlargest, counter); //find kth largest from received message

        counter++;

        free(bufptr);
    }
    mq_close(mq);

    int totaSizeForOut = kthlargest * processCountN;

  /*  for (int i = 0; i < kthlargest * processCountN; i++)
    {
        printf("%d\n", data[i]);
    } */
    

    //Create integer array with these data
    printf("Messaging queue is read by Parent\n");
   
    char *outputStr = malloc(10000000 * sizeof(char));
    char temp[10];

    bubbleSort(data, totaSizeForOut);

    for (int i = 0; i < totaSizeForOut; i++)
    {
        printf("%d\n", data[i]);
    }
    
    for (int i = totaSizeForOut - 1; i >= totaSizeForOut - kthlargest; i--)
    {
        sprintf(temp, "%d", data[i]);
        if(strlen(outputStr) == 0){
            strcpy(outputStr, temp);
            strcat(outputStr, "\n");
        }else{
            strcat(outputStr, temp);
            strcat(outputStr, "\n");
        }
    }
    

    //open output file
    int fileProne = open(argv[argc - 1], O_WRONLY | O_CREAT | O_TRUNC, 0666); 
    if (fileProne < 0) 
    { 
        perror("r1"); 
        exit(1); 
    }


    sizeWrite = write(fileProne, outputStr, strlen(outputStr));
    close(fileProne);

    t = clock() - t; 
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds 
  
    printf("Execution took %f seconds to execute \n", time_taken); 
    
    printf("\nMain thread terminates\n");

    //buffers are created now time to write it into a file
    free(buffer);
    free(values);
    free(outputStr);

    return 0;
}