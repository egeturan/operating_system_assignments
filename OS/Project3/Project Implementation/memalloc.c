#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

// printfs are for debugging; remove them when you use/submit your library

//SYSTEM MANAGEMENT
int allocationMethod = 99;
void *basePointer = NULL;
int totalChunkSize;
int chunkSizeSOfFar = 0;
pthread_mutex_t lock;

//Link List for allocation
struct hole *head;
struct hole *emptyHead;

//Each Allocated area has struct to hold is information
struct hole 
{ 
    int holeSize, empty, flag; 
    struct hole *nextAvailable;
    void *dataPtr;
    void *insertionPoint; 
}; 


int mem_init (void *chunkpointer, int chunksize, int method)
{
    pthread_mutex_lock(&lock);

    //Take base address of the memory
    basePointer = chunkpointer;
    allocationMethod = method; //all allocation space will be determined with this method
    totalChunkSize = chunksize * 1024;

    
    head = ((struct hole *) chunkpointer);
    head->holeSize = 0;

    head->nextAvailable = NULL;
    head->dataPtr = NULL;
    head->insertionPoint = head + sizeof(struct hole);
    printf("insertionPoint: %p\n", head->insertionPoint);

    chunkSizeSOfFar += sizeof(struct hole);

    pthread_mutex_unlock(&lock); 

    return (0);// if success
}

void controller(){
    pthread_mutex_lock(&lock);
    printf("Our data sizes are: \n");
    for (struct hole *cur = head; cur != NULL; cur = cur->nextAvailable)
    {
        printf("%d\n", cur->holeSize);
    }
    pthread_mutex_unlock(&lock);
   // printf("Remaining size: %d\n", totalChunkSize - chunkSizeSOfFar);
}

void controller2(){
    pthread_mutex_lock(&lock);
    printf("Our empty list is: \n");
    for (struct hole *cur = emptyHead; cur != NULL; cur = cur->nextAvailable)
    {
        printf("%d\n", cur->holeSize);
    }
    pthread_mutex_unlock(&lock);
   // printf("Remaining size: %d\n", totalChunkSize - chunkSizeSOfFar);
}

