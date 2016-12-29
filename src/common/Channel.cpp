#include "Channel.hpp"

static bool pairCompare(PairStrInt a, PairStrInt b){ return (a.second > b.second);}
static void sortPairStrIntVec(std::vector<PairStrInt>& vec){std::sort(vec.begin(), vec.end(), pairCompare);}

static bool pairCompareTopology(PairTopologyInt a, PairTopologyInt b){ return (a.second > b.second);}
static void sortPairTopologyClassesIntVec(std::vector<PairTopologyInt>& vec){std::sort(vec.begin(), vec.end(), pairCompareTopology);}


Channel::Channel(unsigned int channelId, Peer* serverPeer, bool dynamicTopologyArrangement, uint8_t peerPercentChangeAlowed,
		int bandwidthJoinClassB, int bandwidthJoinClassC, int bandwidthJoinClassD)
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
        this->bandwidthJoinClassB=bandwidthJoinClassB * 65536;
        this->bandwidthJoinClassC=bandwidthJoinClassC * 65536;
        this->bandwidthJoinClassD=bandwidthJoinClassD * 65536;

        this->peerPercentChangeAlowed = peerPercentChangeAlowed;
        this->firstTimeOverlay = true;
        this->indicateClassPosition = false;
        this->dynamicTopologyArrangement = dynamicTopologyArrangement;
        if (dynamicTopologyArrangement){

            TopologyData* NewClass;

            NewClass = new TopologyData(0, 20, 0, 0, 0);          // 0.0Mb/s
			classTopologySettings[classA] =(*NewClass);

            NewClass = new TopologyData(this->bandwidthJoinClassB, 20, 1, 38, 40);
			classTopologySettings[classB] = (*NewClass);

            NewClass = new TopologyData(this->bandwidthJoinClassC, 20, 18, 22, 45);
			classTopologySettings[classC] = (*NewClass);

            NewClass = new TopologyData(this->bandwidthJoinClassD, 20, 46, 0, 15);
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


void Channel::ConvertePeerClasse(TopologyClasses newClass, vector<PairStrInt>::iterator pos, vector<PairStrInt>* ordinaryNodeVector){
	peerList[pos->first].SetSizePeerListOutOld(peerList[pos->first].GetSizePeerListOutInformed());
	peerList[pos->first].SetSizePeerListOutInformed(classTopologySettings[newClass].GetOut());

	peerList[pos->first].SetSizePeerListOutOld_FREE(peerList[pos->first].GetSizePeerListOutInformed_FREE());
	peerList[pos->first].SetSizePeerListOutInformed_FREE(classTopologySettings[newClass].GetOut_free());
}

/***************************************************************************************************************************
 * SE FOR DA CLASSE REFERENCIA NÃO FAZ NADA
 * SE FOR DIFERNTE (MAIOR OU MENOR)
 * 		SE FOR DA CLASSE C -> PASSA PARA A CLASSE REFERNCIA
 * 		SENÃO, PASSA PARA A CLASSE C
 */

int Channel::NormalizeClasses(TopologyClasses classBase, TopologyClasses classRef, vector<PairStrInt>* ordinaryNodeVector, int start, int stop, uint8_t percent){

	int contador = 0;
	vector<PairStrInt>::iterator common;
	int totalAltera = (ordinaryNodeVector->size())*percent/100;

	if (classBase == classRef){
		for (common = ordinaryNodeVector->begin() + start; common != ordinaryNodeVector->begin() + stop ; common++)
			if (peerList[common->first].GetSizePeerListOutInformed() != classTopologySettings[classBase].GetOut()){
				this->ConvertePeerClasse(classBase, common, ordinaryNodeVector);
				contador = contador + 1;
			}
	}
	else
	{
		if (classTopologySettings[classRef].GetOut() > classTopologySettings[classBase].GetOut()){
			for (common = ordinaryNodeVector->begin() + start; common != ordinaryNodeVector->begin() + stop ; common++)
			{
				if (contador < totalAltera){
					if (peerList[common->first].GetSizePeerListOutInformed() != classTopologySettings[classRef].GetOut()){
						if (peerList[common->first].GetSizePeerListOutInformed() == classTopologySettings[classBase].GetOut())
							this->ConvertePeerClasse(classRef, common, ordinaryNodeVector);   // vem para a classe REf
						else
							this->ConvertePeerClasse(classBase, common, ordinaryNodeVector);	// vai para a classe Base
						contador = contador + 1;
					}
				}
				else break;
			}
		}
		else
		{
			for (common = ordinaryNodeVector->begin() + (stop - 1) ; common != ordinaryNodeVector->begin() + (start +1) ; common--)
				{
					if (contador < totalAltera){
						if (peerList[common->first].GetSizePeerListOutInformed() != classTopologySettings[classRef].GetOut()){
							if (peerList[common->first].GetSizePeerListOutInformed() == classTopologySettings[classBase].GetOut())
								this->ConvertePeerClasse(classRef, common, ordinaryNodeVector);   // vem para a classe REf
							else
								this->ConvertePeerClasse(classBase, common, ordinaryNodeVector);	// vai para a classe Base
							contador = contador + 1;
						}
					}
					else break;
				}
		}

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

		int posClassD =             commonSize * classTopologySettings[classD].GetPercentOfPeer()/100;
		int posClassC = posClassD + commonSize * classTopologySettings[classC].GetPercentOfPeer()/100;
		int posClassB = posClassC + commonSize * classTopologySettings[classB].GetPercentOfPeer()/100;

		cout<<"Vector Size: "<<commonSize<<" Classes index: ["<<posClassD<<","<<posClassC<<","<<posClassB<<"]"<<endl;

		int count_peerClassD = this->NormalizeClasses(classC, classD, &ordinaryNodeVector, 0        , posClassD,this->peerPercentChangeAlowed);
		int count_peerClassC = this->NormalizeClasses(classC, classC, &ordinaryNodeVector, posClassD, posClassC,this->peerPercentChangeAlowed);
		int count_peerClassB = this->NormalizeClasses(classC, classB, &ordinaryNodeVector, posClassC, commonSize,this->peerPercentChangeAlowed);

		cout<<"Normalized Classes ["<<count_peerClassD<<", "<<count_peerClassC<<", "<<count_peerClassB<<", "<<freeRiderVector.size()<<"]"<<endl;
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
    		(peer->GetID() != this->GetServer()->GetID()) and              // non server
    	    (peerList[peer->GetID()].GetLimitUpload() > 0) and             // common peer
			peerList[peer->GetID()].GetSizePeerListOutInformed() == 0)     // primeira conexão
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



















