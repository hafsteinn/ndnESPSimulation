#pragma once

#include "Block.hpp"

namespace BlockNDN
{

	class CheckBlock
	{
	public:
		bool isBlockLegal(Block block, std::vector<unsigned char> prevBlockHash);
	};
}


