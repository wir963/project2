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

#include "ns3/gu-search-message.h"
#include "ns3/log.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("GUSearchMessage");
NS_OBJECT_ENSURE_REGISTERED (GUSearchMessage);

GUSearchMessage::GUSearchMessage ()
{
}

GUSearchMessage::~GUSearchMessage ()
{
}

GUSearchMessage::GUSearchMessage (GUSearchMessage::MessageType messageType, uint32_t transactionId)
{
  m_messageType = messageType;
  m_transactionId = transactionId;
}

TypeId 
GUSearchMessage::GetTypeId (void)
{
  static TypeId tid = TypeId ("GUSearchMessage")
    .SetParent<Header> ()
    .AddConstructor<GUSearchMessage> ()
  ;
  return tid;
}

TypeId
GUSearchMessage::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}


uint32_t
GUSearchMessage::GetSerializedSize (void) const
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
      case STORE_REQ:
        size += m_message.storeReq.GetSerializedSize ();
        break;  
      case FETCH_REQ:
        size += m_message.fetchReq.GetSerializedSize ();
        break;
      case FETCH_RSP:
        size += m_message.fetchRsp.GetSerializedSize ();
        break;
      default:
        NS_ASSERT (false);
    }
  return size;
}

void
GUSearchMessage::Print (std::ostream &os) const
{
  os << "\n****GUSearchMessage Dump****\n" ;
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
      case STORE_REQ:
        m_message.storeReq.Print (os);
        break;  
      case FETCH_REQ:
        m_message.fetchReq.Print (os);
        break;
      case FETCH_RSP:
        m_message.fetchRsp.Print (os);
        break;        
      default:
        break;  
    }
  os << "\n****END OF MESSAGE****\n";
}

void
GUSearchMessage::Serialize (Buffer::Iterator start) const
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
      case STORE_REQ:
        m_message.storeReq.Serialize (i);
        break;  
      case FETCH_REQ:
        m_message.fetchReq.Serialize (i);
        break;
      case FETCH_RSP:
        m_message.fetchRsp.Serialize (i);
        break;         
      default:
        NS_ASSERT (false);   
    }
}

uint32_t 
GUSearchMessage::Deserialize (Buffer::Iterator start)
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
      case STORE_REQ:
        size += m_message.storeReq.Deserialize (i);
        break;  
      case FETCH_REQ:
        size += m_message.fetchReq.Deserialize (i);
        break;
      case FETCH_RSP:
        size += m_message.fetchRsp.Deserialize (i);
        break;
      default:
        NS_ASSERT (false);
    }
  return size;
}

/* PING_REQ */

uint32_t 
GUSearchMessage::PingReq::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + pingMessage.length();
  return size;
}

void
GUSearchMessage::PingReq::Print (std::ostream &os) const
{
  os << "PingReq:: Message: " << pingMessage << "\n";
}

void
GUSearchMessage::PingReq::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (pingMessage.length ());
  start.Write ((uint8_t *) (const_cast<char*> (pingMessage.c_str())), pingMessage.length());
}

uint32_t
GUSearchMessage::PingReq::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  pingMessage = std::string (str, length);
  free (str);
  return PingReq::GetSerializedSize ();
}

void
GUSearchMessage::SetPingReq (std::string pingMessage)
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

GUSearchMessage::PingReq
GUSearchMessage::GetPingReq ()
{
  return m_message.pingReq;
}

/* PING_RSP */

uint32_t 
GUSearchMessage::PingRsp::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + pingMessage.length();
  return size;
}

void
GUSearchMessage::PingRsp::Print (std::ostream &os) const
{
  os << "PingReq:: Message: " << pingMessage << "\n";
}

void
GUSearchMessage::PingRsp::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (pingMessage.length ());
  start.Write ((uint8_t *) (const_cast<char*> (pingMessage.c_str())), pingMessage.length());
}

uint32_t
GUSearchMessage::PingRsp::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  pingMessage = std::string (str, length);
  free (str);
  return PingRsp::GetSerializedSize ();
}

void
GUSearchMessage::SetPingRsp (std::string pingMessage)
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

GUSearchMessage::PingRsp
GUSearchMessage::GetPingRsp ()
{
  return m_message.pingRsp;
}

