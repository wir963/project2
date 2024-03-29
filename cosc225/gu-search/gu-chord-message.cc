/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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

#include "ns3/gu-chord-message.h"
#include "ns3/log.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("GUChordMessage");
NS_OBJECT_ENSURE_REGISTERED (GUChordMessage);

GUChordMessage::GUChordMessage ()
{
}

GUChordMessage::~GUChordMessage ()
{
}

GUChordMessage::GUChordMessage (GUChordMessage::MessageType messageType, uint32_t transactionId)
{
  m_messageType = messageType;
  m_transactionId = transactionId;
}

TypeId 
GUChordMessage::GetTypeId (void)
{
  static TypeId tid = TypeId ("GUChordMessage")
    .SetParent<Header> ()
    .AddConstructor<GUChordMessage> ()
  ;
  return tid;
}

TypeId
GUChordMessage::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}


uint32_t
GUChordMessage::GetSerializedSize (void) const
{
  // size of messageType, transaction id
  uint32_t size = sizeof (uint8_t) + sizeof (uint32_t);
  switch (m_messageType)
    {
      case PING_REQ:
        size += m_message.pingReq.GetSerializedSize ();
        break;
      case PING_RSP:
        size += m_message.pingRsp.GetSerializedSize ();
        break;
      case JOIN_REQ:
        size += m_message.joinReq.GetSerializedSize ();
        break;
      case JOIN_RSP:
        size += m_message.joinRsp.GetSerializedSize ();
        break;
      case DEPARTURE_REQ:
        size += m_message.departureReq.GetSerializedSize ();
        break;
      case STABILIZE_REQ:
        size += m_message.stabilizeReq.GetSerializedSize ();
        break;
      case STABILIZE_RSP:
        size += m_message.stabilizeRsp.GetSerializedSize ();
        break;
      case RING_STATE_PING:
        size += m_message.ringStatePing.GetSerializedSize ();
        break;
      case FIND_SUCCESSOR_REQ:
        size += m_message.findSuccessorReq.GetSerializedSize ();
        break;
      case FIND_SUCCESSOR_RSP:
        size += m_message.findSuccessorRsp.GetSerializedSize ();
        break;
      case LOOKUP_REQ:
        size += m_message.lookupReq.GetSerializedSize ();
        break;
      case LOOKUP_RSP:
        size += m_message.lookupRsp.GetSerializedSize ();
        break;
      default:
        NS_ASSERT (false);
    }
  return size;
}

void
GUChordMessage::Print (std::ostream &os) const
{
  os << "\n****GUChordMessage Dump****\n" ;
  os << "messageType: " << m_messageType << "\n";
  os << "transactionId: " << m_transactionId << "\n";
  os << "PAYLOAD:: \n";
  
  switch (m_messageType)
    {
      case PING_REQ:
        m_message.pingReq.Print (os);
        break;
      case PING_RSP:
        m_message.pingRsp.Print (os);
        break;
      case JOIN_REQ:
        m_message.joinReq.Print (os);
        break;
      case JOIN_RSP:
        m_message.joinRsp.Print (os);
        break;
      case DEPARTURE_REQ:
        m_message.departureReq.Print (os);
        break;
      case STABILIZE_REQ:
        m_message.stabilizeReq.Print (os);
        break;
      case STABILIZE_RSP:
        m_message.stabilizeRsp.Print (os);
        break;
      case RING_STATE_PING:
        m_message.ringStatePing.Print (os);
        break;
      case FIND_SUCCESSOR_REQ:
        m_message.findSuccessorReq.Print (os);
        break;
      case FIND_SUCCESSOR_RSP:
        m_message.findSuccessorRsp.Print (os);
        break;
      case LOOKUP_REQ:
        m_message.lookupReq.Print (os);
        break;
      case LOOKUP_RSP:
        m_message.lookupRsp.Print (os);
        break;
      default:
        break;  
    }
  os << "\n****END OF MESSAGE****\n";
}

