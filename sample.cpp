#include <cstdio>
#include <iostream>
using namespace std;
int test_fn() {
  for (int i = 0; i < 9; i = i + 1) {
    cout << 'n' << 'o' << 'n' << '\n';
  }
  return 9 - 10 + (12 - 9);
}
int main() {
  int a, b;
  cin >> a >> b;
  int c[9];
  int d = a + b;
  cin >> c[0] >> c[1] >> c[2] >> c[d % 9] >> c[(d + 1) % 9];
  if (a > b) {
    cout << 'a' << ' ' << '>' << ' ' << 'b' << test_fn() << '\n';
  }
  cout << a * b << ' ' << c[0] << ' ' << c[1] << ' ' << c[2] << ' ' << c[d % 9] << ' ' << c[(d + 1) % 9] << endl;
}