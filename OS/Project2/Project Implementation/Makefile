all: topk_process_synchron topk_thread_synchron

topk_process_synchron: topk_process_synchron.c
	gcc -o topk_process_synchron topk_process_synchron.c -lrt -lpthread

topk_thread_synchron: topk_thread_synchron.c
	gcc -o topk_thread_synchron topk_thread_synchron.c -lpthread

clean:
	rm fr topk_process_synchron topk_process_synchron.o *~ 
	rm fr topk_thread_synchron topk_thread_synchron.o *~
