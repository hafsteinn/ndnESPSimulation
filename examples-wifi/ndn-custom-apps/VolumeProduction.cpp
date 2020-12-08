#include "VolumeProduction.hpp"
#include "BlockChainProxy.hpp"
#include <iostream>
#include "Utils.hpp"

using namespace BlockNDN;

void VolumeProduction::initVolumeProduction(BlockChainProxy bchainProxy) {
	std::cout << "Starting initVolumeProduction..." << std::endl;

	int height = bchainProxy.getBlockHeight();
	std::cout << height << std::endl;
	Utils util;

	std::vector<unsigned char> prevBlockHash = bchainProxy.getBlockChainGenesisHash();
	Block block = bchainProxy.getBlock(prevBlockHash);
	
	if (height > 0) { //this was 1 but then the process never goes into this if statement
		std::cout << "Inside if-height in VolumeProduction" << std::endl;
		if (&block != 0) { //this is supposted to be a while loop with a null check on the block

			//create a producer that will listed to the prefix of prevBlockHash

			prevBlockHash = util.digestsha256(block); //create a hash from the current block
			block = bchainProxy.getBlock(prevBlockHash); //use the hash to get the next block in the chain, if it exists
		}
	}

	std::cout << "End of initVolumeProduction" << std::endl;
};