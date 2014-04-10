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


#include "ns3/gu-log.h"

using namespace ns3;

GULog::GULog ()
{
  g_nodeId = "Unknown";
  g_moduleName = "Unknown";
  g_errorVerbose = true;
  g_statusVerbose = true;
  g_trafficVerbose = false;
  g_debugVerbose = false;
  g_chordVerbose = false;
  g_searchVerbose = false;
}

GULog::~GULog ()
{}

void
GULog::SetTrafficVerbose (bool on)
{ 
  g_trafficVerbose = on;
}

void
GULog::SetErrorVerbose (bool on)
{ 
  g_errorVerbose = on;
}

void
GULog::SetDebugVerbose (bool on)
{ 
  g_debugVerbose = on;
}

void
GULog::SetStatusVerbose (bool on)
{
  g_statusVerbose = on;
}

void
GULog::SetChordVerbose (bool on)
{
  g_chordVerbose = on;
}

void
GULog::SetSearchVerbose (bool on)
{
  g_searchVerbose = on;
}


void
GULog::SetNodeId (std::string nodeId)
{
  g_nodeId = nodeId;
}

std::string
GULog::GetNodeId ()
{
  return g_nodeId;
}


void
GULog::SetModuleName (std::string moduleName)
{
  g_moduleName = moduleName;
}
