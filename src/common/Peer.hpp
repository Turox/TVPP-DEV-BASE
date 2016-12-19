#ifndef PEER_H
#define PEER_H

#include <string>
#include <iostream>

#define IP_PORT_SEPARATOR ":"

using namespace std;

class Peer
{
    private:        
        string ID;
        string IP;
        string port;
        
        int sizePeerListOutInformed;        // ECM used to store the out quantity the peer can manage
        int sizePeerListOutInformed_FREE;   // ECM lista de pares pobres, quando existir

        int sizePeerListOutNew;             // ECM used to store the new out quantity the peer can manage
        int sizePeerListOutNew_FREE;        // ECM lista de pares pobres, quando existir

        int limitUpload;

        void ConstructorAux(string IP, string port, int sizePeerListOutInformed, int sizePeerListOutInformed_FREE, int limitUpload);
        void ResetID();
    public:
        Peer(string IP_port = "", int sizePeerListOutInformed = -1, int sizePeerListOutInformed_FREE = -1, int limitUpload = -1);
        Peer(string IP, string port, int sizePeerListOutInformed = -1, int sizePeerListOutInformed_FREE = -1, int limitUpload = -1);

        void SetID(string ID);
        void SetIP(string IP);
        void SetPort(string port);
        string GetID();
        string GetIP();
        string GetPort();

        int GetSizePeerListOutInformed();
        int GetSizePeerListOutInformed_FREE();

        int GetSizePeerListOutNew();
        int GetSizePeerListOutNew_FREE();

        int GetLimitUpload();
        void SetLimitUpload(int limitUpload);


        void SetSizePeerListOutInformed(int sizePeerListOutInformed);
        void SetSizePeerListOutInformed_FREE(int sizePeerListOutInformed_FREE);

        void SetSizePeerListOutNew(int sizePeerListOutNew);
        void SetSizePeerListOutNew_FREE(int sizePeerListOutNew_FREE);


        friend bool operator==(const Peer &a, const Peer &b) {return a.ID==b.ID;};
        friend bool operator!=(const Peer &a, const Peer &b) {return a.ID!=b.ID;};
        friend std::ostream& operator<<(std::ostream& os, const Peer& p) {os << p.ID; return os;};
};

#endif
