#ifndef ESPUTILS_H_
#define ESPUTILS_H_

#include <vector>
#include "ESPBlock.hpp"

namespace ESPBlockNDN
{
	class Utils
	{
	private:

	public:
		std::string vectorToString(std::vector<unsigned char> vin);
		std::vector<unsigned char> digestsha256(Block block);
		std::vector<unsigned char> digestsha256(std::string s);
		std::vector<unsigned char> digestsha256mix(Block block);
		std::vector<unsigned char> blockToByteArray(Block newBlock);
		Block unpackDataPacket(const unsigned char *in, size_t length);
		int bytesToInt(std::vector<unsigned char> vin);
		long bytesToLong(std::vector<unsigned char> vin);
		std::vector<unsigned char> intToBytes(int data);
		std::vector<unsigned char> longToBytes(long data);
		bool byteArrayCopy(std::vector<unsigned char> &array, std::vector<unsigned char> bytea, int from, int to);
		std::string getPrimaryPublicKey();
	};
}

#endif