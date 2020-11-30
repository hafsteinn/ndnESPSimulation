/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

// custom-app.cpp

#include "custom-app-consumertwo.hpp"

#include <ndn-cxx/encoding/buffer-stream.hpp>

#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"

#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"

#include "ns3/random-variable-stream.h"

//Blockchain includes
#include "BlockChainProxy.hpp"
#include "Block.hpp"
#include "BFT.hpp"

#include <chrono>
#include <thread>



NS_LOG_COMPONENT_DEFINE("ConsumerTwoApp");

using namespace BlockNDN;

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(ConsumerTwoApp);

// register NS-3 type
TypeId
ConsumerTwoApp::GetTypeId()
{
  static TypeId tid = TypeId("ConsumerTwoApp")
  .SetParent<ndn::App>()
  .AddConstructor<ConsumerTwoApp>();
  return tid;
}

// Processing upon start of the application
void
ConsumerTwoApp::StartApplication()
{
  this->myName = "ConsumerTwoApp";


  NS_LOG_DEBUG("[" + this->myName + "]: STARTING....");

  // initialize ndn::App
  ndn::App::StartApplication();

  // Add entry to FIB for `/prefix/sub`
  ndn::FibHelper::AddRoute(GetNode(), "/prefix/sub", m_face, 0);


  //Set the current View
  this->currentView = 1;
  this->numberOfRequest = 1;
  //is this the primary node
  this->isPrimary = false;

  //configure the quorum
  int totalNumberOfNodes = 5;//                                 <----only change this
  int maxFaultyNodes = floor((totalNumberOfNodes-1)/3);
  //quorum needed for BFT floor((n-1)/3)
  this->quorum = totalNumberOfNodes - maxFaultyNodes;

  //Read something from ESP8266
  //SerialWrite();

  //When we created blockChainProxy, the genesisblock was created and added to our local blockchain
  //we start by initialising the hash value to the genesisblock hash value
  this->hash = this->blockChainProxy.getPreviousHash();

  //reguest sensor data
  //Simulator::Schedule(Seconds(40),&ConsumerTwoApp::requestStaticSensorData, this);

  //enable this to print the entire blockchain at the end of the simulation
  //Simulator::Schedule(Seconds(59),&ConsumerTwoApp::PrintLocalBlockchain, this);
  //Simulator::Schedule(Seconds(59),&ConsumerTwoApp::PrintPrepareLog, this);
}


// Processing when application is stopped
void
ConsumerTwoApp::StopApplication()
{

  // cleanup ndn::App
  ndn::App::StopApplication();

}

