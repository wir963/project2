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

#ifndef GU_CHORD_MESSAGE_H
#define GU_CHORD_MESSAGE_H

#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/packet.h"
#include "ns3/object.h"

using namespace ns3;

#define IPV4_ADDRESS_SIZE 4

class GUChordMessage : public Header
{
  public:
    GUChordMessage ();
    virtual ~GUChordMessage ();


    enum MessageType
      {
        PING_REQ = 1,
        PING_RSP = 2,
        JOIN_REQ = 3,
        JOIN_RSP = 4,
        DEPARTURE_REQ = 5,
        STABILIZE_RSP = 6,
        STABILIZE_REQ = 7       
      };

    GUChordMessage (GUChordMessage::MessageType messageType, uint32_t transactionId);

    /**
    *  \brief Sets message type
    *  \param messageType message type
    */
    void SetMessageType (MessageType messageType);

    /**
     *  \returns message type
     */
    MessageType GetMessageType () const;

    /**
     *  \brief Sets Transaction Id
     *  \param transactionId Transaction Id of the request
     */
    void SetTransactionId (uint32_t transactionId);

    /**
     *  \returns Transaction Id
     */
    uint32_t GetTransactionId () const;

  private:
    /**
     *  \cond
     */
    MessageType m_messageType;
    uint32_t m_transactionId;
    /**
     *  \endcond
     */
  public:
    static TypeId GetTypeId (void);
    virtual TypeId GetInstanceTypeId (void) const;
    void Print (std::ostream &os) const;
    uint32_t GetSerializedSize (void) const;
    void Serialize (Buffer::Iterator start) const;
    uint32_t Deserialize (Buffer::Iterator start);

    
    struct PingReq
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        // Payload
        std::string pingMessage;
      };

    struct PingRsp
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        // Payload
        std::string pingMessage;
      };
    
    struct JoinReq
    {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        

        // Payload
        std::string joinReqMessage;

        uint32_t sender_node_id;
        Ipv4Address sender_node_ip_address;
    };
    
    struct JoinRsp
    {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        
        // Payload
        std::string joinRspMessage;

        uint32_t successor_node_id;
        Ipv4Address successor_node_ip_address;
    };
    
    struct DepartureReq
    {

        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        
        // Payload
        std::string departureReqMessage;        

        uint32_t sender_node_id;
        Ipv4Address sender_node_ip_address;
        uint32_t conn_node_id;
        Ipv4Address conn_node_ip_address;
    };
    
    struct StabilizeReq
    {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        
        // Payload
        std::string stabilizeReqMessage;        

        uint32_t sender_node_id;
        Ipv4Address sender_node_ip_address;
    };
    
    struct StabilizeRsp
    {

        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        
        // Payload
        std::string stabilizeRspMessage;

        uint32_t predecessor_node_id;
        Ipv4Address predecessor_node_ip_address;
    };


  private:
    struct
      {
        PingReq pingReq;
        PingRsp pingRsp;
        JoinReq joinReq;
        JoinRsp joinRsp;
        DepartureReq departureReq;
        StabilizeReq stabilizeReq;
        StabilizeRsp stabilizeRsp;

      } m_message;
    
  public:
    /**
     *  \returns PingReq Struct
     */
    PingReq GetPingReq ();

    /**
     *  \brief Sets PingReq message params
     *  \param message Payload String
     */

    void SetPingReq (std::string message);

    /**
     * \returns PingRsp Struct
     */
    PingRsp GetPingRsp ();
    /**
     *  \brief Sets PingRsp message params
     *  \param message Payload String
     */
    void SetPingRsp (std::string message);

    JoinReq GetJoinReq ();
    void SetJoinReq (std::string message);

    JoinRsp GetJoinRsp ();
    void SetJoinRsp (std::string message);

    DepartureReq GetDepartureReq ();
    void SetDepartureReq (std::string message);

    StabilizeReq GetStabilizeReq ();
    void SetStabilizeReq (std::string message);

    StabilizeRsp GetStabilizeRsp ();
    void SetStabilizeRsp (std::string message);

}; // class GUChordMessage

static inline std::ostream& operator<< (std::ostream& os, const GUChordMessage& message)
{
  message.Print (os);
  return os;
}

#endif
