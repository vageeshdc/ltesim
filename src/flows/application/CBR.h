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



#ifndef CBR_H_
#define CBR_H_

#include "../../core/eventScheduler/simulator.h"
#include "Application.h"

class CBR : public Application {
public:
	CBR();
	virtual ~CBR();

	virtual void DoStart (void);
	virtual void DoStop (void);

    void
    ScheduleTransmit (double time);
    void
    Send (void);
    void
    SendPkt (Packet* pRed);
    void
    SetSize(int size);
    int
    GetSize (void) const;
    void
    SetInterval(double interval);
    double
    GetInterval (void) const;

private:

	double m_interval;
	int m_size;
};

#endif /* CBR_H_ */
