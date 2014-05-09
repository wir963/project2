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


#include "gu-chord.h"

#include "ns3/random-variable.h"
#include "ns3/inet-socket-address.h"

#include <vector>

#include <sstream>

#include <stdlib.h>

using namespace ns3;

TypeId
GUChord::GetTypeId ()
{
  static TypeId tid = TypeId ("GUChord")
    .SetParent<GUApplication> ()
    .AddConstructor<GUChord> ()
    .AddAttribute ("AppPort",
                   "Listening port for Application",
                   UintegerValue (10001),
                   MakeUintegerAccessor (&GUChord::m_appPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PingTimeout",
                   "Timeout value for PING_REQ in milliseconds",
                   TimeValue (MilliSeconds (2000)),
                   MakeTimeAccessor (&GUChord::m_pingTimeout),
                   MakeTimeChecker ())
    ;
  return tid;
}

GUChord::GUChord ()
  : m_auditPingsTimer (Timer::CANCEL_ON_DESTROY)
{
  RandomVariable random;
  SeedManager::SetSeed (time (NULL));
  random = UniformVariable (0x00000000, 0xFFFFFFFF);
  m_currentTransactionId = random.GetInteger ();

}

GUChord::~GUChord ()
{

}

void
GUChord::DoDispose ()
{
  StopApplication ();
  GUApplication::DoDispose ();
}

std::string
GUChord::ipHash(Ipv4Address ip_address) 
{

    std::stringstream strs;
    strs << ip_address;
    std::string ip_string = strs.str();
    char const* ip_char_star = ip_string.c_str();

    unsigned char const* ip_char_star_2 = NULL;

    ip_char_star_2 = reinterpret_cast<unsigned char const *>(ip_char_star);

    unsigned char sha_input[20];
  
    SHA1(ip_char_star_2, strlen(ip_char_star), sha_input);

    std::string node_key_hex = "";

    for (int i = 0; i < 20; i++) {

        std::ostringstream strys;
        strys << std::hex << std::setfill('0') << std::setw(2)
              << static_cast<int>(sha_input[i]);
        std::string temp = strys.str();

        node_key_hex.append(temp);    
    }

    return node_key_hex;


}

void
GUChord::StartApplication (void)
{

  counter = 0;
  stabilization_messages = false;
  show_next_stabilize = false;

  my_node_key_hex = "";

  my_node_key_hex = ipHash(GetLocalAddress());
  std::cout << "Node: " << atoi(ReverseLookup(GetLocalAddress()).c_str()) <<  " " << my_node_key_hex << std::endl;

  if (m_socket == 0)
    { 
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny(), m_appPort);
      m_socket->Bind (local);
      m_socket->SetRecvCallback (MakeCallback (&GUChord::RecvMessage, this));
    }  


  // Configure timers
  m_auditPingsTimer.SetFunction (&GUChord::AuditPings, this);
  // Start timers
  m_auditPingsTimer.Schedule (m_pingTimeout);
    stabilize_timeout = MilliSeconds (2000);
    stabilize_timer.SetFunction (&GUChord::RunStabilize, this);
    stabilize_timer.Schedule(stabilize_timeout);

}

void
GUChord::StopApplication (void)
{
  // Close socket
  if (m_socket)
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      m_socket = 0;
    }

  // Cancel timers
  m_auditPingsTimer.Cancel ();
    stabilize_timer.Cancel();

  m_pingTracker.clear ();
}

Ipv4Address
GUChord::findSuccessor()
{
    

        return GetLocalAddress();
}

void GUChord::SendChordLookup(std::string, uint32_t)
{



}

// i = 1 means manipulating index 0
void
GUChord::FingerFix(uint32_t i)
{
  if (finger_table.size() != 160)
    return;
  mpz_t prev_finger_start;
  mpz_t prev_finger_node;
  mpz_t curr_finger_start;
  for (;i <= 160; i++)
  {
    mpz_init_set_str(prev_finger_start, finger_table[i-2].start_value.c_str(), 16);
    mpz_init_set_str(prev_finger_node, finger_table[i-2].finger_key_hash.c_str(), 16);
    mpz_init_set_str(curr_finger_start, finger_table[i-1].start_value.c_str(), 16);
    if (isInBetween(prev_finger_start, curr_finger_start, prev_finger_node))
    {
      uint32_t transactionId = GetNextTransactionId ();

      Ptr<Packet> packet = Create<Packet> ();
      GUChordMessage guChordMessage = GUChordMessage (GUChordMessage::FIND_SUCCESSOR_REQ, transactionId );

      guChordMessage.SetFindSuccessorReq (atoi(ReverseLookup(GetLocalAddress()).c_str()), GetLocalAddress(), finger_table[i-1].start_value, i-1);
      packet->AddHeader (guChordMessage);
      m_socket->SendTo (packet, 0 , InetSocketAddress (successor_ip_address, m_appPort));
      break;
    }
  }
}

