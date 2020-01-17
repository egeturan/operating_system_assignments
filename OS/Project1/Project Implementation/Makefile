all: findtopk findtopk_mqueue findtopk_thread

findtopk: findtopk.c
	gcc -o findtopk findtopk.c

findtopk_mqueue: findtopk_mqueue.c
	gcc -o findtopk_mqueue findtopk_mqueue.c -lrt

findtopk_thread: findtopk_thread.c
	gcc -o findtopk_thread findtopk_thread.c -pthread

clean:
	rm fr fintopk findtopk.o *~ 
	rm fr findtopk_mqueue findtopk_mqueue.o *~
	rm fr findtopk_thread findtopk_thread.o *~ 
