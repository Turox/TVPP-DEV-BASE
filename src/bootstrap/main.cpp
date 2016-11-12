#include <iostream>
#include <string.h>
#include <map>

#include <boost/thread.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/bind.hpp>

#include "../common/XPConfig.hpp"

#include "bootstrap.hpp"

#define TCPPORT "5111"    // the port users will be connecting to
#define UDPPORT "4950"    // the port users will be connecting to

#define SERVER_UDPPORT "4951" // the default port that server uses

using namespace std;

/** 
 * Função principal
 *  chama as threads necessárias para execução do programa
 */
int main(int argc, char* argv[]) {
    string myTCPPort = TCPPORT;
    string myUDPPort = UDPPORT;
    string peerlistSelectorStrategy = "";
    unsigned int peerListSharedSize = 20;

    string arg1 = "";
    if( argv[1] != NULL)
    	arg1 = argv[1];

    if(arg1 == "--help")
    {
        cout << "\nUsage: ./bootstrap [OPTIONS]" <<endl;
        cout <<"\nMain operation mode:"<<endl;
        cout <<"\n";
        cout <<"  -tcpPort                     define the tcp bootstrap port (default: "<<myTCPPort<<")"<<endl;
        cout <<"  -udpPort                     define the tcp bootstrap port (default: "<<myUDPPort<<")"<<endl;
        cout <<"  -peerlistSelectorStrategy    define the tcp bootstrap port (default: RandomStrategy)"<<endl;
        cout <<"  -peerListSharedSize          define the peer quantity to be shared each time between bootstrap and peer ()(default: "<<peerListSharedSize<<")"<<endl;

        cout <<endl;
        cout <<"  --isolaVirtutalPeerSameIP    permit only different IP partner "<<endl;
        exit(1);
    }

    XPConfig::Instance()->OpenConfigFile("");
    XPConfig::Instance()->SetBool("isolaVirtutalPeerSameIP", false);

    int optind=1;
    // decode arguments
    while ((optind < argc) && (argv[optind][0]=='-')) {
        string swtc = argv[optind];

        if (swtc=="-tcpPort") {
            optind++;
            myTCPPort = argv[optind];
        }
        else if (swtc=="-udpPort") {
            optind++;
            myUDPPort = argv[optind];
        }
        else if (swtc=="-peerlistSelectorStrategy")
        {
            optind++;
            peerlistSelectorStrategy = argv[optind];
        }
        else if (swtc=="--isolaVirtutalPeerSameIP")
        {
            XPConfig::Instance()->SetBool("isolaVirtutalPeerSameIP", true);
        }
        else if (swtc=="-peerListSharedSize")
        {
            optind++;
            peerListSharedSize = atoi(argv[optind]);
        }
        else
        {
            cout << "Invalid Arguments"<<endl; 
            exit(1);
        }
        optind++;
    }

    XPConfig::Instance()->OpenConfigFile("");
    Bootstrap bootstrapInstance(myUDPPort, peerlistSelectorStrategy, peerListSharedSize);
    
    boost::thread TTCPSERVER(boost::bind(&Bootstrap::TCPStart, &bootstrapInstance, myTCPPort.c_str()));
    boost::thread TUDPSERVER(boost::bind(&Bootstrap::UDPStart, &bootstrapInstance));
    boost::thread TUDPCONSUME(boost::bind(&Bootstrap::UDPReceive, &bootstrapInstance));
    boost::thread TVERIFICA(boost::bind(&Bootstrap::CheckPeerList, &bootstrapInstance));
    boost::thread TCONTAPEERS(boost::bind(&Bootstrap::HTTPLog, &bootstrapInstance));

    TTCPSERVER.join();
    //TUDPSERVER.join();
    //TUDPRECV.join();
    //TUDPCONSUME.join();
    //TVERIFICA.join();
    //TCONTAPEERS.join();
    return 0;

}
