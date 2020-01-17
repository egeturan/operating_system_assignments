#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h> 
#include <pthread.h> 
#include <time.h>
#include <sys/time.h>
#include <time.h> 

typedef struct DataPass {
    int threadId;
    char *inputD;
} thdata;


struct node 
{ 
    int value; 
    struct node *left, *right; 
}; 

//Global Variables
int kthLargest;
int threadCount;
struct node *root =  NULL;
pthread_mutex_t mutex;
int order = 0;
int bstSize = 0;

struct node* insertNode(struct node* currNode, int num){
    if(root == NULL){
        printf("ROOT CREATED\n");
        struct node *temp =  (struct node *) malloc(sizeof(struct node));
        temp->value = num;
        temp->left = NULL;
        temp->right = NULL;
        root = temp;
        return currNode;
    }

    if(currNode == NULL){
        struct node *temp =  (struct node *) malloc(sizeof(struct node));
        temp->value = num;
        temp->left = NULL;
        temp->right = NULL;
        return temp;
    }

    if(num < currNode->value){
        currNode->left = insertNode(currNode->left, num);
    }
    else
        currNode->right = insertNode(currNode->right, num);

    return currNode;
}

int sizeOfBST(struct node * root){
    if(root == NULL){
        return 0;
    }else{
        return sizeOfBST(root->left) + sizeOfBST(root->right) + 1;
    }
}

int minValue(struct node * root)
{
    int returnVal;
    for (struct node * curr = root; curr != NULL; curr = curr->left)
    {
       returnVal = curr->value;
    }
    return returnVal;
} 

struct node * minValueNode(struct node* node) 
{ 
    struct node* current = node; 
  
    /* loop down to find the leftmost leaf */
    while (current && current->left != NULL) 
        current = current->left; 
  
    return current; 
} 

struct node* deleteNode(struct node* root, int key) 
{ 
    if (root == NULL) return root; 
  
    if (key < root->value) 
        root->left = deleteNode(root->left, key); 
  
    else if (key > root->value) 
        root->right = deleteNode(root->right, key); 
  
    // if key is same as root's key, then This is the node 
    // to be deleted 
    else
    { 
        // node with only one child or no child 
        if (root->left == NULL) 
        { 
            struct node *temp = root->right; 
            free(root); 
            return temp; 
        } 
        else if (root->right == NULL) 
        { 
            struct node *temp = root->left; 
            free(root); 
            return temp; 
        } 
  
        // node with two children: Get the inorder successor (smallest 
        // in the right subtree) 
        struct node* temp;
        //find minValueNode

         struct node* current = root->right; 
  
            /* loop down to find the leftmost leaf */
            while (current && current->left != NULL) 
                current = current->left; 
        
            temp = current; 
  
        // Copy the inorder successor's content to this node 
        root->value = temp->value; 
  
        // Delete the inorder successor 
        root->right = deleteNode(root->right, temp->value); 
    } 
    return root; 
}





/*he program will have a shared binary search tree (pointed by a global
variable) with size at most K nodes.*/

void *threadFunction(void *param) 
{ 
    //Thread local variables
    thdata *data1; 
    data1 = (thdata *) param;  /* type cast to a pointer to thdata */
    char *inputs = data1->inputD;
    int threadIdN = data1->threadId;
    int arrSize;

    FILE * filePtr = fopen(inputs, "r"); 
    if (filePtr == NULL) 
    { 
        printf("File read is unsuccessfull\n"); 
        return 0; 
    } 

    //int arr;
    //int values[1000000];
    int *values = (int *) malloc(100000000 * sizeof(int));

    int arrIndex = 0;
    int currentValue;

    while( fscanf(filePtr, "%d", &currentValue) == 1){
        values[arrIndex] = currentValue;
        arrIndex++;
    }

    //arrrIndex is now our length
    arrSize = arrIndex;

    //fscanf works corrently
    
    /*for (int i = 0; i < arrIndex; i++)
    {
        printf("Val is: %d\n", values[i]);
    }*/

    //sort the array
    //bubbleSort(values, arrSize); //change sorting order of merge sort

    //sorting completed corrently
    /*for (int i = 0; i < kthLargest; i++)
    {
        printf("Value is: %d\n", values[i]);
    }*/

    int i = 0;
    //Acquire Lock
    pthread_mutex_lock(&mutex);
    
    while (bstSize < kthLargest)
    {
        insertNode(root, values[i]);
        i++;
        bstSize++;
        //Release Lock
        pthread_mutex_unlock(&mutex);

        //Acquire Lock
        pthread_mutex_lock(&mutex);
    }

    //Release Lock
    pthread_mutex_unlock(&mutex);

    //now bst is reach to the kthlargest count
    //to add new nodes we need to delete node


    while(i < arrSize){
        //Acquire Lock
        pthread_mutex_lock(&mutex);
        int minVal = minValue(root);
        //printf("Min value is: %d\n", minVal);
        //printf("Value is: %d\n", values[i]);
            if (values[i] > minVal) // I need to add
            {
                root = deleteNode(root, minVal);
                insertNode(root, values[i]);
            }
             //Release Lock
        pthread_mutex_unlock(&mutex);
        i++;    
    }
 
    //Critical Section

    //Remainder Lock

    printf("Thread %d is completed\n", (threadIdN + 1));

    free(values);

    return 0;
}

