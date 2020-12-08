#include <esp8266ndn.h>
#include "ESPBlock.hpp"
#include "ESPBFT.hpp"
#include "ESPUtils.hpp"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
   
// Set WiFi credentials
#define WIFI_SSID "B44"
#define WIFI_PASS "Borg44260"
#define UDP_PORT 4210

//UDP instance
WiFiUDP UDP;
char packet[255];
char errorReply[] = "err";
char successReply[] = "suc";

using namespace std;
using namespace ESPBlockNDN;

std::vector<Block> localBlockchain; //local Blockchain
std::vector<BFT> preprepareRequest; //pre-prepare request log
std::vector<BFT> prepareLog; //prepare log
std::vector<BFT> commitLog;  //commit log
int currentNumberOfRequest; //the current request
int currentView; //current view/primary node
std::string myName = "ESPPhysical"; //name of this node
Block currentBlockInProgress; //holds the current block in progress
Utils util; //utility lib
int _quorum; //PBFT quorum

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

   // Begin WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
   
  // Connecting to WiFi...
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  // Loop continuously while WiFi is not connected
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }
   
  // Connected to WiFi
  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  // Begin listening to UDP port
  UDP.begin(UDP_PORT);
  Serial.print("Listening on UDP port ");
  Serial.println(UDP_PORT);

  //clear local logs
  commitLog.clear();
  prepareLog.clear();
  preprepareRequest.clear();

  //start at request number 1
  currentNumberOfRequest = 1;
  
  //configure the quorum
  int totalNumberOfNodes = 5;
  int maxFaultyNodes = floor((totalNumberOfNodes-1)/3);
  _quorum = totalNumberOfNodes - maxFaultyNodes;

  //Startup Step 1: Create the genesis Block and add it to the local Blockchain
  std::string genesisString = "This is the genesis block.";
  long btime = 20161209;
  long nonce = 0;
  std::string s = "This is the genesis block, the first block on everyones Blockchain.";
  Block genesisBlock = Block(util.digestsha256(genesisString),btime,nonce,s);
  localBlockchain.push_back(genesisBlock); //Add the genesis Block to the Blockchain

  //print the initial status of the local Blockchain
  printLocalBlockchain();
}

