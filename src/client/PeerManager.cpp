/* Alterado: Eliseu César Miguel
 * 13/01/2015
 * Várias alterações para comportar a separação entre In e Out
 */

#include "PeerManager.hpp"

PeerManager::PeerManager()
{
}

unsigned int PeerManager::GetMaxActivePeers(set<string>* peerActive)
{
	if (peerActive == &peerActiveIn) return maxActivePeersIn;
	if (peerActive == &peerActiveOut) return maxActivePeersOut;
	if (peerActive == &peerActiveOutFREE) return maxActivePeersOutFREE;
	return 0;
}

void PeerManager::SetMaxActivePeersIn(unsigned int maxActivePeers) {this->maxActivePeersIn = maxActivePeers;}
void PeerManager::SetMaxActivePeersOut(unsigned int maxActivePeers) {this->maxActivePeersOut = maxActivePeers;}
void PeerManager::SetMaxActivePeersOutFREE(unsigned int maxActivePeers){this->maxActivePeersOutFREE = maxActivePeers;}

bool PeerManager::AddPeer(Peer* newPeer, int sizePeerListOut, int sizePeerListOut_FREE)
{
	boost::mutex::scoped_lock peerListLock(peerListMutex);
	if (peerList.find(newPeer->GetID()) == peerList.end())
	{
		peerList[newPeer->GetID()] = PeerData(newPeer);
		peerList[newPeer->GetID()].SetSizePeerListOutInformed(sizePeerListOut);
		peerList[newPeer->GetID()].SetSizePeerListOutInformed_FREE(sizePeerListOut_FREE);
		peerListLock.unlock();
		cout<<"Peer "<<newPeer->GetID()<<" added to PeerList"<<endl;
		return true;
	}
	//atualiza sizePeerListOut caso peer já esteja na lista
	if (sizePeerListOut >= 0)
		peerList[newPeer->GetID()].SetSizePeerListOutInformed(sizePeerListOut);
	    peerList[newPeer->GetID()].SetSizePeerListOutInformed_FREE(sizePeerListOut_FREE);
	peerListLock.unlock();
	return false;
}

void PeerManager::SetMaxOutFreeToBeSeparated(unsigned int outLimitToSeparateFree){this->outLimitToSeparateFree = outLimitToSeparateFree;}

set<string>* PeerManager::GetPeerActiveIn(){return &peerActiveIn;}
set<string>* PeerManager::GetPeerActiveOut(bool separatedFree, uint16_t peerOut){

	if ((separatedFree) && (this->outLimitToSeparateFree >= peerOut))
		return  &peerActiveOutFREE;
	return &peerActiveOut;
}

map<string, unsigned int>* PeerManager::GetPeerActiveCooldown(set<string>* peerActive)
{
	if (peerActive == &peerActiveIn) return &peerActiveCooldownIn;
	if (peerActive == &peerActiveOut) return &peerActiveCooldownOut;
	if (peerActive == &peerActiveOutFREE) return &peerActiveCooldownOutFREE;
	return NULL;
}

//ECM - efetivamente, insere o par em uma das lista In ou Out
bool PeerManager::ConnectPeer(string peer, set<string>* peerActive)
{
	boost::mutex* peerActiveMutex = this->GetPeerActiveMutex(peerActive);
	map<string, unsigned int>* peerActiveCooldown = this->GetPeerActiveCooldown(peerActive);
	if (peerActiveCooldown->find(peer) == (*peerActiveCooldown).end())
	{
		boost::mutex::scoped_lock peerActiveLock(*peerActiveMutex);

		bool inserted = false;
		if (peerActive->size() < this->GetMaxActivePeers(peerActive))
			inserted = peerActive->insert(peer).second;
		else if ((peerActive != &peerActiveIn) && (this->removeWorsePartner))
            inserted = this->ConnectSpecial(peer,peerActive);

		if (inserted)
		{
	    	string list;
		    if (*(peerActive) == peerActiveIn){
			   this->peerList[peer].SetTTLIn(TTLIn);
			   list = "In";
		    }
		    else{
		    	this->peerList[peer].SetTTLOut(TTLOut);
		    	if (*(peerActive) == peerActiveOut){list = "Out";}
    		    else {list = "Out-FREE";}
		    }
		}
		peerActiveLock.unlock();
		return inserted;
	}
	return false;
}


