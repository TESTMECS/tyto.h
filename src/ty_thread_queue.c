//! @file ty_thread_queue.c
//! 	Impl for Circular buffer.
//! TODO(drew): windows compat.
#define _GNU_SOURCE

#include "pthread.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/memfd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//! Linux
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
//!
#include <ty_thread_queue.h>

//! for [memfd_create] syscall
#if (__GLIBC__ < 2) || (__GLIBC__ == 2 && __GLIBC_MINOR__ < 27)
static inline int
memfd_create(const char* name, unsigned int flags)
{
    return syscall(__NR_memfd_create, name, flags);
}
#endif

//! Error handling.
static inline void
ty_queue_error(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "queue error: ");
    vfprintf(stderr, fmt, args);
    fputc('\n', stderr);
    va_end(args);
    abort();
}

static inline void
ty_queue_errno(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "queue error: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "(errno %d)\n", errno);
    va_end(args);
    abort();
}

void
ty_queue_init(ty_queue_t* q, size_t s)
{
    //! PORTABILITY ISSUE:
    //! 1. mmap two adjacent pages that point to same virtual memory.
    //! Page_A | Page_B
    //!   |> VM_A <|
    //! Will be used to optimize memory access.
    if (s % (size_t)getpagesize() != 0)
        ty_queue_error(
            "Requested size (%lu) is not a multiple of the page size (%d)",
            s,
            getpagesize());
    //! Create anon file backed by memory.
    if ((q->fd = memfd_create("queue_region", 0)) == -1)
        ty_queue_errno("Could not create anonymous file");
    //! Set buffer size
    if (ftruncate(q->fd, (off_t)s) != 0)
        ty_queue_errno("Could not set size of anonymous file");
    //! Ask for good address.
    if ((q->buffer = mmap(
             NULL, 2 * s, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)) ==
        MAP_FAILED)
        ty_queue_errno("Could not allocate virtual memory");
    //! mmap first region.
    if (mmap(
            q->buffer,
            s,
            PROT_READ | PROT_WRITE,
            MAP_SHARED | MAP_FIXED,
            q->fd,
            0) == MAP_FAILED)
        ty_queue_errno("Could not allocate virtual memory");
    //! Mirror the region so the buffer wraps seamlessly.
    if (mmap(
            q->buffer + s,
            s,
            PROT_READ | PROT_WRITE,
            MAP_SHARED | MAP_FIXED,
            q->fd,
            0) == MAP_FAILED)
        ty_queue_errno("Could not mirror virtual memory");
    //! Initialize synchronization primitives.
    if (pthread_mutex_init(&q->lock, NULL) != 0)
        ty_queue_errno("Could not initialize mutex");
    if (pthread_cond_init(&q->readable, NULL) != 0)
        ty_queue_errno("Could not initialize condition variable");
    if (pthread_cond_init(&q->writeable, NULL) != 0)
        ty_queue_errno("Could not initialize condition variable");
    //! Initialize remaining members
    q->size          = s;
    q->head          = 0;
    q->tail          = 0;
    q->head_sequence = 0;
    q->tail_sequence = 0;
}

void
ty_queue_free(ty_queue_t* q)
{
    if (munmap(q->buffer + q->size, q->size) != 0)
        ty_queue_errno("Could not unmap buffer");
    if (munmap(q->buffer, q->size) != 0)
        ty_queue_errno("Could not unmap buffer");
    if (close(q->fd) != 0)
        ty_queue_errno("Could not close anonymous file");
    if (pthread_mutex_destroy(&q->lock) != 0)
        ty_queue_errno("Could not destroy mutex");
    if (pthread_cond_destroy(&q->readable) != 0)
        ty_queue_errno("Could not destroy condition variable (readable).");
    if (pthread_cond_destroy(&q->writeable) != 0)
        ty_queue_errno("Could not destroy condition variable (writeable).");
}

void
ty_queue_put(ty_queue_t* q, uint8_t* buffer, size_t size)
{
    pthread_mutex_lock(&q->lock);
    //! Wait for space.
    while (q->size - (q->tail - q->head) < size + sizeof(ty_message_t))
        pthread_cond_wait(&q->writeable, &q->lock);
    //! Create header.
    ty_message_t m;
    m.len = size;
    m.seq = q->tail_sequence++;
    //! Write message.
    memcpy(&q->buffer[q->tail], &m, sizeof(ty_message_t));
    memcpy(&q->buffer[q->tail + sizeof(ty_message_t)], buffer, size);
    //! Increment write index
    q->tail += size + sizeof(ty_message_t);
    pthread_cond_signal(&q->readable);
    pthread_mutex_unlock(&q->lock);
}

size_t
ty_queue_get(ty_queue_t* q, uint8_t* buffer, size_t max)
{
    pthread_mutex_lock(&q->lock);

    //! Wait for message to consume.
    ty_message_t m;
    for (;;) {
        //! Wwwwwwwwaiting.....
        while ((q->tail - q->head) == 0)
            pthread_cond_wait(&q->readable, &q->lock);
        //! Read message header.
        memcpy(&m, &q->buffer[q->head], sizeof(ty_message_t));
        //! Message too long, wait for someone else to consume it.
        if (m.len > max) {
            while (q->head_sequence == m.seq)
                pthread_cond_wait(&q->writeable, &q->lock);
            continue;
        }
        break;
    }
    //! Read message body
    memcpy(buffer, &q->buffer[q->head + sizeof(ty_message_t)], m.len);
    //! Consume the message by incrementing the read pointer.
    q->head += m.len + sizeof(ty_message_t);
    q->head_sequence++;
    //! When read buffer moves into 2nd region,
    //! Reset to the first region.
    if (q->head >= q->size) {
        q->head -= q->size;
        q->tail -= q->size;
    }
    pthread_cond_signal(&q->writeable);
    pthread_mutex_unlock(&q->lock);
    return m.len;
}
