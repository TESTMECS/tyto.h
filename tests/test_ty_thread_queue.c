#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
//!
#include <ty_thread_queue.h>

#define BUFFER_SIZE         ((size_t)getpagesize())
#define NUMBER_THREADS      (8)
#define MESSAGES_PER_THREAD ((size_t)getpagesize() * 2)

void*
consumer_loop(void* arg)
{
    ty_queue_t* q     = (ty_queue_t*)arg;
    size_t      count = 0;
    size_t      i;
    for (i = 0; i < MESSAGES_PER_THREAD; i++) {
        size_t x;
        ty_queue_get(q, (uint8_t*)&x, sizeof(size_t));
        count++;
    }
    return (void*)count;
}

void*
publisher_loop(void* arg)
{
    ty_queue_t* q = (ty_queue_t*)arg;
    size_t      i;
    for (i = 0; i < NUMBER_THREADS * MESSAGES_PER_THREAD; i++)
        ty_queue_put(q, (uint8_t*)&i, sizeof(size_t));
    return (void*)i;
}

int
main(void)
{
    ty_queue_t q;
    ty_queue_init(&q, BUFFER_SIZE);

    pthread_t publisher;
    pthread_t consumers[NUMBER_THREADS];

    pthread_attr_t attr;
    pthread_attr_init(&attr);

    pthread_create(&publisher, &attr, &publisher_loop, (void*)&q);

    intptr_t i;
    for (i = 0; i < NUMBER_THREADS; i++)
        pthread_create(&consumers[i], &attr, &consumer_loop, (void*)&q);

    intptr_t sent;
    pthread_join(publisher, (void**)&sent);
    printf("Publisher sent (%ld) messages\n", sent);

    intptr_t recv[NUMBER_THREADS];
    for (i = 0; i < NUMBER_THREADS; i++) {
        pthread_join(consumers[i], (void**)&recv[i]);
        printf("consumer (%ld) received (%ld) messages\n", i, recv[i]);
    }
    pthread_attr_destroy(&attr);
    ty_queue_free(&q);
    return 0;
}
