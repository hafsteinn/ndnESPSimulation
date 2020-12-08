#include "BlockChainProxy.hpp"
#include "sha.hpp"
#include <iostream>


using namespace BlockNDN;

BlockChainProxy::BlockChainProxy() {
	//std::cout << "[BlockChainProxy]: creating a new blockchain..." << std::endl;
	bChain = BlockChain();
};

void BlockChainProxy::PrintBlockChainToScreen(std::string whoami)
{
	std::cout << "***************** " << whoami << " START *************" << std::endl;
	for (size_t i = 0; i < this->bChain.blockChain.size(); i++)
	{
		std::string gh(this->bChain.genesisHash.begin(),this->bChain.genesisHash.end());
		std::cout << "*******************" << std::endl;
		std::cout << "Size of BlockChain	: " << this->bChain.blockChain.size() << std::endl;
		std::cout << "String (s)			: " << this->bChain.blockChain.at(i).s << std::endl;
		std::cout << "Nonce 				: " << this->bChain.blockChain.at(i).nonce << std::endl;
		std::cout << "This Blocks Hash		: " << util.vectorToString(util.digestsha256mix(this->bChain.blockChain.at(i))) << std::endl;
		std::cout << "genesHash				: " << gh << std::endl; 
		std::cout << "prevBlock				: " << util.vectorToString(this->bChain.blockChain.at(i).prevBlock) << std::endl;
		std::cout << "*******************" << std::endl;
	}
	std::cout << "***************** " << whoami << " END *************" << std::endl;
}

BlockChainProxy::BlockChainProxy(BlockChain bChain) {
	bChain = bChain;
};

std::vector<unsigned char> BlockChainProxy::getBlockChainGenesisHash() {
	return bChain.getGenesisHash();
};

Block BlockChainProxy::getBlockAt(int index)
{
	return bChain.getBlockFromBlockChain(index);
}

Block BlockChainProxy::getBlock(std::vector<unsigned char> previousBlock) {
	std::cout << bChain.getBlockChainLength() << std::endl;

	for (int i = 0; i < bChain.getBlockChainLength(); i++)
	{
		if (previousBlock == bChain.getBlockFromBlockChain(i).prevBlock)
			return bChain.getBlockFromBlockChain(i);
	}

	Block empty;
	empty.s = "no";
	return empty;
};

bool BlockChainProxy::addBlock(Block block) {
	//std::cout << "trying to add a new block..." << std::endl;
	if (isBlockLegal(block, getPreviousHash()))
	{
		//std::cout << "I GOT INTO THE ADD BLOCK SECTION" << std::endl;
		bChain.addBlock(block);
		return true;
	}
	return false;
};

std::vector<unsigned char> BlockChainProxy::getPreviousHash() {
	return util.digestsha256mix(bChain.getLastLocalBlock());
};

int BlockChainProxy::getBlockHeight() {
	return bChain.getBlockChainLength();
};

bool BlockChainProxy::isBlockLegal(Block block, std::vector<unsigned char> prevBlockHash) {
	bool checkSize, checkPrevB, checkData, checkPOW;

	checkPOW = !cbu.NrawCheckProofOfWork(block);
	if (block.prevBlock == util.digestsha256mix(bChain.getLastLocalBlock()))
	{
		return checkPOW;
	}
	checkSize = cbu.checkBlockSize(block);
	checkPrevB = cbu.checkPrevHash(prevBlockHash, block);
	checkData = cbu.checkData(block.s);
	return checkSize && checkPrevB && checkData && checkPOW;
};