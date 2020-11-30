#include "ESPUtils.hpp"
#include <iostream>
#include "ESPsha.hpp"
#include <stdlib.h>
#include <string>

using namespace ESPBlockNDN;

std::string Utils::getPrimaryPublicKey(){
	return "-----BEGIN PUBLIC KEY-----\n"\
	"MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCpLwVdn8aCfJ241DIn0/2Ykdvh\n"\
	"Wz3hR+YuUIw+EOLlW3x9ZGAKuqhaHDA7SOCNyj0WyDkFLJt0XAo0vyRMXLQLFeGC\n"\
	"4z4ms6F1UX5jOLeaEKFCQrCznb6co2mI7AOrd3TOevimw3IMVC2uXyRRmUfI1zlz\n"\
	"QJ3ZLXZfp3HEtsb+swIDAQAB\n"\
	"-----END PUBLIC KEY-----\n\0";
}

std::string Utils::vectorToString(std::vector<unsigned char> vin)
{
	std::string str(vin.begin(), vin.end());
	return str;
};

std::vector<unsigned char> Utils::digestsha256(Block block) {
	std::vector<unsigned char> newBlockByte;
	std::vector<unsigned char> blockHash;

	newBlockByte = blockToByteArray(block);
	std::string s(newBlockByte.begin(),newBlockByte.end());

	std::string hash = picosha2::hash256_hex_string(s);
	std::vector<unsigned char> returnHash(hash.begin(), hash.end());

	return returnHash;
};

std::vector<unsigned char> Utils::digestsha256mix(Block block) {
	
	//std::string sNonce = std::to_string(block.nonce);
	//std::string sTime = std::to_string(block.time);
	std::string sHash(block.prevBlock.begin(),block.prevBlock.end());

	std::string raw;
	//raw.append(sNonce);
	//raw.append(sTime);
	raw.append(sHash);
	raw.append(block.s);

	std::string hash = picosha2::hash256_hex_string(raw);
	std::vector<unsigned char> returnHash(hash.begin(), hash.end());

	return returnHash;
};

std::vector<unsigned char> Utils::digestsha256(std::string s) {
	std::vector<unsigned char> blockHash;

	std::string hash = picosha2::hash256_hex_string(s);
	std::vector<unsigned char> returnHash(hash.begin(), hash.end());

	return returnHash;
};

int Utils::bytesToInt(std::vector<unsigned char> vin)
{
	int a = int((unsigned char)(vin.at(0)) << 24 |
            (unsigned char)(vin.at(1)) << 16 |
            (unsigned char)(vin.at(2)) << 8 |
            (unsigned char)(vin.at(3)));

	return a;
}

long Utils::bytesToLong(std::vector<unsigned char> b)
{
	    long l = 0;
    l |= b[0] & 0xFF;
    l <<= 8;
    l |= b[1] & 0xFF;
    l <<= 8;
    l |= b[2] & 0xFF;
    l <<= 8;
    l |= b[3] & 0xFF;
	l <<= 8;
	l |= b[4] & 0xFF;
    l <<= 8;
    l |= b[5] & 0xFF;
    l <<= 8;
    l |= b[6] & 0xFF;
    l <<= 8;
    l |= b[7] & 0xFF;
    return l;
}




Block Utils::unpackDataPacket(const unsigned char *in, size_t length)
{
  std::vector<unsigned char> prevHash;
  std::vector<unsigned char> bsize;
  std::vector<unsigned char> btime;
  std::vector<unsigned char> bnonce;
  std::vector<unsigned char> bstring;

  //populate prevHash
  for (size_t i = 0; i < 64; i++)
  {
    prevHash.push_back(in[i]);
  }

  //populate bsize
  for (size_t i = 64; i < 68; i++)
  {
    bsize.push_back(in[i]);
  }

  //populate btime
  for (size_t i = 68; i < 76; i++)
  {
    btime.push_back(in[i]);
  }

  //populate bnonce
  for (size_t i = 76; i < 84; i++)
  {
    bnonce.push_back(in[i]);
  }

    //populate bnonce
  for (size_t i = 84; i < length; i++)
  {
    bstring.push_back(in[i]);
  }

  //convert arrays to datatypes
  std::string finalPrevHashValue(prevHash.begin(),prevHash.end());
  int finalBlockSizeValue = bytesToInt(bsize);
  long finalTimeValue = bytesToLong(btime);
  long finalNonceValue = bytesToLong(bnonce);
  std::string finalStringValue(bstring.begin(),bstring.end());

   	// std::cout << "-----------------------------------" << std::endl;
	// std::cout << "Received prevBlock: " << finalPrevHashValue << std::endl;
	// std::cout << "Received blockSize: " << finalBlockSizeValue << std::endl;
	// std::cout << "Received time: " << finalTimeValue << std::endl;
	// std::cout << "Received nonce: " << finalNonceValue << std::endl;
  	// std::cout << "Received text: " << finalStringValue << std::endl;
  	// std::cout << "-----------------------------------" << std::endl;


	//create the Block object
	Block returnBlock = Block(prevHash, finalTimeValue, finalNonceValue, finalStringValue);

	return returnBlock;
  
}

