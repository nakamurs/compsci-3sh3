#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *deposit(void *value);
void *withdraw(void *value);

int amount = 0;
pthread_mutex_t mutex;

int main(int argc, char *argv[]){
    /* Check if exactly two command line arguments are provided */
    if (argc != 3) {
        printf("Usage: ./PLmutex <deposit_amount> <withdraw_amount>\n");
    }

    /* Declaring threads */
    pthread_t tid[6]; 
    pthread_attr_t attr; 
    pthread_attr_init(&attr);

     /* Initialize mutex */
    if (pthread_mutex_init(&mutex, NULL) !=0){
        printf("Error in initializing mutex\n"); 
    }

    /* Create threads */
    for (int i=0; i<6; i++){
        if (i % 2 == 0){    /* 3 threads call deposit() */
            pthread_create(&tid[i], &attr, deposit, (void*)(long)atoi(argv[1]));
        } else {            /* 3 threads call withdraw() */
            pthread_create(&tid[i], &attr, withdraw, (void*)(long)atoi(argv[2]));
        }
    }
    
    /* Wait for the threads to exit */
    for (int i=0; i<6; i++){
        pthread_join(tid[i], NULL);
    }

    /* Print the final amount */
    printf("Final amount = %d\n", amount);

    /* Destroy mutex */
    pthread_mutex_destroy(&mutex);
    
    return 0;
}

void *deposit(void *value){
    int v = (int)(long)value;

    /*Acquire mutex lock*/ 
    pthread_mutex_lock(&mutex); 

    /*Critical Section*/ 
    amount += v;
    printf("Deposit amount = %d\n", amount);

    /*Release mutex locks*/
    pthread_mutex_unlock(&mutex);

    pthread_exit(0);
}

void *withdraw(void *value){
    int v = (int)(long)value;

    /*Acquire mutex lock*/ 
    pthread_mutex_lock(&mutex); 

    /*Critical Section*/ 
    amount -= v;
    printf("Withdrawal amount = %d\n", amount);

    /*Release mutex locks*/
    pthread_mutex_unlock(&mutex);

    pthread_exit(0);
}
