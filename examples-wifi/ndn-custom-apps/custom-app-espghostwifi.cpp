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
#include "custom-app-espghostwifi.hpp"

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
#include <ctime>




NS_LOG_COMPONENT_DEFINE("ESPGhostWIFI");

using namespace BlockNDN;

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(ESPGhostWIFI);

// register NS-3 type
TypeId
ESPGhostWIFI::GetTypeId()
{
  static TypeId tid = TypeId("ESPGhostWIFI").SetParent<ndn::App>().AddConstructor<ESPGhostWIFI>();
  return tid;
}

// Processing upon start of the application
void
ESPGhostWIFI::StartApplication()
{
  
  
	try {
    ip::udp::socket socket(this->io_service);
    auto remote = ip::udp::endpoint(ip::address::from_string("192.168.86.43"), 4210);
		socket.open(boost::asio::ip::udp::v4());
 
		// send the message
		socket.send_to(buffer("dsf"), remote);

    //wait for a reply
    NS_LOG_DEBUG("[" + this->myName + "]: UDP SENT....");
	
	} catch (const boost::system::system_error& ex) {
		// Exception thrown!
		// Examine ex.code() and ex.what() to see what went wrong!
		NS_LOG_DEBUG("[" + this->myName + "]: UDP ERROR....");
	}



  // this->ESPLogFile.open("ESPTimerLog.txt");
  // this->ESPLogFile << "Interest " << "ESPGETSENSOR " << "Data " << "ESPVERIFYSENSOR" << "\n";

  // this->myName = "ESPGhostWIFI";

  // NS_LOG_DEBUG("[" + this->myName + "]: STARTING....");

  // // initialize ndn::App
  // ndn::App::StartApplication();

  // // Add entry to FIB for `/prefix/sub`
  // ndn::FibHelper::AddRoute(GetNode(), "/prefix/sub", m_face, 0);
  
  // //The total number of seconds that the ESPGhostWIFI node should keep asking for sensor data
  // this->totalSimulationRunTime = 40.0;
  // //we cannot start requesting sensor packets until AFTER the initial configuration phase 1 and 2 are finished
  // int startSensorRequestsAt = 20; //we expect that phase one and two take less than 20 seconds
  // Simulator::Schedule(Seconds(startSensorRequestsAt),&ESPGhostWIFI::requestStaticSensorData, this);

  //Simulator::Schedule(Seconds(1.1),&ESPGhostWIFI::sync, this); //get in sync with the network
  //Simulator::Schedule(Seconds(59.9),&ESPGhostWIFI::PrintBlockchain, this);
}

void ESPGhostWIFI::requestStaticSensorData()
{  
  //write to log file  
  this->ESPLogFile << Simulator::Now ().ToDouble (Time::S) << "   ";

  //This function tells the ESP to "create" an interest for sensor data
  auto t1 = std::chrono::high_resolution_clock::now();
  std::string pr = ESPCommunicate(1,"getsensor","a","b","c","d");
  auto t2 = std::chrono::high_resolution_clock::now();

  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();

  //write to log file  
  this->ESPLogFile << duration << "   ";
  this->ESPget = this->ESPget + duration;

  std::cout << "ESPGhostWIFI:requestStaticSensorData: Time of ESP8266 execution: " << duration << std::endl;
  

  if(pr != "e")
  {
    std::string content_digest = "cedb72fa48c8f52983e3b0afea2d7b21908885a8fdc7ee2fdbd373412598d341";

    int v1 = rand() % 100000;
    //send the interest asking the producer for the sensor data
    auto interest = std::make_shared<ndn::Interest>("/prefix/sub/sensordata/" + content_digest + "/" + this->myName + "/" + std::to_string(v1));
    Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
    interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
    //interest->setInterestLifetime(ndn::time::seconds(1));

    NS_LOG_DEBUG("[" + this->myName + "]: I sent our an interest asking for the SENSOR DATA: " << *interest);

    // Call trace (for logging purposes)
    m_transmittedInterests(interest, this, m_face);

    m_appLink->onReceiveInterest(*interest);
  }

}

void ESPGhostWIFI::sync()
{
  //ask the ESP for the highest hash
  std::string ESPHighestHash = ESPCommunicate(64,"presync","","","","");
  std::cout << "THE HIGHEST HASH IS: " << ESPHighestHash.substr(0,64) << std::endl;

  //Create an interest with the block hash (to ask for the block itself)
  auto interest = std::make_shared<ndn::Interest>("/prefix/sub/sync/" + ESPHighestHash.substr(0,64));
  Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
  interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  // //interest->setInterestLifetime(ndn::time::seconds(1));

  NS_LOG_DEBUG("[" + this->myName + "]: I logged the pre-prepare request and sent an interest asking for the block: " << *interest);

  // Call trace (for logging purposes)
  m_transmittedInterests(interest, this, m_face);

  m_appLink->onReceiveInterest(*interest);
}

