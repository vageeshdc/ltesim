/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010,2011,2012,2013 TELEMATICS LAB, Politecnico di Bari
 *
 * This file is part of LTE-Sim
 *
 * LTE-Sim is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation;
 *
 * LTE-Sim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LTE-Sim; if not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Giuseppe Piro <g.piro@poliba.it>
 */
#include "../channel/LteChannel.h"
#include "../phy/enb-lte-phy.h"
#include "../phy/ue-lte-phy.h"
#include "../core/spectrum/bandwidth-manager.h"
#include "../networkTopology/Cell.h"
#include "../protocolStack/packet/packet-burst.h"
#include "../protocolStack/packet/Packet.h"
#include "../core/eventScheduler/simulator.h"
#include "../flows/application/InfiniteBuffer.h"
#include "../flows/application/VoIP.h"
#include "../flows/application/CBR.h"
#include "../flows/application/TraceBased.h"
#include "../device/IPClassifier/ClassifierParameters.h"
#include "../flows/QoS/QoSParameters.h"
#include "../flows/QoS/QoSForEXP.h"
#include "../flows/QoS/QoSForFLS.h"
#include "../flows/QoS/QoSForM_LWDF.h"
#include "../componentManagers/FrameManager.h"
#include "../utility/seed.h"
#include "../utility/RandomVariable.h"
#include "../utility/UsersDistribution.h"
#include "../channel/propagation-model/macrocell-urban-area-channel-realization.h"
#include "../load-parameters.h"
#include <iostream>
#include <queue>
#include <fstream>
#include <stdlib.h>
#include <cstring>
#include "../channel/LteChannel.h"
#include "../core/spectrum/bandwidth-manager.h"
#include "../networkTopology/Cell.h"
#include "../core/eventScheduler/simulator.h"
#include "../flows/application/InfiniteBuffer.h"
#include "../flows/QoS/QoSParameters.h"
#include "../componentManagers/FrameManager.h"
#include "../componentManagers/FlowsManager.h"
#include "../device/ENodeB.h"


static void Simple (float radx,int flag,int flag_val)
{

  // CREATE COMPONENT MANAGERS
  Simulator *simulator = Simulator::Init();
  FrameManager *frameManager = FrameManager::Init();
  NetworkManager* networkManager = NetworkManager::Init();
  FlowsManager* flowsManager = FlowsManager::Init ();


  //Create CHANNELS
  LteChannel *dlCh1 = new LteChannel ();
  LteChannel *ulCh1 = new LteChannel ();

  LteChannel *dlCh2 = new LteChannel ();
  LteChannel *ulCh2 = new LteChannel ();

  // CREATE SPECTRUM
  //BandwidthManager* spectrum = new BandwidthManager (5, 5, 0, 0);
  std::vector <BandwidthManager*> spectrums = RunFrequencyReuseTechniques (1+1, 3, 5);


  // CREATE CELL
  int idCell = 0;
  int radius = (int)radx; //km
  int minDistance = 0.0035; //km
  int posX = 0;
  int posY = 0;
  Cell* cell1 = networkManager->CreateCell (idCell, radius, minDistance, posX, posY);
  Cell* cell2 = networkManager->CreateCell (idCell+1, radius/2, minDistance, posX, posY);
  
  //Create ENodeB
  int idEnb = 2;
  //here flag =1 means to use the flag_val as power else it is taken as distance to get power
  ENodeB* enb1 = networkManager->CreateEnodebExtended (idEnb, cell1, posX, posY, dlCh1, ulCh1, spectrums.at(0),flag,flag_val);
  enb1->SetDLScheduler (ENodeB::DLScheduler_TYPE_PROPORTIONAL_FAIR);

  ENodeB* enb = networkManager->CreateEnodebExtended (idEnb+1, cell2, posX, posY, dlCh2, ulCh2, spectrums.at(1),flag,flag_val);
  enb->SetDLScheduler (ENodeB::DLScheduler_TYPE_PROPORTIONAL_FAIR);
  

  //Create GW
  Gateway *gw = networkManager->CreateGateway ();


  //Create UE
  int idUe = 4;
  int posX_ue = (int)(radx*1000.0-40.0);//40; //m
  int posY_ue = 0;  //m
  int speed = 0;    //km/h
  double speeDirection = 0;
  UserEquipment* ue1 = networkManager->CreateUserEquipment (idUe, posX_ue, posY_ue, speed, speeDirection, cell1, enb1);
  UserEquipment* ue2 = networkManager->CreateUserEquipment (idUe+1, posX_ue+10, posY_ue+10, 0, speeDirection, cell2, enb);
  UserEquipment* ue3 = networkManager->CreateUserEquipment (idUe+2, posX_ue+10, posY_ue+30, 0, speeDirection, cell2, enb);
  
  //Create an Application
  QoSParameters *qos = new QoSParameters ();
  int applicationID = 0;
  int srcPort = 0;
  int dstPort = 100;
  double startTime = 0.1; //s
  double stopTime = 0.12;  //s
  Application* be = flowsManager->CreateApplication (applicationID,
						  gw, ue1,
						  srcPort, dstPort, TransportProtocol::TRANSPORT_PROTOCOL_TYPE_UDP ,
						  Application::APPLICATION_TYPE_INFINITE_BUFFER,
						  qos,
						  startTime, stopTime);
  
  Application* be1 = flowsManager->CreateApplication (applicationID+1,
						  gw, ue2,
						  srcPort, dstPort, TransportProtocol::TRANSPORT_PROTOCOL_TYPE_UDP ,
						  Application::APPLICATION_TYPE_INFINITE_BUFFER,
						  qos,
						  startTime, stopTime);


  Application* be3 = flowsManager->CreateApplication (applicationID+2,
						  ue2, ue3,
						  srcPort, dstPort, TransportProtocol::TRANSPORT_PROTOCOL_TYPE_UDP ,
						  Application::APPLICATION_TYPE_INFINITE_BUFFER,
						  qos,
						  startTime, stopTime);

  
  simulator->SetStop(0.13);
  simulator->Run ();

}
