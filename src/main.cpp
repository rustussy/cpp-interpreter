#include <chrono>
#include <fstream>

#include "interpreter.hpp"
#include "lexer.hpp"
#include "parser.hpp"

using std::chrono::duration_cast;
using std::chrono::microseconds;

const char *get_source(int argc, char **argv) {
  if (argc < 2) {
    return "source-code.cpp";
  }
  return argv[1];
}

int main(int argc, char **argv) {
  auto st_time = std::chrono::high_resolution_clock::now();
  std::ifstream file(get_source(argc, argv));

  std::string code, temp;

  // ignore #include stuff and `using namespace std;`
  for (int i = 0; i < 3; ++i) {
    std::getline(file, temp);
  }
  while (std::getline(file, temp)) {
    code += temp + '\n';
  }
  code += "main();";

  Lexer lexer(code);
  Parser parser(lexer);
  Interpreter interpreter;

  parser.parse()->accept(interpreter);

  file.close();

  auto ed_time = std::chrono::high_resolution_clock::now();
  std::cerr << "Time elapsed: " << duration_cast<microseconds>(ed_time - st_time).count() << " microseconds\n";
  return 0;
}