//! @file ty_thread_queue.h
//! 	A nice, fast, circular buffer.
//! NOTE: requires <pthread.h>
#ifndef TY_THREAD_QUEUE_H_
#define TY_THREAD_QUEUE_H_
#include <pthread.h>
#include <stdint.h>
//! Blocking queue data structure.
typedef struct ty_queue
{
    uint8_t* buffer;
    size_t   size;
    int      fd;
    size_t   head;
    size_t   tail;
    //! Next consumable message.
    size_t head_sequence;
    //! Last written message.
    size_t tail_sequence;
    //! Atomics.
    pthread_cond_t  readable;
    pthread_cond_t  writeable;
    pthread_mutex_t lock;
} ty_queue_t;

typedef struct ty_message
{
    size_t len;
    size_t seq;
} ty_message_t;

void
ty_queue_init(ty_queue_t* q, size_t s);

void
ty_queue_free(ty_queue_t* q);

void
ty_queue_put(ty_queue_t* q, uint8_t* buffer, size_t size);

size_t
ty_queue_get(ty_queue_t* q, uint8_t* buffer, size_t max);

#endif  // TY_THREAD_QUEUE_H_
