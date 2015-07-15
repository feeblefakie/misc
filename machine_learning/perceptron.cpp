#include <iostream>
#include <string>
#include <fstream>
#include <math.h>

#define MAX_LOOPS 50000
#define MAX_TEACHERS 40
#define IN_SIZE 3
#define OUT_SIZE 1
#define TDELIM ","
#define ETA 0.05

bool read_teacher(char *inputfile, double teacher[][IN_SIZE]);
bool is_learned(double teacher[][IN_SIZE], double *in, double *out, double weights[][OUT_SIZE]);
double step_func(double output);

int main(int argc, char *argv[])
{
  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " inputfile" << std::endl;
    exit(1);
  }
  double teacher[MAX_TEACHERS][IN_SIZE];
  if (!read_teacher(argv[1], teacher)) {
    std::cerr << "read_teacher failed" << std::endl;
    exit(1);
  }

	double in[IN_SIZE], out[OUT_SIZE];
	double weights[IN_SIZE][OUT_SIZE];

  // initialize weights
	for (int i = 0; i < IN_SIZE; i++) {
		for (int j = 0; j < OUT_SIZE; j++) {
			weights[i][j] = 0; // works only for perceptron ? (should be random numbers)
		}
	}

  // learning
  bool is_learning = true;
  int num_loops = 0;
	while (is_learning) {
		for (int j = 0; j < MAX_TEACHERS; j++) {
      for (int k = 0; k < IN_SIZE - 1; k++) {
        in[k] = teacher[j][k];
      }
      in[IN_SIZE-1] = 1;

      for (int k = 0; k < OUT_SIZE; k++) {
        // calculating the current output
        double o = 0;
        for (int l = 0; l < IN_SIZE; l++) {
          o += weights[l][k] * in[l];
        }
        // check if it fires or not
        out[k] = step_func(o);
      }
      if (is_learned(teacher, in, out, weights)) {
        is_learning = false;
        break;
      }
      // update weights
      for (int k = 0; k < OUT_SIZE; k++) {
        for (int l = 0; l < IN_SIZE; l++) {
          weights[l][k] += ETA * (teacher[j][IN_SIZE-1] - out[k]) * in[l];
        }
      }
		}
    if (++num_loops == MAX_LOOPS) {
      std::cout << "not solved" << std::endl;
      break;
    }
  }
  std::cout << "num_loops: " << num_loops << std::endl;
  for (int k = 0; k < OUT_SIZE; k++) {
    for (int l = 0; l < IN_SIZE; l++) {
      std::cout << weights[l][k] << " ";
    }
    std::cout << std::endl;
  }

  // check if learning works
  for (int j = 0; j < MAX_TEACHERS; j++) {
    for (int k = 0; k < IN_SIZE - 1; k++) {
      in[k] = teacher[j][k];
    }

    for (int k = 0; k < OUT_SIZE; k++) {
      // calculating the current output
      double o = 0;
      for (int l = 0; l < IN_SIZE; l++) {
        o += weights[l][k] * in[l];
      }
      std::cout << "in: " << in[0] << ", " << in[1] << " - o: " << o << std::endl;
    }
  }

	return 0;
}

bool is_learned(double teacher[][IN_SIZE], double *in, double *out, double weights[][OUT_SIZE])
{
  for (int j = 0; j < MAX_TEACHERS; j++) {
    for (int k = 0; k < IN_SIZE - 1; k++) {
      in[k] = teacher[j][k];
    }

    for (int k = 0; k < OUT_SIZE; k++) {
      // calculating the current output
      double o = 0;
      for (int l = 0; l < IN_SIZE; l++) {
        o += weights[l][k] * in[l];
      }
      if (teacher[j][IN_SIZE-1] != step_func(o)) {
        return false;
      }
    }
  }
  return true;
}

bool read_teacher(char *inputfile, double teacher[][IN_SIZE])
{
  std::ifstream fin;
  fin.open(inputfile, std::ios::in);
  if (!fin) { return false; }

  int i = 0;
  std::string line;
  while (getline(fin, line)) {
    char *str = (char *) line.c_str();
    int j = 0;
    for (char *p = strtok(str, TDELIM); p; p = strtok(NULL, TDELIM)) {
      teacher[i][j++] = atof(p);
    }
    if (++i == MAX_TEACHERS) {
      break;
    }
  }

  fin.close();
  return true;
}

double step_func(double output) {
	return output < 0 ? 0 : 1;
}