void loop() {

  // If packet received...
  int packetSize = UDP.parsePacket();
  if (packetSize) {
    Serial.print("Received packet! Size: ");
    Serial.println(packetSize); 
    int len = UDP.read(packet, 255);
    if (len > 0)
    {
      packet[len] = '\0';
    }

    Serial.print("Packet received: ");
    Serial.println(packet);

    //get the prefix from the UDP packet
    String sPacket(packet);
    String UDPPrefix = getUDPValue(sPacket,0);
    
    //prepare for reply
    UDP.beginPacket(UDP.remoteIP(),UDP.remotePort());    
    delay(1);
    if(UDPPrefix == "preprepare")
    {
      String view = getUDPValue(sPacket,1);
      String number = getUDPValue(sPacket,2);
      String blockHash = getUDPValue(sPacket,3);

      currentNumberOfRequest = number.toInt();
      currentView = view.toInt();

      if(PrePrepare(view.toInt(), number.toInt(), blockHash.c_str()))
      {
        UDP.write(successReply);
        Serial.println("pre-prepare: SUCCESS");
      } else {
        UDP.write(errorReply);
        Serial.println("pre-prepare: ERROR");
      }
    }
    else if(UDPPrefix == "prepare") //incoming prepare log
    {
      String view = getUDPValue(sPacket,1);
      String number = getUDPValue(sPacket,2);
      String newBlockHash = getUDPValue(sPacket,3);
      String userID = getUDPValue(sPacket,4);
      
      if(Prepare(view.toInt(), number.toInt(), newBlockHash.c_str(), userID.c_str()))
      {
        UDP.write(successReply);
        Serial.println("prepare: SUCCESS");
      } else {
        UDP.write(errorReply);
        Serial.println("prepare: ERROR");
      }
    }
    else if(UDPPrefix == "commit") //incoming commit log
    {
      String view = getUDPValue(sPacket,1);
      String number = getUDPValue(sPacket,2);
      String userID = getUDPValue(sPacket,3);
      if(Commit(view.toInt(),number.toInt(),userID))
      {
        UDP.write(successReply);
        Serial.println("commit: SUCCESS");
      } else {
        UDP.write(errorReply);
        Serial.println("commit: ERROR");
      }
    }
    else if(UDPPrefix == "data") //incoming data packet
    {
      String prevHash = getUDPValue(sPacket,1);
      String nonce = getUDPValue(sPacket,2);
      String transactions = getUDPValue(sPacket,3);
      String btime = getUDPValue(sPacket,4);
      std::vector<unsigned char> blHash(prevHash.begin(),prevHash.end());
          
      if(BlockReceived(blHash,nonce,transactions,btime))
      {
        UDP.write(successReply);
        Serial.println("data: SUCCESS");
      } else {
        UDP.write(errorReply);
        Serial.println("data: ERROR");
      }
    }
    else if(UDPPrefix == "getsensor") //ask for sensor data from Producer
    {     
      if(getSensorData())
      {
        UDP.write(successReply);
        Serial.println("getSensor: SUCCESS");
      } else {
        UDP.write(errorReply);
        Serial.println("getSensor: ERROR");
      }
    }
    else if(UDPPrefix == "verifysensor") //verify sensor data from Producer
    {
      String ppkd = getUDPValue(sPacket,1);
      String content_digest = getUDPValue(sPacket,2);
      
      if(verifySensorData(ppkd,content_digest))
      {
        UDP.write(successReply);
        Serial.println("verifysensor: SUCCESS");
      } else {
        UDP.write(errorReply);
        Serial.println("verifysensor: ERROR");
      }
    }
    else if(UDPPrefix == "print") //incoming print command
    {
      for(int i = 0; i < localBlockchain.size();i++)
      {
        Serial.println("---------------------------");
        Serial.print("This blocks Hash: ");
        Serial.println(util.vectorToString(util.digestsha256mix(localBlockchain[i])).c_str());
        Serial.print("prevHash: ");
        Serial.println(util.vectorToString(localBlockchain[i].prevBlock).c_str());
        Serial.print("Time: ");
        Serial.println(localBlockchain[i].time);
        Serial.print("Nonce: ");
        Serial.println(localBlockchain[i].nonce);
        Serial.print("s: ");
        Serial.println(localBlockchain[i].s.c_str());
        Serial.print("Size of Blockchain: ");
        Serial.println(localBlockchain.size());
        Serial.println("---------------------------");
      }
    
      Serial.println(ESP.getFreeHeap());
      //printLocalBlockchain();
      //printPrepareLog();

      UDP.write(successReply);
    }
    else if(UDPPrefix == "presync") //incoming presync command
    {
      //return the highest hash
      std::string hh = util.vectorToString(util.digestsha256mix(localBlockchain.back()));
      Serial.print("The highest hash is: ");
      Serial.println(hh.c_str()); 
      
      if(hh != "") //found a hash
      {
        char highestHash[64];
        strcpy(highestHash, hh.c_str());
        UDP.write(highestHash);
      }
      else
      {
        UDP.write(errorReply);
      }

    }
    else if(UDPPrefix == "precon") //incoming prepare consensus command
    {
      bool con = checkPrepareConsensus();

      if(con)
      {
        //log the commit message
        BFT bft(currentView,currentNumberOfRequest,"","someUser");
        commitLog.push_back(bft);
  
        UDP.write(successReply);
      }
      else
      {
        UDP.write(errorReply);
      }
    }
    else{
      UDP.write(errorReply);
    }

    //delay(1000);
    UDP.endPacket();
   
  }

  

}

