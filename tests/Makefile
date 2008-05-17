
# for now first run make in ../

thread_pool_test:
	gcc -g -ansi -pedantic -Wall -c ../lib/task_pool.c
	gcc -g -ansi -pedantic -Wall -c ../lib/log.c
	gcc -g -ansi -pedantic -Wall -c ../lib/sbuf.c
	gcc -g -ansi -pedantic -Wall -c -I../ task_pool_test.c
	gcc -g -ansi -pedantic -Wall -c ../lib/xstdlib.c
	gcc -g -ansi -pedantic -Wall -o  task_pool_test task_pool_test.o task_pool.o log.o xstdlib.o -lpthread
	gcc -g -ansi -pedantic -Wall -o  sbuf_test sbuf_test.c sbuf.o log.o xstdlib.o 


clean:
	rm -f task_pool_test sbuf *.o