void
GUChordMessage::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU8 (m_messageType);
  i.WriteHtonU32 (m_transactionId);

  switch (m_messageType)
    {
      case PING_REQ:
        m_message.pingReq.Serialize (i);
        break;
      case PING_RSP:
        m_message.pingRsp.Serialize (i);
        break;
      case JOIN_REQ:
        m_message.joinReq.Serialize (i);
        break;
      case JOIN_RSP:
        m_message.joinRsp.Serialize (i);
        break;
      case DEPARTURE_REQ:
        m_message.departureReq.Serialize (i);
        break;
      case STABILIZE_REQ:
        m_message.stabilizeReq.Serialize (i);
        break;
      case STABILIZE_RSP:
        m_message.stabilizeRsp.Serialize (i);
        break;
      case RING_STATE_PING:
        m_message.ringStatePing.Serialize (i);
        break;
      case FIND_SUCCESSOR_REQ:
        m_message.findSuccessorReq.Serialize (i);
        break;
      case FIND_SUCCESSOR_RSP:
        m_message.findSuccessorRsp.Serialize (i);
        break;
      case LOOKUP_REQ:
        m_message.lookupReq.Serialize (i);
        break;
      case LOOKUP_RSP:
        m_message.lookupRsp.Serialize (i);
        break;
      default:
        NS_ASSERT (false);   
    }
}

uint32_t 
GUChordMessage::Deserialize (Buffer::Iterator start)
{
  uint32_t size;
  Buffer::Iterator i = start;
  m_messageType = (MessageType) i.ReadU8 ();
  m_transactionId = i.ReadNtohU32 ();

  size = sizeof (uint8_t) + sizeof (uint32_t);

  switch (m_messageType)
    {
      case PING_REQ:
        size += m_message.pingReq.Deserialize (i);
        break;
      case PING_RSP:
        size += m_message.pingRsp.Deserialize (i);
        break;
      case JOIN_REQ:
        size += m_message.joinReq.Deserialize (i);
        break;
      case JOIN_RSP:
        size += m_message.joinRsp.Deserialize (i);
        break;
      case DEPARTURE_REQ:
        size += m_message.departureReq.Deserialize (i);
        break;
      case STABILIZE_REQ:
        size += m_message.stabilizeReq.Deserialize (i);
        break;
      case STABILIZE_RSP:
        size += m_message.stabilizeRsp.Deserialize (i);
        break;
      case RING_STATE_PING:
        size += m_message.ringStatePing.Deserialize (i);
        break;
      case FIND_SUCCESSOR_REQ:
        m_message.findSuccessorReq.Deserialize (i);
        break;
      case FIND_SUCCESSOR_RSP:
        m_message.findSuccessorRsp.Deserialize (i);
        break;
      case LOOKUP_REQ:
        m_message.lookupReq.Deserialize (i);
        break;
      case LOOKUP_RSP:
        m_message.lookupRsp.Deserialize (i);
        break;
      default:
        NS_ASSERT (false);
    }
  return size;
}

/* PING_REQ */

uint32_t 
GUChordMessage::PingReq::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + pingMessage.length();
  return size;
}

void
GUChordMessage::PingReq::Print (std::ostream &os) const
{
  os << "PingReq:: Message: " << pingMessage << "\n";
}

void
GUChordMessage::PingReq::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (pingMessage.length ());
  start.Write ((uint8_t *) (const_cast<char*> (pingMessage.c_str())), pingMessage.length());
}

uint32_t
GUChordMessage::PingReq::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  pingMessage = std::string (str, length);
  free (str);
  return PingReq::GetSerializedSize ();
}

void
GUChordMessage::SetPingReq (std::string pingMessage)
{
  if (m_messageType == 0)
    {
      m_messageType = PING_REQ;
    }
  else
    {
      NS_ASSERT (m_messageType == PING_REQ);
    }
  m_message.pingReq.pingMessage = pingMessage;
}

GUChordMessage::PingReq
GUChordMessage::GetPingReq ()
{
  return m_message.pingReq;
}

/* PING_RSP */

uint32_t 
GUChordMessage::PingRsp::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + pingMessage.length();
  return size;
}

void
GUChordMessage::PingRsp::Print (std::ostream &os) const
{
  os << "PingReq:: Message: " << pingMessage << "\n";
}

void
GUChordMessage::PingRsp::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (pingMessage.length ());
  start.Write ((uint8_t *) (const_cast<char*> (pingMessage.c_str())), pingMessage.length());
}

uint32_t
GUChordMessage::PingRsp::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  pingMessage = std::string (str, length);
  free (str);
  return PingRsp::GetSerializedSize ();
}

