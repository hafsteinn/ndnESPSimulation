#include <string>
#include <iostream>
#include "BlockChain.hpp"


using namespace BlockNDN;

BlockChain::BlockChain() {
	blockChain = std::vector<Block>();
	Block genesisBlock = genesisBlockLoad();
	this->genesisHash = util.digestsha256mix(genesisBlock);
	this->blockChain.push_back(genesisBlock);
};

void BlockChain::addBlock(Block newBlock) {

	if (getBlockChainCapacity() > 0) {
		blockChain.push_back(newBlock);
	}
};

Block BlockChain::getBlockFromBlockChain(int index)
{
	return blockChain.at(index);
};

int BlockChain::getBlockChainLength() {
	return blockChain.size();
};

int BlockChain::getBlockChainCapacity() {
	return blockChain.capacity();
};

void BlockChain::clearBlockChain() {
	blockChain.clear();
};

Block BlockChain::getLastLocalBlock() {
	return blockChain.back();
};

std::vector<unsigned char> BlockChain::getGenesisHash() {
	return genesisHash;
};

Block BlockChain::genesisBlockLoad() {

	std::string genesisString = "This is the genesis block.";
	long time = 20161209;
	long nonce = 0;
	std::string s = "This is the genesis block, the first block on everyones Blockchain.";
	Block genesisBlock = Block(util.digestsha256(genesisString), time, nonce, s);

	return genesisBlock;
};


