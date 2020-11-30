#pragma once
#include <string>
#include "Block.hpp"
#include "Configure.hpp"
#include "Utils.hpp"

namespace BlockNDN
{
	class CheckBlockUtils
	{
		Configure conf;
		Utils util;

	public:
		bool checkData(std::string s);
		bool checkBlockSize(Block block);
		bool checkPrevHash(std::vector<unsigned char> prevHash, Block block);
		bool NrawCheckProofOfWork(Block block);
	};
}


