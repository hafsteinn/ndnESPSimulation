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

#include "custom-app-primarynode.hpp"

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




NS_LOG_COMPONENT_DEFINE("PrimaryNode");

using namespace BlockNDN;


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(PrimaryNode);

// register NS-3 type
TypeId
PrimaryNode::GetTypeId()
{
  static TypeId tid = TypeId("PrimaryNode").SetParent<ndn::App>().AddConstructor<PrimaryNode>();
  return tid;
}

// Processing upon start of the application
void
PrimaryNode::StartApplication()
{
  this->myName = "PrimaryNode";
  NS_LOG_DEBUG("[" + this->myName + "]: STARTING....");

  // initialize ndn::App
  ndn::App::StartApplication();

  // Add entry to FIB for `/prefix/sub`
  ndn::FibHelper::AddRoute(GetNode(), "/prefix/sub", m_face, 0);
  
  //Set the current View
  this->currentView = 1;
  this->numberOfRequest = 0;

  //is this the primary node?
  this->isPrimary = true;

  //configure the quorum
  int totalNumberOfNodes = 5;//                                 <----only change this
  int maxFaultyNodes = floor((totalNumberOfNodes-1)/3);
  //quorum needed for BFT floor((n-1)/3)
  this->quorum = totalNumberOfNodes - maxFaultyNodes;

  //Number of blocks to create
  this->blocksToCreate = 0; //The primary will create blocksToCreate + 2

  //When we created blockChainProxy, the genesisblock was created and added to our local blockchain
  //we start by initialising the hash value to the genesisblock hash value
  this->newBlockHash = this->blockChainProxy.getPreviousHash();

  // Schedule an event that will create and broadcast one block
  // When this is finished, the process will immediately schedule another event like this one
  Simulator::Schedule(Seconds(1),&PrimaryNode::ConfigurationPhase, this);
   

  //enable this to print the entire blockchain at the end of the simulation
  //Simulator::Schedule(Seconds(58),&PrimaryNode::PrintPrepareLog, this);
}


void PrimaryNode::ConfigurationPhase(){
  
  //we currently have the following nodes in our network
    //Primary
    //ESPGhost
    //ConsumerApp
    //ConsumerOneApp
    //ConsumerTwoApp
    //ConsumerThreeApp

  //pre install Primary Public Key on all nodes
  //The public key of the Primary node is accessible to all nodes through the Utils class

  //Create Certificates and broadcast to Blockchain
  Simulator::Schedule(Seconds(1),&PrimaryNode::ConfigureESPGhostNode, this);
  Simulator::Schedule(Seconds(3),&PrimaryNode::ConfigureConsumerOneNode, this);
  Simulator::Schedule(Seconds(5),&PrimaryNode::ConfigureConsumerTwoNode, this);
  Simulator::Schedule(Seconds(7),&PrimaryNode::ConfigureConsumerThreeNode, this);
  Simulator::Schedule(Seconds(9),&PrimaryNode::ConfigureProducerNode, this);
  //register IKCB rules
  Simulator::Schedule(Seconds(11),&PrimaryNode::ProducerAppIKCB, this);
  Simulator::Schedule(Seconds(12),&PrimaryNode::ProducerAppPrefixRegistration, this);
}

