struct Test {
  Test(int _score)
  : score(_score)
  {
    attr = (char *) &score;
  }

  ~Test(void)
  {}

  Test(const Test &t)
  {
    score = t.score;
    attr = (char *) &score;
  }

  int score;
  char *attr;
};