void *mem_allocate (int objectsize)
{
    pthread_mutex_lock(&lock);
    int remaining = totalChunkSize - chunkSizeSOfFar;
    //Determine is there enough memory?
    if(remaining < (objectsize + sizeof(struct hole))){
        printf("No available place to allocate");
        pthread_mutex_unlock(&lock);
        return (NULL);
    }
    
    void *newObjectPlace;

    if(allocationMethod == 99){
        printf("Allocation Method is not specified\n");
        pthread_mutex_unlock(&lock);
        return (NULL);
    }

    if(head->holeSize == 0 && emptyHead == NULL){      //Chunk memory is empty so insert into head directly

        printf("Head and emptyHead is NULL so insert to the end\n");
        struct hole *currentW;
        
        printf("newObjectPlace: %p\n", head->insertionPoint);
        newObjectPlace = head->insertionPoint; //find the place to place

        //printf("hole size: %d, and %p \n", sizeof(struct hole), sizeof(struct hole));

        //printf("newObjectPlace: %p\n", newObjectPlace);
        //printf("head: %p\n", head);
 
        currentW = newObjectPlace + objectsize; //new node's address

        currentW->holeSize = (objectsize + sizeof(struct hole));
        //printf("hole size is: %d\n", currentW->holeSize);

        currentW->dataPtr = newObjectPlace;
        currentW->flag = 0;

        currentW->nextAvailable = NULL; //nextAvailable is NULL so next insertion will goes here

        head->nextAvailable = currentW;
        head->insertionPoint = currentW + sizeof(struct hole);
        printf("insertionPoint: %p\n", head->insertionPoint);

        chunkSizeSOfFar += currentW->holeSize;
        head->holeSize++;
        //create empty
   

    }else{ //first search the emptyList to find a proper place

        struct hole* curEmpty = emptyHead;
        struct hole* prev = NULL;

        if(allocationMethod == 0){  // *********************************
            //First fit approach so loop empty list untill you find a proper place 

            int decision = 0;

            if(emptyHead != NULL){
                printf("empty is not NULL **Searching EmptyLisst**\n");

            while (curEmpty->nextAvailable != NULL) //loop empty list and determine block size to if it fit
            {
                if(curEmpty->holeSize > (objectsize + sizeof(struct hole) )){ //we can give that hole to the request. Loop need to stop because of first fit
                    decision = 1;
                    break;
                }
                prev = curEmpty;
                curEmpty = curEmpty->nextAvailable;
            }

            if(curEmpty->nextAvailable == NULL && decision == 0){ //check the last element if we have not find a place yet
                if(curEmpty->holeSize > (objectsize + sizeof(struct hole) )){
                    decision = 1;
                }
            }

            if(prev == NULL && decision == 1){ //deleting head
                emptyHead = emptyHead->nextAvailable;
                curEmpty->nextAvailable = NULL;
            }else if(prev != NULL && decision == 1){ //arada bir yerde bulduysa
                prev->nextAvailable = curEmpty->nextAvailable;
                curEmpty->nextAvailable = NULL;
            }

        }

        head->holeSize++;

        if(decision == 0){ //If we cannot find a place from empty list, give a location from the end (unused memory)
        
            printf("++++++++Add from new empty place++++++\n");
            newObjectPlace = head->insertionPoint;

            struct hole *newNode = head->insertionPoint + objectsize;
            

            if(head->nextAvailable == NULL){
                head->nextAvailable = newNode;
                newNode->nextAvailable = NULL;
            }else{
            
            struct hole * cako = head->nextAvailable;
            while(cako->nextAvailable != NULL){
                cako = cako->nextAvailable;
            }

            cako->nextAvailable = newNode;
            newNode->nextAvailable = NULL;
 

            }


            newNode->dataPtr = newObjectPlace;
            newNode->flag = 0;
            newNode->holeSize = sizeof(struct hole) + objectsize;
            
            head->insertionPoint = newNode + sizeof(struct hole);
            chunkSizeSOfFar = chunkSizeSOfFar + sizeof(struct hole) + objectsize;
            
  
        }else if(decision == 1){  //give memory from empty list
            printf("--------Add from empty--------\n");

            newObjectPlace = (void *) curEmpty->dataPtr;

            printf("+++++curempty %d\n", curEmpty->holeSize);

            struct hole * prevvv = NULL;
            
            int foundBefore = 0; //we need to find allocated on before

            //printf("curEmpty=%d \n", curEmpty->holeSize);
            
            //printf("head=%lx \n", head);
            if(head->nextAvailable != NULL){
                if(curEmpty < head->nextAvailable){
                    //headin nextine bağlarız
                    curEmpty->nextAvailable = head->nextAvailable;
                    head->nextAvailable = curEmpty;
                }
            }else{

        struct hole* currentAlloc = head;
        prevvv = head;

        while(foundBefore == 0){ //curre mepty headib nextinnin ilerisinde
            if(currentAlloc > curEmpty){
                foundBefore = 1;
                break;
            }

            if(currentAlloc->nextAvailable != NULL){
                prevvv = currentAlloc;
                currentAlloc = currentAlloc->nextAvailable;
            }else{
                prevvv = currentAlloc;
                break;
            }
        }

        if(foundBefore == 0){
            currentAlloc->nextAvailable = curEmpty;
            curEmpty->nextAvailable = NULL;
            // curEMpty en sonda
        }else if(foundBefore == 1){
            curEmpty->nextAvailable = prevvv->nextAvailable;
            prevvv->nextAvailable = curEmpty;
        }

            }




        } //empty addition ends [1]  // *********************************

        }else if(allocationMethod == 1){ //Best fit approach

              int decision = 0;

              struct hole * minHole = NULL;
              struct hole * minHolePrev = NULL;
              

        if(emptyHead != NULL){ //sdfsdfsdf
            printf("Head is not NULL, Searching emptyList for best fit\n");

            int min = __INT_MAX__;
            struct hole * ege = emptyHead;
            while (ege != NULL){
                int newMin = ege->holeSize - (sizeof(struct hole) + objectsize);
                if(min > newMin && newMin > 0){
                    min = newMin;
                    minHole = ege;
                    minHolePrev = prev;
                    decision = 1;
                }
                prev = ege;
                ege = ege->nextAvailable;
            }


            if(minHolePrev == NULL && decision == 1){  //head için minhole head  gerisi null
                //Silenecek olan nodu empty head tutuyor demektir
                emptyHead = minHole->nextAvailable;
                minHole->nextAvailable = NULL;
                printf("minhole %d\n", minHole->holeSize);
                printf("minholeprev is: NULL\n");
            }else if(minHolePrev != NULL){
                minHolePrev->nextAvailable = minHole->nextAvailable;
                minHole->nextAvailable = NULL;
                printf("minhole is: %d\n", minHole->holeSize);
                printf("minholeprev is: %d\n", minHolePrev->holeSize);
            }


        } 

        //   +++++++++++++++++++++++++++++++++++++++++++++++++++++ DECISION 0

        head->holeSize++;

         if(decision == 0){ //If we cannot find a place from empty list, give a location from the end (unused memory)
        
            printf("++++++++Add from new empty place++++++\n");
            newObjectPlace = head->insertionPoint;

            struct hole *newNode = head->insertionPoint + objectsize;
            

            if(head->nextAvailable == NULL){
                head->nextAvailable = newNode;
                newNode->nextAvailable = NULL;
            }else{
            
            struct hole * cako = head->nextAvailable;
            while(cako->nextAvailable != NULL){
                cako = cako->nextAvailable;
            }

            cako->nextAvailable = newNode;
            newNode->nextAvailable = NULL;
 

            }


            newNode->dataPtr = newObjectPlace;
            newNode->flag = 0;
            newNode->holeSize = sizeof(struct hole) + objectsize;
            head->insertionPoint = newNode + sizeof(struct hole);
            chunkSizeSOfFar = chunkSizeSOfFar + sizeof(struct hole) + objectsize;
        
  
        }else if(decision == 1){
           printf("Now we can add them\n");
           // ++++++++++++++++++++++++++++++++++++++++++

            printf("--------Add from empty BEST FIT--------\n");

            newObjectPlace = (void *) minHole->dataPtr;

            printf("+++++minHole %d\n", minHole->holeSize);

            struct hole * prevvv = NULL;
            
            int foundBefore = 0; //we need to find allocated on before

            //printf("minHole=%d \n", minHole->holeSize);
            
            //printf("head=%lx \n", head);
            if(head->nextAvailable != NULL){
                if(minHole < head->nextAvailable){
                    //headin nextine bağlarız
                    minHole->nextAvailable = head->nextAvailable;
                    head->nextAvailable = minHole;
                }
            }else{

        struct hole* currentAlloc = head;
        prevvv = head;

        while(foundBefore == 0){ //curre mepty headib nextinnin ilerisinde
            if(currentAlloc > minHole){
                foundBefore = 1;
                break;
            }

            if(currentAlloc->nextAvailable != NULL){
                prevvv = currentAlloc;
                currentAlloc = currentAlloc->nextAvailable;
            }else{
                prevvv = currentAlloc;
                break;
            }
        }

        if(foundBefore == 0){
            currentAlloc->nextAvailable = minHole;
            minHole->nextAvailable = NULL;
            // minHole en sonda
        }else if(foundBefore == 1){
            minHole->nextAvailable = prevvv->nextAvailable;
            prevvv->nextAvailable = minHole;
        }

            }



           // +++++++++++++++++++++++++++++++++++++++

        }



        }else if(allocationMethod == 2){
                 int decision = 0;

              struct hole * maxHole = NULL;
              struct hole * maxHolePrev = NULL;
              

        if(emptyHead != NULL){ //sdfsdfsdf
            printf("Head is not NULL, Searching emptyList for worst fit\n");

            int max = 0;
            struct hole * ege = emptyHead;
            while (ege != NULL){
                int newMax = ege->holeSize - (sizeof(struct hole) + objectsize);
                if(max < newMax && newMax > 0){
                    max = newMax;
                    maxHole = ege;
                    maxHolePrev = prev;
                    decision = 1;
                }
                prev = ege;
                ege = ege->nextAvailable;
            }


            if(maxHolePrev == NULL && decision == 1){  //head için maxHole head  gerisi null
                //Silenecek olan nodu empty head tutuyor demektir
                emptyHead = maxHole->nextAvailable;
                maxHole->nextAvailable = NULL;
                printf("maxHole %d\n", maxHole->holeSize);
                printf("maxHolePrev is: NULL\n");
            }else if(maxHolePrev != NULL){
                maxHolePrev->nextAvailable = maxHole->nextAvailable;
                maxHole->nextAvailable = NULL;
                printf("maxHole is: %d\n", maxHole->holeSize);
                printf("maxHolePrev is: %d\n", maxHolePrev->holeSize);
            }


        } //empty head null bitti

        //+++++++++++++++++++++++++++++++++

        head->holeSize++;

        if(decision == 0){ //If we cannot find a place from empty list, give a location from the end (unused memory)
        
            printf("++++++++Add from new empty place++++++\n");
            newObjectPlace = head->insertionPoint;

            struct hole *newNode = head->insertionPoint + objectsize;
            

            if(head->nextAvailable == NULL){
                head->nextAvailable = newNode;
                newNode->nextAvailable = NULL;
            }else{
            
            struct hole * cako = head->nextAvailable;
            while(cako->nextAvailable != NULL){
                cako = cako->nextAvailable;
            }

            cako->nextAvailable = newNode;
            newNode->nextAvailable = NULL;
 

            }


            newNode->dataPtr = newObjectPlace;
            newNode->flag = 0;
            newNode->holeSize = sizeof(struct hole) + objectsize;
            head->insertionPoint = newNode + sizeof(struct hole);
            chunkSizeSOfFar = chunkSizeSOfFar + sizeof(struct hole) + objectsize;
        
  
        }

        //+++++++++++++++++++++++++++++++++

        else if(decision == 1){
             printf("Now we can add them\n");
           // ++++++++++++++++++++++++++++++++++++++++++

            printf("--------Add from empty WORST FIT--------\n");

            newObjectPlace = (void *) maxHole->dataPtr;

            printf("+++++maxHole %d\n", maxHole->holeSize);

            struct hole * prevvv = NULL;
            
            int foundBefore = 0; //we need to find allocated on before

            //printf("maxHole=%d \n", maxHole->holeSize);
            
            //printf("head=%lx \n", head);
            if(head->nextAvailable != NULL){
                if(maxHole < head->nextAvailable){
                    //headin nextine bağlarız
                    maxHole->nextAvailable = head->nextAvailable;
                    head->nextAvailable = maxHole;
                }
            }else {

        struct hole* currentAlloc = head;
        prevvv = head;

        while(foundBefore == 0){ //curre mepty headib nextinnin ilerisinde
            if(currentAlloc > maxHole){
                foundBefore = 1;
                break;
            }

            if(currentAlloc->nextAvailable != NULL){
                prevvv = currentAlloc;
                currentAlloc = currentAlloc->nextAvailable;
            }else{
                prevvv = currentAlloc;
                break;
            }
        }

        if(foundBefore == 0){
            currentAlloc->nextAvailable = maxHole;
            maxHole->nextAvailable = NULL;
            // maxHole en sonda
        }else if(foundBefore == 1){
            maxHole->nextAvailable = prevvv->nextAvailable;
            prevvv->nextAvailable = maxHole;
        }
        }


        }


        }  //allocation method 2 bitti
      

    }
    
    pthread_mutex_unlock(&lock);

    return newObjectPlace;// if not success
}