//The primary node uses this function to add a new block
//The function should create an interest on the prefix "/prefix/sub/preprepare/{v}/{n}/{newblockhash}"
//Where newblockhash is the hash of the newly created block that this application wants to append to the blockchain
void PrimaryNode::initiateBlockAdd(){

  // Create and configure ndn::Interest
  std::string newhash(this->newBlockHash.begin(),this->newBlockHash.end());
  auto interest = std::make_shared<ndn::Interest>("/prefix/sub/preprepare/" + std::to_string(this->currentView) + "/" + std::to_string(this->numberOfRequest) + "/" + newhash);
  //std::cout << "PRIMARY SEND -> VIEW: " << this->currentView << " and NUMBER: " << this->numberOfRequest << " and HASH: " << newhash << std::endl;
  Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
  interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  //interest->setInterestLifetime(ndn::time::seconds(1));

  //add pre-prepare to log
  //log the preprepare request for future validation
  BFT bft(this->currentView,this->numberOfRequest,newhash,this->myName);
  this->preprepareRequest.push_back(bft);

  NS_LOG_DEBUG("[" + this->myName + "]: Initiated a block add process and added pre-prepare to log: " << *interest);

  // Call trace (for logging purposes)
  m_transmittedInterests(interest, this, m_face);

  m_appLink->onReceiveInterest(*interest);


  //add yourself to the prepare log
  BFT bftprepare(this->currentView,this->numberOfRequest,newhash,this->myName);
  this->prepareLog.push_back(bftprepare);

  //checkPrepareConsensus();


  //also send out a prepare Interest for the current block
  //send out an interest saying that you vote YES to adding the block
  //Create an interest with the block hash (to ask for the block itself)
  auto interestp = std::make_shared<ndn::Interest>("/prefix/sub/prepare/" + std::to_string(this->currentView) + "/" + std::to_string(this->numberOfRequest) + "/" + util.vectorToString(util.digestsha256mix(this->currentBlockInProcess)) + "/" + this->myName);
  Ptr<UniformRandomVariable> randp = CreateObject<UniformRandomVariable>();
  interestp->setNonce(randp->GetValue(0, std::numeric_limits<uint32_t>::max()));
  //interest->setInterestLifetime(ndn::time::seconds(1))

  NS_LOG_DEBUG("[" + this->myName + "]: I sent out the preprepare interest and a prepare interest for others to log.");

  // Call trace (for logging purposes)
  m_transmittedInterests(interestp, this, m_face);

  m_appLink->onReceiveInterest(*interestp);

}


void PrimaryNode::createNewBlock(std::string transaction)
{
  //Create a new Block
	long time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch()).count();
	long nonce = rand() % 10000 + 1;
  std::string sint = std::to_string(nonce);
	std::string s = transaction;
  Block block = Block(this->blockChainProxy.getPreviousHash(), time, nonce, s);

//  NS_LOG_DEBUG("[" + this->myName + "]: The genesis hash is: " + util.vectorToString(block.prevBlock));

  //set the newblockhash variable to the newly created block hash
  this->newBlockHash = util.digestsha256mix(block);

  //increment the value of n by one
  this->numberOfRequest++;

  //store the block locally while it is getting voted on
  this->currentBlockInProcess = block;

  //initiate voting for the newly created block
  Simulator::ScheduleNow(&PrimaryNode::initiateBlockAdd, this);
}

// Processing when application is stopped
void
PrimaryNode::StopApplication()
{
  // cleanup ndn::App
  ndn::App::StopApplication();
}

