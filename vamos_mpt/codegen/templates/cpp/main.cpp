#include <cassert>
#include <iostream>

#include "monitor.h"
#include "inputs.h"

int monitor(Inputs &inputs);

int main(int argc, char *argv[]) {
  Inputs inputs(argv + 1, argc - 1);
  return monitor(inputs);
}
