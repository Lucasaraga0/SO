#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>

int main()
{
 int tempo;
 srand( (unsigned)time(NULL) );
 int x = rand()%10;

 if(x == 0)
 tempo = 15;
 else if(x > 0 && x <= 3)
 tempo = 5;
 else
 tempo = 1;

 printf("vou abrir demanda, cliente %d\n", getpid());
 FILE* demanda = fopen("demanda.txt", "w+");
 fprintf(demanda, "%d", tempo);
 fclose(demanda);
 printf("fechei demanda, cliente %d\n", getpid());

 raise(SIGSTOP);

 sem_t *sem = sem_open("/sem_atend", O_RDWR);
 if(sem != SEM_FAILED) sem_wait(sem);

 printf("cliente %d conseguiu liberacao do semaforo sem_atend\n", getpid());
 usleep(tempo);

 if(sem != SEM_FAILED) sem_post(sem);
 printf("cliente %d liberou do semaforo sem_atend\n", getpid());

 return 0;

}