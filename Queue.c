#include "Queue.h"

void initalizeQueue()
{
  queue = malloc(sizeof(rcb) * sizeOfQueue);

  head = 0;
  tail = 0;
}

void enqueue(rcb toEnqueue)
{
  queue[tail] = toEnqueue;
  tail = (tail + 1) % sizeOfQueue;
}

rcb dequeue()
{
  rcb returnValue = queue[head];

  queue[head] = 0;

  head = (head + 1) % sizeOfQueue;

  return returnValue;
}

void printQueueContents()
{
  for (int i = 0; i < sizeOfQueue; i++)
    printf("%d -> ", queue[i]);

  printf("\n");
}
