#pragma once

class Utils {
  public:
    static void Delay(int ms);
    static int normalizeReadings(float* readings, int length);
    static int normalizeReadings(int* readings, int length);
};
