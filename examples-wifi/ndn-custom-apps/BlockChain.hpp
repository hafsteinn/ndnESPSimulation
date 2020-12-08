#include <vector>
#include "Block.hpp"
#include "sha.hpp"
#include "Utils.hpp"

namespace BlockNDN
{
	class BlockChain
	{
	private:


	public:
			std::vector<Block> blockChain;
		std::vector<unsigned char> genesisHash;
		Utils util;
		BlockChain();
		void addBlock(Block newBlock);
		Block getBlockFromBlockChain(int index);
		int getBlockChainLength();
		int getBlockChainCapacity();
		void clearBlockChain();
		Block getLastLocalBlock();
		std::vector<unsigned char> getGenesisHash();
		Block genesisBlockLoad();
		unsigned char* VarToBytes(Block& data);
	};
}


