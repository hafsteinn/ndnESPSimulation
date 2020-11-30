#ifndef BFT_H_
#define BFT_H_

#include <vector>
#include <string>

namespace BlockNDN
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
        // BFT(int v, int n, std::string h); //prepare phase
	};

}

#endif