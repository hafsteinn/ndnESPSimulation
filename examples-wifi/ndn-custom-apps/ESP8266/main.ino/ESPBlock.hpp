#ifndef ESPBlock_H_
#define ESPBlock_H_

#include <string>
#include <vector>

namespace ESPBlockNDN
{
	class Block
	{
	public:
		std::vector<unsigned char> prevBlock;					//32
		long time;								//8
		long nonce;								//8
		std::string s;
		int blockSize;
		Block();
		Block(std::vector<unsigned char> prevBlock, long time, long nonce, std::string s);
	};
}

#endif