bool PeerManager::ConnectSpecial(string peer, set<string>* peerActive){

	if (peerActive->size() == 0 ) return false;

    set<string>::iterator smaller;
    smaller = peerActive->begin();

    if (*(peerActive) == peerActiveOutFREE)
    {
       	for (set<string>::iterator count = peerActive->begin(); count != peerActive->end(); count++)
	    	if (peerList[*count].GetSizePeerListOutInformed_FREE() < peerList[*smaller].GetSizePeerListOutInformed_FREE())
		    	smaller = count;
	    if (peerList[*smaller].GetSizePeerListOutInformed_FREE() < peerList[peer].GetSizePeerListOutInformed_FREE())
	    {
		string text="Out-FREE";
		cout <<"removing "<<*smaller<<" "<<text<<" list["<<peerList[*smaller].GetSizePeerListOutInformed_FREE()<<"] to insert "<<peer<<" out list["<<peerList[peer].GetSizePeerListOutInformed_FREE()<<"]"<<endl;
		peerActive->erase(smaller);
		peerActive->insert(peer);
		this->SetRemoveWorsePartner(false);
		return true;
	    }

	}
    else
    {
       	for (set<string>::iterator count = peerActive->begin(); count != peerActive->end(); count++)
	    	if (peerList[*count].GetSizePeerListOutInformed() < peerList[*smaller].GetSizePeerListOutInformed())
		    	smaller = count;
	    if (peerList[*smaller].GetSizePeerListOutInformed() < peerList[peer].GetSizePeerListOutInformed())
	    {
		string text="Out";
		cout <<"removing "<<*smaller<<" "<<text<<" list["<<peerList[*smaller].GetSizePeerListOutInformed()<<"] to insert "<<peer<<" out list["<<peerList[peer].GetSizePeerListOutInformed()<<"]"<<endl;
		peerActive->erase(smaller);
		peerActive->insert(peer);
		this->SetRemoveWorsePartner(false);
		return true;
	    }
    }

	return false;
}

void PeerManager::DisconnectPeer(string peer, set<string>* peerActive)
{
	boost::mutex* peerActiveMutex = this->GetPeerActiveMutex(peerActive);
	map<string, unsigned int>* peerActiveCooldown = this->GetPeerActiveCooldown(peerActive);
	boost::mutex::scoped_lock peerActiveLock(*peerActiveMutex);
	peerActive->erase(peer);
	peerActiveLock.unlock();
	(*peerActiveCooldown)[peer] = PEER_ACTIVE_COOLDOWN;
	string list;
	if (*(peerActive) == peerActiveIn)	list = "In";
	else{
		if (*(peerActive) == peerActiveOut)	list = "Out";
		else list = "Out-Free";
	}
    cout<<"Peer "<<peer<<" disconnected from PeerActive_"<<list<<endl;
}

void PeerManager::RemovePeer(string peer)
{
	boost::mutex::scoped_lock peerListLock(peerListMutex);
	cout<<"Peer "<<peer<<" removed from PeerList: TTLIn["<<peerList[peer].GetTTLIn()<<"] TTLOut["<<peerList[peer].GetTTLOut()<<"]"<<endl;
	peerList.erase(peer);
	peerListLock.unlock();
}

unsigned int PeerManager::GetPeerActiveSize(set<string>* peerActive)
{
	boost::mutex* peerActiveMutex = this->GetPeerActiveMutex(peerActive);
	boost::mutex::scoped_lock peerActiveLock(*peerActiveMutex);
	unsigned int size = peerActive->size();
	peerActiveLock.unlock();
	return size;
}

unsigned int PeerManager::GetPeerActiveSize_OPENED(set<string>* peerActive)
{
	return  peerActive->size();
}

// Gera o total de parceiros somando In e Out sem repeticoes

unsigned int PeerManager::GetPeerActiveSizeTotal()
{
	unsigned int size = this->GetPeerActiveSize(&peerActiveIn);
	boost::mutex::scoped_lock peerActiveInLock(peerActiveMutexIn);
	boost::mutex::scoped_lock peerActiveOutLock(peerActiveMutexOut);

	for (set<string>::iterator i = peerActiveOut.begin(); i != peerActiveOut.end(); i++)
	{
		if (peerActiveIn.find(*i) == peerActiveIn.end())
			size++;
	}
	peerActiveOutLock.unlock();
	boost::mutex::scoped_lock peerActiveOutFREELock(peerActiveMutexOutFREE);

	for (set<string>::iterator i = peerActiveOutFREE.begin(); i != peerActiveOutFREE.end(); i++)
	{
		if (peerActiveIn.find(*i) == peerActiveIn.end())
			size++;
	}

	peerActiveInLock.unlock();
	peerActiveOutFREELock.unlock();
    return size;
}

