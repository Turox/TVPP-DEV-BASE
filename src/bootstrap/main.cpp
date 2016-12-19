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

    // ECM ***********

    unsigned int peerListSharedSize = 20;
    uint8_t minimumBandwidth = 0;               // minimum bandwidth to share peer in peerListShare
    uint8_t minimumBandwidth_FREE = 0;          // only if --separatedFreeOutList
    uint16_t timeToSetNewOut = 0;


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
        cout <<"  -minimalOUTsend              define the minimum OUT to share a peer                     (defautl: "<<(int)minimumBandwidth<<")"<<endl;
        cout <<"  -minimalOUTFREEsend          define the minimum OUT_FRER to share a peer to Free Rider  (defautl: "<<(int)minimumBandwidth_FREE<<")"<<endl;
        cout <<"                               **(If chosen this automatically sets -separatedFreeOutList=true)"<<endl;
        cout <<"  -timeToSetNewOut             define bootstrap' s wait time, in seconds, before calculate new OUt and OUT-FREE(default: "<<timeToSetNewOut<<")"<<endl;
        cout <<"                                **(If chosen this automatically sets --dynamicTopologyArrangement)"<<endl;
        cout <<endl;
        cout <<"  --separatedFreeOutList       share peer per listOut or ListOut_FREE "<<endl;
        cout <<"  --isolaVirtutalPeerSameIP    permit only different IP partner "<<endl;
        cout <<"  --dynamicTopologyArrangement permit new OUT and OUT-FREE size from bootastrap"<<endl;
        exit(1);
    }


    XPConfig::Instance()->OpenConfigFile("");
    XPConfig::Instance()->SetBool("isolaVirtutalPeerSameIP", false);
    XPConfig::Instance()->SetBool("separatedFreeOutList",false);
    XPConfig::Instance()->SetBool("dynamicTopologyArrangement",false);

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
        else if (swtc=="-peerlistSelectorStrategy"){
            optind++;
            peerlistSelectorStrategy = argv[optind];
        }

        else if (swtc=="-peerListSharedSize") {
                 optind++;
                 peerListSharedSize = atoi(argv[optind]);
        }
        else if (swtc=="-minimalOUTsend") {
            optind++;
            minimumBandwidth = atoi(argv[optind]);
         }
        else if (swtc=="-minimalOUTFREEsend") {
            optind++;
            minimumBandwidth_FREE = atoi(argv[optind]);
            XPConfig::Instance()->SetBool("separatedFreeOutList",true);
         }
        else if (swtc=="-timeToSetNewOut") {
            optind++;
            timeToSetNewOut = atoi(argv[optind]);
            timeToSetNewOut = timeToSetNewOut / 10;
            XPConfig::Instance()->SetBool("dynamicTopologyArrangement",true);
         }

        else if (swtc=="--isolaVirtutalPeerSameIP")
        {
            XPConfig::Instance()->SetBool("isolaVirtutalPeerSameIP", true);
        }

        else if (swtc=="--dynamicTopologyArrangement")
        {
            XPConfig::Instance()->SetBool("dynamicTopologyArrangement", true);
        }

        else {
            cout << "Invalid Arguments"<<endl; 
            exit(1);
        }
        optind++;
    }

    XPConfig::Instance()->OpenConfigFile("");
    Bootstrap bootstrapInstance(myUDPPort, peerlistSelectorStrategy, peerListSharedSize, minimumBandwidth, minimumBandwidth_FREE, timeToSetNewOut);
    
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