void mem_free(void *objectptr)
{
    pthread_mutex_lock(&lock);
    printf("free called\n");
    int found = 0;
    int isH = 0;

    //nothing to delete
    if(head->nextAvailable == NULL){
        printf("Chunk Memory is empty cannot delete desired memory block\n");
        pthread_mutex_unlock(&lock);
        return;
    }
    
    struct hole * curr = head->nextAvailable;
    struct hole * prev = head; 

    //Sadece bir node varsa ayrı bakmam gerekir ve alttaki while loopa girememesi lazım
    if(curr->nextAvailable == NULL){
        if(curr->dataPtr == objectptr){
            isH = 1;
            found = 1;
        }
    }

    //Find the desired memory block
    while(curr->nextAvailable != NULL && isH == 0){
        if(curr->dataPtr == objectptr){
            found = 1;
            break;
        }
        prev = curr;
        curr = curr->nextAvailable;
    }

    //Check the end of the linked list
    if(curr->nextAvailable == NULL && found == 0 && isH == 0){
        if(curr->dataPtr == objectptr){
            found = 1;
        }
    }

    if(found == 0){
        printf("Given object ptr does not exist\n");
        pthread_mutex_unlock(&lock);
        return;
    }

    //aşağıdaki cod istenilen nodu empty listten çıkartıyor ve bağlantıları kopartıyor
    //ilk if silinecek node ilk sırada is yani head->nextAvailable = silmek istediğimiz adamsa
    if(found == 1 && isH == 1){
        //--We find desired memory block, --deletion start--
        struct hole *deleted = curr;
        //Silenecek yerin gerisini ilerisine bağla;
        head->nextAvailable = deleted->nextAvailable;
        deleted->nextAvailable = NULL;
        deleted->flag = 1;//Marked as deleted
        deleted->dataPtr = NULL;

    }else if(found == 1 && isH == 0){// burası ise aradığımız block listin içinde ortada bir yerlerde ise
        struct hole *deleted = curr;
        //Silenecek yerin gerisini ilerisine bağla;
        prev->nextAvailable = deleted->nextAvailable;
        deleted->nextAvailable = NULL;
        deleted->flag = 1;//Marked as deleted
        deleted->dataPtr = NULL;
    }

    

    //Sildiğimiz bloğu empty linked listine bağla
    if(emptyHead == NULL){ //boş ise ilk elemanına
        emptyHead = curr;
    }else{//değilse empty listin sonuna git
        struct hole *currE = emptyHead;
        while (currE->nextAvailable != NULL)
        {
            currE = currE->nextAvailable;
        }
        currE->nextAvailable = curr; //Son boş bloğa bağla
    }

    head->holeSize--;

    //printf("dataPtr=%p \n", curr->dataPtr);
    //printf("objectptr=%p \n", objectptr);

    objectptr = NULL;



    pthread_mutex_unlock(&lock);
    return;
}


