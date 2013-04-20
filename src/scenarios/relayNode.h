/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010,2011,2012 TELEMATICS LAB, Politecnico di Bari
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
#include "../device/ENodeB.h"
#include "../device/UserEquipment.h"
#include <iostream>
#include <queue>
#include <fstream>
#include <stdlib.h>
#include <cstring>



static void relayNode ( double radius,
                       double maxDelay)
{

  int seed = 0.1;
  int nbCell = 2;
  int nbUE = 2;
  // define simulation times
  double duration = 12;
  double flow_duration = 10;

  int cluster = 3;
  double bandwidth = 5;

  // CREATE COMPONENT MANAGER
  Simulator *simulator = Simulator::Init();
  FrameManager *frameManager = FrameManager::Init();
  NetworkManager* nm = NetworkManager::Init();

  // CONFIGURE SEED
  if (seed >= 0)
  {
	  int commonSeed = GetCommonSeed (seed);
	  srand (commonSeed);
	}
  else
	{
	  srand (time(NULL));
	}
  std::cout << "Simulation with SEED = " << seed << std::endl;

  // SET SCHEDULING ALLOCATION SCHEME
  ENodeB::ULSchedulerType uplink_scheduler_type;
  /*
  switch (sched_type)
	{
	  case 1:
		uplink_scheduler_type = ENodeB::ULScheduler_TYPE_MAXIMUM_THROUGHPUT;
		std::cout << "Scheduler MAXIMUM_THROUGHPUT "<< std::endl;
		break;
	  case 2:*/
		uplink_scheduler_type = ENodeB::ULScheduler_TYPE_FME;
		std::cout << "Scheduler FME "<< std::endl;
		/*break;
	  case 3:
		uplink_scheduler_type = ENodeB::ULScheduler_TYPE_ROUNDROBIN;
		std::cout << "Scheduler ROUNDROBIN "<< std::endl;
		break;
	  default:
		uplink_scheduler_type = ENodeB::ULScheduler_TYPE_MAXIMUM_THROUGHPUT;
		break;
	}
*/
  // SET FRAME STRUCTURE
  FrameManager::FrameStructure frame_structure;
  frameManager->SetFrameStructure(FrameManager::FRAME_STRUCTURE_FDD);




  //create cells
  std::vector <Cell*> *cells = new std::vector <Cell*>;
  for (int i = 0; i < nbCell; i++)
    {
	  //CartesianCoordinates center =
	//		  GetCartesianCoordinatesForCell(i, radius * 1000.);

	  Cell *c = new Cell (i, (float)radius*(0.5+0.5*i), 0.035, (1-i)*1, (1-i)*1);
	  cells->push_back (c);
	  nm->GetCellContainer ()->push_back (c);

	  std::cout << "Created Cell, id " << c->GetIdCell ()
			  <<", position: " << c->GetCellCenterPosition ()->GetCoordinateX ()
			  << " " << c->GetCellCenterPosition ()->GetCoordinateY () << std::endl;
    }


  std::vector <BandwidthManager*> spectrums = RunFrequencyReuseTechniques (nbCell, cluster, bandwidth);

  //Create a set of a couple of channels
  std::vector <LteChannel*> *dlChannels = new std::vector <LteChannel*>;
  std::vector <LteChannel*> *ulChannels = new std::vector <LteChannel*>;
  for (int i= 0; i < nbCell; i++)
    {
	  LteChannel *dlCh = new LteChannel ();
	  dlCh->SetChannelId (i);
	  dlChannels->push_back (dlCh);

	  LteChannel *ulCh = new LteChannel ();
	  ulCh->SetChannelId (i);
	  ulChannels->push_back (ulCh);
    }


  //create eNBs
  std::vector <ENodeB*> *eNBs = new std::vector <ENodeB*>;
  for (int i = 0; i < nbCell; i++)
    {
	  ENodeB* enb = new ENodeB (i, cells->at (i));
	  enb->GetPhy ()->SetDlChannel (dlChannels->at (i));
	  enb->GetPhy ()->SetUlChannel (ulChannels->at (i));
	  enb->GetPhy ()->SetBandwidthManager (spectrums.at (i));
	  
	  if(i == 0){
	    enb->isRelay = true;
	  }
	  else{
	    enb->isRelay = false;
	  }
	  enb->SetULScheduler (uplink_scheduler_type);   // added UL
	  enb->SetDLScheduler (ENodeB::DLScheduler_TYPE_PROPORTIONAL_FAIR); // setted by default
      
          
	
	  nm->GetENodeBContainer ()->push_back (enb);
	  eNBs->push_back (enb);
          ulChannels->at (i)->AddDevice((NetworkNode*) enb);
          
	  std::cout << "Created enb, id " << enb->GetIDNetworkNode()
			  << ", cell id " << enb->GetCell ()->GetIdCell ()
			  <<", position: " << enb->GetMobilityModel ()->GetAbsolutePosition ()->GetCoordinateX ()
			  << " " << enb->GetMobilityModel ()->GetAbsolutePosition ()->GetCoordinateY ()
			  << ", channels id " << enb->GetPhy ()->GetDlChannel ()->GetChannelId ()
			  << enb->GetPhy ()->GetUlChannel ()->GetChannelId ()  << std::endl;

	  spectrums.at (i)->Print ();


    }
  //eNBs->at(1)->isRelay = true;


int nbVoIP = 2;
  //Define Application Container
  VoIP VoIPApplication[nbVoIP];
  //TraceBased VideoApplication[nbVideo*nbCell*nbUE];
  //InfiniteBuffer BEApplication[nbBE*nbCell*nbUE];
  //CBR CBRApplication[nbCBR*nbCell*nbUE];
  int voipApplication = 0;
  int videoApplication = 0;
  int cbrApplication = 0;
  int beApplication = 0;
  int destinationPort = 101;
  int applicationID = 0;



  //Create GW
  Gateway *gw = new Gateway ();
  nm->GetGatewayContainer ()->push_back (gw);


  //nbUE is the number of users that are into each cell at the beginning of the simulation
  int idUE = nbCell;
  for (int j = 0; j < nbCell; j++)
    {

	  //users are distributed uniformly intio a cell
	  vector<CartesianCoordinates*> *positions = GetUniformUsersDistribution (j, nbUE);

      //Create UEs
      //for (int i = 0; i < nbUE; i++)
        //{
	      //ue's random position
    	  //double posX = positions->at (i)->GetCoordinateX ();
    	  //double posY = positions->at (i)->GetCoordinateY ();
    	  //double speedDirection = (double)(rand() %360) * ((2*3.14)/360);;

    	  UserEquipment* ue = new UserEquipment (idUE,
												 (1+j), 1, 0, 0,
												 cells->at (j),
												 eNBs->at (j),
												 1, //HO activated!
												 Mobility::RANDOM_DIRECTION);
	  if(j == 0){
	    ue->isRelay = false;
	  }
	  else{
	    ue->isRelay = true;
	  }

		  std::cout << "Created UE - id " << idUE << " position " << 1 << " " <<1
				  << ", cell " <<  ue->GetCell ()->GetIdCell ()
				  << ", target enb " << ue->GetTargetNode ()->GetIDNetworkNode () << std::endl;
                // ue->GetMobilityModel()->GetAbsolutePosition()->Print(); // in single cll
		  ue->GetPhy ()->SetDlChannel (eNBs->at (j)->GetPhy ()->GetDlChannel ());
		  ue->GetPhy ()->SetUlChannel (eNBs->at (j)->GetPhy ()->GetUlChannel ());

		  FullbandCqiManager *cqiManager = new FullbandCqiManager ();
		  cqiManager->SetCqiReportingMode (CqiManager::PERIODIC);
		  cqiManager->SetReportingInterval (1);
		  cqiManager->SetDevice (ue);
		  ue->SetCqiManager (cqiManager);

                  // A1 - Should'nt this error model be used in multicell?
         //              WidebandCqiEesmErrorModel *errorModel = new WidebandCqiEesmErrorModel ();
    //  ue->GetPhy ()->SetErrorModel (errorModel);
                  // end A1
      
		  nm->GetUserEquipmentContainer ()->push_back (ue);
                  ue->SetTargetNode (eNBs->at (j)); // added UL

		  // register ue to the enb
		  eNBs->at (j)->RegisterUserEquipment (ue);

		  // define the channel realization
		  MacroCellUrbanAreaChannelRealization* c_dl = new MacroCellUrbanAreaChannelRealization (eNBs->at (j), ue);
		  c_dl->SetChannelType (ChannelRealization::CHANNEL_TYPE_PED_A);
		  eNBs->at (j)->GetPhy ()->GetDlChannel ()->GetPropagationLossModel ()->AddChannelRealization (c_dl);

                  // start added for uplink
                 MacroCellUrbanAreaChannelRealization* c_ul = new MacroCellUrbanAreaChannelRealization ( ue, eNBs->at (j));
                  c_ul->SetChannelType (ChannelRealization::CHANNEL_TYPE_PED_A);
                  eNBs->at (j)->GetPhy ()->GetUlChannel ()->GetPropagationLossModel ()->AddChannelRealization (c_ul);
          
                 ue->GetPhy ()->GetDlChannel ()->AddDevice (ue);
          
          // fin added

		  // CREATE DOWNLINK APPLICATION FOR THIS UE
		  

		  // *** voip application
		  //for (int j_ = 0; j_ < nbVoIP; j_++)
			//{
			  // create application
			  VoIPApplication[voipApplication].SetSource (ue);
			  VoIPApplication[voipApplication].SetDestination (eNBs->at (j));
			  VoIPApplication[voipApplication].SetApplicationID (applicationID);
			  voipApplication++;
			  applicationID++;
			//}

		  

		  idUE++;

		//}
    }
    
    //nm->GetUserEquipmentContainer ()->at(1)->isRelay = true;
    
    ((Application*)&VoIPApplication[0])->setDestApplication((Application*)&VoIPApplication[1]);
    
    double start_time = 0.1; // + GetRandomVariable (5.);
		  double duration_time = start_time + flow_duration;
		  
		  voipApplication = 0;
		  for(int j_ = 0;j_ < nbVoIP;j_++){
    VoIPApplication[voipApplication].SetStartTime(start_time);
			  VoIPApplication[voipApplication].SetStopTime(duration_time);

			  // create qos parameters

		  if (uplink_scheduler_type == ENodeB::ULScheduler_TYPE_MAXIMUM_THROUGHPUT)
			{
			  //QoSForEXP *qos = new QoSForEXP ();
			  //qos->SetMaxDelay (maxDelay);
			  //VoIPApplication[voipApplication].SetQoSParameters (qos);
			  QoSParameters *qos = new QoSParameters ();
			  qos->SetMaxDelay (maxDelay);
			  VoIPApplication[voipApplication].SetQoSParameters (qos);

			}
		  else if (uplink_scheduler_type == ENodeB::ULScheduler_TYPE_FME)
			{
			 // QoSForM_LWDF *qos = new QoSForM_LWDF ();
			  //qos->SetMaxDelay (maxDelay);
			  //VoIPApplication[voipApplication].SetQoSParameters (qos);
			  QoSParameters *qos = new QoSParameters ();
			  qos->SetMaxDelay (maxDelay);
			  VoIPApplication[voipApplication].SetQoSParameters (qos);
			}
		  else if (uplink_scheduler_type == ENodeB::ULScheduler_TYPE_ROUNDROBIN)
			{
			 // QoSForM_LWDF *qos = new QoSForM_LWDF ();
			  //qos->SetMaxDelay (maxDelay);
			  //VoIPApplication[voipApplication].SetQoSParameters (qos);
			  QoSParameters *qos = new QoSParameters ();
			  qos->SetMaxDelay (maxDelay);
			  VoIPApplication[voipApplication].SetQoSParameters (qos);
			}
		  else
			{
			  QoSParameters *qos = new QoSParameters ();
			  qos->SetMaxDelay (maxDelay);
			  VoIPApplication[voipApplication].SetQoSParameters (qos);
			}


			  //create classifier parameters
			  ClassifierParameters *cp = new ClassifierParameters (nm->GetUserEquipmentContainer ()->at(j_)->GetIDNetworkNode(),
													   eNBs->at (j_)->GetIDNetworkNode(), // j because it is multicell scenario?
													   0,
													   destinationPort,
													   TransportProtocol::TRANSPORT_PROTOCOL_TYPE_UDP);
			  VoIPApplication[voipApplication].SetClassifierParameters (cp);

			  std::cout << "CREATED VOIP APPLICATION, ID " << applicationID << std::endl;

			  //update counter
			  destinationPort++;
			  applicationID++;
			  voipApplication++;
		  }
  simulator->SetStop(duration);
  simulator->Schedule(duration-10, &Simulator::PrintMemoryUsage, simulator);
  simulator->Run ();
  
  
  //Delete created objects
  cells->clear ();
  delete cells;
  eNBs->clear ();
  delete eNBs;
  delete frameManager;
  //delete nm;
  delete simulator;

}