/* STORE_REQ */
uint32_t 
GUSearchMessage::StoreReq::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + key.length();
  size += sizeof(uint32_t);
  for (std::set<std::string>::iterator it = documents.begin(); it != documents.end(); it++) {
    size += sizeof(uint16_t);
    size += (*it).length();
  }
  return size;
}

void
GUSearchMessage::StoreReq::Print (std::ostream &os) const
{
  os << "StoreReq:: Key: " << key << " Documents: " ; 
  for (std::set<std::string>::iterator it = documents.begin(); it != documents.end(); it++) {
    os << *it << ", ";
  }
  os << "\n";
}

void
GUSearchMessage::StoreReq::Serialize (Buffer::Iterator &start) const
{
  //Print(std::cout);
  start.WriteU16 (key.length ());
  start.Write ((uint8_t *) (const_cast<char*> (key.c_str())), key.length());
  
  start.WriteHtonU32(documents.size());
  
  for (std::set<std::string>::iterator it = documents.begin(); it != documents.end(); it++) {
    start.WriteU16 ((*it).length());
    start.Write ((uint8_t *) (const_cast<char*> ((*it).c_str())), (*it).length());
  }
}

uint32_t
GUSearchMessage::StoreReq::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  key = std::string (str, length);
  free (str);
  
  uint32_t dlen = start.ReadNtohU32();
  for (uint32_t i = 0; i < dlen; i++) {
    uint16_t length = start.ReadU16 ();
    char* str = (char*) malloc (length);
    start.Read ((uint8_t*)str, length);
    documents.insert(std::string (str, length));
    free (str);
  }
  
  //Print(std::cout);
  
  return StoreReq::GetSerializedSize ();
}

void
GUSearchMessage::SetStoreReq (std::string key, std::set<std::string> documents)
{
  if (m_messageType == 0)
    {
      m_messageType = STORE_REQ;
    }
  else
    {
      NS_ASSERT (m_messageType == STORE_REQ);
    }
  m_message.storeReq.key = key;
  m_message.storeReq.documents = documents;
}

GUSearchMessage::StoreReq
GUSearchMessage::GetStoreReq ()
{
  return m_message.storeReq;
}


/* FETCH_REQ */
uint32_t 
GUSearchMessage::FetchReq::GetSerializedSize (void) const
{
  uint32_t size = 0;
  size += sizeof(uint32_t);
  size += sizeof(uint16_t) + key.length();
  
  size += sizeof(uint32_t);
  for (std::set<std::string>::iterator it = searchKeys.begin(); it != searchKeys.end(); it++) {
    size += sizeof(uint16_t);
    size += (*it).length();
  }
  
  size += sizeof(uint32_t);
  for (std::set<std::string>::iterator it = documents.begin(); it != documents.end(); it++) {
    size += sizeof(uint16_t);
    size += (*it).length();
  }
  return size;
}

void
GUSearchMessage::FetchReq::Print (std::ostream &os) const
{
  os << "FetchReq:: OriginatorNum: " << originatorNum << " Key: " << key << " Documents: " ; 
  for (std::set<std::string>::iterator it = documents.begin(); it != documents.end(); it++) {
    os << *it << ", ";
  }
  os << " Search Keys: ";
  for (std::set<std::string>::iterator it = searchKeys.begin(); it != searchKeys.end(); it++) {
    os << *it << ", ";
  }
  os << "\n";
}

void
GUSearchMessage::FetchReq::Serialize (Buffer::Iterator &start) const
{
  start.WriteHtonU32(originatorNum);
  
  start.WriteU16 (key.length ());
  start.Write ((uint8_t *) (const_cast<char*> (key.c_str())), key.length());
  
  start.WriteHtonU32(searchKeys.size());
  
  for (std::set<std::string>::iterator it = searchKeys.begin(); it != searchKeys.end(); it++) {
    start.WriteU16 ((*it).length());
    start.Write ((uint8_t *) (const_cast<char*> ((*it).c_str())), (*it).length());
  }
  
  start.WriteHtonU32(documents.size());
  
  for (std::set<std::string>::iterator it = documents.begin(); it != documents.end(); it++) {
    start.WriteU16 ((*it).length());
    start.Write ((uint8_t *) (const_cast<char*> ((*it).c_str())), (*it).length());
  }
}

