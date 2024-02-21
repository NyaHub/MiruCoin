#include "block.h"
#include <cstdio>
#include <string>
namespace Mirucoin {
class Miner {
private:
  std::string address;

public:
  Miner() {}
  Miner(std::string addr) { address = addr; }
  Block mine(Block &blk) {
    std::cout << "Mining..." << std::endl;
    while (!Block::chekHash(blk)) {
      blk.updateHeader(blk.getNonce() + 1);
      printf("%s\r", blk.getHash().c_str());
    }
    std::cout << std::endl;
    return blk;
  }
};
} // namespace Mirucoin