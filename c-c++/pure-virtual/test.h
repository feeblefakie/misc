class TestBase {
public:
  TestBase(void);
  virtual ~TestBase(void) = 0;
  virtual void process(void);

protected:
  int testbase;
  virtual void exec(void) = 0;

  void method(void);
};