int main(int argc,char* argv[]) {
    
    //variables
    int inputCount = 0;
    pthread_t tidA[20]; /* id of the created thread */
    pthread_t tid;
	pthread_attr_t attr;  /* set of thread attributes */

    thdata data[20]; // datastruct to pass information to the threads
    clock_t t; 

    printf("Program Starts\n");

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
    else if(argc > 13){
        printf("N cannot be bigger than 10 thus there cannot be more than 10 input file");
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

        if(threadCount > 10 || threadCount < 1){
            printf("N cannot be bigger than 10");
            return 0;
        }

        if(kthLargest < 100 || kthLargest > 10000){
            printf("K need to be between 100 and 10000");
            return 0;
        }

        //Last validations are completed
        //Computations are starting

        //Create Threads
        int j = 0;
        char *temp; 
        t = clock();

         if (pthread_mutex_init(&mutex, NULL) != 0) 
        { 
            printf("\n mutex init has failed\n"); 
            return 1; 
        } 

        do{
        printf("Thread %d is created\n", j + 1);
        temp = argv[3 + j];
        data[j].inputD = temp; // add the inputfile name to the thread data structure
        data[j].threadId = j;  // id of the thread for thread order arrangement
        pthread_attr_init (&attr); 	
	    pthread_create (&tid, &attr, threadFunction, (void *) &data[j]); 
        tidA[j] = tid; 
        j++;
    }while(j < threadCount);
    

    for (int i = 0; i < threadCount; i++)
    {
        pthread_join (tidA[i], NULL); //wait thread to finish its job.
    }

    pthread_mutex_destroy(&mutex); 

        //Parent active again

       //Main process continues to work

    }
    //control the size of the BST
    printf("Size of BST is: %d\n", sizeOfBST(root));

    //Read the BST and write to a file
    int finalKthValues[1000000];
    int index = 0;
    for (int i = 0; i < kthLargest; i++)
    {
        int smallestCurrent = minValue(root);
        finalKthValues[index] = smallestCurrent;
        index++;
        root = deleteNode(root, smallestCurrent);
    }
    
    //open output file
    int fileProne = open(argv[argc - 1], O_WRONLY | O_CREAT | O_TRUNC, 0666); 
    if (fileProne < 0) 
    { 
        perror("r1"); 
        exit(1); 
    }
    
    char *output = (char *) malloc(1000000 * sizeof(char));
    char tempStr[10];

    for (int i = index - 1; i >= 0; i--)
    {
        sprintf(tempStr, "%d", finalKthValues[i]);
             if(strlen(output) == 0){
                strcpy(output, tempStr);
                strcat(output, "\n");  
             }else{
                 strcat(output, tempStr);
                strcat(output, "\n");  
             }
    }

    t = clock() - t; 
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds 
  
    printf("Execution took %f seconds to execute \n", time_taken); 
    
    int sizeWrite = write(fileProne, output, strlen(output));
    close(fileProne);

    free(output);

    printf("Writing is completed\n");

    printf("Parent is terminated\n");

    return 0;
}