// Callback that will be called when Interest arrives
void
ConsumerTwoApp::OnInterest(std::shared_ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest(interest);

  //NS_LOG_DEBUG("[" + this->myName + "]: Received Interest packet for " << interest->getName());

  //check if the interest name is to initiate a voting process
  std::string prefix(interest->getName().get(2).toUri());
  std::string prefix2(interest->getName().get(3).toUri());
  
  //NS_LOG_DEBUG("[ConsumerTwoApp]: Received Interest packet for " << prefix.substr(0,10));
  
  if(prefix == "sync")
  {
    //get the hash value from the url
    std::string reqHash(interest->getName().get(3).toUri());

    //Get the hash value of the highest block in the local Blockchain
    std::vector<unsigned char> highestHashVector = this->blockChainProxy.getPreviousHash(); 
    std::string localHighestHash(highestHashVector.begin(), highestHashVector.end()); //convert to string

    if(reqHash != localHighestHash)
    {
      NS_LOG_DEBUG("[" + this->myName + "]: Im in sync and the hashes match....**********************************************************************************");

      int localBlockchainSize = blockChainProxy.getBlockHeight();
      int latestBlockIndex;
      std::vector<uint8_t> returnVector;
      bool foundBlock = false;

      //find the position of request Hash in the local Blockchain IF it exists
      for (size_t i = 0; i < localBlockchainSize; i++)
      {
        if(util.vectorToString(util.digestsha256mix(blockChainProxy.getBlockAt(i))) == reqHash)
        {
          int latestBlockIndex = i;
          foundBlock = true;
        }
      }

      //we have the requested hash in our Blockchain and its not the highest one
      if (foundBlock)
      {
        //collect all blocks that are "higher" than the requested hash
        for (int j = latestBlockIndex+1; j < localBlockchainSize; j++)
        {
          //create data packets and send
          Block bl = blockChainProxy.getBlockAt(j);
          
          returnVector = util.blockToByteArray(bl);

          //send a data packet which includes an array of blocks
          auto data = std::make_shared<ndn::Data>(interest->getName());
          //Add the array to the data packets content
          data->setContent(reinterpret_cast<const uint8_t*>(&returnVector.front()), returnVector.size()); //set the content
          //sign the packet
          ndn::StackHelper::getKeyChain().sign(*data);
          //log it
          NS_LOG_DEBUG("[" + this->myName + "]: Sending SYNC Data packet for " << data->getName());
          // Call trace (for logging purposes)
          m_transmittedDatas(data, this, m_face);

          m_appLink->onReceiveData(*data);
        }
      }        
    }
  }
  if(prefix == "preprepare")
  {
    //The primary node has started the preprepare phase (it wants to add a block to the blockchain)
    //send an interest back and ask for the Block itself
    std::string cview(interest->getName().get(3).toUri());
    std::string cnumber(interest->getName().get(4).toUri());
    std::string newBlockHash(interest->getName().get(5).toUri()) ;
    //log the preprepare request for future validation

    bool first = true;
    /*
      A node should only accept a pre-prepare message if it has not received another
      request with the same v and same n but different block hash
    */
    for (size_t i = 0; i < this->preprepareRequest.size(); i++)
    {
      if (this->preprepareRequest[i].view == std::stoi(cview) && this->preprepareRequest[i].requestNumber == std::stoi(cnumber) && this->preprepareRequest[i].hash == newBlockHash)
      {
        first = false;
      }
    }

    if(first)
    {
      clearLocalData();
      
      //set the current request number
      this->numberOfRequest = std::stoi(cnumber);

      BFT bft(std::stoi(cview),std::stoi(cnumber),newBlockHash,"PRIMARY_SIGNATURE");
      this->preprepareRequest.push_back(bft);

      //Create an interest with the block hash (to ask for the block itself)
      auto interest = std::make_shared<ndn::Interest>("/prefix/sub/primary/getblock/" + newBlockHash + "/" + this->myName);
      Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
      interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
      //interest->setInterestLifetime(ndn::time::seconds(1));

      NS_LOG_DEBUG("[" + this->myName + "]: I logged the pre-prepare request and sent an interest asking for the block: " << *interest);
  
      // Call trace (for logging purposes)
      m_transmittedInterests(interest, this, m_face);

      m_appLink->onReceiveInterest(*interest);
    }
  }
  else if (prefix == "prepare")
  {
    //A node has issued a prepare interest

    //add this prepare message to our local prepare log
    std::string cview(interest->getName().get(3).toUri());
    std::string cnumber(interest->getName().get(4).toUri());
    std::string newBlockHash(interest->getName().get(5).toUri());
    std::string userID(interest->getName().get(6).toUri());
    

    //retreive senders signature
    // const ndn::Signature& sig = data->getSignature();
    // const ndn::Block& ssig = sig.getValue();
    // const unsigned char* sssig = ssig.value();
    // std::string aa = std::string(reinterpret_cast<const char*>(sssig));
    // std::vector<unsigned char> si = util.digestsha256mix(aa);
    // std::string sigReady(si.begin(),si.end());

    //get the name of the interest in order to figure out who is sending it
    // const ndn::Name n = interest->getName();
    // std::string sn = n.toUri();
    // std::cout << "HELLO, My name is: " << sn << std::endl;

    BFT bft(std::stoi(cview),std::stoi(cnumber),newBlockHash,userID);
    this->prepareLog.push_back(bft);

    NS_LOG_DEBUG("[" + this->myName + "]: I received a prepare from " << userID << " and logged it.");

    bool consensus = checkPrepareConsensus();

    if(consensus)
    {
      //consensus has been reached!
      //start Commit phase by issuing a commit interest
      auto interest = std::make_shared<ndn::Interest>("/prefix/sub/commit/" + std::to_string(this->currentView) + "/" + std::to_string(this->numberOfRequest) + "/" + this->myName);
      Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
      interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
      //interest->setInterestLifetime(ndn::time::seconds(1));

      //add the commit to my own log
      BFT bft(std::stoi(cview),std::stoi(cnumber),newBlockHash,this->myName);
      this->commitLog.push_back(bft);

      NS_LOG_DEBUG("[" + this->myName + "]: I just sent a COMMIT interest and logged my own commit to my commit log: " << *interest);
      // Call trace (for logging purposes)
      m_transmittedInterests(interest, this, m_face);

      m_appLink->onReceiveInterest(*interest);
    }


  }
  else if(prefix == "commit")
  {
    //we just received a COMMIT interest
    //we should
      //add a log to our commit log
      //check if we have enough commit logs
        //if false
          //do nothing
        //if true
          //add the block to our blockchain
          //clear all local data from last process

      std::string cview(interest->getName().get(3).toUri());
      std::string cnumber(interest->getName().get(4).toUri());
      std::string user(interest->getName().get(5).toUri());
    
      BFT bft(std::stoi(cview),std::stoi(cnumber),"",user);
      this->commitLog.push_back(bft);

      int quorum = 0;
      bool commit = false;

      for (size_t i = 0; i < this->commitLog.size(); i++)
      {
        if(this->commitLog[i].view == this->currentView && this->commitLog[i].requestNumber == this->numberOfRequest)
        {
          quorum++; 
        }

        if(quorum >= this->quorum)
        {
          commit = true;
        }
      }

      if(commit)
      {
        //add the block!
        this->blockChainProxy.addBlock(this->currentBlockInProcess);
        //clear variables
        clearLocalData();

        NS_LOG_DEBUG("[" + this->myName + "]: COMMIT quorum reached, adding block...");
      }
  }
  
}

