#include "Disconnector.hpp"

Disconnector::Disconnector(Strategy *disconnectorStrategy, PeerManager* peerManager, uint64_t timerPeriod, set<string>* peerActive) : Temporizable(timerPeriod)
{
	this->strategy = disconnectorStrategy;
	this->peerManager = peerManager;
	this->peerActive = peerActive;
	this->quantity = quantity;
}

void Disconnector::Disconnect()
{
	vector<PeerData*> peers;
	boost::mutex::scoped_lock peerListLock(*peerManager->GetPeerListMutex());
	for (set<string>::iterator i = peerActive->begin(); i != peerActive->end(); i++)
	{
		peers.push_back(peerManager->GetPeerData(*i));
	}

	/* Esse desconector, caso chamado, elimina um parceiro apenas
	 *   Para mudar isso: a) usar a quantidade, passada em parâmetro
	 *                    b) trocar o if por um for.*/

	strategy->Execute(&peers, NULL, 1);

	if (!peers.empty() && peers[0])
	{
		peerManager->DisconnectPeer(peers[0]->GetPeer()->GetID(),peerActive);
	}
	peerListLock.unlock();
}

void Disconnector::TimerAlarm(uint64_t timerPeriod, string timerName)
{
	Disconnect();
}