// Callback that will be called when Interest arrives
void
PrimaryNode::OnInterest(std::shared_ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest(interest);

  //NS_LOG_DEBUG("[" + this->myName + "]: Received Interest packet for " << interest->getName());

  //check if the interest name is to initiate a voting process
  std::string prefix(interest->getName().get(2).toUri());
  std::string prefix2(interest->getName().get(3).toUri());
  
  //someone is asking for the block
  if (prefix == "primary" && prefix2 == "getblock")
  {

    if(this->isPrimary)
    {
      //create a data object with the current block as the content
      std::string temp(interest->getName().get(4).toUri());
      std::vector<unsigned char> v(temp.begin(),temp.end());

      if(util.digestsha256mix(this->currentBlockInProcess) == v)
      {
        //send a data packet which include the block that you want to vote on
        auto data = std::make_shared<ndn::Data>(interest->getName());
        //Add the block that we found to the data packets content
        std::vector<uint8_t> b = util.blockToByteArray(this->currentBlockInProcess); //create a vector array from the block
        data->setContent(reinterpret_cast<const uint8_t*>(&b.front()), b.size()); //set the content
        //sign the packet
        ndn::StackHelper::getKeyChain().sign(*data);

        //log it
        NS_LOG_DEBUG("[" + this->myName + "]: Sending Data packet for " << data->getName());
        // Call trace (for logging purposes)
        m_transmittedDatas(data, this, m_face);
        //TODO: what does this do?
        m_appLink->onReceiveData(*data);

        
      } 
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

    //add the requester to the prepare log
    BFT bft(std::stoi(cview),std::stoi(cnumber),newBlockHash,userID);
    this->prepareLog.push_back(bft);

    NS_LOG_DEBUG("[" + this->myName + "]: I received a prepare from " << userID << " and logged it.");

    //each time we receive a prepare message, we should check if we have a quorum

    //We need to check if we have both the Pre-prepare and 2f matching prepare messages
    //for the same v , n and BlockHash

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
      BFT bft(std::stoi(cview),std::stoi(cnumber),newBlockHash,"PRIMARY_SIGNATURE");
      this->commitLog.push_back(bft);

      NS_LOG_DEBUG("[" + this->myName + "]: I just sent a COMMIT interest and logged my own commit to my commit log: " << *interest);
      // Call trace (for logging purposes)
      m_transmittedInterests(interest, this, m_face);

      m_appLink->onReceiveInterest(*interest);
    }

  }
  else if(prefix == "commit")
  {

    std::string cview(interest->getName().get(3).toUri());
    std::string cnumber(interest->getName().get(4).toUri());
    std::string user(interest->getName().get(5).toUri());
    std::cout << "PRIMARY: Received COMMIT from " << user << std::endl;
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
      std::cout << "PRIMARY: quorumCounter is: " << quorum << " and myQuorum is: " << this->quorum << " So I am adding the block!" << std::endl;
      //add the block!
      this->blockChainProxy.addBlock(this->currentBlockInProcess);
      //clear variables
      clearLocalData();
      
      NS_LOG_DEBUG("[" + this->myName + "]: COMMIT quorum reached, adding block...");

      // if(this->blockCounter < this->blocksToCreate)
      // {
      //   //std::this_thread::sleep_for(std::chrono::milliseconds(50));
      //   Simulator::ScheduleNow(&PrimaryNode::createNewBlock, this);
      //   this->blockCounter++;
      // }
      
    }
  }
  
}

