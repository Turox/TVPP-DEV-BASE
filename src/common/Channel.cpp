#include "Channel.hpp"

static bool pairCompare(PairStrInt a, PairStrInt b){ return (a.second > b.second);}
static void sortPairStrIntVec(std::vector<PairStrInt>& vec){std::sort(vec.begin(), vec.end(), pairCompare);}

static bool pairCompareTopology(PairTopologyInt a, PairTopologyInt b){ return (a.second > b.second);}
static void sortPairTopologyClassesIntVec(std::vector<PairTopologyInt>& vec){std::sort(vec.begin(), vec.end(), pairCompareTopology);}


Channel::Channel(unsigned int channelId, Peer* serverPeer, bool dynamicTopologyArrangement)
{
    if (channelId != 0 || serverPeer != NULL) //Avoid creation by map[]
    {
        this->channelId = channelId;
        this->serverPeer = serverPeer;
        if(serverPeer)
            AddPeer(serverPeer);
        serverEstimatedStreamRate = 0;

        /*
         * ler de arquivo de configuração
         */
        this->firstTimeOverlay = true;
        this->indicateClassPosition = false;
        this->dynamicTopologyArrangement = dynamicTopologyArrangement;
        if (dynamicTopologyArrangement){

            TopologyData* NewClass;

            NewClass = new TopologyData(0, 20, 0, 0, 0);          // 0.0Mb/s
			classTopologySettings[classA] =(*NewClass);

            NewClass = new TopologyData(131072, 20, 1, 38, 40);   // [1.0Mb/s, 2.0Mb/s)
			classTopologySettings[classB] = (*NewClass);

            NewClass = new TopologyData(262144, 20, 18, 22, 30);  // [2.0Mb/s, 3.5Mb/s)
			classTopologySettings[classC] = (*NewClass);

            NewClass = new TopologyData(458752, 20, 46, 0, 15);   // >= 3.5Mb/s
			classTopologySettings[classD] = (*NewClass);
        }

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
int Channel::NormalizeClasses(TopologyClasses classRef, vector<PairStrInt>* ordinaryNodeVector, int start, int stop){
    int contador = 0;
	for (vector<PairStrInt>::iterator common = ordinaryNodeVector->begin() + start; common != ordinaryNodeVector->begin() + stop ; common++)
	{
		peerList[common->first].SetSizePeerListOutOld(peerList[common->first].GetSizePeerListOutInformed());
		peerList[common->first].SetSizePeerListOutInformed(classTopologySettings[classRef].GetOut());

		peerList[common->first].SetSizePeerListOutOld_FREE(peerList[common->first].GetSizePeerListOutInformed_FREE());
		peerList[common->first].SetSizePeerListOutInformed_FREE(classTopologySettings[classRef].GetOut_free());
		contador++;
	}
	return contador;
}

void Channel::RenewOUTALL()
{
	if (this->firstTimeOverlay)
	{
	    for (map<string, PeerData>::iterator i = peerList.begin(); i != peerList.end(); i++)
	        i->second.Set_peerSentChunks(0);
	    firstTimeOverlay = false;
	    return;
	}
	else
	{
		std::vector<std::string> freeRiderVector;
		std::vector<PairStrInt> ordinaryNodeVector;

		this->GetPeerData(this->GetServer()).Set_peerSentChunks(0);
		for (map<string, PeerData>::iterator i = peerList.begin(); i != peerList.end(); i++)
		{
			if ((i->second.GetSizePeerListOutInformed() > 0) or
					(i->second.GetSizePeerListOutInformed_FREE() > 0))
					{
    				if ((i->second.GetHit_count() == 0) and (i->first != this->GetServer()->GetID()))
    				{
    					ordinaryNodeVector.push_back(i->second.GetPairStrInt());
    					i->second.Set_peerSentChunks(0);
    				}
			}
			else
				freeRiderVector.push_back(i->first);
		}

		sortPairStrIntVec(ordinaryNodeVector);        //Ordem decrescente de contribuição
		int commonSize = ordinaryNodeVector.size();

		/****** CKmeans application *****/
		int posClassD = commonSize * classTopologySettings[classD].GetPercentOfPeer()/100;
		int posClassB = commonSize * (100 - classTopologySettings[classC].GetPercentOfPeer() )/100;

		cout<<"normalização. Vetor Size: "<<commonSize<<" ClasseD index: "<<posClassD<<" ClassB index: "<<posClassB<<endl;

		int count_peerClassD = this->NormalizeClasses(classD, &ordinaryNodeVector, 0        , posClassD);
		int count_peerClassC = this->NormalizeClasses(classC, &ordinaryNodeVector, posClassD, posClassB);
		int count_peerClassB = this->NormalizeClasses(classB, &ordinaryNodeVector, posClassB, commonSize);

		cout<<"Normalized Classes ["<<count_peerClassD<<", "<<count_peerClassC<<", "<<count_peerClassB<<", "<<freeRiderVector.size()<<"]"<<endl;

        /*
		int totalIn_Free = 0, totalOut_Free = 0, totalIn_Common = 0, totalOut_Common = 0;
		totalIn_Free = freeRiderVector.size() * classTopologySettings["classA"].GetIn();
		totalIn_Common = count_peerClassD * classTopologySettings["classD"].GetIn() +
			         count_peerClassC * classTopologySettings["classC"].GetIn() +
			         count_peerClassB * classTopologySettings["classB"].GetIn();

		for (vector<PairStrInt>::iterator common = ordinaryNodeVector.begin(); common != ordinaryNodeVector.end(); common++){
			totalOut_Common = totalOut_Common  + peerList[common->first].GetSizePeerListOutInformed();
			totalOut_Free   = totalOut_Free    + peerList[common->first].GetSizePeerListOutInformed_FREE();	}
		*/
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

void Channel::AddPeer(Peer* peer, uint16_t hit_count)
{
    peerList[peer->GetID()] = PeerData(peer);
    peerList[peer->GetID()].SetHit_count(hit_count);

    if ((this->dynamicTopologyArrangement) and
    		(peer->GetID() != this->GetServer()->GetID()) and            // non server
    	    (peerList[peer->GetID()].GetLimitUpload() > 0))              // common peer
    	{
    	if (this->indicateClassPosition)                                 // indicator selected
    	{
    		TopologyClasses classPeer = this->SugestedClass(peerList[peer->GetID()].GetLimitUpload());
    		if (classPeer != error) {
    			peerList[peer->GetID()].SetSizePeerListOutInformed(this->classTopologySettings[classPeer].GetOut());
    			peerList[peer->GetID()].SetSizePeerListOutInformed_FREE(this->classTopologySettings[classPeer].GetOut_free());
    			peerList[peer->GetID()].SetSizePeerListOutOld(this->classTopologySettings[classPeer].GetOut());
    			peerList[peer->GetID()].SetSizePeerListOutOld_FREE(this->classTopologySettings[classPeer].GetOut_free());
    		}

    	}
    	else{
    		TopologyClasses classPeer = classC;
        	peerList[peer->GetID()].SetSizePeerListOutInformed(this->classTopologySettings[classPeer].GetOut());
        	peerList[peer->GetID()].SetSizePeerListOutInformed_FREE(this->classTopologySettings[classPeer].GetOut_free());
        	peerList[peer->GetID()].SetSizePeerListOutOld(this->classTopologySettings[classPeer].GetOut());
        	peerList[peer->GetID()].SetSizePeerListOutOld_FREE(this->classTopologySettings[classPeer].GetOut_free());

    	}

    }

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
    	selectedPeers = allPeers;
    else
    {
        strategy->Execute(&allPeers, srcPeer, peerQuantity);
        selectedPeers.insert(selectedPeers.begin(),allPeers.begin(),allPeers.begin()+peerQuantity);
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

void Channel::SetIndicateClassPosition(bool indicateClassPosition){
	this->indicateClassPosition = indicateClassPosition;
}

TopologyClasses Channel::SugestedClass(int bandwidth){
	if (classTopologySettings.size() <= 0) {cout<<"Class peer suggest error"<<endl; return error;}
	vector<PairTopologyInt> vectorClasses;
	for (map<TopologyClasses,TopologyData>::iterator i = classTopologySettings.end(); i != classTopologySettings.begin(); i--){
	   vectorClasses.push_back(PairTopologyInt(i->first,i->second.GetJoinMinimunBandwidth()));
	}
	sortPairTopologyClassesIntVec(vectorClasses);
    for (vector<PairTopologyInt>::iterator j = vectorClasses.begin(); j != vectorClasses.end(); j++){
  	  if ((int)j->second <= bandwidth)
 		  return j->first;
    }
	return error;
}



