void
GUChordMessage::SetPingRsp (std::string pingMessage)
{
  if (m_messageType == 0)
    {
      m_messageType = PING_RSP;
    }
  else
    {
      NS_ASSERT (m_messageType == PING_RSP);
    }
  m_message.pingRsp.pingMessage = pingMessage;
}

GUChordMessage::PingRsp
GUChordMessage::GetPingRsp ()
{
  return m_message.pingRsp;
}


/* JOIN_REQ */

uint32_t 
GUChordMessage::JoinReq::GetSerializedSize (void) const
{
  uint32_t size;
  size = 2*IPV4_ADDRESS_SIZE + 2*sizeof(uint32_t);
  return size;
}

void
GUChordMessage::JoinReq::Print (std::ostream &os) const
{
  //os << "JoinReq:: Message: " << joinReqMessage << "\n";
}

void
GUChordMessage::JoinReq::Serialize (Buffer::Iterator &start) const
{
  start.WriteU32 (landmark_id);
  start.WriteHtonU32(landmark_ip_address.Get());
  start.WriteU32 (request_id);
  start.WriteHtonU32(request_ip_address.Get());
}

uint32_t
GUChordMessage::JoinReq::Deserialize (Buffer::Iterator &start)
{
  landmark_id = start.ReadU32();
  landmark_ip_address = Ipv4Address (start.ReadNtohU32 ());
  request_id = start.ReadU32();
  request_ip_address = Ipv4Address (start.ReadNtohU32 ());
  return JoinReq::GetSerializedSize ();
}

void
GUChordMessage::SetJoinReq (JoinReq joinRequest)
{
  if (m_messageType == 0)
    {
      m_messageType = JOIN_REQ;
    }
  else
    {
      NS_ASSERT (m_messageType == JOIN_REQ);
    }

    m_message.joinReq.request_id = joinRequest.request_id;
    m_message.joinReq.request_ip_address = joinRequest.request_ip_address;
    m_message.joinReq.landmark_id = joinRequest.landmark_id;
    m_message.joinReq.landmark_ip_address = joinRequest.landmark_ip_address;
}

void
GUChordMessage::SetJoinReq (uint32_t sender_node_number, Ipv4Address sender_ip_address, uint32_t receiving_node_number, Ipv4Address receiving_ip_address)
{

        if (m_messageType == 0)
            {
              m_messageType = JOIN_REQ;
            }
          else
            {
              NS_ASSERT (m_messageType == JOIN_REQ);
            }
          
        m_message.joinReq.request_id = sender_node_number;
        m_message.joinReq.request_ip_address = sender_ip_address;
        m_message.joinReq.landmark_id = receiving_node_number;
        m_message.joinReq.landmark_ip_address = receiving_ip_address;

}
          

GUChordMessage::JoinReq
GUChordMessage::GetJoinReq ()
{
  return m_message.joinReq;
}

/* JOIN_RSP */

uint32_t 
GUChordMessage::JoinRsp::GetSerializedSize (void) const
{
    uint32_t size;
    size = 3*IPV4_ADDRESS_SIZE + 3*sizeof(uint32_t);
    return size;
}

void
GUChordMessage::JoinRsp::Print (std::ostream &os) const
{
  //os << "JoinRsp:: Message: " << joinRspMessage << "\n";
}

void
GUChordMessage::JoinRsp::Serialize (Buffer::Iterator &start) const
{
    start.WriteU32 (landmark_id);
    start.WriteHtonU32(landmark_ip_address.Get());
    start.WriteU32 (request_id);
    start.WriteHtonU32(request_ip_address.Get());
    start.WriteU32 (successor_id);
    start.WriteHtonU32(successor_ip_address.Get());
}

uint32_t
GUChordMessage::JoinRsp::Deserialize (Buffer::Iterator &start)
{  
    landmark_id = start.ReadU32();
    landmark_ip_address = Ipv4Address (start.ReadNtohU32 ());
    request_id = start.ReadU32();
    request_ip_address = Ipv4Address (start.ReadNtohU32 ());
    successor_id = start.ReadU32();
    successor_ip_address = Ipv4Address (start.ReadNtohU32 ());
    return JoinRsp::GetSerializedSize ();
}