bool verifySensorData(String ppkd, String content_digest)
{
  bool contentDigestSuccess = false;
  bool prefixSuccess = false;
  
  Serial.println("SSSSSSSSSTART");
  
  //search for the data name to get the data content digest
  for(int i = 1; i < localBlockchain.size();i++)
    {
      if(localBlockchain.at(i).s.substr(10,10) == "sensordata")
      {
        String a = localBlockchain.at(i).s.substr(86,64).c_str();
        //Serial.println(a);
        if(a == content_digest.c_str())
        {
         contentDigestSuccess = true; 
        } 
      }
    }

  //is the producer allowed to serve data on this prefix?
  for(int i = 1; i < localBlockchain.size();i++)
    {
      String tname = localBlockchain.at(i).s.substr(1,8).c_str();
      Serial.println(tname);
      if(tname == "REGISTER")
      {
        String sub = localBlockchain.at(i).s.substr(10,22).c_str();
        String pk = localBlockchain.at(i).s.substr(33,64).c_str();
        Serial.println(sub);
        Serial.println(pk);
        if(sub == "/prefix/sub/sensordata" && pk == ppkd)
        {
         prefixSuccess = true; 
        } 
      }
    }
 
    
    if(contentDigestSuccess && prefixSuccess)
    {
      return true;
    }
    else
    {
      return false;
    }  
}

