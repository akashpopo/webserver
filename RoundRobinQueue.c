#include "RoundRobinQueue.h"

void submitToQueue(rcb r)
{
  r -> max = Quantum;

  enqueue(r);
}

rcb getNextRCB()
{
  return dequeue();
}
