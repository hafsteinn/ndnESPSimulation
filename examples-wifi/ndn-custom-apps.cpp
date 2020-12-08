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

// ndn-custom-apps.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/ndnSIM-module.h"

using ns3::ndn::GlobalRoutingHelper;
namespace ns3 {

/**
 * This scenario simulates a one-node two-custom-app scenario:
 *
 *   +------+ <-----> (CustomApp)
 *   | Node |
 *   +------+ <-----> (Hijacker)
 *
 *     NS_LOG=CustomApp ./waf --run=ndn-custom-apps
 */
int
main(int argc, char* argv[])
{
  /************     CONFIGURATION START     **************/
  // Add constraints to p2p connections
  //Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("1Mbps"));
  //Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));

  int availableCacheSlots = 20;  //maximum of packets allowed in each entities CS

  std::string prefix = "/prefix/sub"; //this prefix is shared by all applications

  std::string strategy = "ncc"; //NFD forwarding strategy. See available strategies at https://ndnsim.net/2.1/fw.html

  double simulationTime = 15.0; //The simulation will stop when all events have been run OR when the time of the simulation has reached the simulationTime 
  /************     CONFIGURATION END     **************/

  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  /*----WIFI START-------------*/

  std::string phyMode ("DsssRate1Mbps");

  // Creating 3x3 topology
  AnnotatedTopologyReader topologyReader("", 1);
  topologyReader.SetFileName("src/ndnSIM/examples/topologies/topo-grid-3x3.txt");
  topologyReader.Read();

  NodeContainer applicationNodes;
  applicationNodes.Add(Names::Find<Node>("Primary"));
  applicationNodes.Add(Names::Find<Node>("Consumer1"));
  applicationNodes.Add(Names::Find<Node>("Consumer2"));
  applicationNodes.Add(Names::Find<Node>("Consumer3"));
  applicationNodes.Add(Names::Find<Node>("ESP"));
  applicationNodes.Add(Names::Find<Node>("Producer"));


  NodeContainer accessPoints;
  accessPoints.Add(Names::Find<Node>("AP"));

  NodeContainer nodes;
  nodes = topologyReader.GetNodes();

  //////////////////////
  WifiHelper wifi;
  // wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
  wifi.SetStandard(WIFI_PHY_STANDARD_80211b);
  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  // ns-3 supports RadioTap and Prism tracing extensions for 802.11
  wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);

  YansWifiChannelHelper wifiChannel;
  // reference loss must be changed since 802.11b is operating at 2.4GHz
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel",
                                  "Exponent", DoubleValue (3.0),
                                  "ReferenceLoss", DoubleValue (40.0459));

  wifiPhy.SetChannel (wifiChannel.Create ());

  // Add a non-QoS upper mac, and disable rate control
WifiMacHelper wifiMac;
wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                              "DataMode",StringValue (phyMode),
                              "ControlMode",StringValue (phyMode));

// Setup the rest of the upper mac
Ssid ssid = Ssid ("wifi-default");

// setup ap. (access points)
wifiMac.SetType ("ns3::ApWifiMac",
                 "Ssid", SsidValue (ssid));
NetDeviceContainer apDevice = wifi.Install (wifiPhy, wifiMac, accessPoints);
NetDeviceContainer devices = apDevice;

// setup sta. (Client)
wifiMac.SetType ("ns3::StaWifiMac",
                 "Ssid", SsidValue (ssid),
                 "ActiveProbing", BooleanValue (false));
NetDeviceContainer staDevice = wifi.Install (wifiPhy, wifiMac, applicationNodes);
devices.Add (staDevice);

