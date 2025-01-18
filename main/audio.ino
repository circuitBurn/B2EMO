/**
 * Map the 10 controller buttons to an index
 */
int getSoundIndex(int triggerValue)
{
  if (triggerValue >= 1649 && triggerValue < 1680)
  {
    return 1;
  }
  else if (triggerValue >= 1520 && triggerValue < 1540)
  {
    return 2;
  }
  else if (triggerValue >= 1387 && triggerValue < 1407)
  {
    return 3;
  }
  else if (triggerValue >= 1254 && triggerValue < 1274)
  {
    return 4;
  }
  else if (triggerValue >= 1120 && triggerValue < 1140)
  {
    return 5;
  }
  else if (triggerValue >= 982 && triggerValue < 1002)
  {
    return random(1, MP3_NUM_FILES); // Top right button is a random file
  }
  else if (triggerValue >= 797 && triggerValue < 817)
  {
    return 7;
  }
  else if (triggerValue >= 600 && triggerValue < 620)
  {
    return 8;
  }
  else if (triggerValue >= 391 && triggerValue < 411)
  {
    return 9;
  }
  else if (triggerValue >= 162 && triggerValue < 182)
  {
    return 10;
  }
  else
  {
    return -1;
  }
}