/*
static void relayNode ( double radius,double maxDelay)
{

  // define simulation times
  double duration = 60;
  double flow_duration = 50;

  int cluster = 3;
  double bandwidth = 5;
  int seed;

  // CREATE COMPONENT MANAGER
  Simulator *simulator = Simulator::Init();
  FrameManager *frameManager = FrameManager::Init();
  NetworkManager* nm = NetworkManager::Init();

  // CONFIGURE SEED
  if (seed >= 0)
	{
	  int commonSeed = GetCommonSeed (seed);
	  srand (commonSeed);
	}
  else
	{
	  srand (time(NULL));
	}
  std::cout << "Simulation with SEED = " << seed << std::endl;

  // SET SCHEDULING ALLOCATION SCHEME
  ENodeB::ULSchedulerType uplink_scheduler_type;
  
  uplink_scheduler_type = ENodeB::ULScheduler_TYPE_FME;
  std::cout << "Scheduler FME "<< std::endl;

  // SET FRAME STRUCTURE
  FrameManager::FrameStructure frame_structure;
  frameManager->SetFrameStructure(FrameManager::FRAME_STRUCTURE_FDD);




  //create cells
  std::vector <Cell*> *cells = new std::vector <Cell*>;
  int i = 0;

	  Cell *c = new Cell (i, radius, 0.035, 0, 0);
	  cells->push_back (c);
	  nm->GetCellContainer ()->push_back (c);
	  i++;

	  
	  Cell *c1 = new Cell (i, radius/2, 0.035, 1, 1);
	  cells->push_back (c1);
	  nm->GetCellContainer ()->push_back (c1);
	  i++;
	  

  std::vector <BandwidthManager*> spectrums = RunFrequencyReuseTechniques (2, cluster, bandwidth);

  //Create a set of a couple of channels
  std::vector <LteChannel*> *dlChannels = new std::vector <LteChannel*>;
  std::vector <LteChannel*> *ulChannels = new std::vector <LteChannel*>;
  for (int i= 0; i < 2; i++)
    {
	  LteChannel *dlCh = new LteChannel ();
	  dlCh->SetChannelId (i);
	  dlChannels->push_back (dlCh);

	  LteChannel *ulCh = new LteChannel ();
	  ulCh->SetChannelId (i);
	  ulChannels->push_back (ulCh);
    }


  //create eNBs
  std::vector <ENodeB*> *eNBs = new std::vector <ENodeB*>;
  i = 0;
	  ENodeB* enb = new ENodeB (i, cells->at (i),1,100);
	  enb->GetPhy ()->SetDlChannel (dlChannels->at (i));
	  enb->GetPhy ()->SetUlChannel (ulChannels->at (i));
	  enb->GetPhy ()->SetBandwidthManager (spectrums.at (i));

	  enb->SetULScheduler (uplink_scheduler_type);   // added UL
	  enb->SetDLScheduler (ENodeB::DLScheduler_TYPE_PROPORTIONAL_FAIR); // setted by default
      
          
	
	  nm->GetENodeBContainer ()->push_back (enb);
	  eNBs->push_back (enb);
          ulChannels->at (i)->AddDevice((NetworkNode*) enb);
          
	  spectrums.at (i)->Print ();

	  i++;
	  ENodeB* enb1 = new ENodeB (i, cells->at (i),1,30);
	  enb1->GetPhy ()->SetDlChannel (dlChannels->at (i));
	  enb1->GetPhy ()->SetUlChannel (ulChannels->at (i));
	  enb1->GetPhy ()->SetBandwidthManager (spectrums.at (i));

	  enb1->SetULScheduler (uplink_scheduler_type);   // added UL
	  enb1->SetDLScheduler (ENodeB::DLScheduler_TYPE_PROPORTIONAL_FAIR); // setted by default
	  //enb1->isRelay = true;// !!
	  //enb1->relayID = i;
          
	
	  nm->GetENodeBContainer ()->push_back (enb1);
	  eNBs->push_back (enb1);
          ulChannels->at (i)->AddDevice((NetworkNode*) enb1);
          
	  std::cout << "Created enb, id " << enb->GetIDNetworkNode()
			  << ", cell id " << enb->GetCell ()->GetIdCell ()
			  <<", position: " << enb->GetMobilityModel ()->GetAbsolutePosition ()->GetCoordinateX ()
			  << " " << enb->GetMobilityModel ()->GetAbsolutePosition ()->GetCoordinateY ()
			  << ", channels id " << enb->GetPhy ()->GetDlChannel ()->GetChannelId ()
			  << enb->GetPhy ()->GetUlChannel ()->GetChannelId ()  << std::endl;

	  spectrums.at (i)->Print ();




  //Define Application Container
  VoIP VoIPApplication[2];
  //TraceBased VideoApplication[nbVideo*nbCell*nbUE];
  //InfiniteBuffer BEApplication[nbBE*nbCell*nbUE];
  //CBR CBRApplication[nbCBR*nbCell*nbUE];
  int voipApplication = 0;
  int videoApplication = 0;
  int cbrApplication = 0;
  int beApplication = 0;
  int destinationPort = 101;
  int applicationID = 0;



  //Create GW
  Gateway *gw = new Gateway ();
  nm->GetGatewayContainer ()->push_back (gw);


  //nbUE is the number of users that are into each cell at the beginning of the simulation
  int idUE = 2;

	  //users are distributed uniformly intio a cell
	  

	      //ue's random position
    	  double speedDirection = 0;

    	  UserEquipment* ue = new UserEquipment (idUE,
												 1, 0, 0, speedDirection,
												 cells->at (1),
												 eNBs->at (1),
												 1, //HO activated!
												 Mobility::RANDOM_DIRECTION);

                // ue->GetMobilityModel()->GetAbsolutePosition()->Print(); // in single cll
		  ue->GetPhy ()->SetDlChannel (eNBs->at (1)->GetPhy ()->GetDlChannel ());
		  ue->GetPhy ()->SetUlChannel (eNBs->at (1)->GetPhy ()->GetUlChannel ());

		  FullbandCqiManager *cqiManager = new FullbandCqiManager ();
		  cqiManager->SetCqiReportingMode (CqiManager::PERIODIC);
		  cqiManager->SetReportingInterval (1);
		  cqiManager->SetDevice (ue);
		  ue->SetCqiManager (cqiManager);

                  // A1 - Should'nt this error model be used in multicell?
         //              WidebandCqiEesmErrorModel *errorModel = new WidebandCqiEesmErrorModel ();
    //  ue->GetPhy ()->SetErrorModel (errorModel);
                  // end A1
      
		  nm->GetUserEquipmentContainer ()->push_back (ue);
                  ue->SetTargetNode (eNBs->at (1)); // added UL

		  // register ue to the enb
		  eNBs->at (1)->RegisterUserEquipment (ue);

		  // define the channel realization
		  MacroCellUrbanAreaChannelRealization* c_dl = new MacroCellUrbanAreaChannelRealization (eNBs->at (1), ue);
		  c_dl->SetChannelType (ChannelRealization::CHANNEL_TYPE_PED_A);
		  eNBs->at (1)->GetPhy ()->GetDlChannel ()->GetPropagationLossModel ()->AddChannelRealization (c_dl);

                  // start added for uplink
                 MacroCellUrbanAreaChannelRealization* c_ul = new MacroCellUrbanAreaChannelRealization ( ue, eNBs->at (1));
                  c_ul->SetChannelType (ChannelRealization::CHANNEL_TYPE_PED_A);
                  eNBs->at (1)->GetPhy ()->GetUlChannel ()->GetPropagationLossModel ()->AddChannelRealization (c_ul);
          
                 ue->GetPhy ()->GetDlChannel ()->AddDevice (ue);
          
		 // version 2 - relay
		 idUE++;
		 
		 UserEquipment* ue1 = new UserEquipment (idUE,
												 1,1,0, 0,
												 cells->at (0),
												 eNBs->at (0),
												 1, //HO activated!
												 Mobility::RANDOM_DIRECTION);
		 //ue1->isRelay = true;
		 //ue1->relayID = 1;

                // ue->GetMobilityModel()->GetAbsolutePosition()->Print(); // in single cll
		  ue1->GetPhy ()->SetDlChannel (eNBs->at (0)->GetPhy ()->GetDlChannel ());
		  ue1->GetPhy ()->SetUlChannel (eNBs->at (0)->GetPhy ()->GetUlChannel ());

		  FullbandCqiManager *cqiManager1 = new FullbandCqiManager ();
		  cqiManager1->SetCqiReportingMode (CqiManager::PERIODIC);
		  cqiManager1->SetReportingInterval (1);
		  cqiManager1->SetDevice (ue1);
		  ue1->SetCqiManager (cqiManager1);

                  // A1 - Should'nt this error model be used in multicell?
         //              WidebandCqiEesmErrorModel *errorModel = new WidebandCqiEesmErrorModel ();
    //  ue->GetPhy ()->SetErrorModel (errorModel);
                  // end A1
      
		  nm->GetUserEquipmentContainer ()->push_back (ue1);
                  ue1->SetTargetNode (eNBs->at (0)); // added UL

		  // register ue to the enb
		  eNBs->at (0)->RegisterUserEquipment (ue1);

		  // define the channel realization
		  MacroCellUrbanAreaChannelRealization* c_dl1 = new MacroCellUrbanAreaChannelRealization (eNBs->at (0), ue1);
		  c_dl1->SetChannelType (ChannelRealization::CHANNEL_TYPE_PED_A);
		  eNBs->at (0)->GetPhy ()->GetDlChannel ()->GetPropagationLossModel ()->AddChannelRealization (c_dl1);

                  // start added for uplink
                 MacroCellUrbanAreaChannelRealization* c_ul1 = new MacroCellUrbanAreaChannelRealization ( ue1, eNBs->at (0));
                  c_ul1->SetChannelType (ChannelRealization::CHANNEL_TYPE_PED_A);
                  eNBs->at (0)->GetPhy ()->GetUlChannel ()->GetPropagationLossModel ()->AddChannelRealization (c_ul1);
          
                 ue1->GetPhy ()->GetDlChannel ()->AddDevice (ue1);
		 
		 
          // fin added

		  // CREATE DOWNLINK APPLICATION FOR THIS UE
		  double start_time = 0.1; // + GetRandomVariable (5.);
		  double duration_time = start_time + flow_duration;

		  // *** voip application
		  
			  // create application
			  VoIPApplication[voipApplication].SetSource (ue);
			  VoIPApplication[voipApplication].SetDestination (eNBs->at (1));
			  
			  VoIPApplication[voipApplication+1].SetSource (ue1);
			  VoIPApplication[voipApplication+1].SetDestination (eNBs->at (0));
			  
			  //VoIPApplication[voipApplication].setDestApplication(&VoIPApplication[voipApplication+1]);
			  
			  //continue
			  VoIPApplication[voipApplication].SetApplicationID (applicationID);
			  VoIPApplication[voipApplication].SetStartTime(start_time);
			  VoIPApplication[voipApplication].SetStopTime(duration_time);
			  
			  QoSParameters *qos = new QoSParameters ();
			  qos->SetMaxDelay (maxDelay);
			  VoIPApplication[voipApplication].SetQoSParameters (qos);
			

			  //create classifier parameters
			  ClassifierParameters *cp = new ClassifierParameters (ue->GetIDNetworkNode(),
													   eNBs->at (1)->GetIDNetworkNode(), // j because it is multicell scenario?
													   0,
													   destinationPort,
													   TransportProtocol::TRANSPORT_PROTOCOL_TYPE_UDP);
			  VoIPApplication[voipApplication].SetClassifierParameters (cp);

			  std::cout << "CREATED VOIP APPLICATION, ID " << applicationID << std::endl;

			  //update counter
			  destinationPort++;
			  applicationID++;
			  voipApplication++;
			  
			  ////
			  // create application
			  // - missing
			  VoIPApplication[voipApplication].SetApplicationID (applicationID);
			  VoIPApplication[voipApplication].SetStartTime(start_time);
			  VoIPApplication[voipApplication].SetStopTime(duration_time);

			  QoSParameters *qos1 = new QoSParameters ();
			  qos1->SetMaxDelay (maxDelay);
			  VoIPApplication[voipApplication].SetQoSParameters (qos1);
			

			  //create classifier parameters
			  ClassifierParameters *cp1 = new ClassifierParameters (ue1->GetIDNetworkNode(),
													   eNBs->at (0)->GetIDNetworkNode(), // j because it is multicell scenario?
													   0,
													   destinationPort,
													   TransportProtocol::TRANSPORT_PROTOCOL_TYPE_UDP);
			  VoIPApplication[voipApplication].SetClassifierParameters (cp1);

			  std::cout << "CREATED VOIP APPLICATION, ID " << applicationID << std::endl;

			  //update counter
			  destinationPort++;
			  applicationID++;
			  voipApplication++;
			

		  idUE++;

	

  simulator->SetStop(duration);
  simulator->Schedule(duration-10, &Simulator::PrintMemoryUsage, simulator);
  simulator->Run ();
  
  
  //Delete created objects
  cells->clear ();
  delete cells;
  eNBs->clear ();
  delete eNBs;
  delete frameManager;
  //delete nm;
  delete simulator;

}
*/
