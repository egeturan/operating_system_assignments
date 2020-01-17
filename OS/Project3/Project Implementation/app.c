#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "memalloc.h"

void testChunk(char * printer, int size){
    printf("Values are: \n");
    for (int i = 0; i < size; i++)
    {
        printf("%c", printer[i]);
    }
    
}


int main(int argc, char *argv[])
{
    void *chunkptr;
    void *endptr;
    char *charptr;
    int ret;
    int i;
    int size;
    void *x1 , *x2, *x3, *x4,*x5, *x6,*x7; //, *x3;// object pointers
    if (argc != 2)
     {
         printf("usage: app <size in KB>\n");
         exit(1);
    }
         
    size = atoi(argv[1]);// unit is in KB
    
    // allocate a chunk 
    chunkptr = sbrk(0); // end of data segment

    sbrk(size * 1024); // extend data segment by indicated amount (bytes)

    endptr = sbrk(0);// new end of data segment

    printf("chunkstart=%lx, chunkend=%lx, chunksize=%lu bytes\n",
    (unsigned long)chunkptr,
    (unsigned long)endptr, (unsigned long)(endptr -chunkptr));


    //test the chunk 
    printf("---starting testing chunk\n");
    charptr = (char *)chunkptr;

    for (i = 0; i < size; ++i)
        charptr[i] = 'i';
    
    printf("---chunk test ended -success\n");


    //   printf("current=%lx\n" ,  (unsigned long)current);


    ret = mem_init(chunkptr, size, WORST_FIT);

    //printf("current=%d\n" ,  chunkptr->holeSize);
    

    if (ret == -1) 
    {
        printf("could not initialize \n");
        exit(1);
    }


    x1 = mem_allocate(800);
    x2 = mem_allocate(1000);
    x3 = mem_allocate(1200);
    x4 = mem_allocate(5000);

    mem_print();

    mem_free(x1);
    mem_free(x2);
    mem_free(x3);
    mem_free(x4);

    mem_print();

    x1 = mem_allocate(10);
       mem_print();
        printf("ASDASDASDASDASDASD\n");
    x1 = mem_allocate(40);
    x1 = mem_allocate(40);
    x1 = mem_allocate(40);
    x1 = mem_allocate(40);
    x1 = mem_allocate(40);
   mem_print();
  

        
 






/*
    
    x1 = mem_allocate(800);
      mem_print();
    x2 = mem_allocate(1000);
    x3 = mem_allocate(1200);
    x4 = mem_allocate(5000);

     mem_print();

    mem_free(x2);
    mem_free(x3);
    mem_print();

    x5 = mem_allocate(1099);
     mem_print();

*/



/*
    x6 = mem_allocate(8000);
*/


  /*  x2 = mem_allocate(1000);
    x3 = mem_allocate(1200);
    mem_free(x1);
    mem_free(x2);
    mem_free(x3);
    x4 = mem_allocate(5000);
    x5 = mem_allocate(1100);
    x6 = mem_allocate(300);*/

  /*  x2 = mem_allocate(1000);
    x3 = mem_allocate(1200);
    //mem_print();
    mem_free(x1);
    mem_free(x2);
    mem_free(x3);
    //printf("deletion of 1000 :\n");mem_print();
    x4 = mem_allocate(5000);
    //mem_print();
    x5 = mem_allocate(1100);
    x6 = mem_allocate(500);
    x7 = mem_allocate(450);*/
    //x1 = mem_allocate(300);




    /*
    mem_free(x2);
    mem_free(x3);
    
    x1 = mem_allocate(50);
    
    x2 = mem_allocate(900);
    
    x4 = mem_allocate(10);
    /*
    //mem_free()
    */

 
 /*   for (int i = 0; i < 10; i++)
    {
        x2 = mem_allocate(10);
    }
*/





    

    

    /*
    x3 = mem_allocate(1300);
    
    
    mem_free(x2);
    mem_free(x3);
    
    */
    return 0;

}