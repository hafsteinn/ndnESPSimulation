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

#include "custom-app-producer.hpp"

#include <ndn-cxx/encoding/buffer-stream.hpp>

#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"

#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"

#include "ns3/random-variable-stream.h"

//#include "SerialReadWrite.hpp"

#include <chrono>
#include <thread>

NS_LOG_COMPONENT_DEFINE("ProducerApp");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(ProducerApp);

// register NS-3 type
TypeId
ProducerApp::GetTypeId()
{
  static TypeId tid = TypeId("ProducerApp").SetParent<ndn::App>().AddConstructor<ProducerApp>();
  return tid;
}

// Processing upon start of the application
void
ProducerApp::StartApplication()
{
  NS_LOG_DEBUG("[PRODUCER]: STARTING....");

  this->counter = 1;

  // initialize ndn::App
  ndn::App::StartApplication();

  // Add entry to FIB for `/prefix/sub`
  ndn::FibHelper::AddRoute(GetNode(), "/prefix/sub/sensordata", m_face, 0);

}

// Processing when application is stopped
void
ProducerApp::StopApplication()
{
  // cleanup ndn::App
  ndn::App::StopApplication();
}

void
ProducerApp::SendInterest(){}

// Callback that will be called when Interest arrives
void
ProducerApp::OnInterest(std::shared_ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest(interest);

  NS_LOG_DEBUG("[PRODUCER]: Received Interest packet for " << interest->getName());

  std::string prefix(interest->getName().get(2).toUri());

  if (prefix == "sensordata")
  {
    auto data = std::make_shared<ndn::Data>(interest->getName());
    //data->setFreshnessPeriod(ndn::time::milliseconds(1000));
    //int randomSensorValue = rand() % 100000;
    //std::vector<uint8_t> b = {randomSensorValue}; //create a vector array from the block
    //data->setContent(reinterpret_cast<const uint8_t*>(&b.front()), b.size()); //set the content
    data->setContent(std::make_shared< ::ndn::Buffer>(1024));
    ndn::StackHelper::getKeyChain().sign(*data);

    NS_LOG_DEBUG("Sending Data packet for " << data->getName());

    this->counter++;

    // Call trace (for logging purposes)
    m_transmittedDatas(data, this, m_face);

    m_appLink->onReceiveData(*data);
  } 
}

// Callback that will be called when Data arrives
void
ProducerApp::OnData(std::shared_ptr<const ndn::Data> data)
{

}

} // namespace ns3
