#include <esp8266ndn.h>
#include "ESPBlock.hpp"
#include "ESPBFT.hpp"
#include "ESPUtils.hpp"

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
  //printLocalBlockchain();
}

void loop() {

  //Anything coming over the USB serial port?
  if(Serial.available() > 0)
  {
    //simulation uses 'x' to denote the end of message
    String request = Serial.readStringUntil('x'); 
    String prefix = getValue(request,0);

    if(prefix == "preprepare") //incoming preprepare Interest packet
    {
      String view = getValue(request,1);
      String number = getValue(request,2);
      String blockHash = getValue(request,3);

      currentNumberOfRequest = number.toInt();
      currentView = view.toInt();
      
      PrePrepare(view.toInt(), number.toInt(), blockHash.c_str());
    }
    else if(prefix == "data") //incoming data packet
    {
      String prevHash = getValue(request,1);
      String nonce = getValue(request,2);
      String transactions = getValue(request,3);
      String btime = getValue(request,4);
      std::vector<unsigned char> blHash(prevHash.begin(),prevHash.end());
          
      BlockReceived(blHash,nonce,transactions,btime);
    }
    else if(prefix == "prepare") //incoming prepare log
    {
      String view = getValue(request,1);
      String number = getValue(request,2);
      String newBlockHash = getValue(request,3);
      String userID = getValue(request,4);
      
      Prepare(view.toInt(), number.toInt(), newBlockHash.c_str(), userID.c_str());
    }
    else if(prefix == "commit") //incoming commit log
    {
      String view = getValue(request,1);
      String number = getValue(request,2);
      String userID = getValue(request,3);
      Commit(view.toInt(),number.toInt(),userID);
    }
    else if(prefix == "print") //incoming print command
    {
      printLocalBlockchain();
      //printPrepareLog();
    }
    else if(prefix == "presync") //incoming presync command
    {
      //return the highest hash
      std::string hh = util.vectorToString(util.digestsha256mix(localBlockchain.back()));
      Serial.println(hh.c_str()); 
    }
    else if(prefix == "precon") //incoming prepare consensus command
    {
      bool con = checkPrepareConsensus();

      if(con)
      {
        //log the commit message
        BFT bft(currentView,currentNumberOfRequest,"","someUser");
        commitLog.push_back(bft);
  
        Serial.println("s");
      }
      else
      {
        Serial.println("e");
      }
    }
    else if(prefix == "getsensor") //ask for sensor data from Producer
    {
      getSensorData();
    }
    else if(prefix == "verifysensor") //verify sensor data from Producer
    {
      String dataPacketContentDigest = getValue(request,1);
      verifySensorData(dataPacketContentDigest);
    }
  }

}

void verifySensorData(String dpcd)
{
  bool contentDigestSuccess = false;
  bool prefixSuccess = false;
  
  std::vector<unsigned char> fakeDigest = util.digestsha256("Some content");
  
  //search for the data name to get the data content digest
  for(int i = 1; i < localBlockchain.size();i++)
    {
      if(localBlockchain.at(i).s.substr(10,10) == "sensordata")
      {
        if(localBlockchain.at(i).s.substr(86.64) == dpcd.c_str())
        {
         contentDigestSuccess = true; 
        } 
      }
    }

  //because we only have one producer
  for(int i = 1; i < localBlockchain.size();i++)
    {
      if(localBlockchain.at(i).s.substr(2,8) == "REGISTER")
      {
        if(localBlockchain.at(i).s.substr(10.21) == "/prefix/sub/sensordata"
        && localBlockchain.at(i).s.substr(30,64) == dpcd.c_str()
        )
        {
         prefixSuccess = true; 
        } 
      }
    }
    
    if(contentDigestSuccess && prefixSuccess)
    {
      Serial.println("s");
    }
    else
    {
      Serial.println("e");
    }  
}

void getSensorData()
{
  bool success = false;
  
  for(int i = 1; i < localBlockchain.size();i++)
  {
    if(localBlockchain.at(i).s.substr(10,10) == "sensordata")
    {
      std::string content_digest = localBlockchain.at(i).s.substr(86.64);
      //create fake Interest
      ndnph::StaticRegion<1024> region;  
      ndnph::Interest interest = region.create<ndnph::Interest>();
      success = true; 
    }
  }

  if(success)
  {
    Serial.println("s");
  }
  else
  {
    Serial.println("e");
  }
}

void Commit(int v, int n, String userID)
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
    //add block to local blockchain
    localBlockchain.push_back(currentBlockInProgress);
    
    //clear local data
    clearLocalData();

    Serial.println("s");
  }
  else
  {
    Serial.println("e");
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

void BlockReceived(std::vector<unsigned char> bHash, String nonce, String transactions, String btime)
{
  //validate the block
  //check if bHash (previous has) is the same as local top hash
  if(util.digestsha256mix(localBlockchain.back()) == bHash)
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
    Serial.println("s");
  }
  else{
    //the block is not valid
    Serial.println("e");
  }
}

void Prepare(int v, int n, std::string bHash, std::string userID){

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

    //reply with a success message 
    Serial.println("s");
  }
  else
  {
    //int tempPrepare = prepareLog.size();
    //Serial.println(tempPrepare);
    Serial.println("e");
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


void PrePrepare(int v, int n, std::string bHash){
 
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
    Serial.println("s");
  }
  else //reply with a 7 byte answer
  {
    Serial.println("e");
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

void clearLocalData()
{
  //clear local logs
  commitLog.clear();
  prepareLog.clear();
}
