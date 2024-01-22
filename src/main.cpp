#include "./utils.h"
#include <iostream>

int main(int, char **) {
  std::cout << sha256String("some string") << std::endl;
  std::cout << sha512String("some string") << std::endl;
  return 0;
}