void
GUChordMessage::SetJoinRsp (JoinRsp joinResponse)
{
  if (m_messageType == 0)
    {
      m_messageType = JOIN_RSP;
    }
  else
    {
      NS_ASSERT (m_messageType == JOIN_RSP);
    }
    m_message.joinRsp.request_id = joinResponse.request_id;
    m_message.joinRsp.request_ip_address = joinResponse.request_ip_address;
    m_message.joinRsp.successor_id = joinResponse.successor_id;
    m_message.joinRsp.successor_ip_address = joinResponse.successor_ip_address;
    m_message.joinRsp.landmark_id = joinResponse.landmark_id;
    m_message.joinRsp.landmark_ip_address = joinResponse.landmark_ip_address;
}

void
GUChordMessage::SetJoinRsp (JoinReq joinRequest, uint32_t succ_id, Ipv4Address succ_ip)
{
    if (m_messageType == 0)
    {
        m_messageType = JOIN_RSP;
    }
    else
    {
        NS_ASSERT (m_messageType == JOIN_RSP);
    }
    m_message.joinRsp.successor_ip_address = succ_ip;
    m_message.joinRsp.successor_id = succ_id;
    m_message.joinRsp.request_id = joinRequest.request_id;
    m_message.joinRsp.request_ip_address = joinRequest.request_ip_address;
    m_message.joinRsp.landmark_id = joinRequest.landmark_id;
    m_message.joinRsp.landmark_ip_address = joinRequest.landmark_ip_address;
}

GUChordMessage::JoinRsp
GUChordMessage::GetJoinRsp ()
{
  return m_message.joinRsp;
}

/* DEPARTURE_REQ */

uint32_t 
GUChordMessage::DepartureReq::GetSerializedSize (void) const
{
    uint32_t size;
    size = 2*IPV4_ADDRESS_SIZE + 2*sizeof(uint32_t);
    return size;
}

void
GUChordMessage::DepartureReq::Print (std::ostream &os) const
{
  //os << "DepartureReq:: Message: " << departureReqMessage << "\n";
}

void
GUChordMessage::DepartureReq::Serialize (Buffer::Iterator &start) const
{
    start.WriteU32 (sender_node_id);
    start.WriteHtonU32(sender_node_ip_address.Get());
    start.WriteU32 (conn_node_id);
    start.WriteHtonU32(conn_node_ip_address.Get());
}

uint32_t
GUChordMessage::DepartureReq::Deserialize (Buffer::Iterator &start)
{  
    sender_node_id = start.ReadU32();
    sender_node_ip_address = Ipv4Address (start.ReadNtohU32 ());
    conn_node_id = start.ReadU32();
    conn_node_ip_address = Ipv4Address (start.ReadNtohU32 ());
    return DepartureReq::GetSerializedSize ();
}

void
GUChordMessage::SetDepartureReq (uint32_t sender_node_number, Ipv4Address sender_ip_address, uint32_t conn_node_number, Ipv4Address conn_ip_address)
{

        if (m_messageType == 0)
            {
              m_messageType = DEPARTURE_REQ;
            }
          else
            {
              NS_ASSERT (m_messageType == DEPARTURE_REQ);
            }
          
        m_message.departureReq.sender_node_id = sender_node_number;
        m_message.departureReq.sender_node_ip_address = sender_ip_address;
        m_message.departureReq.conn_node_id = conn_node_number;
        m_message.departureReq.conn_node_ip_address = conn_ip_address;

}

GUChordMessage::DepartureReq
GUChordMessage::GetDepartureReq ()
{
  return m_message.departureReq;
}


/* STABILIZE_REQ */

uint32_t 
GUChordMessage::StabilizeReq::GetSerializedSize (void) const
{
    uint32_t size;
    size = IPV4_ADDRESS_SIZE + sizeof(uint32_t);
    return size;
}

void
GUChordMessage::StabilizeReq::Print (std::ostream &os) const
{
  //os << "StabilizeReq:: Message: " << stabilizeReqMessage << "\n";
}

void
GUChordMessage::StabilizeReq::Serialize (Buffer::Iterator &start) const
{
    start.WriteU32 (sender_node_id);
    start.WriteHtonU32(sender_node_ip_address.Get());
}

uint32_t
GUChordMessage::StabilizeReq::Deserialize (Buffer::Iterator &start)
{  
    sender_node_id = start.ReadU32();
    sender_node_ip_address = Ipv4Address (start.ReadNtohU32 ());
  return StabilizeReq::GetSerializedSize ();
}

