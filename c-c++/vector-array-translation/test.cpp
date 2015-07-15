#include <iostream>
#include <vector>

using namespace std;

void puts_array(int *p, int len) {
    for (int i = 0; i < len; i++) {
          cout << p[i] << endl;
            }
}

int main() {
    vector<int> v;

      for (int i = 0; i < 10; i++) {
            v.push_back(i);
              }

        puts_array(&v[0], v.size());

          return 0;
}