//This function tells the ESPGhost node to send a getSensorData Interest
bool getSensorData()
{
  bool success = false;
  for(int i = 1; i < localBlockchain.size();i++)
  {
    //find the block containint the sensordata transaction in the local Blockchain
    Serial.println(localBlockchain.at(i).s.substr(10,10).c_str());
    if(localBlockchain.at(i).s.substr(10,10) == "sensordata")
    {
      // <register,sensordata,
      //get the content_digest for the sensor data name
      std::string content_digest = localBlockchain.at(i).s.substr(86.64);
      Serial.println(content_digest.c_str());
      //create fake Interest
      ndnph::StaticRegion<1024> region;  
      ndnph::Interest interest = region.create<ndnph::Interest>();
      success = true; 
    }
  }

  if(success)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool Commit(int v, int n, String userID)
{
  //log the commit message
  BFT bft(v,n,"",userID.c_str());
  commitLog.push_back(bft);

  int quorum = 0;
  bool commit = false;

  for(int i = 0; i < commitLog.size(); i++)
  {
    if(commitLog[i].view == currentView && commitLog[i].requestNumber == currentNumberOfRequest)
    {
      quorum++;
    }
    if(quorum >= _quorum)
    {
      commit = true;
    }
  }

  if(commit)
  {

    localBlockchain.push_back(currentBlockInProgress);
    
    //clear local data
    clearLocalData();

    return true;
  }
  else
  {
    return false;
  }
}

void printLocalBlockchain()
{
  for(int i = 0; i < localBlockchain.size();i++)
  {
    Serial.println("---------------------------");
    Serial.print("This blocks Hash: ");
    Serial.println(util.vectorToString(util.digestsha256mix(localBlockchain[i])).c_str());
    Serial.print("prevHash: ");
    Serial.println(util.vectorToString(localBlockchain[i].prevBlock).c_str());
    Serial.print("Time: ");
    Serial.println(localBlockchain[i].time);
    Serial.print("Nonce: ");
    Serial.println(localBlockchain[i].nonce);
    Serial.print("s: ");
    Serial.println(localBlockchain[i].s.c_str());
    Serial.print("Size of Blockchain: ");
    Serial.println(localBlockchain.size());
    Serial.println("---------------------------");
  }

  Serial.println(ESP.getFreeHeap());
}

void printPrepareLog()
{
  for(int i = 0; i < prepareLog.size();i++)
  {
    Serial.println("---------------------------");
    Serial.print("Hash: ");
    Serial.println(prepareLog[i].hash.c_str());
    Serial.print("View: ");
    Serial.println(prepareLog[i].view);
    Serial.print("Number: ");
    Serial.println(prepareLog[i].requestNumber);
    Serial.print("User: ");
    Serial.println(prepareLog[i].requestersSignature.c_str());
    Serial.print("size: ");
    Serial.println(prepareLog.size());
    Serial.println("---------------------------");
  }

  Serial.println(ESP.getFreeHeap());
}

bool BlockReceived(std::vector<unsigned char> bHash, String nonce, String transactions, String btime)
{

    //create the block and store globally
    currentBlockInProgress.s = transactions.c_str();
    currentBlockInProgress.prevBlock = bHash;
    currentBlockInProgress.nonce = atol(nonce.c_str());
    currentBlockInProgress.time = atol(btime.c_str());

    //Add your own prepare statement to the prepare log
    BFT bft(currentView,currentNumberOfRequest,util.vectorToString(util.digestsha256mix(currentBlockInProgress)),myName);
    prepareLog.push_back(bft);

    //send a success reply
    return true;

}

bool Prepare(int v, int n, std::string bHash, std::string userID){

  //add the prepare request to the log
  BFT bft(v,n,bHash,userID);
  prepareLog.push_back(bft);

  bool consensus = checkPrepareConsensus();

  if(consensus)
  {
    //create a fake Interest to iniate COMMIT phase
    ndnph::StaticRegion<1024> region;  
    ndnph::Interest interest = region.create<ndnph::Interest>();
      
    //add my commit to my own log
    BFT bft(v,n,bHash,myName);
    commitLog.push_back(bft);

    return true;
  }
  else
  {
    return false;
  }
  
}

bool checkPrepareConsensus()
{
  int quorum = 0;
  bool consensus = false;

  for(int i = 0; i < prepareLog.size(); i++)
  {
    std::string currentBlock = preprepareRequest.back().hash;
    int currentView = preprepareRequest.back().view;
    int currentRequestNumber = preprepareRequest.back().requestNumber;
     
    if(prepareLog[i].hash == currentBlock && prepareLog[i].view == currentView && prepareLog[i].requestNumber == currentRequestNumber)
    {
      quorum++; 
    }

    if(quorum >= _quorum)
    {
      consensus = true;
    }
  }
  return consensus;
}


bool PrePrepare(int v, int n, std::string bHash){
 
  bool first = true;

  //check if this is the first time that this specific Hash has been broadcast for this specific v and n
  for(int i = 0; i < preprepareRequest.size();i++)
  {
    if(preprepareRequest[i].view == v && preprepareRequest[i].requestNumber == n && preprepareRequest[i].hash == bHash) 
    {
      //when we run the simulation repeadetly, we will always get to this point since we are running
      //the same pre-prepare request every time. 
      first = false; 
    }
  }

  if(first)
  {
    //clear local data
    clearLocalData();
        
    currentNumberOfRequest = n;

    //log the pre-prepare request
    BFT bft(v,n,bHash,"PRIMARY_SIGNATURE");
    preprepareRequest.push_back(bft);
    
    //create fake Interest
    ndnph::StaticRegion<1024> region;  
    ndnph::Interest interest = region.create<ndnph::Interest>();

    //let serial know that the interest packet is ready
    return true;
  }
  else //reply with a 7 byte answer
  {
    return false;
  }
}

String getValue(String data, int index)
{
  int found = 0;
  int strIndex[] = {0,-1};
  int maxIndex = data.length() - 1;

  for(int i = 0; i <= maxIndex && found <= index; i++)
  {
    if(data.charAt(i) == ':' || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

String getUDPValue(String data, int index)
{
  
  int found = 0;
  int strIndex[] = {0,-1};
  int maxIndex = data.length() - 1;

  for(int i = 0; i <= maxIndex && found <= index; i++)
  {
    if(data.charAt(i) == ':' || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void clearLocalData()
{
  //clear local logs
  commitLog.clear();
  prepareLog.clear();
}
