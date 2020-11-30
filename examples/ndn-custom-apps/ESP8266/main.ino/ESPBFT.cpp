#include "ESPBFT.hpp"
#include <string>

using namespace ESPBlockNDN;

//default constructor
BFT::BFT(){};

//pre-prepare request
BFT::BFT(int v, int n, std::string h, std::string rs){
    this->hash = h;
    this->view = v;
    this->requestNumber = n;
    this->requestersSignature = rs;
};
