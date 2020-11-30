#pragma once

#include <string>

namespace BlockNDN {

	class RSAUtil
	{
        public:
            char* signMessage(std::string rsaprivateKey, std::string plainText);
            bool verifySignature(std::string rsapublicKey, std::string plainText, char* signatureBase64);
    };
}
