#ifndef ESPBFT_H_
#define ESPBFT_H_

#include <vector>
#include <string>

namespace ESPBlockNDN
{
	class BFT
	{
	private:

	public:
        int view;
        int requestNumber;
        std::string hash;
        std::string requestersSignature;
        BFT();
        BFT(int v, int n, std::string h, std::string rs); //pre-prepare phase
	};

}

#endif