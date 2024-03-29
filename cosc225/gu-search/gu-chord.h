/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 University of Pennsylvania
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef GU_CHORD_H
#define GU_CHORD_H

#include "ns3/gu-application.h"
#include "ns3/gu-chord-message.h"
#include "ns3/ping-request.h"

#include <openssl/sha.h>
#include <stdio.h>
#include <iomanip>

#include "ns3/ipv4-address.h"
#include <map>
#include <set>
#include <vector>
#include <string>
#include "ns3/socket.h"
#include "ns3/nstime.h"
#include "ns3/timer.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"

#include "gmp.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace ns3;

class GUChord : public GUApplication
{
  public:
    static TypeId GetTypeId (void);
    GUChord ();
    virtual ~GUChord ();

    void RunStabilize ();
    void SendPing (Ipv4Address destAddress, std::string pingMessage);
    void RecvMessage (Ptr<Socket> socket);
    void ProcessPingReq (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessPingRsp (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessJoinReq (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void SendJoinRsp (GUChordMessage, uint16_t);
    void ProcessJoinRsp (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessDepartureReq (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessStabilizeReq (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessStabilizeRsp (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessRingStatePing (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessFindSuccessorReq (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessFindSuccessorRsp (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessLookupReq (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessLookupRsp (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    
    void AuditPings ();
    uint32_t GetNextTransactionId ();
    void StopChord ();

    // Callback with Application Layer (add more when required)
    void SetPingSuccessCallback (Callback <void, Ipv4Address, std::string> pingSuccessFn);
    void SetPingFailureCallback (Callback <void, Ipv4Address, std::string> pingFailureFn);
    void SetPingRecvCallback (Callback <void, Ipv4Address, std::string> pingRecvFn);
    
    void SetChordLookupCallback (Callback <void, Ipv4Address, uint32_t, std::string, uint32_t> chordLookup);

    void SetChordLeaveCallback (Callback <void, Ipv4Address, uint32_t> chordLeave);

    void SetPredecessorChangeCallback (Callback <void, Ipv4Address, std::string> predChange);

    // From GUApplication
    virtual void ProcessCommand (std::vector<std::string> tokens);
       
    void FingerInit(int);

    void SendChordLookup(std::string, uint32_t);

    bool isSuccessor(mpz_t, mpz_t, mpz_t);
    bool isInBetween(mpz_t, mpz_t, mpz_t);
    
    void FingerFix(uint32_t);

    struct FingerTableEntry {

        std::string start_value;
        Ipv4Address finger_ip_address;
        std::string finger_node_id;
        std::string finger_key_hash;

     };
    

     // start of new Chord variables
    uint32_t successor_id;
    uint32_t predecessor_id;
    Ipv4Address successor_ip_address;
    Ipv4Address predecessor_ip_address;
    std::string successor_node_key_hex;
    std::string predecessor_node_key_hex;
    
    Timer stabilize_timer;
    Time stabilize_timeout;

    bool in_ring;
    bool show_next_stabilize;
    bool stabilization_messages;
    int counter;

    std::string my_node_key_hex;

  protected:
    virtual void DoDispose ();
    
  private:
    virtual void StartApplication (void);
    virtual void StopApplication (void);

    std::string ipHash(Ipv4Address);

    uint32_t m_currentTransactionId;
    Ptr<Socket> m_socket;
    Time m_pingTimeout;
    uint16_t m_appPort;
    // Timers
    Timer m_auditPingsTimer;
    // Ping tracker
    std::map<uint32_t, Ptr<PingRequest> > m_pingTracker;
    // Callbacks
    Callback <void, Ipv4Address, std::string> m_pingSuccessFn;
    Callback <void, Ipv4Address, std::string> m_pingFailureFn;
    Callback <void, Ipv4Address, std::string> m_pingRecvFn;
    Callback <void, Ipv4Address, uint32_t, std::string, uint32_t> m_chordLookup;
    Callback <void, Ipv4Address, uint32_t> m_chordLeave;
    Callback <void, Ipv4Address, std::string> m_predChange;
    

    /*// start of new Chord variables
    uint32_t successor_id;
    uint32_t predecessor_id;
    Ipv4Address successor_ip_address;
    Ipv4Address predecessor_ip_address;
    std::string successor_node_key_hex;
    std::string predecessor_node_key_hex;
    
    Timer stabilize_timer;
    Time stabilize_timeout;

    bool in_ring;
    bool show_next_stabilize;
    bool stabilization_messages;
    int counter;

    std::string my_node_key_hex;*/

    //std::map<std::string, FingerInfo> finger_table;
    std::vector<FingerTableEntry> finger_table;

};

#endif