uint32_t
GUSearchMessage::FetchReq::Deserialize (Buffer::Iterator &start)
{  
  originatorNum = start.ReadNtohU32();
  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  key = std::string (str, length);
  free (str);
  
  uint32_t dlen = start.ReadNtohU32();
  for (uint32_t i = 0; i < dlen; i++) {
    uint16_t length = start.ReadU16 ();
    char* str = (char*) malloc (length);
    start.Read ((uint8_t*)str, length);
    searchKeys.insert(std::string (str, length));
    free (str);
  }
  
  dlen = start.ReadNtohU32();
  for (uint32_t i = 0; i < dlen; i++) {
    uint16_t length = start.ReadU16 ();
    char* str = (char*) malloc (length);
    start.Read ((uint8_t*)str, length);
    documents.insert(std::string (str, length));
    free (str);
  }
  
  return FetchReq::GetSerializedSize ();
}

void
GUSearchMessage::SetFetchReq (uint32_t originatorNum, std::string key, std::set<std::string> searchKeys, std::set<std::string> documents)
{
  if (m_messageType == 0)
    {
      m_messageType = FETCH_REQ;
    }
  else
    {
      NS_ASSERT (m_messageType == FETCH_REQ);
    }
  m_message.fetchReq.originatorNum = originatorNum ;
  m_message.fetchReq.key = key;
  m_message.fetchReq.searchKeys = searchKeys;
  m_message.fetchReq.documents = documents;
}

GUSearchMessage::FetchReq
GUSearchMessage::GetFetchReq ()
{
  return m_message.fetchReq;
}

/* FETCH_RSP */
uint32_t 
GUSearchMessage::FetchRsp::GetSerializedSize (void) const
{
  uint32_t size = 0;
  size += sizeof(uint32_t);
  for (std::set<std::string>::iterator it = documents.begin(); it != documents.end(); it++) {
    size += sizeof(uint16_t);
    size += (*it).length();
  }
  return size;
}

void
GUSearchMessage::FetchRsp::Print (std::ostream &os) const
{
  os << "FetchRsp:: Documents: " ; 
  for (std::set<std::string>::iterator it = documents.begin(); it != documents.end(); it++) {
    os << *it << ", ";
  }
  os << "\n";
}

void
GUSearchMessage::FetchRsp::Serialize (Buffer::Iterator &start) const
{ 
  start.WriteHtonU32(documents.size());
  
  for (std::set<std::string>::iterator it = documents.begin(); it != documents.end(); it++) {
    start.WriteU16 ((*it).length());
    start.Write ((uint8_t *) (const_cast<char*> ((*it).c_str())), (*it).length());
  }
}

uint32_t
GUSearchMessage::FetchRsp::Deserialize (Buffer::Iterator &start)
{  
  uint32_t dlen = start.ReadNtohU32();
  for (uint32_t  i = 0; i < dlen; i++) {
    uint16_t length = start.ReadU16 ();
    char* str = (char*) malloc (length);
    start.Read ((uint8_t*)str, length);
    documents.insert(std::string (str, length));
    free (str);
  }
  
  return FetchRsp::GetSerializedSize ();
}

void
GUSearchMessage::SetFetchRsp (std::set<std::string> documents)
{
  if (m_messageType == 0)
    {
      m_messageType = FETCH_RSP;
    }
  else
    {
      NS_ASSERT (m_messageType == FETCH_RSP);
    }
  m_message.fetchRsp.documents = documents;
}

GUSearchMessage::FetchRsp
GUSearchMessage::GetFetchRsp ()
{
  return m_message.fetchRsp;
}


//
//
//

void
GUSearchMessage::SetMessageType (MessageType messageType)
{
  m_messageType = messageType;
}

GUSearchMessage::MessageType
GUSearchMessage::GetMessageType () const
{
  return m_messageType;
}

void
GUSearchMessage::SetTransactionId (uint32_t transactionId)
{
  m_transactionId = transactionId;
}

uint32_t 
GUSearchMessage::GetTransactionId (void) const
{
  return m_transactionId;
}

