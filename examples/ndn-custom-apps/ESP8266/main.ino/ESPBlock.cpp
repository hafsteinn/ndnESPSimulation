#include "ESPBlock.hpp"
#include <string>

using namespace ESPBlockNDN;

Block::Block() {};

Block::Block(std::vector<unsigned char> prevBlock, long time, long nonce, std::string s)
{
	int prevBlockSize = prevBlock.size();
	
	if (prevBlock.size() == 64)
	{
		this->prevBlock = prevBlock;
		blockSize = 100;//52 + s.length();
		this->time = time;
		this->nonce = nonce;
		this->s = s;
	}
};