#include "BFT.hpp"
#include <string>
#include <iostream>

using namespace BlockNDN;

//default constructor
BFT::BFT(){};

//pre-prepare request
BFT::BFT(int v, int n, std::string h, std::string rs){
    this->hash = h;
    this->view = v;
    this->requestNumber = n;
    this->requestersSignature = rs;
};

// //prepare request
// BFT::BFT(int v, int n, std::string h){
//     this->hash = h;
//     this->view = v;
//     this->requestNumber = n;
// };