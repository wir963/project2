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

void
GUChord::StartApplication (void)
{
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

  m_pingTracker.clear ();
}

void
GUChord::ProcessCommand (std::vector<std::string> tokens)
{
  std::vector<std::string>::iterator iterator = tokens.begin();
  std::string command = *iterator;
  // let's print out the command
  std::cout << "In Process Command" << std::endl;
  std::cout << "The command is " << command << std::endl;

  if (command == "join" || command == "JOIN")
    {
          iterator++;
          std::istringstream sin (*iterator);
          std::string nodeNumber;
          sin >> nodeNumber;
          //std::cout << nodeNumber << std::endl;
          Ipv4Address destAddress = ResolveNodeIpAddress (nodeNumber);
          uint32_t transactionId = GetNextTransactionId ();

          Ptr<Packet> packet = Create<Packet> ();
          GUChordMessage guChordMessage = GUChordMessage (GUChordMessage::JOIN_REQ, transactionId );
          //guChordMessage.SetPingReq (destAddress, pingMessage);
          packet->AddHeader (guChordMessage);
          m_socket->SendTo (packet, 0 , InetSocketAddress (destAddress, m_appPort));
        
    }
  else if (command == "leave")
    {
      
    }
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

std::cout << "received " << message.GetMessageType() << " message at node" << message.GetJoinReq ().request_ip_address << std::endl;
    // will only get this if you are the landmark node
    // need to figure out which node should be the successor for the sender/ new node
    
    // then need to send the IP Address of this node to the sender
    // landmark_node is the node that received the original message
    // request_node is the node that asked to join the network
    uint32_t request_node_id = message.GetJoinReq().request_id;
    Ipv4Address my_ip = GetLocalAddress();
    // check to see if m_mainAddress exists?
    uint32_t my_id = atoi(ReverseLookup(my_ip).c_str());
    if (my_id < request_node_id && request_node_id < successor_id)
    {
        SendJoinRsp(message, sourcePort);
    }
    else if (request_node_id > my_id && my_id > successor_id)
    {
        SendJoinRsp(message, sourcePort);
    }
    else if (successor_id == my_id)
    {
        SendJoinRsp(message, sourcePort);
    }
    else
    {
        // need to keep searching so forward the message along
        GUChordMessage resp = GUChordMessage (GUChordMessage::JOIN_REQ, message.GetTransactionId());
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
    Ipv4Address my_ip = GetLocalAddress();
    // check to see if m_mainAddress exists?
    uint32_t my_id = atoi(ReverseLookup(my_ip).c_str());
    // if you are the landmark node, then send this information to the request node
    if (my_id == message.GetJoinRsp().landmark_id)
    {
        GUChordMessage resp = GUChordMessage (GUChordMessage::JOIN_RSP, message.GetTransactionId());
        resp.SetJoinRsp (message.GetJoinRsp());
        Ptr<Packet> packet = Create<Packet> ();
        packet->AddHeader (resp);
        m_socket->SendTo (packet, 0 , InetSocketAddress (message.GetJoinRsp().request_ip_address, sourcePort));
    }
    // if you are the original requester
    else if (my_id == message.GetJoinRsp().request_id)
    {
        successor_ip_address = message.GetJoinRsp().successor_ip_address;
        successor_id = message.GetJoinRsp().successor_id;
        // joined the tree!
    }
    else
    {
        // forward the message
        GUChordMessage resp = GUChordMessage (GUChordMessage::JOIN_RSP, message.GetTransactionId());
        resp.SetJoinRsp (message.GetJoinRsp());
        Ptr<Packet> packet = Create<Packet> ();
        packet->AddHeader (resp);
        m_socket->SendTo (packet, 0 , InetSocketAddress (successor_ip_address, sourcePort));
    }
    
    // otherwise, forward it along to your successor
    // set successor = message.getJoinRsp().successor_ip_address
    // will successor be a member of this class?
}

void
GUChord::ProcessDepartureReq (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{
    // check to see if sourceAddress came from my successor or predecessor
    // if successor, set successor = message.getDepartureReq().conn_node_ip_address
    // if predecessor, set predecessor = message.getDepartureReq().conn_node_ip_address
    
}

void
GUChord::ProcessStabilizeReq (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{
    // only nodes that think you are their successor will send you these messages
    uint32_t sender_id = atoi(ReverseLookup(sourceAddress).c_str());
    // compare predecessor with sourceAddress
    if ()
    {
        
    }
    // if sourceAddress is > predecessor, set predecessor = sourceAddress
    // send a ProcessStabilizeRsp message with predecessor to the sender
    
}

void
GUChord::ProcessStabilizeRsp (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{
    // should have been sent by your successor
    // if message.getStabilizeReq().predecessor != me, then make them your new successor
    
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


