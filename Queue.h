#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>

#include "rcb.h"

#define sizeOfQueue 100

rcb * queue;

int head;
int tail;

void initalizeQueue();
void enqueue(rcb toEnqueue);
rcb dequeue();

#endif