bool PeerManager::IsPeerActive(string peer,set<string>* peerActive)
{
	boost::mutex* peerActiveMutex = this->GetPeerActiveMutex(peerActive);
	boost::mutex::scoped_lock peerActiveLock(*peerActiveMutex);
	if (peerActive->find(peer) != peerActive->end())
	{
		peerActiveLock.unlock();
		return true;
	}
	peerActiveLock.unlock();
	return false;
}


PeerData* PeerManager::GetPeerData(string peer){return &peerList[peer];}
map<string, PeerData>* PeerManager::GetPeerList(){return &peerList;}
boost::mutex* PeerManager::GetPeerListMutex(){return &peerListMutex;}


boost::mutex* PeerManager::GetPeerActiveMutex(set<string>* peerActive)
{
	if (peerActive == &peerActiveIn) return &peerActiveMutexIn;
	if (peerActive == &peerActiveOut) return &peerActiveMutexOut;
	if (peerActive == &peerActiveOutFREE) return &peerActiveMutexOutFREE;
	return NULL;
}

//ECM metodo privado criado para ser chamado duas vezes (In e Out) em CheckPeerList()
void PeerManager::CheckpeerActiveCooldown(map<string, unsigned int>* peerActiveCooldown)
{
	set<string> deletedPeer;
	for (map<string, unsigned int>::iterator i = peerActiveCooldown->begin(); i != peerActiveCooldown->end(); i++)
	   {
		i->second--;
		if (i->second == 0)
			deletedPeer.insert(i->first);
	}
	for (set<string>::iterator i = deletedPeer.begin(); i != deletedPeer.end(); i++) { peerActiveCooldown->erase(*i);}
	deletedPeer.clear();
}

void PeerManager::CheckPeerList()
{
	//ECM Tabela de decisao para remover peer.

	//|  ttlOut      PeerActiveOUtFREE                                Desconectar OutFREE                   caso
	//|      0           pertence                                             x                               0
	//|----------------------------------------------------------------------------------------------------------|
	//| ttlIn ttlOut PeerActiveIn    PeerActiveOut |  Desconectar In | Desconectar Out | Remover PeerList | caso |
	//|----------------------------------------------------------------------------------------------------------|
	//|   0    <>0     pertence        pertence    |       X         |                 |                  |   1  |
	//|  <>0    0      pertence        pertence    |                 |        X        |                  |   2  |
	//|   0     0      pertence        pertence    |       X         |        X        |     X            |   3  |
	//|   0    <>0     pertence      nao pertence  |       X         |                 |     X            |   4  |
	//|  <>0    0    nao pertence      pertence    |                 |        X        |     X            |   5  |
    //|----------------------------------------------------------------------------------------------------------|

    set<string> desconectaPeerIn;  //DesconectarIn
    set<string> desconectaPeerOut; //DesconectarOut
    set<string> desconectaPeerOutFREE; //DesconectarOutFREE
    set<string> deletaPeer;        //Remover

    bool isPeerActiveIn = false;
    bool isPeerActiveOut  = false;
    bool isPeerActiveOutFREE  = false;

    boost::mutex::scoped_lock peerActiveInLock(peerActiveMutexIn);
    boost::mutex::scoped_lock peerActiveOUTLock(peerActiveMutexOut);
    boost::mutex::scoped_lock peerActiveOUTFREELock(peerActiveMutexOutFREE);

    //gera lista com todos os pares ativos
    set<string> temp_allActivePeer (peerActiveIn);
    for (set<string>::iterator i = peerActiveOut.begin(); i != peerActiveOut.end(); i++)
    	temp_allActivePeer.insert(*i);
    for (set<string>::iterator i = peerActiveOutFREE.begin(); i != peerActiveOutFREE.end(); i++)
    	temp_allActivePeer.insert(*i);


    for (set<string>::iterator i = temp_allActivePeer.begin(); i != temp_allActivePeer.end(); i++)
    {
    	isPeerActiveIn = peerActiveIn.find(*i) != peerActiveIn.end();
    	if (isPeerActiveIn)
    	{
    		peerList[*i].DecTTLIn();
    		if (peerList[*i].GetTTLIn() <= 0)
    		{
    			desconectaPeerIn.insert(*i);
    			isPeerActiveIn = false;
    		}
    	}
    	isPeerActiveOut = peerActiveOut.find(*i) != peerActiveOut.end();
    	if (isPeerActiveOut)
    	{
    		peerList[*i].DecTTLOut();
    		if (peerList[*i].GetTTLOut() <= 0)
    	    {
    			desconectaPeerOut.insert(*i);
    			isPeerActiveOut = false;
    		}

    	}
    	isPeerActiveOutFREE = peerActiveOutFREE.find(*i) != peerActiveOutFREE.end();
    	    	if (isPeerActiveOutFREE)
    	    	{
    	    		peerList[*i].DecTTLOut();
    	    		if (peerList[*i].GetTTLOut() <= 0)
    	    	    {
    	    			desconectaPeerOutFREE.insert(*i);
    	    			isPeerActiveOutFREE = false;
    	    		}

    	    	}

    	if ((!isPeerActiveIn) && (!isPeerActiveOut) && (!isPeerActiveOutFREE))
    		deletaPeer.insert(*i);
    }
    peerActiveInLock.unlock();
	peerActiveOUTLock.unlock();
	peerActiveOUTFREELock.unlock();

    for (set<string>::iterator i = desconectaPeerIn.begin(); i != desconectaPeerIn.end(); i++)
    {
    	DisconnectPeer(*i, &peerActiveIn);
    }
    for (set<string>::iterator i = desconectaPeerOut.begin(); i != desconectaPeerOut.end(); i++)
    {
      	DisconnectPeer(*i, &peerActiveOut);
    }
    for (set<string>::iterator i = desconectaPeerOutFREE.begin(); i != desconectaPeerOutFREE.end(); i++)
    {
     	DisconnectPeer(*i, &peerActiveOutFREE);
    }
    for (set<string>::iterator i = deletaPeer.begin(); i != deletaPeer.end(); i++)
    {
        RemovePeer(*i);
    }

    this->CheckpeerActiveCooldown(&peerActiveCooldownIn);
    this->CheckpeerActiveCooldown(&peerActiveCooldownOut);
    this->CheckpeerActiveCooldown(&peerActiveCooldownOutFREE);
}