bool GUChord::isInBetween(mpz_t my_key_gmp, mpz_t lookup_key_gmp, mpz_t successor_key_gmp)
{
  if(mpz_cmp(lookup_key_gmp, successor_key_gmp) <= 0 && mpz_cmp(lookup_key_gmp, my_key_gmp) > 0)
        return true;
  if (mpz_cmp(my_key_gmp, successor_key_gmp) > 0 && mpz_cmp(lookup_key_gmp, my_key_gmp) > 0)
        return true;
  if (mpz_cmp(my_key_gmp, successor_key_gmp) > 0 && mpz_cmp(lookup_key_gmp, successor_key_gmp) < 0)
        return true;
  return false;
}

// i = 1 when manipulating index 0
void
GUChord::FingerInit(int i)
{

    mpz_t my_key_gmp;
    mpz_init_set_str(my_key_gmp, my_node_key_hex.c_str() , 16);

    mpz_t mod_value;
    mpz_init_set_ui(mod_value, 0);
    mpz_ui_pow_ui(mod_value, 2, 160);

    //for (unsigned int i = 1; i <= 160; i++)
    //{

        mpz_t add_value;
        mpz_init(add_value);
        mpz_ui_pow_ui(add_value, 2, i-1);

        mpz_t start_value;
        mpz_init_set(start_value, add_value); 

        mpz_add(start_value, start_value, my_key_gmp);
        mpz_mod(start_value, start_value, mod_value);

        char* temp = NULL;
        temp = mpz_get_str(temp, 16, start_value);
        std::string start_value_string(temp);

        FingerTableEntry entry;
        entry.start_value = start_value_string;
        
        finger_table.push_back(entry);

        uint32_t transactionId = GetNextTransactionId ();
        uint32_t index = i - 1;

        CHORD_LOG ("Sending FIND_SUCCESSOR_REQ to Node: " << ReverseLookup(successor_ip_address) << " IP: " << successor_ip_address << " transactionId: " << transactionId); 

         Ptr<Packet> packet = Create<Packet> ();
         GUChordMessage guChordMessage = GUChordMessage (GUChordMessage::FIND_SUCCESSOR_REQ, transactionId );

         guChordMessage.SetFindSuccessorReq (atoi(ReverseLookup(GetLocalAddress()).c_str()), GetLocalAddress(), start_value_string, index );
         packet->AddHeader (guChordMessage);
         m_socket->SendTo (packet, 0 , InetSocketAddress (successor_ip_address, m_appPort));

} 

