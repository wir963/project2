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


#include "gu-search.h"
#include <fstream>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <ios>
#include <iomanip>
#include "ns3/random-variable.h"
#include "ns3/inet-socket-address.h"

using namespace ns3;

TypeId
GUSearch::GetTypeId ()
{
  static TypeId tid = TypeId ("GUSearch")
    .SetParent<GUApplication> ()
    .AddConstructor<GUSearch> ()
    .AddAttribute ("AppPort",
                   "Listening port for Application",
                   UintegerValue (10000),
                   MakeUintegerAccessor (&GUSearch::m_appPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("ChordPort",
                   "Listening port for Application",
                   UintegerValue (10001),
                   MakeUintegerAccessor (&GUSearch::m_chordPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PingTimeout",
                   "Timeout value for PING_REQ in milliseconds",
                   TimeValue (MilliSeconds (2000)),
                   MakeTimeAccessor (&GUSearch::m_pingTimeout),
                   MakeTimeChecker ())
    ;
  return tid;
}

GUSearch::GUSearch ()
  : m_auditPingsTimer (Timer::CANCEL_ON_DESTROY)
{
  m_chord = NULL;
  RandomVariable random;
  SeedManager::SetSeed (time (NULL));
  random = UniformVariable (0x00000000, 0xFFFFFFFF);
  m_currentTransactionId = random.GetInteger ();
}

GUSearch::~GUSearch ()
{

}

void
GUSearch::DoDispose ()
{
  StopApplication ();
  GUApplication::DoDispose ();
}

void
GUSearch::StartApplication (void)
{
  // Create and Configure GUChord
  ObjectFactory factory;
  factory.SetTypeId (GUChord::GetTypeId ());
  factory.Set ("AppPort", UintegerValue (m_chordPort));
  m_chord = factory.Create<GUChord> ();
  m_chord->SetNode (GetNode ());
  m_chord->SetNodeAddressMap (m_nodeAddressMap);
  m_chord->SetAddressNodeMap (m_addressNodeMap);
  m_chord->SetModuleName ("CHORD");
  std::string nodeId = GetNodeId ();
  m_chord->SetNodeId (nodeId);
  m_chord->SetLocalAddress(m_local);

  if (GUApplication::IsRealStack ())
  {
    m_chord->SetRealStack (true);
  } 

  // Configure Callbacks with Chord
  m_chord->SetPingSuccessCallback (MakeCallback (&GUSearch::HandleChordPingSuccess, this)); 
  m_chord->SetPingFailureCallback (MakeCallback (&GUSearch::HandleChordPingFailure, this));
  m_chord->SetPingRecvCallback (MakeCallback (&GUSearch::HandleChordPingRecv, this)); 
  m_chord->SetChordLookupCallback (MakeCallback (&GUSearch::HandleChordLookupCallback, this));
  m_chord->SetChordLeaveCallback (MakeCallback (&GUSearch::HandleChordLeaveRequest, this));
  m_chord->SetPredecessorChangeCallback (MakeCallback (&GUSearch::HandlePredecessorChangeCallback, this));
  
  // Start Chord
  m_chord->SetStartTime (Simulator::Now());
  m_chord->Start ();
  if (m_socket == 0)
    { 
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny(), m_appPort);
      m_socket->Bind (local);
      m_socket->SetRecvCallback (MakeCallback (&GUSearch::RecvMessage, this));
    }  
  
  // Configure timers
  m_auditPingsTimer.SetFunction (&GUSearch::AuditPings, this);
  // Start timers
  m_auditPingsTimer.Schedule (m_pingTimeout);
}

void
GUSearch::StopApplication (void)
{
  //Stop chord
  m_chord->StopChord ();
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
GUSearch::ProcessCommand (std::vector<std::string> tokens)
{
  std::vector<std::string>::iterator iterator = tokens.begin();
  std::string command = *iterator;
  if (command == "CHORD")
    { 
      // Send to Chord Sub-Layer
      tokens.erase (iterator);
      m_chord->ProcessCommand (tokens);
    } 
  if (command == "PING")
    {
      if (tokens.size() < 3)
        {
          ERROR_LOG ("Insufficient PING params..."); 
          return;
        }
      iterator++;
      if (*iterator != "*")
        {
          std::string nodeId = *iterator;
          iterator++;
          std::string pingMessage = *iterator;
          SendPing (nodeId, pingMessage);
        }
      else
        {
          iterator++;
          std::string pingMessage = *iterator;
          std::map<uint32_t, Ipv4Address>::iterator iter;
          for (iter = m_nodeAddressMap.begin () ; iter != m_nodeAddressMap.end (); iter++)  
            {
              std::ostringstream sin;
              uint32_t nodeNumber = iter->first;
              sin << nodeNumber;
              std::string nodeId = sin.str();    
              SendPing (nodeId, pingMessage);
            }
        }
    }
  if(command == "PUBLISH" || command == "publish") {
    iterator++;
    std::string filename = *iterator;
    CreateInvertedList(filename);

    PublishList();
  }

  if(command == "SEARCH" || command == "search") {
	
    std::string requestingNodeStr = g_nodeId;
    std::istringstream sin (requestingNodeStr);
    uint32_t requestingNodeNum;
    sin >> requestingNodeNum; //the node to which the final response be sent
    
    iterator++;
    std::string viaNodeStr= *iterator;
    std::istringstream sin2 (viaNodeStr);
    uint32_t viaNodeNum;	
    sin2 >> viaNodeNum; //the node in the chord ring via which the search query is initiated

    std::set<std::string> searchKeys;
    iterator++; 
    std::string searchKeysForPrint;
    while(iterator != tokens.end()){
      searchKeys.insert(*iterator);
      searchKeysForPrint += *iterator + " ";
      iterator++;
    }
    
    //SEARCH_LOG("Search< "<< searchKeysForPrint <<">");
    
    // create empty results
    std::set<std::string> empty;
    
    //Send search request to via node
    SendSearchRequest(viaNodeNum, requestingNodeNum, searchKeys, empty);
 
  } 

  if (command == "PRINT_DOCS" || command == "print_docs") {
    PrintMyDocuments();
  } 
}

void     
GUSearch::SendSearchRequest(uint32_t viaNodeNum, uint32_t requestingNodeNum, 
                            std::set<std::string>searchKeys, std::set<std::string> existingDocuments){
//Send FETCH_REQ to viaNodeNum

  std::stringstream nodeNumStream;
  nodeNumStream << viaNodeNum;
  std::string nodeNumStr = nodeNumStream.str();

  uint32_t transId = GetNextTransactionId();
  Ipv4Address destAddress = ResolveNodeIpAddress(nodeNumStr);
  GUSearchMessage searchReqMsg = GUSearchMessage (GUSearchMessage::FETCH_REQ, transId);
  
  Ptr<Packet> packet = Create<Packet> ();
  
  std::stringstream ss;
  std::set<std::string>::iterator b;
  for(b = searchKeys.begin(); b != searchKeys.end(); b++){  
    ss << *b << " ";
  }
  SEARCH_LOG("Search< " << ss.str() << ">");
  
  
  searchReqMsg.SetFetchReq (requestingNodeNum, "", searchKeys, existingDocuments);
  packet->AddHeader (searchReqMsg);
  m_socket->SendTo (packet, 0 , InetSocketAddress (destAddress, m_appPort));
}

void
GUSearch::PublishList() {
  //print all the index
  std::map<std::string,std::set<std::string> >::iterator key_it;
  std::set<std::string>::iterator doc_it;
  
  for(key_it = m_index.begin(); key_it != m_index.end(); key_it++){
    
    std::string key = key_it->first;
    
    // 1. hash the key
    unsigned char temp[20];
    SHA1((unsigned char *)key.c_str(), strlen(key.c_str()), temp);
    std::ostringstream s;
    s << std::hex << std::setfill('0');
    for (int i = 0; i < 20; i++) {
      s << std::setw(2) << static_cast<int>(temp[i]);
    }
    std::string lookupKey = s.str();
    
    // 2. send chord lookup
    uint32_t transId = GetNextTransactionId();
    
    KeyLookupInformation kli;
    kli.lookupKey = lookupKey;
    kli.actualKey = key;
    kli.operationType = STORE;
    m_keyRequestTracker[transId] = kli;
    
    std::set<std::string> results = m_index[key];
    std::stringstream ss;
    for(std::set<std::string>::iterator i = results.begin(); i != results.end(); i++){  
      ss << *i << " ";
    }
    SEARCH_LOG("Publish< " << key << ", " << ss.str() << ">");
    
    m_chord->SendChordLookup(lookupKey, transId);
    
  }
}

void 
GUSearch::CreateInvertedList(std::string filename){
  //std::cout<<"Creating inverted list"<<std::endl;
  std::cout<<"Metadata file: "<<filename<<std::endl; 
  
  std::string line;
  
  std::ifstream file (filename.c_str());
  if (file.is_open()) {
    while ( getline (file,line) ) {
      std::istringstream iss(line);
      
      std::string key_term; 
      std::string document;
      std::string temp;
      uint32_t count = 0;
      
      while (std::getline(iss,temp, ' '))
      {
        if(count == 0){ 
          //this is the document
          document = temp;
          count++;
          continue; 
        }else{
          key_term = temp;
        }
        //Add document to key_term index
        std::set<std::string> existing_docs (m_index[key_term]);
        existing_docs.insert(document);
        m_index[key_term] = existing_docs;
        
        std::set<std::string>::iterator it;
      }
    }
    file.close();
  }
  
  //print all the index
  typedef std::set<std::string> SET;
  std::map<std::string,SET>::iterator key_it;
  std::set<std::string>::iterator doc_it;
  
  for(key_it = m_index.begin(); key_it != m_index.end(); key_it++){
    
    std::string key = key_it->first;
    std::set<std::string> tempSet = key_it->second;
    
    std::cout<< key << ": ";
    for(doc_it = tempSet.begin(); doc_it != tempSet.end(); doc_it++){
      
      std::cout<< *doc_it << " ";
      
    }
    std::cout<<std::endl;
  }
  
}

void
GUSearch::SendPing (std::string nodeId, std::string pingMessage)
{
  // Send Ping Via-Chord layer 
  SEARCH_LOG ("Sending Ping via Chord Layer to node: " << nodeId << " Message: " << pingMessage);
  Ipv4Address destAddress = ResolveNodeIpAddress(nodeId);
  m_chord->SendPing (destAddress, pingMessage);
}

void
GUSearch::SendGUSearchPing (Ipv4Address destAddress, std::string pingMessage)
{
  if (destAddress != Ipv4Address::GetAny ())
    {
      uint32_t transactionId = GetNextTransactionId ();
      SEARCH_LOG ("Sending PING_REQ to Node: " << ReverseLookup(destAddress) << " IP: " << destAddress << " Message: " << pingMessage << " transactionId: " << transactionId);
      Ptr<PingRequest> pingRequest = Create<PingRequest> (transactionId, Simulator::Now(), destAddress, pingMessage);
      // Add to ping-tracker
      m_pingTracker.insert (std::make_pair (transactionId, pingRequest));
      Ptr<Packet> packet = Create<Packet> ();
      GUSearchMessage message = GUSearchMessage (GUSearchMessage::PING_REQ, transactionId);
      message.SetPingReq (pingMessage);
      packet->AddHeader (message);
      m_socket->SendTo (packet, 0 , InetSocketAddress (destAddress, m_appPort));
    }


}

void
GUSearch::RecvMessage (Ptr<Socket> socket)
{
  Address sourceAddr;
  Ptr<Packet> packet = socket->RecvFrom (sourceAddr);
  InetSocketAddress inetSocketAddr = InetSocketAddress::ConvertFrom (sourceAddr);
  Ipv4Address sourceAddress = inetSocketAddr.GetIpv4 ();
  uint16_t sourcePort = inetSocketAddr.GetPort ();
  GUSearchMessage message;
  packet->RemoveHeader (message);

  switch (message.GetMessageType ())
    {
      case GUSearchMessage::PING_REQ:
        ProcessPingReq (message, sourceAddress, sourcePort);
        break;
      case GUSearchMessage::PING_RSP:
        ProcessPingRsp (message, sourceAddress, sourcePort);
        break;
      case GUSearchMessage::STORE_REQ:
        ProcessStoreReq (message, sourceAddress, sourcePort);
        break;        
      case GUSearchMessage::FETCH_REQ:
        ProcessFetchReq (message, sourceAddress, sourcePort);
        break;
      case GUSearchMessage::FETCH_RSP:
        ProcessFetchRsp (message, sourceAddress, sourcePort);
        break;
      default:
        ERROR_LOG ("Unknown Message Type!");
        break;
    }
}

void
GUSearch::ProcessPingReq (GUSearchMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{

    // Use reverse lookup for ease of debug
    std::string fromNode = ReverseLookup (sourceAddress);
    SEARCH_LOG ("Received PING_REQ, From Node: " << fromNode << ", Message: " << message.GetPingReq().pingMessage);
    // Send Ping Response
    GUSearchMessage resp = GUSearchMessage (GUSearchMessage::PING_RSP, message.GetTransactionId());
    resp.SetPingRsp (message.GetPingReq().pingMessage);
    Ptr<Packet> packet = Create<Packet> ();
    packet->AddHeader (resp);
    m_socket->SendTo (packet, 0 , InetSocketAddress (sourceAddress, sourcePort));
}

void
GUSearch::ProcessPingRsp (GUSearchMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{
  // Remove from pingTracker
  std::map<uint32_t, Ptr<PingRequest> >::iterator iter;
  iter = m_pingTracker.find (message.GetTransactionId ());
  if (iter != m_pingTracker.end ())
    {
      std::string fromNode = ReverseLookup (sourceAddress);
      SEARCH_LOG ("Received PING_RSP, From Node: " << fromNode << ", Message: " << message.GetPingRsp().pingMessage);
      m_pingTracker.erase (iter);
    }
  else
    {
      DEBUG_LOG ("Received invalid PING_RSP!");
    }
}

void 
GUSearch::ProcessStoreReq (GUSearchMessage message, Ipv4Address sourceAddress, uint16_t sourcePort) {

  std::set<std::string> documents = message.GetStoreReq().documents;
  std::stringstream ss;
  for (std::set<std::string>::iterator it = documents.begin(); it != documents.end(); it++) {
    m_documents[message.GetStoreReq().key].insert(*it);
    ss << *it << " ";
  }

  SEARCH_LOG("Store< " << message.GetStoreReq().key << ", " << ss.str() << ">");
}

void 
GUSearch::ProcessFetchReq(GUSearchMessage message, Ipv4Address sourceAddress, uint16_t sourcePort){

  std::string firstKey = message.GetFetchReq().key;
  std::set<std::string> l_searchKeys = message.GetFetchReq().searchKeys;
  
  std::set<std::string> resultDocuments;
  
  if (firstKey == "" && !l_searchKeys.empty()) {
    // we are first!
    
    std::set<std::string>::iterator it = l_searchKeys.begin();
    firstKey = *it;
    l_searchKeys.erase(it); 
  
    // 1. hash the key
    unsigned char temp[20];
    SHA1((unsigned char *)firstKey.c_str(), strlen(firstKey.c_str()), temp);
    std::ostringstream s;
    s << std::hex << std::setfill('0');
    for (int i = 0; i < 20; i++) {
      s << std::setw(2) << static_cast<int>(temp[i]);
    }
    std::string lookupKey = s.str();
    
    // 2. send chord lookup
    uint32_t transId = GetNextTransactionId();
  
    KeyLookupInformation kli;
    kli.lookupKey = lookupKey;
    kli.actualKey = firstKey;
    kli.operationType = FETCH;
    GUSearchMessage::FetchReq fetchReq;
    fetchReq.key = firstKey;
    fetchReq.originatorNum = message.GetFetchReq().originatorNum;
    fetchReq.searchKeys = l_searchKeys;
    fetchReq.documents = message.GetFetchReq().documents;
    kli.fetchReq = fetchReq;
    m_keyRequestTracker[transId] = kli;
    
    m_chord->SendChordLookup(lookupKey, transId);
    
  } else {
    // we are not first
    
    std::set<std::string> myResults;
    if (m_documents.find(firstKey) != m_documents.end())
      myResults = (m_documents.find(firstKey))->second;
    
    if (myResults.empty()) {
      
      SEARCH_LOG("SearchResults<" << g_nodeId << ",\"EmptyList\">");
      
      //  send "no results" to message.GetFetchReq().originatorNum
      Ptr<Packet> packet = Create<Packet> ();
      GUSearchMessage fetchRsp = GUSearchMessage (GUSearchMessage::FETCH_RSP, GetNextTransactionId());
      uint32_t nodeNum = message.GetFetchReq().originatorNum;
      std::stringstream nodeNumStream;
      nodeNumStream << nodeNum;
      std::string nodeNumStr = nodeNumStream.str();
      
      fetchRsp.SetFetchRsp(myResults);
      packet->AddHeader(fetchRsp);
      m_socket->SendTo (packet, 0 , InetSocketAddress (ResolveNodeIpAddress(nodeNumStr), m_appPort));
      
      return;
    }
    
    if (message.GetFetchReq().documents.empty()) {
      resultDocuments = myResults;
    } else {
      
      std::set<std::string> receivedDocuments = message.GetFetchReq().documents;
      
      // resultDocuments = receivedDocuments INTERSECT myResults
      for(std::set<std::string>::iterator myit = myResults.begin(); myit != myResults.end(); myit++) {
        for(std::set<std::string>::iterator rcvit = receivedDocuments.begin(); 
            rcvit != receivedDocuments.end(); rcvit++) {
          if(*myit == *rcvit){
            resultDocuments.insert(*myit);
          } 
        }
      }
      
    }
    
    
    if (l_searchKeys.empty()){
    
      //  send result to message.GetFetchReq().originatorNum
      Ptr<Packet> packet = Create<Packet> ();
      GUSearchMessage fetchRsp = GUSearchMessage (GUSearchMessage::FETCH_RSP, GetNextTransactionId());

      uint32_t nodeNum = message.GetFetchReq().originatorNum;
      std::stringstream nodeNumStream;
      nodeNumStream << nodeNum;
      std::string nodeNumStr = nodeNumStream.str();

      /*      
      std::set<std::string>::iterator d;
      std::stringstream res;
      for(d = resultDocuments.begin(); d != resultDocuments.end(); d++){  
        res << *d << " ";
      }
      
      if (resultDocuments.empty()){
        SEARCH_LOG("SearchResults<" << g_nodeId << ",\"EmptyList\">");
      } else {
        SEARCH_LOG("SearchResults< "<< nodeNum <<", " << res.str() << " >");
      }
      */

      fetchRsp.SetFetchRsp(resultDocuments);
      packet->AddHeader(fetchRsp);
      m_socket->SendTo (packet, 0 , InetSocketAddress (ResolveNodeIpAddress(nodeNumStr), m_appPort));
      
    } else {
      // extract key
      std::set<std::string> remainingSearchKeys = message.GetFetchReq().searchKeys;
      std::set<std::string>::iterator it = remainingSearchKeys.begin();
      std::string extractedKey = *it;
      remainingSearchKeys.erase(it); 
            
      // 1. hash the key
      unsigned char temp[20];
      SHA1((unsigned char *)extractedKey.c_str(), strlen(extractedKey.c_str()), temp);
      std::ostringstream s;
      s << std::hex << std::setfill('0');
      for (int i = 0; i < 20; i++) {
        s << std::setw(2) << static_cast<int>(temp[i]);
      }
      std::string lookupKey = s.str();
      
      // 2. send chord lookup
      uint32_t transId = GetNextTransactionId();
    
      KeyLookupInformation kli;
      kli.lookupKey = lookupKey;
      kli.actualKey = extractedKey;
      kli.operationType = FETCH;
      GUSearchMessage::FetchReq fetchReq;
      fetchReq.key = extractedKey;
      fetchReq.originatorNum = message.GetFetchReq().originatorNum;
      fetchReq.searchKeys = remainingSearchKeys;
      fetchReq.documents = resultDocuments;
      kli.fetchReq = fetchReq;
      m_keyRequestTracker[transId] = kli;
      
      m_chord->SendChordLookup(lookupKey, transId);
      
      std::stringstream res;
      for(std::set<std::string>::iterator i = resultDocuments.begin(); i != resultDocuments.end(); i++){  
        res << *i << " ";
      }
      SEARCH_LOG("InvertedListShip< "<< extractedKey <<", " << res.str() << " >");
      
    }
  }
  
}
    
void 
GUSearch::ProcessFetchRsp (GUSearchMessage message, Ipv4Address sourceAddress, uint16_t sourcePort) {
  std::set<std::string> results = message.GetFetchRsp().documents;

  std::set<std::string>::iterator d;
  std::stringstream res;
  for(d = results.begin(); d != results.end(); d++){  
    res << *d << " ";
  }

  if (results.empty()){
    SEARCH_LOG("SearchResults<" << g_nodeId << ",\"EmptyList\">");
  } else {
    SEARCH_LOG("SearchResults< "<< g_nodeId <<", " << res.str() << " >");
  }
}

void
GUSearch::PrintMyDocuments() {
  
  std::cout << "DOCUMENTS FOR NODE " << g_nodeId << ": "<<std::endl;
  
  //m_documents
  std::map<std::string,std::set<std::string> >::iterator a;
  std::set<std::string>::iterator b;
  for(a = m_documents.begin(); a != m_documents.end(); a++){
    std::string key = a->first;
    std::cout << " " << key << ":";
    std::set<std::string> tempSet = a->second;
    for(b = tempSet.begin(); b != tempSet.end(); b++){  
      std::cout<< *b << ",";
    }
    std::cout<<std::endl;
  }
}


void
GUSearch::AuditPings ()
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
GUSearch::GetNextTransactionId ()
{
  return m_currentTransactionId++;
}

// Handle Chord Callbacks

void
GUSearch::HandleChordPingFailure (Ipv4Address destAddress, std::string message)
{
  SEARCH_LOG ("Chord Ping Expired! Destination nodeId: " << ReverseLookup(destAddress) << " IP: " << destAddress << " Message: " << message);
}

void
GUSearch::HandleChordPingSuccess (Ipv4Address destAddress, std::string message)
{
  SEARCH_LOG ("Chord Ping Success! Destination nodeId: " << ReverseLookup(destAddress) << " IP: " << destAddress << " Message: " << message);
  // Send ping via search layer 
  SendGUSearchPing (destAddress, message);
}

void
GUSearch::HandleChordPingRecv (Ipv4Address destAddress, std::string message)
{
  SEARCH_LOG ("Chord Layer Received Ping! Source nodeId: " << ReverseLookup(destAddress) << " IP: " << destAddress << " Message: " << message);
}

void
GUSearch::HandleChordLeaveRequest (Ipv4Address destAddress, uint32_t successorNodeNum)
{
  
  std::stringstream nodeNumStream;
  nodeNumStream << successorNodeNum;
  std::string nodeNumStr = nodeNumStream.str();

  std::map<std::string,std::set<std::string> >::iterator a;
  for(a = m_documents.begin(); a != m_documents.end(); a++){
    std::string key = a->first;
    std::set<std::string> tempSet = a->second;
    
    Ptr<Packet> packet = Create<Packet> ();
    GUSearchMessage storeReq = GUSearchMessage (GUSearchMessage::STORE_REQ, GetNextTransactionId());
    
    storeReq.SetStoreReq (key, tempSet);
    packet->AddHeader (storeReq);
    m_socket->SendTo (packet, 0 , InetSocketAddress (ResolveNodeIpAddress(nodeNumStr), m_appPort));
  }
  m_documents.clear();
}

void
GUSearch::HandlePredecessorChangeCallback (Ipv4Address destAddress, std::string message) {

  std::stringstream nodeNumStream;
  nodeNumStream << m_chord->predecessor_id;
  std::string nodeNumStr = nodeNumStream.str(); 

  std::map<std::string,std::set<std::string> >::iterator a;
  std::set<std::string>::iterator b;
  for(a = m_documents.begin(); a != m_documents.end(); a++){
    std::string key = a->first;
        
    // 1. hash the key
    unsigned char temp[20];
    SHA1((unsigned char *)key.c_str(), strlen(key.c_str()), temp);
    std::ostringstream s;
    s << std::hex << std::setfill('0');
    for (int i = 0; i < 20; i++) {
      s << std::setw(2) << static_cast<int>(temp[i]);
    }
    std::string lookupKeyStr = s.str();
    
    // 2. compare yourself
    
    bool mine = false;

    mpz_t my_key_gmp;
    mpz_init_set_str(my_key_gmp, m_chord->my_node_key_hex.c_str() , 16);

    mpz_t predecessor_key_gmp;
    mpz_init_set_str(predecessor_key_gmp, m_chord->predecessor_node_key_hex.c_str() , 16);

    mpz_t max;
    mpz_init_set_str(max, "ffffffffffffffffffffffffffffffffffffffff" , 16);

    mpz_t lookupKey;
    mpz_init_set_str(lookupKey, lookupKeyStr.c_str() , 16);
    
    if ( mpz_cmp(predecessor_key_gmp, my_key_gmp) < 0 ) {
      // I am bigger than my predecessor
      if ( mpz_cmp(lookupKey, my_key_gmp) <= 0 && mpz_cmp(lookupKey, predecessor_key_gmp) > 0) {
        mine = true;
      }
    } else {
      // I am less or equal than predecessor - crosses hash boundary
      mpz_t modifiedMe;
      mpz_init_set(modifiedMe, my_key_gmp);
      mpz_add(modifiedMe, modifiedMe, max);
      
      if ( mpz_cmp(lookupKey, modifiedMe) <= 0 && mpz_cmp(lookupKey, predecessor_key_gmp) > 0) {
        mine = true;
      } else {
        mpz_t modifiedKey;
        mpz_init_set(modifiedKey, lookupKey);
        mpz_add(modifiedKey, modifiedKey, max);
        
        if ( mpz_cmp(modifiedKey, modifiedMe) <= 0 && mpz_cmp(modifiedKey, predecessor_key_gmp) > 0) {
          mine = true;
        }
      }
    }
    
    if (!mine) {
      GUSearchMessage storeReq = GUSearchMessage (GUSearchMessage::STORE_REQ, GetNextTransactionId());
      Ptr<Packet> packet = Create<Packet> ();
      storeReq.SetStoreReq (key, m_documents[key]);
      packet->AddHeader (storeReq);
      m_socket->SendTo (packet, 0 , InetSocketAddress (ResolveNodeIpAddress(nodeNumStr), m_appPort));
      
      // erase that key from documents since I already sent it
      m_documents.erase(key);
    }

  }
}

void
GUSearch::HandleChordLookupCallback (Ipv4Address destAddress, uint32_t nodeNum, std::string nodeHash, uint32_t transId)
{
  TRAFFIC_LOG ("Chord Layer Received Lookup Response! Source nodeId: " << ReverseLookup(destAddress) << " IP: " << destAddress << " ResultNodeNum: " << nodeNum << " ResultNodeHash: " << nodeHash << " Transaction ID:" << transId);
  
  std::stringstream nodeNumStream;
  nodeNumStream << nodeNum;
  std::string nodeNumStr = nodeNumStream.str();
  
  KeyLookupInformation kli = m_keyRequestTracker[transId];
  std::string key = kli.actualKey;
  OperationType opType = kli.operationType;
  GUSearchMessage::FetchReq fetchRq = kli.fetchReq;
  
  GUSearchMessage storeReq = GUSearchMessage (GUSearchMessage::STORE_REQ, transId);
  GUSearchMessage fetchReq = GUSearchMessage (GUSearchMessage::FETCH_REQ, transId);
  Ptr<Packet> packet = Create<Packet> ();
  
  switch (opType) {
    case STORE:
      // send the key + documents to ResolveNodeIpAddress(nodeNum) 
      // send Store Request 
      storeReq.SetStoreReq (key, m_index[key]);
      packet->AddHeader (storeReq);


      m_socket->SendTo (packet, 0 , InetSocketAddress (ResolveNodeIpAddress(nodeNumStr), m_appPort));

      // erase that key from documents since I already sent it
      m_index.erase(key);
      
      // erase transaction ID from key request tracker
      m_keyRequestTracker.erase(transId);
      break;
    case FETCH:
      // std::cout << "FETCH" << std::endl;
      
      fetchReq.SetFetchReq(fetchRq.originatorNum, fetchRq.key, fetchRq.searchKeys, fetchRq.documents);
      packet->AddHeader(fetchReq);
      m_socket->SendTo (packet, 0 , InetSocketAddress (ResolveNodeIpAddress(nodeNumStr), m_appPort));
      
      m_keyRequestTracker.erase(transId);
      
      break;
    case CHECK:
      if (nodeNumStr != g_nodeId) {
        // it is not mine, send it..
        storeReq.SetStoreReq (key, m_documents[key]);
        packet->AddHeader (storeReq);
        m_socket->SendTo (packet, 0 , InetSocketAddress (ResolveNodeIpAddress(nodeNumStr), m_appPort));
        
        // erase that key from documents since I already sent it
        m_documents.erase(key);
      } 
      m_keyRequestTracker.erase(transId);
      break;
    default:
      std::cout << "ALARM! SOMETHING IS REALLY WRONG! UNKNOWN OPERATION TYPE FOR KEY LOOKUP " << key << std::endl;
      break;
  }
  
}

// Override GULog
void
GUSearch::SetTrafficVerbose (bool on)
{ 
  m_chord->SetTrafficVerbose (on);
  g_trafficVerbose = on;
}

void
GUSearch::SetErrorVerbose (bool on)
{ 
  m_chord->SetErrorVerbose (on);
  g_errorVerbose = on;
}

void
GUSearch::SetDebugVerbose (bool on)
{
  m_chord->SetDebugVerbose (on);
  g_debugVerbose = on;
}

void
GUSearch::SetStatusVerbose (bool on)
{
  m_chord->SetStatusVerbose (on);
  g_statusVerbose = on;
}

void
GUSearch::SetChordVerbose (bool on)
{
  m_chord->SetChordVerbose (on);
  g_chordVerbose = on;
}

void
GUSearch::SetSearchVerbose (bool on)
{
  m_chord->SetSearchVerbose (on);
  g_searchVerbose = on;
}
