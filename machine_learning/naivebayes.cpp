#include <iostream>

#define NUM_TEACHERS 14
#define NUM_ITEMS 5

typedef enum {
  BETA,
  SHINSETSU,
  ZARAME
} snow;

typedef enum {
  KIRI,
  HARE,
  KAZE
} weather;

typedef enum {
  LOW,
  HIGH,
  MID
} season;

typedef enum {
  RECOVERED,
  INJURED,
  TIRED
} condition;

typedef enum {
  YES,
  NO
} yesno;

typedef enum {
  SNOW = 0,
  WEATHER = 1,
  SEASON = 2,
  CONDITION = 3,
  SKI = 4,
} teacher_idx;

int teacher[NUM_TEACHERS][NUM_ITEMS] = {
  BETA, KIRI, LOW, RECOVERED, NO,
  SHINSETSU, HARE, LOW, INJURED, NO,
  SHINSETSU, HARE, LOW, RECOVERED, YES,
  SHINSETSU, HARE, HIGH, RECOVERED, YES,
  SHINSETSU, HARE, MID, RECOVERED, YES,
  ZARAME, KAZE, HIGH, TIRED, NO,
  BETA, HARE, LOW, RECOVERED, YES,
  ZARAME, KIRI, MID, RECOVERED, NO,
  SHINSETSU, KAZE, LOW, RECOVERED, YES,
  SHINSETSU, KAZE, LOW, RECOVERED, YES,
  SHINSETSU, KIRI, LOW, RECOVERED, YES,
  SHINSETSU, KIRI, LOW, RECOVERED, YES,
  BETA, HARE, MID, RECOVERED, YES,
  ZARAME, KIRI, LOW, INJURED, NO
};

double get_prob(teacher_idx idx, int val);
double get_cond_prob(teacher_idx idx, int val, yesno yn);

int main(int argc, char *argv[])
{
  double p_y = get_prob(SKI, YES);
  double p_n = get_prob(SKI, NO);

  double p_beta_y = get_cond_prob(SNOW, BETA, YES);
  double p_kaze_y = get_cond_prob(WEATHER, KAZE, YES);
  double p_high_y = get_cond_prob(SEASON, HIGH, YES);
  double p_tired_y = get_cond_prob(CONDITION, TIRED, YES);

  double p_beta_n = get_cond_prob(SNOW, BETA, NO);
  double p_kaze_n = get_cond_prob(WEATHER, KAZE, NO);
  double p_high_n = get_cond_prob(SEASON, HIGH, NO);
  double p_tired_n = get_cond_prob(CONDITION, TIRED, NO);

  double p_y_by_cond = p_y * p_beta_y * p_kaze_y * p_high_y * p_tired_y;
  double p_n_by_cond = p_n * p_beta_n * p_kaze_n * p_high_n * p_tired_n;

  std::cout << "\nsking ?" << std::endl;
  std::cout << "yes: " << p_y_by_cond << std::endl;
  std::cout << "no: " << p_n_by_cond << std::endl;
  if (p_y_by_cond > p_n_by_cond) {
    std::cout << "answer: YES" << std::endl;
  } else {
    std::cout << "answer: NO" << std::endl;
  }

  return 0;
}

double get_prob(teacher_idx idx, int val)
{
  int cnt = 0;
  for (int i = 0; i < NUM_TEACHERS; ++i) {
    if (teacher[i][idx] == val) {
      ++cnt;
    }
  }
  return (double) cnt / NUM_TEACHERS;
}

double get_cond_prob(teacher_idx idx, int val, yesno yn)
{
  int yn_cnt = 0;
  int cnt = 0;
  for (int i = 0; i < NUM_TEACHERS; ++i) {
    if (teacher[i][NUM_ITEMS-1] == yn) {
      ++yn_cnt;
      if (teacher[i][idx] == val) {
        ++cnt;
      }
    }
  }
  return (double) (cnt+1) / (yn_cnt+1);
}
