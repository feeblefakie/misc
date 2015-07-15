#include <iostream>
#include<time.h>
#include <math.h>

#define MAX_LOOPS 100000
#define IN_SIZE 3
#define MID_SIZE 3
#define ETA 0.01

double activation_func(double output);
bool is_learned(double teacher[][IN_SIZE], double *in, double *mid, double w_ij[][MID_SIZE], double *w_jk);

double teacher[4][3] = { 
  {0, 0, 0}, 
  {0, 1, 1}, 
  {1, 0, 1}, 
  {1, 1, 0}
};

int main(int argc, char *argv[])
{
  double in[IN_SIZE], mid[MID_SIZE], out;
	double w_ij[IN_SIZE][MID_SIZE]; // weights between i and j
	double w_jk[MID_SIZE]; // weights between j and k

	srand((unsigned)time(NULL));
  for(int i = 0; i < IN_SIZE; ++i) {
    for(int j = 0; j < MID_SIZE; ++j) {
			w_ij[i][j] = ((double) rand() / RAND_MAX);
    }
  }
  for(int i = 0; i < MID_SIZE; ++i){
    w_jk[i] = ((double) rand() / RAND_MAX);
  }

  // learning
  int num_loops = 0;
  while (1) {
    for (int i = 0; i < 4; i++) {

      for (int j = 0; j < IN_SIZE - 1; j++) {
        in[j] = teacher[i][j];
      }   
      in[IN_SIZE-1] = 1;

      // in -> mid
      for (int j = 0; j < MID_SIZE - 1; ++j) {
        double o = 0;
        for (int k = 0; k < IN_SIZE; ++k) {
          o += in[k] * w_ij[k][j];
          //std::cout << "in[k]: " << in[k] << ", w_ij[k][j]: " << w_ij[k][j] << std::endl;
        }
        mid[j] = activation_func(o);
      }
      mid[MID_SIZE-1] = 1;

      // mid -> out
      out = 0;
      for (int j = 0; j < MID_SIZE; ++j) {
        out += mid[j] * w_jk[j];
      }
      //std::cout << "out: " << out << std::endl;
      out = activation_func(out);

      // out -> mid
      double p = (teacher[i][2] - out) * out * (1 - out);
      //std::cout << "p: " << p << ", teacher: " << teacher[i][2] << ", out: " << out << std::endl;
      double prev_w_jk[3];
      for (int j = 0; j < MID_SIZE; ++j) {
        prev_w_jk[j] = w_jk[j]; // preserving for future use
        w_jk[j] += ETA * mid[j] * p;
      }

      // mid -> in
      for (int j = 0; j < MID_SIZE - 1; ++j) {
        for (int k = 0; k < IN_SIZE; ++k) {
          w_ij[k][j] += ETA * in[k] * mid[j] * (1 - mid[j]) * prev_w_jk[j] * p;
        }
      }
    }
    ++num_loops;
    
    bool succeeded = is_learned(teacher, in, mid, w_ij, w_jk);
    if (succeeded) {
      std::cout << "It seems that it correctly learns. - iteration: " << num_loops << std::endl;
      break;
    }

    if (num_loops == MAX_LOOPS) {
      std::cout << "It doesn't seem that it correctly learns." << std::endl;
      break;
    }
  }

  return 0;
}

double activation_func(double output) {
	//return (tanh(output/2) + 1) / 2; // sigmoid
  //return 1 / (1 + exp(-output)); // sigmoid 
  return tanh(output); // tanh
}

bool is_learned(double teacher[][IN_SIZE], double *in, double *mid, double w_ij[][MID_SIZE], double *w_jk)
{
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < IN_SIZE - 1; j++) {
      in[j] = teacher[i][j];
    }
    in[IN_SIZE-1] = 1;

    // in -> mid
    for (int j = 0; j < MID_SIZE - 1; ++j) {
      double o = 0;
      for (int k = 0; k < IN_SIZE; ++k) {
        o += in[k] * w_ij[k][j];
      }
      mid[j] = activation_func(o);
    }
    mid[MID_SIZE-1] = 1;

    // mid -> out
    double out = 0;
    for (int j = 0; j < MID_SIZE; ++j) {
      out += mid[j] * w_jk[j];
    }
    out = activation_func(out);
    if (out < teacher[i][2]-0.1 || out > teacher[i][2]+0.1) {
      return false;
    }
  }
  return true;
}