void
GUChordMessage::SetStabilizeReq (uint32_t node_id, Ipv4Address ip_address)
{
  if (m_messageType == 0)
    {
      m_messageType = STABILIZE_REQ;
    }
  else
    {
      NS_ASSERT (m_messageType == STABILIZE_REQ);
    }
    m_message.stabilizeReq.sender_node_id = node_id;
    m_message.stabilizeReq.sender_node_ip_address = ip_address;
}

GUChordMessage::StabilizeReq
GUChordMessage::GetStabilizeReq ()
{
  return m_message.stabilizeReq;
}

/* STABILIZE_RSP */

uint32_t 
GUChordMessage::StabilizeRsp::GetSerializedSize (void) const
{
    uint32_t size;
    size = IPV4_ADDRESS_SIZE + sizeof(uint32_t);
    return size;
}

void
GUChordMessage::StabilizeRsp::Print (std::ostream &os) const
{
  //os << "StabilizeRsp:: Message: " << stabilizeRspMessage << "\n";
}

void
GUChordMessage::StabilizeRsp::Serialize (Buffer::Iterator &start) const
{
    start.WriteU32 (predecessor_node_id);
    start.WriteHtonU32(predecessor_node_ip_address.Get());
}

uint32_t
GUChordMessage::StabilizeRsp::Deserialize (Buffer::Iterator &start)
{  
  predecessor_node_id = start.ReadU32();
  predecessor_node_ip_address = Ipv4Address (start.ReadNtohU32 ());
  return StabilizeRsp::GetSerializedSize ();
}

void
GUChordMessage::SetStabilizeRsp (uint32_t node_id, Ipv4Address ip_address)
{
  if (m_messageType == 0)
    {
      m_messageType = STABILIZE_RSP;
    }
  else
    {
      NS_ASSERT (m_messageType == STABILIZE_RSP);
    }
    m_message.stabilizeRsp.predecessor_node_id = node_id;
    m_message.stabilizeRsp.predecessor_node_ip_address = ip_address;
}

GUChordMessage::StabilizeRsp
GUChordMessage::GetStabilizeRsp ()
{
  return m_message.stabilizeRsp;
}

/* RING_STATE_PING */

uint32_t 
GUChordMessage::RingStatePing::GetSerializedSize (void) const
{
    uint32_t size;
    size = IPV4_ADDRESS_SIZE + sizeof(uint32_t);
    return size;
}

void
GUChordMessage::RingStatePing::Print (std::ostream &os) const
{
  //os << "StabilizeRsp:: Message: " << stabilizeRspMessage << "\n";
}

void
GUChordMessage::RingStatePing::Serialize (Buffer::Iterator &start) const
{
    start.WriteU32 (originator_node_id);
    start.WriteHtonU32(originator_node_ip_address.Get());
}

uint32_t
GUChordMessage::RingStatePing::Deserialize (Buffer::Iterator &start)
{  
  originator_node_id = start.ReadU32();
  originator_node_ip_address = Ipv4Address (start.ReadNtohU32 ());
  return RingStatePing::GetSerializedSize ();
}

void
GUChordMessage::SetRingStatePing (uint32_t originator_id, Ipv4Address originator_ip)
{
  if (m_messageType == 0)
    {
      m_messageType = RING_STATE_PING;
    }
  else
    {
      NS_ASSERT (m_messageType == RING_STATE_PING);
    }
  
  m_message.ringStatePing.originator_node_id = originator_id;
  m_message.ringStatePing.originator_node_ip_address = originator_ip;

}


GUChordMessage::RingStatePing
GUChordMessage::GetRingStatePing ()
{
  return m_message.ringStatePing;
}

/* FIND_SUCCESSOR_REQ */

uint32_t 
GUChordMessage::FindSuccessorReq::GetSerializedSize (void) const
{
    uint32_t size;
    size = IPV4_ADDRESS_SIZE + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint16_t) + start_value.length();
    return size;
}

void
GUChordMessage::FindSuccessorReq::Print (std::ostream &os) const
{
  //os << "StabilizeReq:: Message: " << stabilizeReqMessage << "\n";
}

