#include "./blockchain.h"
#include "./utils.h"
#include <iostream>
#include <ostream>

int main(int, char **) {
  Mirucoin::Blockchain chain{"chain.sqlite"};

  Mirucoin::Block blk{chain.getlastHash(), chain.getLastIndex(), "first block"};

  chain.addBlock(&blk);

  chain.print();

  return 0;
}