bool ConsumerTwoApp::checkPrepareConsensus(){
  int quorum = 0;
  bool consensus = false;

  for (size_t i = 0; i < this->prepareLog.size(); i++)
  {
    std::string currentBlock = this->preprepareRequest.back().hash;
    int currentView = this->preprepareRequest.back().view;
    int currentRequestNumber = this->preprepareRequest.back().requestNumber;

    if(this->prepareLog[i].hash == currentBlock && this->prepareLog[i].view == currentView && this->prepareLog[i].requestNumber == currentRequestNumber)
    {
    quorum++; 
    }

    if(quorum >= this->quorum)
    {
      consensus = true;
    }
  }

  return consensus;
}






void
ConsumerTwoApp::PrintLocalBlockchain()
{
  this->blockChainProxy.PrintBlockChainToScreen(this->myName);
};

void
ConsumerTwoApp::PrintPrepareLog()
{
    std::cout << "********************************************************" << std::endl;
    std::cout << "****************" << this->myName << "********************************" << std::endl;
    std::cout << "********************************************************" << std::endl;
    std::cout << "PRE-PREPARE Log Information" << std::endl;
    std::cout << "HASH: " << this->preprepareRequest.back().hash << std::endl;
    std::cout << "VIEW: " << this->preprepareRequest.back().view << std::endl;
    std::cout << "NUMBER: " << this->preprepareRequest.back().requestNumber << std::endl;
    std::cout << "USER: " << this->preprepareRequest.back().requestersSignature << std::endl;
    std::cout << "********************************************************" << std::endl;
    std::cout << "********************************************************" << std::endl;
    std::cout << "PREPARE LOG LENGTH: " << this->prepareLog.size() << std::endl;
    std::cout << "COMMIT LOG LENGTH: " << this->commitLog.size() << std::endl;
    std::cout << "BLOCKCHAIN LENGTH: " << this->blockChainProxy.getBlockHeight() << std::endl;
    std::cout << "********************************************************" << std::endl;
    
    // for (size_t i = 0; i < this->prepareLog.size(); i++)
    // {
    //   std::cout << "HASH: " << this->prepareLog[i].hash << std::endl;
    //   std::cout << "VIEW: " << this->prepareLog[i].view << std::endl;
    //   std::cout << "NUMBER: " << this->prepareLog[i].requestNumber << std::endl;
    //   std::cout << "USER: " << this->prepareLog[i].requestersSignature << std::endl;
    //   std::cout << "------" << std::endl;
    // }
    // std::cout << "*************************END************************" << std::endl;
}