void
GUChord::ProcessFindSuccessorRsp (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{

    std::string fromNode = ReverseLookup (sourceAddress);
    CHORD_LOG ("Received FIND_SUCCESSOR_RSP, From Node: " << fromNode);

    std::string successor_node_key_hex = ipHash(message.GetFindSuccessorRsp().successor_node_ip_address);
    
    finger_table.at(message.GetFindSuccessorRsp().start_value_index).start_value = message.GetFindSuccessorRsp().start_value;
    finger_table.at(message.GetFindSuccessorRsp().start_value_index).finger_ip_address = message.GetFindSuccessorRsp().successor_node_ip_address;
    finger_table.at(message.GetFindSuccessorRsp().start_value_index).finger_node_id = ReverseLookup(message.GetFindSuccessorRsp().successor_node_ip_address);
    finger_table.at(message.GetFindSuccessorRsp().start_value_index).finger_key_hash = successor_node_key_hex;
    
    if (finger_table.size() != 160) {
        FingerInit(message.GetFindSuccessorRsp().start_value_index+2);
    }
    else {        
        // then call it with index+1
        FingerFix(message.GetFindSuccessorRsp().start_value_index+2);
    }
    
}

void
GUChord::ProcessCommand (std::vector<std::string> tokens)
{
  std::vector<std::string>::iterator iterator = tokens.begin();
  std::string command = *iterator;

  Ipv4Address my_ip = GetLocalAddress();
  uint32_t my_id = atoi(ReverseLookup(my_ip).c_str());
  
  std::cout << "**************************************************************************" << std::endl;
  std::cout << "\nThe command is " << my_id << " GUSEARCH CHORD " << command;

  if (command == "join" || command == "JOIN")
    {

          iterator++;
          std::istringstream sin (*iterator);
          std::string nodeNumber;
          sin >> nodeNumber;

          std::cout << " " << nodeNumber << std::endl;
          std::cout << "\n**************************************************************************";

          Ipv4Address destAddress = ResolveNodeIpAddress (nodeNumber);
          uint32_t transactionId = GetNextTransactionId ();

          Ipv4Address my_ip = GetLocalAddress();
          uint32_t my_id = atoi(ReverseLookup(my_ip).c_str());
          uint32_t recipient_id = atoi(nodeNumber.c_str());
          std::string recipient_node_key_hex = ipHash(destAddress);


          mpz_t recipient_key_gmp;
          mpz_init_set_str(recipient_key_gmp, recipient_node_key_hex.c_str() , 16);
  
          mpz_t my_key_gmp;
          mpz_init_set_str(my_key_gmp, my_node_key_hex.c_str() , 16);

          if(mpz_cmp(recipient_key_gmp, my_key_gmp) == 0) {

              successor_id = my_id;
              successor_ip_address = my_ip;
              successor_node_key_hex = my_node_key_hex;
              predecessor_id = my_id;
              predecessor_ip_address = my_ip;
              predecessor_node_key_hex = my_node_key_hex;
                
              mpz_t mod_value;
              mpz_init_set_ui(mod_value, 0);
              mpz_ui_pow_ui(mod_value, 2, 160);
              
              for (unsigned int i = 1; i <= 160; i++)
              {

                  mpz_t add_value;
                  mpz_init(add_value);
                  mpz_ui_pow_ui(add_value, 2, i-1);
             
                  mpz_t start_value;
                  mpz_init_set(start_value, add_value);
             
                  mpz_add(start_value, start_value, my_key_gmp);
                  mpz_mod(start_value, start_value, mod_value);
             
                  char* temp = NULL;
                  temp = mpz_get_str(temp, 16, start_value);
                  std::string start_value_string(temp);

                  FingerTableEntry entry;
                  entry.start_value = start_value_string;
                  entry.finger_node_id = ReverseLookup(GetLocalAddress());
                  entry.finger_ip_address = GetLocalAddress();
                  entry.finger_key_hash = my_node_key_hex;
                  //std::cout << "HI HATERZ" << GetLocalAddress() << std::endl;
                  
                  finger_table.push_back(entry);
              }

                in_ring = true;

          }

          else {

          CHORD_LOG ("Sending JOIN_REQ to Node: " << ReverseLookup(destAddress) << " IP: " << destAddress << " transactionId: " << transactionId);

          Ptr<Packet> packet = Create<Packet> ();
          GUChordMessage guChordMessage = GUChordMessage (GUChordMessage::JOIN_REQ, transactionId );

          guChordMessage.SetJoinReq (my_id, my_ip, recipient_id, destAddress);
          packet->AddHeader (guChordMessage);
          m_socket->SendTo (packet, 0 , InetSocketAddress (destAddress, m_appPort)); 
          
          }
        
    }
  else if (command == "leave" || command == "LEAVE")
    {
          std::cout << std::endl;
          std::cout << "\n**************************************************************************";

          uint32_t transactionId = GetNextTransactionId ();
          Ipv4Address my_ip = GetLocalAddress();
          uint32_t my_id = atoi(ReverseLookup(my_ip).c_str());

          in_ring = false;

          CHORD_LOG ("Sending DEPARTURE_REQ to Node: " << ReverseLookup(successor_ip_address) << " IP: " << successor_ip_address << " transactionId: " << transactionId);

          Ptr<Packet> packet = Create<Packet> ();
          GUChordMessage guChordMessage = GUChordMessage (GUChordMessage::DEPARTURE_REQ, transactionId );

          guChordMessage.SetDepartureReq (my_id, my_ip, predecessor_id, predecessor_ip_address);
          packet->AddHeader (guChordMessage);
          m_socket->SendTo (packet, 0 , InetSocketAddress (successor_ip_address, m_appPort));

          CHORD_LOG ("Sending DEPARTURE_REQ to Node: " << ReverseLookup(predecessor_ip_address) << " IP: " << predecessor_ip_address << " transactionId: " << transactionId);

          Ptr<Packet> packet2 = Create<Packet> ();
          GUChordMessage guChordMessage2 = GUChordMessage        (GUChordMessage::DEPARTURE_REQ, transactionId );

          guChordMessage2.SetDepartureReq (my_id, my_ip, successor_id, successor_ip_address);
          packet2->AddHeader (guChordMessage2);
          m_socket->SendTo (packet2, 0 , InetSocketAddress (predecessor_ip_address, m_appPort));


    }

    else if (command == "ringstate" || command == "RINGSTATE")
    {

          std::cout << std::endl;
          std::cout << "\n**************************************************************************";

          uint32_t transactionId = GetNextTransactionId ();
          Ipv4Address my_ip = GetLocalAddress();
          uint32_t my_id = atoi(ReverseLookup(my_ip).c_str());

for(unsigned int i = 0; i < finger_table.size(); i++) {

            std::cout << "index: " << i << " start_value: " << finger_table[i].start_value << "succ id: " << finger_table[i].finger_ip_address << std::endl;

        }

          CHORD_LOG ("\nRingState<" << my_id << ">: Pred<" << predecessor_id << ", " << predecessor_node_key_hex << ">, Succ<" << successor_id << ", " << successor_node_key_hex << ">");

          //CHORD_LOG ("Sending RING_STATE_PING to Node: " << ReverseLookup(successor_ip_address) << " IP: " << successor_ip_address << " transactionId: " << transactionId);

          Ptr<Packet> packet = Create<Packet> ();
          GUChordMessage guChordMessage = GUChordMessage (GUChordMessage::RING_STATE_PING, transactionId );

          guChordMessage.SetRingStatePing (my_id, my_ip);
          packet->AddHeader (guChordMessage);
          m_socket->SendTo (packet, 0 , InetSocketAddress (successor_ip_address, m_appPort));

    }

    else if (command == "stable" || command == "STABLE") {

        iterator++;
        std::istringstream sin (*iterator);
        std::string flag;
        sin >> flag;

        std::cout << " " << flag << std::endl;
          std::cout << "\n**************************************************************************\n";

        if(flag == "on" || flag == "ON")
           stabilization_messages = true;

        else if(flag == "off" || flag == "OFF"){
           stabilization_messages = false;
           counter = 1999; }

        else
           std::cout << "Invalid Command" << std::endl;
                


    }
}

void
GUChord::RunStabilize ()
{

    counter += 1;

    if(stabilization_messages == true) {

     if(counter%2000 == 0)
           show_next_stabilize = true;
     else
           show_next_stabilize = false;

    }

    // send stabilize to your successor
    if (in_ring == true)
    {

       Ipv4Address my_ip = GetLocalAddress();
       uint32_t my_id = atoi(ReverseLookup(my_ip).c_str());
       uint32_t transactionId = GetNextTransactionId ();
       GUChordMessage resp = GUChordMessage (GUChordMessage::STABILIZE_REQ, transactionId);
       resp.SetStabilizeReq (my_id, my_ip);

       if (show_next_stabilize == true) {

         CHORD_LOG ("Sending STABILIZE_REQ to Node: " << ReverseLookup(successor_ip_address) << " IP: " << successor_ip_address << " transactionId: " << transactionId);

        }

       Ptr<Packet> packet = Create<Packet> ();
       packet->AddHeader (resp);
       m_socket->SendTo (packet, 0 , InetSocketAddress (successor_ip_address, m_appPort));

       finger_table[0].finger_ip_address = successor_ip_address;
       finger_table[0].finger_node_id = successor_id;
       finger_table[0].finger_key_hash = successor_node_key_hex;

       FingerFix(2);

     }
     

     stabilize_timer.Schedule(stabilize_timeout);

}

void
GUChord::SendPing (Ipv4Address destAddress, std::string pingMessage)
{
  if (destAddress != Ipv4Address::GetAny ())
    {
      uint32_t transactionId = GetNextTransactionId ();
      CHORD_LOG ("Sending PING_REQ to Node: " << ReverseLookup(destAddress) << " IP: " << destAddress << " Message: " << pingMessage << " transactionId: " << transactionId);
      Ptr<PingRequest> pingRequest = Create<PingRequest> (transactionId, Simulator::Now(), destAddress, pingMessage);
      // Add to ping-tracker
      m_pingTracker.insert (std::make_pair (transactionId, pingRequest));
      Ptr<Packet> packet = Create<Packet> ();
      GUChordMessage message = GUChordMessage (GUChordMessage::PING_REQ, transactionId);
      message.SetPingReq (pingMessage);
      packet->AddHeader (message);
      m_socket->SendTo (packet, 0 , InetSocketAddress (destAddress, m_appPort));
    }
  else
    {
      // Report failure   
      m_pingFailureFn (destAddress, pingMessage);
    }
}

void
GUChord::RecvMessage (Ptr<Socket> socket)
{
  Address sourceAddr;
  Ptr<Packet> packet = socket->RecvFrom (sourceAddr);
  InetSocketAddress inetSocketAddr = InetSocketAddress::ConvertFrom (sourceAddr);
  Ipv4Address sourceAddress = inetSocketAddr.GetIpv4 ();
  uint16_t sourcePort = inetSocketAddr.GetPort ();
  GUChordMessage message;
  packet->RemoveHeader (message);

  switch (message.GetMessageType ())
    {
      case GUChordMessage::PING_REQ:
        ProcessPingReq (message, sourceAddress, sourcePort);
        break;
      case GUChordMessage::PING_RSP:
        ProcessPingRsp (message, sourceAddress, sourcePort);
        break;
      case GUChordMessage::JOIN_REQ:
        ProcessJoinReq (message, sourceAddress, sourcePort);
        break;
      case GUChordMessage::JOIN_RSP:
        ProcessJoinRsp (message, sourceAddress, sourcePort);
        break;
      case GUChordMessage::DEPARTURE_REQ:
        ProcessDepartureReq (message, sourceAddress, sourcePort);
        break;
      case GUChordMessage::STABILIZE_REQ:
        ProcessStabilizeReq (message, sourceAddress, sourcePort);
        break;
      case GUChordMessage::STABILIZE_RSP:
        ProcessStabilizeRsp (message, sourceAddress, sourcePort);
        break;
      case GUChordMessage::RING_STATE_PING:
        ProcessRingStatePing (message, sourceAddress, sourcePort);
        break;
      case GUChordMessage::FIND_SUCCESSOR_REQ:
        ProcessFindSuccessorReq (message, sourceAddress, sourcePort);
        break;
      case GUChordMessage::FIND_SUCCESSOR_RSP:
        ProcessFindSuccessorRsp(message, sourceAddress, sourcePort);
        break;
      case GUChordMessage::FIND_PREDECESSOR_REQ:
        ProcessFindPredecessorReq (message, sourceAddress, sourcePort);
        break;
      case GUChordMessage::FIND_PREDECESSOR_RSP:
        ProcessFindPredecessorRsp (message, sourceAddress, sourcePort);
        break;
      case GUChordMessage::FIND_PREDECESSOR_ACK:
        ProcessFindPredecessorAck (message, sourceAddress, sourcePort);
        break;
      default:
        ERROR_LOG ("Unknown Message Type!");
        break;
    }
}

void
GUChord::ProcessPingReq (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{

    // Use reverse lookup for ease of debug
    std::string fromNode = ReverseLookup (sourceAddress);
    CHORD_LOG ("Received PING_REQ, From Node: " << fromNode << ", Message: " << message.GetPingReq().pingMessage);
    // Send Ping Response
    GUChordMessage resp = GUChordMessage (GUChordMessage::PING_RSP, message.GetTransactionId());
    resp.SetPingRsp (message.GetPingReq().pingMessage);
    Ptr<Packet> packet = Create<Packet> ();
    packet->AddHeader (resp);
    m_socket->SendTo (packet, 0 , InetSocketAddress (sourceAddress, sourcePort));
    // Send indication to application layer okay
    m_pingRecvFn (sourceAddress, message.GetPingReq().pingMessage);
}

void
GUChord::ProcessPingRsp (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{
  // Remove from pingTracker
  std::map<uint32_t, Ptr<PingRequest> >::iterator iter;
  iter = m_pingTracker.find (message.GetTransactionId ());
  if (iter != m_pingTracker.end ())
    {
      std::string fromNode = ReverseLookup (sourceAddress);
      CHORD_LOG ("Received PING_RSP, From Node: " << fromNode << ", Message: " << message.GetPingRsp().pingMessage);
      m_pingTracker.erase (iter);
      // Send indication to application layer
      m_pingSuccessFn (sourceAddress, message.GetPingRsp().pingMessage);
    }
  else
    {
      DEBUG_LOG ("Received invalid PING_RSP!");
    }
}

void
GUChord::ProcessJoinReq (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{

    std::string fromNode = ReverseLookup (sourceAddress);
    CHORD_LOG ("Received JOIN_REQ, From Node: " << fromNode);

    std::string request_node_key_hex = ipHash(message.GetJoinReq().request_ip_address);

    mpz_t request_key_gmp;
    mpz_init_set_str(request_key_gmp, request_node_key_hex.c_str() , 16);
  
    mpz_t my_key_gmp;
    mpz_init_set_str(my_key_gmp, my_node_key_hex.c_str() , 16);

    mpz_t successor_key_gmp;
    mpz_init_set_str(successor_key_gmp, successor_node_key_hex.c_str() , 16);

    // request hash is greater than my hash but less than my successor's hash - obvious case
    if(isSuccessor(my_key_gmp, lookup_key_gmp, successor_key_gmp))
    {    
        uint32_t transactionId = GetNextTransactionId ();

        CHORD_LOG ("Sending JOIN_RSP to Node: " << ReverseLookup(successor_ip_address) << " IP: " << successor_ip_address << " transactionId: " << transactionId);

        message.SetTransactionId(transactionId); 
  
        SendJoinRsp(message, sourcePort);
    }
    else
    {
        // need to keep searching so forward the message along

        uint32_t transactionId = GetNextTransactionId ();

        CHORD_LOG ("Sending JOIN_REQ to Node: " << ReverseLookup(successor_ip_address) << " IP: " << successor_ip_address << " transactionId: " << transactionId);

        GUChordMessage resp = GUChordMessage (GUChordMessage::JOIN_REQ, transactionId);
        resp.SetJoinReq (message.GetJoinReq());
        Ptr<Packet> packet = Create<Packet> ();
        packet->AddHeader (resp);
        m_socket->SendTo (packet, 0 , InetSocketAddress (successor_ip_address, sourcePort));
    }

}

void
GUChord::SendJoinRsp(GUChordMessage message, uint16_t sourcePort)
{

    GUChordMessage resp = GUChordMessage (GUChordMessage::JOIN_RSP, message.GetTransactionId());
    resp.SetJoinRsp (message.GetJoinReq(), successor_id, successor_ip_address);
    Ptr<Packet> packet = Create<Packet> ();
    packet->AddHeader (resp);
    m_socket->SendTo (packet, 0 , InetSocketAddress (successor_ip_address, sourcePort));

}

void
GUChord::ProcessJoinRsp (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{

    std::string fromNode = ReverseLookup (sourceAddress);
    CHORD_LOG ("Received JOIN_RSP, From Node: " << fromNode);    

    std::string landmark_node_key_hex = ipHash(message.GetJoinRsp().landmark_ip_address);
    std::string request_node_key_hex = ipHash(message.GetJoinRsp().request_ip_address);

    mpz_t request_key_gmp;
    mpz_init_set_str(request_key_gmp, request_node_key_hex.c_str() , 16);
  
    mpz_t my_key_gmp;
    mpz_init_set_str(my_key_gmp, my_node_key_hex.c_str() , 16);

    mpz_t landmark_key_gmp;
    mpz_init_set_str(landmark_key_gmp, landmark_node_key_hex.c_str() , 16);

    
    // if you are the landmark node, then send this information to the   request node
    if (mpz_cmp(my_key_gmp, landmark_key_gmp) == 0)
    {

        uint32_t transactionId = GetNextTransactionId ();

        CHORD_LOG ("Sending JOIN_RSP to Node: " << ReverseLookup(message.GetJoinRsp().request_ip_address) << " IP: " << message.GetJoinRsp().request_ip_address << " transactionId: " << transactionId);        

        GUChordMessage resp = GUChordMessage (GUChordMessage::JOIN_RSP, transactionId);
        resp.SetJoinRsp (message.GetJoinRsp());
        Ptr<Packet> packet = Create<Packet> ();
        packet->AddHeader (resp);
        m_socket->SendTo (packet, 0 , InetSocketAddress (message.GetJoinRsp().request_ip_address, sourcePort));
    }

    // if you are the original requester
    else if (mpz_cmp(my_key_gmp, request_key_gmp) == 0)
    { 

        successor_ip_address = message.GetJoinRsp().successor_ip_address;
        successor_id = message.GetJoinRsp().successor_id;
        successor_node_key_hex = ipHash(message.GetJoinRsp().successor_ip_address);

        FingerInit(1);

        in_ring = true;
    }
    else
    {

        uint32_t transactionId = GetNextTransactionId ();

        CHORD_LOG ("Sending JOIN_RSP to Node: " << ReverseLookup(successor_ip_address) << " IP: " << successor_ip_address << " transactionId: " << transactionId);        

        GUChordMessage resp = GUChordMessage (GUChordMessage::JOIN_RSP, transactionId);
        resp.SetJoinRsp (message.GetJoinRsp());
        Ptr<Packet> packet = Create<Packet> ();
        packet->AddHeader (resp);
        m_socket->SendTo (packet, 0 , InetSocketAddress (successor_ip_address, sourcePort));
    }

}

void
GUChord::ProcessDepartureReq (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{

        std::string fromNode = ReverseLookup (sourceAddress);
        CHORD_LOG ("Received DEPARTURE_REQ, From Node: " << fromNode);

        std::string sender_node_key_hex = ipHash(message.GetDepartureReq().sender_node_ip_address);

        mpz_t sender_key_gmp;
        mpz_init_set_str(sender_key_gmp, sender_node_key_hex.c_str() , 16);
  
        mpz_t my_key_gmp;
        mpz_init_set_str(my_key_gmp, my_node_key_hex.c_str() , 16);

        mpz_t predecessor_key_gmp;
        mpz_init_set_str(predecessor_key_gmp, predecessor_node_key_hex.c_str() , 16);

        mpz_t successor_key_gmp;        
        mpz_init_set_str(successor_key_gmp, successor_node_key_hex.c_str() , 16);

        if(mpz_cmp(predecessor_key_gmp, sender_key_gmp) == 0) {

          predecessor_id = message.GetDepartureReq ().conn_node_id;
          predecessor_ip_address = message.GetDepartureReq ().conn_node_ip_address;
          predecessor_node_key_hex = ipHash(message.GetDepartureReq ().conn_node_ip_address);

        }

        else if (mpz_cmp(successor_key_gmp, sender_key_gmp) == 0) {

          successor_id = message.GetDepartureReq ().conn_node_id;
          successor_ip_address = message.GetDepartureReq ().conn_node_ip_address;
          successor_node_key_hex = ipHash(message.GetDepartureReq ().conn_node_ip_address); 

        }

}

void
GUChord::ProcessStabilizeReq (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{       
    uint32_t sender_id = atoi(ReverseLookup(message.GetStabilizeReq().sender_node_ip_address).c_str());

    if (show_next_stabilize == true) {

        std::string fromNode = ReverseLookup (sourceAddress);
        CHORD_LOG ("Received STABILIZE_REQ, From Node: " << fromNode);
    
    }

    std::string sender_node_key_hex = ipHash(message.GetStabilizeReq().sender_node_ip_address);

    mpz_t sender_key_gmp;
    mpz_init_set_str(sender_key_gmp, sender_node_key_hex.c_str() , 16);
  
    mpz_t my_key_gmp;
    mpz_init_set_str(my_key_gmp, my_node_key_hex.c_str() , 16);

    mpz_t predecessor_key_gmp;
    mpz_init_set_str(predecessor_key_gmp, predecessor_node_key_hex.c_str() , 16);

    mpz_t successor_key_gmp;        
    mpz_init_set_str(successor_key_gmp, successor_node_key_hex.c_str() , 16);

    if(isSuccessor(my_key_gmp, lookup_key_gmp, successor_key_gmp))
    {
      predecessor_id = sender_id;
      predecessor_ip_address = message.GetStabilizeReq().sender_node_ip_address;
      predecessor_node_key_hex = ipHash(message.GetStabilizeReq().sender_node_ip_address);
    }
    else if (predecessor_node_key_hex.compare(" ") == -1)
    { 
        predecessor_id = sender_id;
        predecessor_ip_address = message.GetStabilizeReq().sender_node_ip_address;
        predecessor_node_key_hex = ipHash(message.GetStabilizeReq().sender_node_ip_address);
    }

    if (show_next_stabilize == true) {

        std::string fromNode = ReverseLookup (sourceAddress);
        CHORD_LOG ("Sending STABILIZE_RSP, to Node: " << fromNode);
    
    }

    GUChordMessage resp = GUChordMessage (GUChordMessage::STABILIZE_RSP, message.GetTransactionId());
    resp.SetStabilizeRsp (predecessor_id, predecessor_ip_address);
    Ptr<Packet> packet = Create<Packet> ();
    packet->AddHeader (resp);
    m_socket->SendTo (packet, 0 , InetSocketAddress (sourceAddress, sourcePort));

}

void
GUChord::ProcessStabilizeRsp (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{

    if (show_next_stabilize == true) {

        std::string fromNode = ReverseLookup (sourceAddress);
        CHORD_LOG ("Received STABILIZE_RSP, From Node: " << fromNode);
        
        show_next_stabilize = false;
    
    }

    std::string pred_node_key_hex = ipHash(message.GetStabilizeRsp().predecessor_node_ip_address);

    mpz_t my_key_gmp;
    mpz_init_set_str(my_key_gmp, my_node_key_hex.c_str() , 16);

    mpz_t predecessor_key_gmp;
    mpz_init_set_str(predecessor_key_gmp, pred_node_key_hex.c_str() , 16);

    if (mpz_cmp(predecessor_key_gmp, my_key_gmp) != 0 ) {

        successor_id = message.GetStabilizeRsp().predecessor_node_id;
        successor_ip_address = message.GetStabilizeRsp().predecessor_node_ip_address;
        successor_node_key_hex = pred_node_key_hex;

    }
    
}

void
GUChord::ProcessRingStatePing (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{

        std::string fromNode = ReverseLookup (sourceAddress);
        //CHORD_LOG ("Received RING_STATE_PING, From Node: " << fromNode);

        Ipv4Address my_ip = GetLocalAddress();
        uint32_t my_id = atoi(ReverseLookup(my_ip).c_str());
        uint32_t transactionId = GetNextTransactionId ();

        std::string originator_node_key_hex = ipHash(message.GetRingStatePing().originator_node_ip_address);

        mpz_t my_key_gmp;
        mpz_init_set_str(my_key_gmp, my_node_key_hex.c_str() , 16);

        mpz_t originator_key_gmp;
        mpz_init_set_str(originator_key_gmp, originator_node_key_hex.c_str() , 16);

        if (mpz_cmp(my_key_gmp, originator_key_gmp) != 0) {

          CHORD_LOG ("\nRingState<" << my_id << ">: Pred<" << predecessor_id << ", " << predecessor_node_key_hex << ">, Succ<" << successor_id << ", " << successor_node_key_hex << ">");

          Ptr<Packet> packet = Create<Packet> ();
          GUChordMessage guChordMessage = GUChordMessage (GUChordMessage::RING_STATE_PING, transactionId );

          guChordMessage.SetRingStatePing (message.GetRingStatePing ().originator_node_id, message.GetRingStatePing ().originator_node_ip_address);
          packet->AddHeader (guChordMessage);
          m_socket->SendTo (packet, 0 , InetSocketAddress (successor_ip_address, m_appPort));

        }

}

void
GUChord::ProcessFindSuccessorReq (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{

        std::string fromNode = ReverseLookup (sourceAddress);
        CHORD_LOG ("Received FIND_SUCCESSOR_REQ, From Node: " << fromNode);

        std::string originator_node_key_hex = ipHash(message.GetFindSuccessorReq().originator_node_ip_address);

        mpz_t my_key_gmp;
        mpz_init_set_str(my_key_gmp, my_node_key_hex.c_str() , 16);

        mpz_t lookup_key_gmp;
        mpz_init_set_str(lookup_key_gmp, message.GetFindSuccessorReq().start_value.c_str() , 16);

        mpz_t successor_key_gmp;
        mpz_init_set_str(successor_key_gmp, successor_node_key_hex.c_str() , 16);

        if(isSuccessor(my_key_gmp, lookup_key_gmp, successor_key_gmp)) {

            uint32_t transactionId = GetNextTransactionId ();

            CHORD_LOG ("Sending FIND_SUCCESSOR_RSP to Node: " << ReverseLookup(message.GetFindSuccessorReq().originator_node_ip_address) << " IP: " << message.GetFindSuccessorReq().originator_node_ip_address << " transactionId: " << transactionId); 

            Ptr<Packet> packet = Create<Packet> ();
            GUChordMessage guChordMessage = GUChordMessage (GUChordMessage::FIND_SUCCESSOR_RSP, transactionId );

            guChordMessage.SetFindSuccessorRsp (successor_id, successor_ip_address, message.GetFindSuccessorReq().start_value, message.GetFindSuccessorReq().start_value_index);
            packet->AddHeader (guChordMessage);
            m_socket->SendTo (packet, 0 , InetSocketAddress (message.GetFindSuccessorReq().originator_node_ip_address, m_appPort));


         }

        else {

            uint32_t transactionId = GetNextTransactionId ();

            CHORD_LOG ("Sending FIND_SUCCESSOR_REQ to Node: " << ReverseLookup(successor_ip_address) << " IP: " << successor_ip_address << " transactionId: " << transactionId); 

            Ptr<Packet> packet = Create<Packet> ();
            GUChordMessage guChordMessage = GUChordMessage (GUChordMessage::FIND_SUCCESSOR_REQ, transactionId );

            guChordMessage.SetFindSuccessorReq (message.GetFindSuccessorReq ().originator_node_id, message.GetFindSuccessorReq().originator_node_ip_address, message.GetFindSuccessorReq().start_value, message.GetFindSuccessorReq().start_value_index);
            packet->AddHeader (guChordMessage);
            m_socket->SendTo (packet, 0 , InetSocketAddress (successor_ip_address, m_appPort));

        }

}

bool GUChord::isSuccessor(mpz_t my_key_gmp, mpz_t lookup_key_gmp, mpz_t successor_key_gmp)
{
  if(mpz_cmp(lookup_key_gmp, successor_key_gmp) <= 0 && mpz_cmp(lookup_key_gmp, my_key_gmp) > 0)
        return true;
  if (mpz_cmp(my_key_gmp, successor_key_gmp) > 0 && mpz_cmp(lookup_key_gmp, my_key_gmp) > 0)
        return true;
  if (mpz_cmp(my_key_gmp, successor_key_gmp) > 0 && mpz_cmp(lookup_key_gmp, successor_key_gmp) < 0)
        return true;
  if (mpz_cmp(successor_key_gmp, my_key_gmp) == 0)
        return true;
  return false;
}

void
GUChord::AuditPings ()
{
  std::map<uint32_t, Ptr<PingRequest> >::iterator iter;
  for (iter = m_pingTracker.begin () ; iter != m_pingTracker.end();)
    {
      Ptr<PingRequest> pingRequest = iter->second;
      if (pingRequest->GetTimestamp().GetMilliSeconds() + m_pingTimeout.GetMilliSeconds() <= Simulator::Now().GetMilliSeconds())
        {
          DEBUG_LOG ("Ping expired. Message: " << pingRequest->GetPingMessage () << " Timestamp: " << pingRequest->GetTimestamp().GetMilliSeconds () << " CurrentTime: " << Simulator::Now().GetMilliSeconds ());
          // Remove stale entries
          m_pingTracker.erase (iter++);
          // Send indication to application layer
          m_pingFailureFn (pingRequest->GetDestinationAddress(), pingRequest->GetPingMessage ());
        }
      else
        {
          ++iter;
        }
    }
  // Rechedule timer
  m_auditPingsTimer.Schedule (m_pingTimeout); 
}

uint32_t
GUChord::GetNextTransactionId ()
{
  return m_currentTransactionId++;
}

void
GUChord::StopChord ()
{
  StopApplication ();
}

void
GUChord::SetPingSuccessCallback (Callback <void, Ipv4Address, std::string> pingSuccessFn)
{
  m_pingSuccessFn = pingSuccessFn;
}


void
GUChord::SetPingFailureCallback (Callback <void, Ipv4Address, std::string> pingFailureFn)
{
  m_pingFailureFn = pingFailureFn;
}

void
GUChord::SetPingRecvCallback (Callback <void, Ipv4Address, std::string> pingRecvFn)
{
  m_pingRecvFn = pingRecvFn;
}


