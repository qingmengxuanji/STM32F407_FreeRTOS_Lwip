#include "user_delay.h"

void user_delayUs(int time)
{
  for (int i = 0; i < time; i++)
  {
    for (int j = 0; j < 24; j++) // 1us
    {
    }
  }
}

void user_delayMs(int time)
{
  for (int i = 0; i < 1000; i++)
  {
    user_delayUs(time);
  }
}