void mem_print (void)
{
    pthread_mutex_lock(&lock);
    int useEmpty = 0;
    struct hole *emptyCur = NULL;
    void *lastPoint = head + chunkSizeSOfFar;
    struct hole *curr = NULL;
    if(emptyHead != NULL){
        emptyCur = emptyHead;
        useEmpty = 1;
    }
    if(head->nextAvailable != NULL){
        curr = head->nextAvailable;
    }

    printf("Memory Structure is: \n");
    if(emptyCur == NULL && curr == NULL){
        printf("No place allocated\n");
        pthread_mutex_unlock(&lock);
        return;
    }
    else if(emptyCur != NULL && curr != NULL){

        printf("*********************************\n");
        printf("empty head %d\t  head%d \n", emptyCur->holeSize, curr->holeSize);

        while(1){
          
            int emp = 0;
            int all = 0;
              if(emptyCur > curr){
                   printf("out Allocated %d\t %lx\n", curr->holeSize, curr);
                   all = 1;

              }else{
                   printf("out Empty Section %d\t %lx\n", emptyCur->holeSize, emptyCur);
                   emp = 1;
              }

              //printf("Empty Section %d\t %lx\n", emptyCur->holeSize, emptyCur);

            if(curr->nextAvailable != NULL && all == 1){
                curr = curr->nextAvailable;
            }
            
            else if(curr->nextAvailable == NULL && all == 1){
                printf("in Empty Section %d\t %lx\n", emptyCur->holeSize, emptyCur);
                while(emptyCur->nextAvailable != NULL){
                    emptyCur = emptyCur->nextAvailable;
                    printf("Empty Section %d\t %lx\n", emptyCur->holeSize, emptyCur);
                }
                break;
            }

            if(emptyCur->nextAvailable != NULL && emp == 1){
                emptyCur = emptyCur->nextAvailable;
            }

            else if(emptyCur->nextAvailable == NULL && emp == 1){
                printf("in Allocated %d\t %lx\n", curr->holeSize, curr);
                while(curr->nextAvailable != NULL){
                    curr = curr->nextAvailable;
                    printf("Allocated %d\t %lx\n", curr->holeSize, curr);
                }
                break;
            }

        }

    }


    else if(emptyCur != NULL && curr == NULL){

        printf("No allocated hole\n");
        printf("Empty Section %d\t %lx\n", emptyCur->holeSize, emptyCur);
        while (emptyCur->nextAvailable != NULL)
        {
            emptyCur = emptyCur->nextAvailable;
            printf("Empty Section %d\t %lx\n", emptyCur->holeSize, emptyCur);
        }
        
    }
    else if(emptyCur == NULL && curr != NULL){

        printf("*********************************\n");
        printf("head %d\t \n", curr->holeSize);

        printf("No empty hole\n");
        printf("Allocated %d\t %lx\n", curr->holeSize, curr);
        while (curr ->nextAvailable != NULL)
        {
            curr = curr->nextAvailable;
            printf("Allocated %d\t %lx\n", curr->holeSize, curr);
        }
    }
   
   
 

    pthread_mutex_unlock(&lock);
    return;
}