void ConsumerTwoApp::clearLocalData()
{
  Block empty;
  BFT bft;
  this->currentBlockInProcess = empty;
  this->commitLog.clear();
  this->newBlockHash.clear();
  this->prepareLog.clear(); 
}




// Callback that will be called when Data arrives
void
ConsumerTwoApp::OnData(std::shared_ptr<const ndn::Data> data)
{
//someone is sending us a data packet with a block that needs to be verified
  NS_LOG_DEBUG("[" + this->myName + "]: Received data packet");

  //Create a ndn Block object to store the content
  const ndn::Block& block = data->getContent();
  //create a pointer that points to the start of the value of the content
  const unsigned char* ddata = block.value();
  //Get the length of the content (number of bytes)
  size_t lengthOfContent = block.value_size();
  //store a copy of this block
  this->currentBlockInProcess = util.unpackDataPacket(ddata, lengthOfContent);

  //create the blocks hash digest
  std::vector<unsigned char> hash = util.digestsha256mix(this->currentBlockInProcess);
  std::string hashString(hash.begin(),hash.end());


  //is the block legit?
  if (this->blockChainProxy.getPreviousHash() == this->currentBlockInProcess.prevBlock)
  {
    
    //send out an interest saying that you vote YES to adding the block
    //Create an interest with the block hash (to ask for the block itself)
    auto interest = std::make_shared<ndn::Interest>("/prefix/sub/prepare/" + std::to_string(this->currentView) + "/" + std::to_string(this->numberOfRequest) + "/" + hashString + "/" + this->myName);
    Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
    interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
    //interest->setInterestLifetime(ndn::time::seconds(1))

    //Add your own prepare statement to the log
    BFT bft(this->currentView,this->numberOfRequest,hashString,this->myName);
    this->prepareLog.push_back(bft);


    //Add the block producer (PRIMARY) to the prepare log
    // BFT bftprim(this->currentView,this->numberOfRequest,hashString,"PRIMARY_SIG");
    // this->prepareLog.push_back(bftprim);

    NS_LOG_DEBUG("[" + this->myName + "]: I logged the prepare request and sent out a prepare interest for others to log.");
 
    // Call trace (for logging purposes)
    m_transmittedInterests(interest, this, m_face);

    m_appLink->onReceiveInterest(*interest);




    bool consensus = checkPrepareConsensus();
    if(consensus)
    {
      //consensus has been reached!
      //start Commit phase by issuing a commit interest
      auto interest = std::make_shared<ndn::Interest>("/prefix/sub/commit/" + std::to_string(this->currentView) + "/" + std::to_string(this->numberOfRequest) + "/" + this->myName);
      Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
      interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
      //interest->setInterestLifetime(ndn::time::seconds(1));

      //add the commit to my own log
      BFT bft(this->currentView,this->numberOfRequest,hashString,this->myName);
      this->commitLog.push_back(bft);

      NS_LOG_DEBUG("[" + this->myName + "]: I just sent a COMMIT interest and logged my own commit to my commit log: " << *interest);
      // Call trace (for logging purposes)
      m_transmittedInterests(interest, this, m_face);

      m_appLink->onReceiveInterest(*interest);
    }
  }
 
  
}

} // namespace ns3
