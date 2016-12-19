#include "Channel.hpp"

Channel::Channel(unsigned int channelId, Peer* serverPeer) 
{
    if (channelId != 0 || serverPeer != NULL) //Avoid creation by map[]
    {
        this->channelId = channelId;
        this->serverPeer = serverPeer;
        if(serverPeer)
            AddPeer(serverPeer);
        serverEstimatedStreamRate = 0;

        //Logging
        struct tm * timeinfo;
        char timestr[20];
        time(&creationTime);
        timeinfo = localtime(&creationTime);
        strftime (timestr,20,"%Y%m%d%H%M",timeinfo);
        string logFilename = "log-";
        logFilename += boost::lexical_cast<string>(channelId) + "-";
        logFilename += serverPeer->GetIP() + "_" + serverPeer->GetPort() + "-";
        logFilename += timestr;
        logFilename += "-";
        string logFilenamePerf = logFilename + "perf.txt";
        string logFilenameOverlay = logFilename + "overlay.txt";
        performanceFile = fopen(logFilenamePerf.c_str(),"w");
        overlayFile = fopen(logFilenameOverlay.c_str(),"w");
    } 
}

void Channel::SetServerNewestChunkID(ChunkUniqueID serverNewestChunkID)
{
    this->serverNewestChunkID = serverNewestChunkID;
}

ChunkUniqueID Channel::GetServerNewestChunkID()
{
    return serverNewestChunkID;
}

void Channel::SetServerEstimatedStreamRate(int serverEstimatedStreamRate)
{
    this->serverEstimatedStreamRate = serverEstimatedStreamRate;
}

int Channel::GetServerEstimatedStreamRate()
{
    return serverEstimatedStreamRate;
}

Peer* Channel::GetServer()
{
    return serverPeer;
}

Peer* Channel::GetPeer(Peer* peer)
{
    if (peerList.count(peer->GetID()) > 0)
        return peerList[peer->GetID()].GetPeer();
    return NULL;
}

bool Channel::HasPeer(Peer* peer)
{
    if (GetPeer(peer))
        return true;
    else
        return false;
}

void Channel::AddPeer(Peer* peer)
{
    peerList[peer->GetID()] = PeerData(peer);
}

void Channel::RemovePeer(Peer* peer)
{
    peerList.erase(peer->GetID());
}

void Channel::RemovePeer(string peerId)
{
    peerList.erase(peerId);
}

PeerData& Channel::GetPeerData(Peer* peer)
{
    return peerList[peer->GetID()];
}

time_t Channel::GetCreationTime()
{
    return creationTime;
}
vector<PeerData*> Channel::SelectPeerList(Strategy* strategy, Peer* srcPeer, unsigned int peerQuantity, bool virtualPeer,
		                                  uint8_t minimumBandwidth, uint8_t minimumBandwidth_FREE, bool separatedFreeOutList)
{
    vector<PeerData*> allPeers, selectedPeers;

    /* virtualPeer controla o envio de vizinhos de mesmo IP
     *
     */
    for (map<string, PeerData>::iterator i = peerList.begin(); i != peerList.end(); i++)
        if (srcPeer->GetID() != i->second.GetPeer()->GetID()){
        	if (!virtualPeer)
        		allPeers.push_back(&(i->second));
            else
            	if (srcPeer->GetIP() != i->second.GetPeer()->GetIP())
            		allPeers.push_back(&(i->second));
        }
    /* selectedAUX elimina vizinhos que não têm a banda de upload mínima  para envio
     *
     */
    vector<PeerData*> selectedPeersAUX;
    if (!(separatedFreeOutList) || (srcPeer->GetSizePeerListOutInformed() > 0))
    {
         for (uint16_t i = 0; i < allPeers.size(); i++) {
    	     if (allPeers[i]->GetSizePeerListOutInformed() >= minimumBandwidth) {
    		    selectedPeersAUX.push_back(allPeers[i]);
    	     }
          }
    }
    else{
          for (uint16_t i = 0; i < allPeers.size(); i++) {
             	if (allPeers[i]->GetSizePeerListOutInformed_FREE() >= minimumBandwidth_FREE) {
           		    selectedPeersAUX.push_back(allPeers[i]);
            	}
          }
    }

    allPeers = selectedPeersAUX;

    if (peerList.size() <= peerQuantity)
//        return allPeers;
    	selectedPeers = allPeers;
    else
    {
        strategy->Execute(&allPeers, srcPeer, peerQuantity);
        selectedPeers.insert(selectedPeers.begin(),allPeers.begin(),allPeers.begin()+peerQuantity);
        //return selectedPeers;
    }

  	cout<<"sending "<<selectedPeers.size()<<" peer to "<<srcPeer->GetID()<<"'s neighbor"<<endl;
  	return selectedPeers;
}

unsigned int Channel::GetPeerListSize()
{
    return peerList.size();
}

void Channel::CheckActivePeers()
{
    vector<string> deletedPeer;
    for (map<string,PeerData>::iterator i = peerList.begin(); i != peerList.end(); i++) 
    {
        i->second.DecTTLChannel();
        if (i->second.GetTTLChannel() <= 0)
            deletedPeer.push_back(i->first);
    }
    for (vector<string>::iterator peerId = deletedPeer.begin(); peerId < deletedPeer.end(); peerId++)
        RemovePeer(*peerId);

    this->printChannelProfile();
}

void Channel::PrintPeerList()
{
    cout<<"Channel ["<<channelId<<"] Tip["<<serverNewestChunkID<<"] Rate["<<serverEstimatedStreamRate<<"] Peer List:"<<endl;
    for (map<string,PeerData>::iterator i = peerList.begin(); i != peerList.end(); i++)
        cout<<"PeerID: "<<i->first<<" Mode: "<<(int)i->second.GetMode()<<" TTL: "<<i->second.GetTTLChannel()<<endl;
}

void Channel::printChannelProfile()
{
	cout<<"###-------"<<endl;
	cout<<"Channel ["<<channelId<<"] profile:"<<endl;
	cout<<"T Peers ["<<peerList.size()<<"]  ";
	cout<<"###-------"<<endl;
}



FILE* Channel::GetPerformanceFile()
{
    return performanceFile;
}

FILE* Channel::GetOverlayFile()
{
    return overlayFile;
}