bool PrimaryNode::checkPrepareConsensus(){
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
PrimaryNode::PrintLocalBlockchain()
{
  this->blockChainProxy.PrintBlockChainToScreen(this->myName);
};

void
PrimaryNode::PrintPrepareLog()
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

void PrimaryNode::clearLocalData()
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
PrimaryNode::OnData(std::shared_ptr<const ndn::Data> data)
{
  
}




/********Configuration phase functions**********/
void PrimaryNode::ConfigureESPGhostNode(){
  //Create a CERTIFICATE for ESPGhost
  std::string PublicKey = keyStore.getESPGhostPublicKey();
  std::vector<unsigned char> PPKD_V = util.digestsha256(PublicKey);
  std::string PPKD(PPKD_V.begin(),PPKD_V.end());
  std::string SIG = rsaUtil.signMessage(keyStore.getPrimaryPrivateKey(), PublicKey);

  std::string transaction = "<CERTIFICATE(ESP),";
  transaction.append(PublicKey); //public key of the entity
  transaction.append(",");
  transaction.append(PPKD); //PublisherPublicKeyDigest (PPKD)
  transaction.append(",");
  transaction.append(SIG); //Primary Signature
  transaction.append(">");

  createNewBlock(transaction);
};

void PrimaryNode::ConfigureConsumerOneNode(){
  //Create a CERTIFICATE for ESPGhost
  std::string PublicKey = keyStore.getConsumerOnePublicKey();
  std::vector<unsigned char> PPKD_V = util.digestsha256(PublicKey);
  std::string PPKD(PPKD_V.begin(),PPKD_V.end());
  std::string SIG = rsaUtil.signMessage(keyStore.getPrimaryPrivateKey(), PublicKey);

  std::string transaction = "<CERTIFICATE(C1),";
  transaction.append(PublicKey); //public key of the entity
  transaction.append(",");
  transaction.append(PPKD); //PublisherPublicKeyDigest (PPKD)
  transaction.append(",");
  transaction.append(SIG); //Primary Signature
  transaction.append(">");

  createNewBlock(transaction);
};

void PrimaryNode::ConfigureConsumerTwoNode(){
  //Create a CERTIFICATE for ESPGhost
  std::string PublicKey = keyStore.getConsumerTwoPublicKey();
  std::vector<unsigned char> PPKD_V = util.digestsha256(PublicKey);
  std::string PPKD(PPKD_V.begin(),PPKD_V.end());
  std::string SIG = rsaUtil.signMessage(keyStore.getPrimaryPrivateKey(), PublicKey);

  std::string transaction = "<CERTIFICATE(C2),";
  transaction.append(PublicKey); //public key of the entity
  transaction.append(",");
  transaction.append(PPKD); //PublisherPublicKeyDigest (PPKD)
  transaction.append(",");
  transaction.append(SIG); //Primary Signature
  transaction.append(">");

  createNewBlock(transaction);
};

void PrimaryNode::ConfigureConsumerThreeNode(){
  //Create a CERTIFICATE for ESPGhost
  std::string PublicKey = keyStore.getConsumerThreePublicKey();
  std::vector<unsigned char> PPKD_V = util.digestsha256(PublicKey);
  std::string PPKD(PPKD_V.begin(),PPKD_V.end());
  std::string SIG = rsaUtil.signMessage(keyStore.getPrimaryPrivateKey(), PublicKey);

  std::string transaction = "<CERTIFICATE(C3),";
  transaction.append(PublicKey); //public key of the entity
  transaction.append(",");
  transaction.append(PPKD); //PublisherPublicKeyDigest (PPKD)
  transaction.append(",");
  transaction.append(SIG); //Primary Signature
  transaction.append(">");

  createNewBlock(transaction);
};

void PrimaryNode::ConfigureProducerNode(){
  //Create a CERTIFICATE for ESPGhost
  std::string PublicKey = keyStore.getProducerPublicKey();
  std::vector<unsigned char> PPKD_V = util.digestsha256(PublicKey);
  std::string PPKD(PPKD_V.begin(),PPKD_V.end());
  std::string SIG = rsaUtil.signMessage(keyStore.getPrimaryPrivateKey(), PublicKey);

  std::string transaction = "<CERTIFICATE(Pro),";
  transaction.append(PublicKey); //public key of the entity
  transaction.append(",");
  transaction.append(PPKD); //PublisherPublicKeyDigest (PPKD)
  transaction.append(",");
  transaction.append(SIG); //Primary Signature
  transaction.append(">");

  createNewBlock(transaction);
};

void PrimaryNode::ProducerAppIKCB(){
  //Create a CERTIFICATE for ESPGhost
  std::string PublicKey = keyStore.getProducerPublicKey();
  std::vector<unsigned char> PPKD_V = util.digestsha256(PublicKey);
  std::string PPKD(PPKD_V.begin(),PPKD_V.end());
  std::string SIG = rsaUtil.signMessage(keyStore.getProducerPrivateKey(), PublicKey);

  std::vector<unsigned char> content = util.digestsha256("This is a sensor reading content");
  std::string content_digest(content.begin(),content.end());
  

  std::string transaction = "<REGISTER,";
  transaction.append("sensordata"); //content_name
  transaction.append(",");
  transaction.append(PPKD); //PublisherPublicKeyDigest (PPKD)
  transaction.append(",");
  transaction.append(content_digest); //content_digest
  transaction.append(",");
  transaction.append(SIG); //Producer Signature
  transaction.append(">");

  createNewBlock(transaction);
}


void PrimaryNode::ProducerAppPrefixRegistration(){
  //Create a CERTIFICATE for ESPGhost
  std::string PublicKey = keyStore.getProducerPublicKey();
  std::vector<unsigned char> PPKD_V = util.digestsha256(PublicKey);
  std::string PPKD(PPKD_V.begin(),PPKD_V.end());
  std::string SIG = rsaUtil.signMessage(keyStore.getProducerPrivateKey(), PublicKey);

  std::vector<unsigned char> content = util.digestsha256("This is a sensor reading content");
  std::string content_digest(content.begin(),content.end());
  

  std::string transaction = "<REGISTER,";
  transaction.append("/prefix/sub/sensordata"); //content_name
  transaction.append(",");
  transaction.append(PPKD); //PublisherPublicKeyDigest (PPKD)
  transaction.append(",");
  transaction.append(content_digest); //content_digest
  transaction.append(",");
  transaction.append(SIG); //Producer Signature
  transaction.append(">");

  createNewBlock(transaction);
}

} // namespace ns3
