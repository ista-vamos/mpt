#include <cassert>
#include <iostream>

#include "monitor.h"
#include "inputs.h"

class Inputs;

int monitor(Inputs &inputs);

int main(int argc, char *argv[]) {
  assert(argc == 2);

  Inputs inputs(argv, 0);

  int res = monitor(inputs);
  std::string arg(argv[1]);
  if (arg == "false") {
    if (res == 0) {
      std::cerr << "Test returned " << res << " but should fail\n";
      return 1;
    }
  } else if (arg == "true") {
    if (res != 0) {
      std::cerr << "Test returned " << res << " but should succeed\n";
      return 1;
    }
  } else {
    std::cerr << "Invalid argument\n";
    return 1;
  }

  return 0;
}
