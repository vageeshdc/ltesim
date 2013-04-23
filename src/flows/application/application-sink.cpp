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


#include "application-sink.h"
#include "../../device/IPClassifier/ClassifierParameters.h"
#include "../../componentManagers/NetworkManager.h"

#include "../../load-parameters.h"
#include "../../device/UserEquipment.h"

ApplicationSink::ApplicationSink()
{
  m_classifierParameters = NULL;
  m_radioBearer = NULL;
  m_sourceApplication = NULL;
}

ApplicationSink::~ApplicationSink()
{
  m_classifierParameters = NULL;
  m_radioBearer = NULL;
  m_sourceApplication = NULL;
}

void
ApplicationSink::SetClassifierParameters (ClassifierParameters* cp)
{
  m_classifierParameters = cp;
}

ClassifierParameters*
ApplicationSink::GetClassifierParameters (void)
{
  return m_classifierParameters;
}


void
ApplicationSink::SetRadioBearerSink (RadioBearerSink* r)
{
  m_radioBearer = r;
}

RadioBearerSink*
ApplicationSink::GetRadioBearerSink (void)
{
  return m_radioBearer;
}

void
ApplicationSink::SetSourceApplication (Application* a)
{
  m_sourceApplication = a;
}

void
ApplicationSink::SetDestApplication (Application* a)
{
  m_destApplication = a;
}

Application*
ApplicationSink::GetSourceApplication (void)
{
  return m_sourceApplication;
}

void
ApplicationSink::Receive (Packet* p)
{
  /*
   * Trace format:
   *
   * TX   APPLICATION_TYPE   BEARER_ID  SIZE   SRC_ID   DST_ID   TIME
   */
  
  if (!_APP_TRACING_) return;

  std::cout << "RX";

  switch (m_sourceApplication->GetApplicationType ())
	{
	  case Application::APPLICATION_TYPE_VOIP:
		{
		  std::cout << " VOIP";
		  break;
		}
	  case Application::APPLICATION_TYPE_TRACE_BASED:
		{
		  std::cout << " VIDEO";
		  break;
		}
	  case Application::APPLICATION_TYPE_CBR:
		{
		  std::cout << " CBR";
		  break;
		}
	  case Application::APPLICATION_TYPE_INFINITE_BUFFER:
		{
		  std::cout << " INF_BUF";
		  break;
		}
	  default:
		{
		  std::cout << " UNDEFINED";
		  break;
		}
	}


  double delay = ((Simulator::Init()->Now() *10000) - (p->GetTimeStamp () *10000)) /10000;
  if (delay < 0.000001) delay = 0.000001;

  UserEquipment* ue = (UserEquipment*) GetSourceApplication ()->GetDestination ();

  std::cout << " ID " << p->GetID ()
                        << " B " << m_sourceApplication->GetApplicationID ()
                        << " SIZE " << p->GetPacketTags ()->GetApplicationSize ()
                        << " SRC " << p->GetSourceID ()
                        << " DST " << p->GetDestinationID ()
                        << " D " << delay
                        << " " << ue->IsIndoor () << std::endl;


			
  
      double currentTime = Simulator::Init()->Now ();
      double timePrimeI,timePrimeF;
      
      double ttim = (1/m_sourceApplication->alfTime)*(currentTime);
      /*
	while((( ((int)ttim) % totalFrames) >= m_sourceApplication->alfStartFrame) && (( ((int)ttim) % totalFrames) <= alfEndFrame)){
	  ttim += alfStep;
	}
	timePrimeI  = ((ttim*alfTime - currentTime) > time)?(ttim*alfTime - currentTime):time;
	std::cout<<"The time value I "<<timePrimeI<<" at "<< ttim*alfTime<<"\n";
	
      ttim = (1/alfTime)*(currentTime+time);
      */
      //if((alfStartFrame == 0) &&(alfEndFrame == totalFrames-1))
      //{
	//do nothing
      //}else{
	while((( ((int)ttim) % m_sourceApplication->totalFrames) < m_sourceApplication->alfStartFrame) || 
	     (( ((int)ttim) % m_sourceApplication->totalFrames) > m_sourceApplication->alfEndFrame)){
	  ttim += m_sourceApplication->alfStep;
	}
      //}
	timePrimeF  = ((ttim*m_sourceApplication->alfTime - currentTime) > 0)?(ttim*m_sourceApplication->alfTime - currentTime):0;
	std::cout<<"The time value F "<<timePrimeF<<" at "<< ttim*m_sourceApplication->alfTime<<"\n";
      
			
  if(GetSourceApplication ()->GetDestination ()->isRelay){
    
    switch(GetSourceApplication()->GetApplicationID()){
      //
      case Application::APPLICATION_TYPE_CBR:
	Simulator::Init()->Schedule(timePrimeF,&CBR::SendPkt , (CBR*)m_destApplication,p);
	break;
      case Application::APPLICATION_TYPE_INFINITE_BUFFER:
	Simulator::Init()->Schedule(timePrimeF,&InfiniteBuffer::DoStart ,(InfiniteBuffer*)m_destApplication);
	std::cout<<"Relayed the pkt\n";
	break;
      //case Application::APPLICATION_TYPE_TRACE_BASED:
	//Simulator::Init()->Schedule(0.02,&TraceBased::SendPkt , m_destApplication,p);
	//break;
      case Application::APPLICATION_TYPE_VOIP:
	Simulator::Init()->Schedule(timePrimeF,&VoIP::SendPkt ,(VoIP*) m_destApplication,p);
	std::cout<<"Relayed the pkt\n";
	break;
      case Application::APPLICATION_TYPE_WEB:
	Simulator::Init()->Schedule(timePrimeF,&WEB::SendPkt , (WEB*)m_destApplication,p);
	break;
    }
  }
			
  delete p;
}