std::vector<uint8_t> Utils::blockToByteArray(Block newBlock) {

	std::vector<unsigned char> empty;
	std::vector<unsigned char> byteArray;
	std::vector<uint8_t> blockSize = intToBytes(newBlock.blockSize);
	std::vector<unsigned char> time = longToBytes(newBlock.time);
	std::vector<unsigned char> nonce = longToBytes(newBlock.nonce);
	std::vector<unsigned char> string(newBlock.s.begin(), newBlock.s.end());

	
	bool prevBlockCopy = byteArrayCopy(byteArray, newBlock.prevBlock, 0, newBlock.prevBlock.size());
	// std::cout << "<<<<<<<<<<<<<<<<<" << std::endl;
	// for (size_t i = 0; i < byteArray.size(); i++)
	// {
	// 	printf("%x", byteArray.at(i));
	// }
	// std::cout << "<<<<<<<<<<<<<<<<<" << byteArray.size() << std::endl;

	bool blockSizeCopy = byteArrayCopy(byteArray, blockSize, newBlock.prevBlock.size(), newBlock.prevBlock.size() + blockSize.size());
	// std::cout << "<<<<<<<<<<<<<<<<<" << std::endl;
	// for (size_t i = 0; i < byteArray.size(); i++)
	// {
	// 	printf("%x", byteArray.at(i));
	// }
	// std::cout << "<<<<<<<<<<<<<<<<<" << byteArray.size() << std::endl;

	bool timeCopy = byteArrayCopy(byteArray, time, newBlock.prevBlock.size() + blockSize.size(),newBlock.prevBlock.size() + blockSize.size()+ time.size());
	// std::cout << "<<<<<<<<<<<<<<<<<" << std::endl;
	// for (size_t i = 0; i < byteArray.size(); i++)
	// {
	// 	printf("%x", byteArray.at(i));
	// }
	// std::cout << "<<<<<<<<<<<<<<<<<" << byteArray.size() << std::endl;
	bool nonceCopy = byteArrayCopy(byteArray, longToBytes(newBlock.nonce), newBlock.prevBlock.size() + blockSize.size() + time.size(),
		newBlock.prevBlock.size() + blockSize.size() + time.size() + nonce.size());
	// std::cout << "<<<<<<<<<<<<<<<<<" << std::endl;
	// for (size_t i = 0; i < byteArray.size(); i++)
	// {
	// 	printf("%x", byteArray.at(i));
	// }
	// std::cout << "<<<<<<<<<<<<<<<<<" << byteArray.size() << std::endl;

	bool stringCopy = byteArrayCopy(byteArray, string, newBlock.prevBlock.size() + blockSize.size() + time.size() + nonce.size(),
		newBlock.prevBlock.size() + blockSize.size() + time.size() + nonce.size() + string.size());
	// std::cout << "<<<<<<<<<<<<<<<<<" << std::endl;
	// for (size_t i = 0; i < byteArray.size(); i++)
	// {
	// 	printf("%x", byteArray.at(i));
	// }
	// std::cout << "<<<<<<<<<<<<<<<<<" << byteArray.size() << std::endl;

	if (prevBlockCopy && stringCopy) {
		return byteArray;
	}
	else {
		return empty;
	}
};

std::vector<unsigned char> Utils::intToBytes(int value) {
	std::vector<unsigned char> result;
    result.push_back((value >> 24) & 0xff);
    result.push_back((value >> 16) & 0xff);
    result.push_back((value >>  8) & 0xff);
    result.push_back((value      ) & 0xff);

    return result;
};

std::vector<unsigned char> Utils::longToBytes(long data) {
	std::vector<unsigned char> bytes;
	bytes.push_back((data >> 56) & 0xff);
	bytes.push_back((data >> 48) & 0xff);
	bytes.push_back((data >> 40) & 0xff);
	bytes.push_back((data >> 32) & 0xff);
	bytes.push_back((data >> 24) & 0xff);
	bytes.push_back((data >> 16) & 0xff);
	bytes.push_back((data >> 8) & 0xff);
	bytes.push_back((data & 0xff));
	
	return bytes;
};

bool Utils::byteArrayCopy(std::vector<unsigned char> &array, std::vector<unsigned char> bytea, int from, int to) {

	for (size_t i = 0; i < bytea.size(); i++)
	{
		array.push_back(bytea[i]);
	}

	return true;
};