void
GUChordMessage::FindSuccessorReq::Serialize (Buffer::Iterator &start) const
{
    start.WriteU32 (originator_node_id);
    start.WriteHtonU32(originator_node_ip_address.Get());

    start.WriteU16 (start_value.length ());
    
    start.Write ((uint8_t *) (const_cast<char*> (start_value.c_str())), start_value.length());

    start.WriteU32 (start_value_index);
}

uint32_t
GUChordMessage::FindSuccessorReq::Deserialize (Buffer::Iterator &start)
{  
    originator_node_id = start.ReadU32();
    originator_node_ip_address = Ipv4Address (start.ReadNtohU32 ());
    
    uint16_t length = start.ReadU16 ();
    char* str = (char*) malloc (length);
    start.Read ((uint8_t*)str, length);
    start_value = std::string (str, length);
    free (str);

    start_value_index = start.ReadU32();

    return FindSuccessorReq::GetSerializedSize ();
}

void
GUChordMessage::SetFindSuccessorReq (uint32_t node_id, Ipv4Address ip_address, std::string start_value, uint32_t index)
{
  if (m_messageType == 0)
    {
      m_messageType = FIND_SUCCESSOR_REQ;
    }
  else
    {
      NS_ASSERT (m_messageType == FIND_SUCCESSOR_REQ);
    }
    m_message.findSuccessorReq.originator_node_id = node_id;
    m_message.findSuccessorReq.originator_node_ip_address = ip_address;
    m_message.findSuccessorReq.start_value = start_value;
    m_message.findSuccessorReq.start_value_index = index;
}

GUChordMessage::FindSuccessorReq
GUChordMessage::GetFindSuccessorReq ()
{
  return m_message.findSuccessorReq;
}

/* FIND_SUCCESSOR_RSP */

uint32_t 
GUChordMessage::FindSuccessorRsp::GetSerializedSize (void) const
{
    uint32_t size;
    size = IPV4_ADDRESS_SIZE + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint16_t) + start_value.length();
    return size;
}

void
GUChordMessage::FindSuccessorRsp::Print (std::ostream &os) const
{
  //os << "StabilizeReq:: Message: " << stabilizeReqMessage << "\n";
}

void
GUChordMessage::FindSuccessorRsp::Serialize (Buffer::Iterator &start) const
{
    start.WriteU32 (successor_node_id);
    start.WriteHtonU32(successor_node_ip_address.Get());

    start.WriteU16 (start_value.length ());

    start.Write ((uint8_t *) (const_cast<char*> (start_value.c_str())), start_value.length());

    start.WriteU32 (start_value_index);
}

uint32_t
GUChordMessage::FindSuccessorRsp::Deserialize (Buffer::Iterator &start)
{  
    successor_node_id = start.ReadU32();
    successor_node_ip_address = Ipv4Address (start.ReadNtohU32 ());
    
    uint16_t length = start.ReadU16 ();
    char* str = (char*) malloc (length);
    start.Read ((uint8_t*)str, length);
    start_value = std::string (str, length);
    free (str);

    start_value_index = start.ReadU32();
  
    return FindSuccessorRsp::GetSerializedSize ();
}

void
GUChordMessage::SetFindSuccessorRsp (uint32_t node_id, Ipv4Address ip_address, std::string start_value, uint32_t index)
{
  if (m_messageType == 0)
    {
      m_messageType = FIND_SUCCESSOR_RSP;
    }
  else
    {
      NS_ASSERT (m_messageType == FIND_SUCCESSOR_RSP);
    }
    m_message.findSuccessorRsp.successor_node_id = node_id;
    m_message.findSuccessorRsp.successor_node_ip_address = ip_address;
    m_message.findSuccessorRsp.start_value = start_value;
    m_message.findSuccessorRsp.start_value_index = index;
}

GUChordMessage::FindSuccessorRsp
GUChordMessage::GetFindSuccessorRsp ()
{
  return m_message.findSuccessorRsp;
}

/* FIND_LOOKUP_REQ */

uint32_t 
GUChordMessage::LookupReq::GetSerializedSize (void) const
{
    uint32_t size;
    size = IPV4_ADDRESS_SIZE + sizeof(uint32_t) + sizeof(uint16_t) + target_key.length();
    return size;
}

void
GUChordMessage::LookupReq::Print (std::ostream &os) const
{
  //os << "StabilizeReq:: Message: " << stabilizeReqMessage << "\n";
}

