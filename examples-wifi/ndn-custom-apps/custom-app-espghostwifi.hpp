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

// custom-app.hpp

#ifndef ESPGHOSTWIFI_H_
#define ESPGHOSTWIFI_H_

#include "ns3/ndnSIM/apps/ndn-app.hpp"
#include "Utils.hpp"
#include "Block.hpp"
#include "BlockChainProxy.hpp"
#include <map>
#include "BFT.hpp"
#include "SerialUtils.hpp"
#include <fstream>
#include "boost/asio.hpp"

using namespace boost::asio;

using namespace BlockNDN;

namespace ns3 {

/**
 * @brief A simple custom application
 *
 * This applications demonstrates how to send Interests and respond with Datas to incoming interests
 *
 * When application starts it "sets interest filter" (install FIB entry) for /prefix/sub, as well as
 * sends Interest for this prefix
 *
 * When an Interest is received, it is replied with a Data with 1024-byte fake payload
 */
class ESPGhostWIFI : public ndn::App {
public:
  std::ofstream ESPLogFile;
  std::vector<unsigned char> hash;
  std::vector<unsigned char> newBlockHash;
  Block currentBlockInProcess;
  std::string winnerHash;
  int currentView;
  int numberOfRequest;
  std::vector<BFT> prepareLog;
  std::vector<BFT> commitLog;
  std::vector<BFT> preprepareRequest;
  bool isPrimary;
  std::string myName;
  // register NS-3 type "CustomApp"
  static TypeId
  GetTypeId();

  // (overridden from ndn::App) Processing upon start of the application
  virtual void
  StartApplication();

  // (overridden from ndn::App) Processing when application is stopped
  virtual void
  StopApplication();

  // (overridden from ndn::App) Callback that will be called when Interest arrives
  virtual void
  OnInterest(std::shared_ptr<const ndn::Interest> interest);

  // (overridden from ndn::App) Callback that will be called when Data arrives
  virtual void
  OnData(std::shared_ptr<const ndn::Data> contentObject);

private:
  boost::asio::io_service io_service;
  double totalSim = 0;
  double ESPget = 0;
  double ESPverify = 0;
  double ESPtotal = 0;
  double totalSimulationRunTime;
  Utils util;
  SerialUtils sutil;
  BlockChainProxy blockChainProxy;
  void
  PrintBlockchain();
  void
  sync();
  std::string 
  ESPCommunicate(int len, std::string pre, std::string v, std::string n, std::string h, std::string i);
  void requestStaticSensorData();
};

} // namespace ns3

#endif // CUSTOM_APP_H_