// Configure mobility
MobilityHelper mobility;
Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
positionAlloc->Add (Vector (0.0, 0.0, 0.0));
positionAlloc->Add (Vector (5.0, 0.0, 0.0));
positionAlloc->Add (Vector (0.0, 5.0, 0.0));
mobility.SetPositionAllocator (positionAlloc);
mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");



  ////////////////
  // 1. Install Wifi
  //NetDeviceContainer wifiNetDevices = wifi.Install(wifiPhyHelper, wifiMacHelper, nodes);

  // 2. Install Mobility model
  mobility.Install(nodes);

  // 3. Install NDN stack
  //NS_LOG_INFO("Installing NDN stack");
  ndn::StackHelper ndnHelper;
  // ndnHelper.AddNetDeviceFaceCreateCallback (WifiNetDevice::GetTypeId (), MakeCallback
  // (MyNetDeviceFaceCallback));
  //ndnHelper.setPolicy("nfd::cs::lru");
  ndnHelper.setCsSize(availableCacheSlots);
  ndnHelper.SetDefaultRoutes(true);
  ndnHelper.Install(nodes);

  // Set BestRoute strategy
  ndn::StrategyChoiceHelper::Install(nodes, "/prefix/sub", "/localhost/nfd/strategy/" + strategy);
  ndn::StrategyChoiceHelper::Install(nodes, "/sensordata", "/localhost/nfd/strategy/" + strategy);

  ndn::Interest::setDefaultCanBePrefix(true);	

  // Set BestRoute strategy
  //ndn::StrategyChoiceHelper::InstallAll(prefix, "/localhost/nfd/strategy/" + strategy);

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();

  /*----WIFI END-------------*/


  /*----p2p START-------------------------------------------------------------------------------------------------------------------------*/
  // int availableCacheSlots = 20;  //maximum of packets allowed in each entities CS
  // std::string prefix = "/prefix/sub"; //this prefix is shared by all applications
  // std::string strategy = "multicast"; //NFD forwarding strategy. See available strategies at https://ndnsim.net/2.1/fw.html
  // double simulationTime = 43.0; //The simulation will stop when all events have been run OR when the time of the simulation has reached the simulationTime 
  // CommandLine cmd;
  // cmd.Parse(argc, argv);
  // AnnotatedTopologyReader topologyReader("", 1);
  // topologyReader.SetFileName("src/ndnSIM/examples/topologies/topo-grid-3x3.txt");
  // topologyReader.Read();
  // ndn::StackHelper ndnHelper;
  // ndnHelper.setCsSize(availableCacheSlots); //number of packet allowed in CS
  // ndnHelper.InstallAll();
  // ndn::StrategyChoiceHelper::InstallAll(prefix, "/localhost/nfd/strategy/" + strategy);
  // ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  // ndnGlobalRoutingHelper.InstallAll();
  /*----p2p END-------------------------------------------------------------------------------------------------------------------------*/
  

  //Nodes
  Ptr<Node> primary = Names::Find<Node>("Primary");                       //PrimaryNode
  ndn::AppHelper app1("PrimaryNode");
  app1.Install(primary);
  ndnGlobalRoutingHelper.AddOrigin(prefix, primary);

  // Ptr<Node> consumerApp = grid.GetNode(1,1);                    //ConsumerApp
  // ndn::AppHelper app2("ConsumerApp");
  // app2.Install(consumerApp);
  // ndnGlobalRoutingHelper.AddOrigin(prefix, consumerApp);

  Ptr<Node> consumerOneApp = Names::Find<Node>("Consumer1");                 //ConsumerOneApp
  ndn::AppHelper app3("ConsumerOneApp");
  app3.Install(consumerOneApp);
  ndnGlobalRoutingHelper.AddOrigin(prefix, consumerOneApp);

  Ptr<Node> consumerTwoApp = Names::Find<Node>("Consumer2");                //ConsumerTwoApp
  ndn::AppHelper app4("ConsumerTwoApp");
  app4.Install(consumerTwoApp);
  ndnGlobalRoutingHelper.AddOrigin(prefix, consumerTwoApp);

  Ptr<Node> consumerThreeApp = Names::Find<Node>("Consumer3");               //ConsumerThreeApp
  ndn::AppHelper app5("ConsumerThreeApp");
  app5.Install(consumerThreeApp);
  ndnGlobalRoutingHelper.AddOrigin(prefix, consumerThreeApp);

  Ptr<Node> ESPGhost = Names::Find<Node>("ESP");                         //ESPGhost
  ndn::AppHelper app6("ESPGhost");
  app6.Install(ESPGhost);
  ndnGlobalRoutingHelper.AddOrigin(prefix, ESPGhost);

  // Ptr<Node> Producer = Names::Find<Node>("Producer");                         //Producer
  // ndn::AppHelper app7("ProducerApp");
  // app7.Install(Producer);
  // ndnGlobalRoutingHelper.AddOrigin("/sensordata", Producer); 


  // Calculate and install FIBs
  ndn::GlobalRoutingHelper::CalculateRoutes();

  Simulator::Stop(Seconds(simulationTime));

  //ndn::CsTracer::InstallAll("cs-trace.txt", Seconds(1));
  //ndn::AppDelayTracer::InstallAll("app-delays-trace.txt");
  ndn::L3RateTracer::InstallAll("rate-trace.txt", Seconds(1.0));
  //L2RateTracer::InstallAll("drop-trace.txt", Seconds(0.2));

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}