void
GUChordMessage::LookupReq::Serialize (Buffer::Iterator &start) const
{
    start.WriteU32 (originator_node_id);
    start.WriteHtonU32(originator_node_ip_address.Get());

    start.WriteU16 (target_key.length ());
    
    start.Write ((uint8_t *) (const_cast<char*> (target_key.c_str())), target_key.length());
}

uint32_t
GUChordMessage::LookupReq::Deserialize (Buffer::Iterator &start)
{  
    originator_node_id = start.ReadU32();
    originator_node_ip_address = Ipv4Address (start.ReadNtohU32 ());
    
    uint16_t length = start.ReadU16 ();
    char* str = (char*) malloc (length);
    start.Read ((uint8_t*)str, length);
    target_key = std::string (str, length);
    free (str);

    return LookupReq::GetSerializedSize ();
}

void
GUChordMessage::SetLookupReq (uint32_t node_id, Ipv4Address ip_address, std::string key)
{
  if (m_messageType == 0)
    {
      m_messageType = LOOKUP_REQ;
    }
  else
    {
      NS_ASSERT (m_messageType == LOOKUP_REQ);
    }
    m_message.lookupReq.originator_node_id = node_id;
    m_message.lookupReq.originator_node_ip_address = ip_address;
    m_message.lookupReq.target_key = key;
}

GUChordMessage::LookupReq
GUChordMessage::GetLookupReq ()
{
  return m_message.lookupReq;
}

/* LOOKUP_RSP */

uint32_t 
GUChordMessage::LookupRsp::GetSerializedSize (void) const
{
    uint32_t size;
    size = IPV4_ADDRESS_SIZE*2 + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint16_t) + target_key.length();
    return size;
}

void
GUChordMessage::LookupRsp::Print (std::ostream &os) const
{
  //os << "StabilizeReq:: Message: " << stabilizeReqMessage << "\n";
}

void
GUChordMessage::LookupRsp::Serialize (Buffer::Iterator &start) const
{
    
    start.WriteU32 (originator_node_id);
    start.WriteHtonU32(originator_node_ip_address.Get());

    start.WriteU32 (successor_node_id);
    start.WriteHtonU32(successor_node_ip_address.Get());

    start.WriteU16 (target_key.length ());

    start.Write ((uint8_t *) (const_cast<char*> (target_key.c_str())), target_key.length());

}

uint32_t
GUChordMessage::LookupRsp::Deserialize (Buffer::Iterator &start)
{  
    
    originator_node_id = start.ReadU32();
    originator_node_ip_address = Ipv4Address (start.ReadNtohU32 ());

    successor_node_id = start.ReadU32();
    successor_node_ip_address = Ipv4Address (start.ReadNtohU32 ());
    
    uint16_t length = start.ReadU16 ();
    char* str = (char*) malloc (length);
    start.Read ((uint8_t*)str, length);
    target_key = std::string (str, length);
    free (str);
  
    return LookupRsp::GetSerializedSize ();
}

void
GUChordMessage::SetLookupRsp (uint32_t orig_node_id, Ipv4Address orig_ip_address, uint32_t succ_node_id, Ipv4Address succ_ip_address, std::string key)
{
  if (m_messageType == 0)
    {
      m_messageType = LOOKUP_RSP;
    }
  else
    {
      NS_ASSERT (m_messageType == LOOKUP_RSP);
    }

    m_message.lookupRsp.originator_node_id = orig_node_id;
    m_message.lookupRsp.originator_node_ip_address = orig_ip_address;
    m_message.lookupRsp.successor_node_id = succ_node_id;
    m_message.lookupRsp.successor_node_ip_address = succ_ip_address;
    m_message.lookupRsp.target_key = key;
}

GUChordMessage::LookupRsp
GUChordMessage::GetLookupRsp ()
{
  return m_message.lookupRsp;
}

//
//
//

void
GUChordMessage::SetMessageType (MessageType messageType)
{
  m_messageType = messageType;
}

GUChordMessage::MessageType
GUChordMessage::GetMessageType () const
{
  return m_messageType;
}

void
GUChordMessage::SetTransactionId (uint32_t transactionId)
{
  m_transactionId = transactionId;
}

uint32_t 
GUChordMessage::GetTransactionId (void) const
{
  return m_transactionId;
}

