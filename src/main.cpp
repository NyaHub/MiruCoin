#include "./blockchain.h"
#include "./utils.h"
#include <iostream>

int main(int, char **) {
  Mirucoin::Blockchain chain{"chain.sqlite"};
  return 0;
}