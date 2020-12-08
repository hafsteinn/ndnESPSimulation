#ifndef BlockChainProxy_H_
#define BlockChainProxy_H_
#include "CheckBlock.hpp"
#include "BlockChain.hpp"
#include "CheckBlockUtils.hpp"
#include <vector>

namespace BlockNDN
{

	class BlockChainProxy : public CheckBlock
	{
	protected:
		BlockChain bChain;
		CheckBlockUtils cbu;
		Utils util;
	public:
		BlockChainProxy();
		BlockChainProxy(BlockChain bChain);
		std::vector<unsigned char> getBlockChainGenesisHash();
		Block getBlock(std::vector<unsigned char> previousBlock);
		Block getBlockAt(int index);
		void PrintBlockChainToScreen(std::string whoami);
		bool addBlock(Block block);
		std::vector<unsigned char> getPreviousHash();
		int getBlockHeight();
		bool isBlockLegal(Block block, std::vector<unsigned char> prevBlockHash);
	};

}

#endif