void
ESPGhostWIFI::PrintBlockchain()
{
  std::string pr = ESPCommunicate(1,"print","a","b","c","d");
};

// Processing when application is stopped
void
ESPGhostWIFI::StopApplication()
{
  // cleanup ndn::App
  ndn::App::StopApplication();
}


std::string 
ESPGhostWIFI::ESPCommunicate(int len, std::string prefix, std::string v, std::string n, std::string h, std::string i)
{
  std::string EspMessage;
  EspMessage.append(prefix);
  EspMessage.append(":");
  EspMessage.append(v);
  EspMessage.append(":");
  EspMessage.append(n);
  EspMessage.append(":");
  EspMessage.append(h);

  if(i != "")
  {
    EspMessage.append(":");
    EspMessage.append(i);
  }

  EspMessage.append("x");

  size_t EspMessageSize = EspMessage.length();

  char sPrefix[EspMessageSize + 1];
  strcpy(sPrefix, EspMessage.c_str());

  //Send to ESP
  sutil.SerialWrite(sPrefix, EspMessageSize);

  //Read from ESP
  std::string ret = sutil.SerialRead(len);
  //std::string ret = "success";
  //std::cout << "I read this from the ESP: " << ret << std::endl;

  return ret;
}

// Callback that will be called when Interest arrives
void
ESPGhostWIFI::OnInterest(std::shared_ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest(interest);

  //NS_LOG_DEBUG("[" + this->myName + "]: Received Interest packet for " << interest->getName());

  //get request function
  std::string prefix(interest->getName().get(2).toUri());
  

  if(prefix == "preprepare")
  {
    //dissect the url 
    std::string cview(interest->getName().get(3).toUri());
    std::string cnumber(interest->getName().get(4).toUri());
    std::string newBlockHash(interest->getName().get(5).toUri());

    this->numberOfRequest = std::stoi(cnumber);
    this->currentView = std::stoi(cview);

    std::string ret = ESPCommunicate(1,"preprepare", cview, cnumber, newBlockHash, "");
    //std::cout << "GHOST: PREPREPARE : view: " << cview << " number: " << cnumber << " Hash: " << newBlockHash << std::endl;
    NS_LOG_DEBUG("[" + this->myName + "]: Received preprepare from PrimaryNode");

    if (ret.substr(0,1) != "e")
    {

      /*
      
      if we get success from ESP, only create Interest and nothing else

      */

      //Create an interest with the block hash (to ask for the block itself)
      auto interest = std::make_shared<ndn::Interest>("/prefix/sub/primary/getblock/" + newBlockHash + "/" + this->myName);
      Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
      interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
      //interest->setInterestLifetime(ndn::time::seconds(1));

      NS_LOG_DEBUG("[" + this->myName + "]: PREPREPARE: Got success from ESP and sent an interest asking for the block: " << *interest);
  
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
    std::string userIdentification(interest->getName().get(6).toUri());
    

    //send info to ESPs
    std::string ret = ESPCommunicate(1,"prepare",cview,cnumber,newBlockHash,userIdentification);
    NS_LOG_DEBUG("[" + this->myName + "]: I received a prepare from " << userIdentification << " and logged it.");

    if(ret.substr(0,1) != "e")
    {

      auto interest = std::make_shared<ndn::Interest>("/prefix/sub/commit/" + cview + "/" + cnumber + "/" + this->myName);
      Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
      interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
      //interest->setInterestLifetime(ndn::time::seconds(1));

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

    NS_LOG_DEBUG("[" + this->myName + "]: COMMIT: Received commit interest from " << user );

    //send info to ESP
    std::string ret = ESPCommunicate(1,"commit",cview,cnumber,user,"");
    //std::cout << "COMMIT SIZE FROM ESP: " << ret << std::endl;
    if(ret.substr(0,1) != "e")
    {
      NS_LOG_DEBUG("[" + this->myName + "]: COMMIT: ESP successfully added a block");
    }
  
  }
  
}

// Callback that will be called when Data arrives
void
ESPGhostWIFI::OnData(std::shared_ptr<const ndn::Data> data)
{
  std::string nameOfData = data->getName().get(3).toUri();
  std::string sensor = data->getName().get(2).toUri();
  std::cout << "Simulation Progress:" << sensor << " - " << Simulator::Now ().ToDouble (Time::S) << "/" << this->totalSimulationRunTime << std::endl; 
  if(sensor == "sensordata")
  {
    //write to log file  
    this->ESPLogFile << Simulator::Now ().ToDouble (Time::S) << "   ";
    NS_LOG_DEBUG("[" + this->myName + "]: SENSOR DATA: Received data packet");

    //tell the ESP to validate the data packet
    std::string content_digest = "cedb72fa48c8f52983e3b0afea2d7b21908885a8fdc7ee2fdbd373412598d341";

    auto t1 = std::chrono::high_resolution_clock::now();
    std::string pr = ESPCommunicate(1,"verifysensor",content_digest,"b","c","d");
    auto t2 = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();
    //write to log file  
    this->ESPLogFile << duration << "   \n";
    this->ESPverify = this->ESPverify + duration;

    //The ESP should compare the content digest he sent (from the blockchain) and the content digest of the data it received

    if(pr != "e")
    {
      std::cout << "ESPGhostWIFI:OnData:sensordata: reply from ESP is : " << pr << std::endl;
      if(Simulator::Now ().ToDouble (Time::S) < this->totalSimulationRunTime)
      {
        //this->sensorRequestCounter++;
        //std::this_thread::sleep_for(std::chrono::milliseconds(50));
        //if we get a successful answer from the ESP, immediately schedule another "getsensor" event
        Simulator::ScheduleNow(&ESPGhostWIFI::requestStaticSensorData, this);
      }
      else{
        this->totalSim = Simulator::Now ().ToDouble (Time::S) - 20;
        this->ESPLogFile << "ESPget" << this->ESPget << "   \n";
        this->ESPLogFile << "ESPverify" << this->ESPverify << "   \n";
        this->ESPLogFile << "ESPTotal" << this->ESPget + this->ESPverify << "   \n";
        this->ESPLogFile << "TotalSimulation" << this->totalSim << "   \n";
        this->ESPLogFile.close();
      }
      
    }
  
  }
  else if (nameOfData == "getblock")
  {
    //someone is sending us a data packet with a block that needs to be verified
    NS_LOG_DEBUG("[" + this->myName + "]: GETBLOCK DATA: Received data packet");

    //Create a ndn Block object to store the content
    const ndn::Block& block = data->getContent();
    //create a pointer that points to the start of the value of the content
    const unsigned char* ddata = block.value();
    //Get the length of the content (number of bytes)
    size_t lengthOfContent = block.value_size();
    //char* reformattedData = reinterpret_cast<char*>(const_cast<unsigned char*>(ddata));
    //TODO: change this to be a local (scoped) Block object 
    this->currentBlockInProcess = util.unpackDataPacket(ddata, lengthOfContent);

    //Send to ESP
    std::string ret = ESPCommunicate(1,"data", util.vectorToString(this->currentBlockInProcess.prevBlock),std::to_string(this->currentBlockInProcess.nonce),this->currentBlockInProcess.s,std::to_string(this->currentBlockInProcess.time));
    //std::cout << "DATA : view: " << this->currentView << " number: " << this->numberOfRequest << "       ESP: " << ret.substr(0,1) << std::endl;    
    if (ret.substr(0,1) != "e")
    {

      /*
      
      If we get success from ESP, only create the Interest and nothing else
      
      */

      //create the blocks hash digest
      std::vector<unsigned char> hash = util.digestsha256mix(this->currentBlockInProcess);
      std::string hashString(hash.begin(),hash.end());
      
      //send out an interest saying that you vote YES to adding the block
      //Create an interest with the block hash (to ask for the block itself)
      auto interest = std::make_shared<ndn::Interest>("/prefix/sub/prepare/" + std::to_string(this->currentView) + "/" + std::to_string(this->numberOfRequest) + "/" + hashString + "/" + this->myName);
      Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
      interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
      //interest->setInterestLifetime(ndn::time::seconds(1))

      NS_LOG_DEBUG("[" + this->myName + "]: GETBLOCK DATA: Got success from ESP so Im sending Prepare Interest.");
  
      // Call trace (for logging purposes)
      m_transmittedInterests(interest, this, m_face);

      m_appLink->onReceiveInterest(*interest);
    }
  }
  else if (nameOfData == "sync")
  {
    //Create a ndn Block object to store the content
    const ndn::Block& block = data->getContent();
    //create a pointer that points to the start of the value of the content
    const unsigned char* ddata = block.value();
    //Get the length of the content (number of bytes)
    size_t lengthOfContent = block.value_size();

    Block tempBlock = util.unpackDataPacket(ddata, lengthOfContent);

    //send the blocks to the ESP
    std::string ret = ESPCommunicate(1,"data", util.vectorToString(tempBlock.prevBlock),std::to_string(tempBlock.nonce),tempBlock.s,std::to_string(tempBlock.time));
  }

  std::string con = ESPCommunicate(1,"precon", "","","","");
    
  if(con.substr(0,1) != "e")
  {
    //consensus has been reached!
    //start Commit phase by issuing a commit interest
    auto interest = std::make_shared<ndn::Interest>("/prefix/sub/commit/" + std::to_string(this->currentView) + "/" + std::to_string(this->numberOfRequest) + "/" + this->myName);
    Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
    interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
    //interest->setInterestLifetime(ndn::time::seconds(1));

    NS_LOG_DEBUG("[" + this->myName + "]: I just sent a COMMIT interest and logged my own commit to my commit log: " << *interest);
    // Call trace (for logging purposes)
    m_transmittedInterests(interest, this, m_face);

    m_appLink->onReceiveInterest(*interest);
  }
  
}

} // namespace ns3
