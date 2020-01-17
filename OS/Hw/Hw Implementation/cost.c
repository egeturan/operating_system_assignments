// C program to illustrate 
// system calls
#include<stdio.h> 
#include<fcntl.h> 
#include<errno.h> 
#include<stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
extern int errno; 
int main() 
{      

	 struct timeval start, end;

	  gettimeofday(&start, NULL);
    int fd = open("egeText.txt", O_RDONLY | O_CREAT);  
    printf("\n");  
    if (fd ==-1) 
    { 
        // Error print 
        printf("Error Number % d\n", errno);  
         
     }

    gettimeofday(&end, NULL);
    int size;
   printf("OPEN SYSCALL\n");
   printf("Time Elapsed for open syscall: %ld\n", ((end.tv_sec * 1000000 + end.tv_usec)
		  - (start.tv_sec * 1000000 + start.tv_usec)));

   int file1, file2, file3, file4, file5;

   char *buffer = (char *) calloc(10, sizeof(char));
   file1 = open("egeText.txt", O_RDONLY);
   file2 = open("egeText.txt", O_RDONLY);
   file3 = open("egeText.txt", O_RDONLY);
   file4 = open("egeText.txt", O_RDONLY);
   file5 = open("egeText.txt", O_RDONLY);
   printf("READ SYSCALLS\n");

//file1
   gettimeofday(&start, NULL);
   size = read(file1,  buffer, 10);
   gettimeofday(&end, NULL);
  printf("Time elapsed for 10 bytes:  %ld\n", ((end.tv_sec * 1000000 + end.tv_usec)
                  - (start.tv_sec * 1000000 + start.tv_usec)));
//file2
  buffer = (char *) calloc(100, sizeof(char));
   gettimeofday(&start, NULL);
    size = read(file2,  buffer, 100);
    gettimeofday(&end, NULL);
  printf("Time elapsed for 100 bytes  %ld\n", ((end.tv_sec * 1000000 + end.tv_usec)
                  - (start.tv_sec * 1000000 + start.tv_usec)));
//file3
 buffer = (char *) calloc(1000, sizeof(char));
   gettimeofday(&start, NULL);
    size = read(file3,  buffer, 1000);
      gettimeofday(&end, NULL);
  printf("Time elapsed for 1000 bytes:  %ld\n", ((end.tv_sec * 1000000 + end.tv_usec)
                  - (start.tv_sec * 1000000 + start.tv_usec)));
//file4

 buffer = (char *) calloc(10000, sizeof(char));
   gettimeofday(&start, NULL);
    size = read(file4,  buffer, 10000);
      gettimeofday(&end, NULL);
  printf("Time elapsed for 10000 bytes:  %ld\n", ((end.tv_sec * 1000000 + end.tv_usec)
                  - (start.tv_sec * 1000000 + start.tv_usec)));

//file5
 buffer = (char *) calloc(100000, sizeof(char));
   gettimeofday(&start, NULL);
    size = read(file5,  buffer, 100000);
      gettimeofday(&end, NULL);
  printf("Time elapsed for 100000 bytes:  %ld\n", ((end.tv_sec * 1000000 + end.tv_usec)
                  - (start.tv_sec * 1000000 + start.tv_usec)));
printf("MKDİR SYSCALL\n");
//Mkdir

gettimeofday(&start, NULL);
mkdir("newDirectory", 0700);
gettimeofday(&end, NULL);
  printf("Time elapsed for mkdir systemcall:  %ld\n", ((end.tv_sec * 1000000 + end.tv_usec)
                  - (start.tv_sec * 1000000 + start.tv_usec)));

printf("GETPID() SYSCALL\n");
//getpid
int pid;
gettimeofday(&start, NULL);
pid = getpid();
gettimeofday(&end, NULL);
printf("Process id is: %d \n", pid);
  printf("Time elapsed for getpid() systemcall:  %ld\n", ((end.tv_sec * 1000000 + end.tv_usec)
                  - (start.tv_sec * 1000000 + start.tv_usec)));
printf("WRİTE SYSCALL\n");
//Write
   char writeBuf1[100];
   char writeBuf2[1000];
   char writeBuf3[10000];
   char writeBuf4[1000000];
//write for 100
   int fileToWrite1= open("write1.txt", O_RDWR, 0666);

	for(int i = 0; i < 100; i++){
		writeBuf1[i] = 'e'; 
}
   gettimeofday(&start, NULL);
   write(fileToWrite1, writeBuf1, strlen(writeBuf1));
gettimeofday(&end, NULL);
  printf("Time elapsed for write 100 bits:  %ld\n", ((end.tv_sec * 1000000 + end.tv_usec)
                  - (start.tv_sec * 1000000 + start.tv_usec)));
close(fileToWrite1);
//write 1000
   int fileToWrite2= open("write2.txt", O_RDWR, 0666);
	   for(int i = 0; i < 1000; i++){
                writeBuf2[i] = 'e';
        }
   gettimeofday(&start, NULL);
   write(fileToWrite2, writeBuf2, strlen(writeBuf2));
gettimeofday(&end, NULL);

  printf("Time elapsed for write 1000 bits:  %ld\n", ((end.tv_sec * 1000000 + end.tv_usec)
                  - (start.tv_sec * 1000000 + start.tv_usec)));
close(fileToWrite2);

//write for 10000
  int fileToWrite3= open("write3.txt", O_RDWR, 0666);

   for(int i = 0; i < 10000; i++){
                writeBuf3[i] = 'e';
        }

   gettimeofday(&start, NULL);
   write(fileToWrite3, writeBuf3, strlen(writeBuf3));

gettimeofday(&end, NULL);
  printf("Time elapsed for write 10000 bits:  %ld\n", ((end.tv_sec * 1000000 + end.tv_usec)
                  - (start.tv_sec * 1000000 + start.tv_usec)));
close(fileToWrite3);

//write 1000000
  int fileToWrite4= open("write4.txt", O_RDWR, 0666);

   for(int i = 0; i < 1000000; i++){
                writeBuf4[i] = 'e';
        }

   gettimeofday(&start, NULL);
   write(fileToWrite4, writeBuf4, strlen(writeBuf4));
   gettimeofday(&end, NULL);
   close(fileToWrite4);

  printf("Time elapsed for write 1000000 bits:  %ld\n", ((end.tv_sec * 1000000 + end.tv_usec)
                  - (start.tv_sec * 1000000 + start.tv_usec)));
   exit(0);
 
} 