//funcao auxiliar usada interna em ShowPeerList para impressao
int PeerManager::showPeerActive(set<string>* peerActive)
{
	boost::mutex* peerActiveMutex = this->GetPeerActiveMutex(peerActive);
	int j = 0; int ttl = 0; string ttlRotulo("");
	boost::mutex::scoped_lock peerActiveLock(*peerActiveMutex);

    for (set<string>::iterator i = (*peerActive).begin(); i != (*peerActive).end(); i++, j++)
	{
    	if (peerActive == &peerActiveIn)
        {
        	ttl = peerList[*i].GetTTLIn();
        	ttlRotulo == "TTLIn";
        }
        else
        {
        	ttl = peerList[*i].GetTTLOut();
        	ttlRotulo == "TTLOut";
        }

	    cout<<"Key: "<<*i<<" ID: "<<peerList[*i].GetPeer()->GetID()<<" Mode: "<<(int)peerList[*i].GetMode()<<ttlRotulo<<": "<<ttl << " PR: "<<peerList[*i].GetPendingRequests() << endl;
	}
	peerActiveLock.unlock();
	return j;

}

void PeerManager::ShowPeerList()
{
 	int k = 0;
    int j = 0;
    int m = 0;
    cout<<endl<<"- Peer List Active -"<<endl;
    k = showPeerActive(&peerActiveIn);
    j = showPeerActive(&peerActiveOut);
    m = showPeerActive(&peerActiveOutFREE);

    cout<<"Total In ["<<k<<"]  Total Out ["<<j<<"] Total Out-FREE ["<<m<<"]"<<endl;
    cout<<"Total different Peers Active: ["<<this->GetPeerActiveSizeTotal()<<"] "<<endl<<endl;
	j = 0;
	cout<<endl<<"- Peer List Total -"<<endl;
	boost::mutex::scoped_lock peerListLock(peerListMutex);

    for (map<string, PeerData>::iterator i = peerList.begin(); i != peerList.end(); i++, j++)
	{
		cout<<"Key: "<<i->first<< endl;
		cout<<"ID: "<<i->second.GetPeer()->GetID()<<" Mode: "<<(int)i->second.GetMode()<<" TTLIn: "<<i->second.GetTTLIn() <<" TTLOut: "<<i->second.GetTTLOut() << " RTT(delay): " <<i->second.GetDelay()<< "s PR: "<<i->second.GetPendingRequests() << endl;
	}
	peerListLock.unlock();
	cout<<"Total PeerList: "<<j<<endl<<endl;

}
void PeerManager::SetRemoveWorsePartner (bool removeWorsePartner)
{
	this->removeWorsePartner = removeWorsePartner;
}

bool PeerManager::GetRemoveWorsePartner (){
	return this->removeWorsePartner;
}


