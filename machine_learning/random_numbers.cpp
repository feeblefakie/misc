#include <iostream>
#include <math.h>
#include <stdio.h>

#define PI 3.14159265358979
#define SAMPLES 20

int main(void)
{
  srand((unsigned)time(NULL));

  /* create random numbers by Box–Muller transform */
  // random numbers from normal distribution of average(1,1) and variance(0.5)
  double sigma = sqrt(0.5);
  double avex = 1, avey = 1;

  int num_samples = 0;
  while (1) {
    double r1 = (double) rand() / RAND_MAX;
    double r2 = (double) rand() / RAND_MAX;
    double x = sigma * sqrt(-2*log(r1)) * cos(2*PI*r2) + avex;
    double y = sigma * sqrt(-2*log(r1)) * sin(2*PI*r2) + avey;

    if (y > -x + 1) { // select only if y > -x + 1
      std::cout << x << "," << y << ",1" << std::endl;
      if (++num_samples == SAMPLES) {
        break;
      }
    }
  }

  // random numbers from normal distribution of average(-1,-1) and variance(1.5)
  sigma = sqrt(1.5);
  avex = -1, avey = -1;

  num_samples = 0;
  while (1) {
    double r1 = (double) rand() / RAND_MAX;
    double r2 = (double) rand() / RAND_MAX;
    double x = sigma*sqrt(-2*log(r1))*cos(2*PI*r2) + avex;
    double y = sigma*sqrt(-2*log(r1))*sin(2*PI*r2) + avey;

    if (y < -x + 1) { // select only if y < -x + 1
      std::cout << x << "," << y << ",0" << std::endl;
      if (++num_samples == SAMPLES) {
        break;
      }
    }
  }

  return 0;
}
