#include "Core.hpp"
#include <cstdlib>
#include <stdlib.h>

int main() {
  Core core = Core(640, 480);
  core.run();

  return EXIT_SUCCESS;
}
