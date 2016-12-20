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
    uint8_t minimumBandwidth = 0;               // minimum bandwidth to share peer in peerListShare
    uint8_t minimumBandwidth_FREE = 0;          // only if --separatedFreeOutList
    uint16_t timeToSetNewOut = 0;
    uint16_t timeNewOutDelayStarts = 60/10;         // 60 segundos

    //for channel
    uint8_t inCommon = 10;
    uint8_t inFree = 10;
    uint8_t percentPeersInClass = 20;
    uint8_t classAmount = 3 ;

    string arg1 = "";
    if( argv[1] != NULL)
    	arg1 = argv[1];

    if(arg1 == "--help")
    {
        cout << "\nUsage: ./bootstrap [OPTIONS]" <<endl;
        cout <<"\nMain operation mode:"<<endl;
        cout <<"\n";
        cout <<"  -tcpPort                     tcp bootstrap port                              (default: "<<myTCPPort<<")"<<endl;
        cout <<"  -udpPort                     tcp bootstrap port                              (default: "<<myUDPPort<<")"<<endl;
        cout <<"  -peerlistSelectorStrategy    tcp bootstrap port                              (default: RandomStrategy)"<<endl;
        cout <<"  -peerListSharedSize          peer quantity to be shared                      (default: "<<peerListSharedSize<<")"<<endl;
        cout <<"  -minimalOUTsend              minimum OUT to share a peer                     (defautl: "<<(int)minimumBandwidth<<")"<<endl;
        cout <<"  -minimalOUTFREEsend          minimum OUT_FRER to share a peer to Free Rider  (defautl: "<<(int)minimumBandwidth_FREE<<")"<<endl;
        cout <<"                               **(If chosen this automatically sets -separatedFreeOutList=true)"<<endl;
        cout <<endl;
        cout <<"   FOR CHANNEL FREE RIDER SLICE TECHNIQUE"                                <<endl;
        cout <<endl;
        cout <<"  -inCommon                    common peers'  IN configuration                 (defautl:"<<inCommon<<")"<<endl;
        cout <<"  -inFree                      free rider peers'  IN configuration             (defautl:"<<inFree<<")"<<endl;
        cout <<"  -percentPeersInClass         minimum peers' percentage in each class         (defautl:"<<percentPeersInClass<<"%)"<<endl;
        cout <<"  -classAmount                 number of class peers' contribution             (defautl:"<<classAmount<<")"<<endl;
        cout <<"  -timeToSetNewOut             seconds before calculate news OUT e OUT-FREE    (default: "<<timeToSetNewOut<<")"<<endl;
        cout <<"                                **(automatically sets --dynamicTopologyArrangement)"<<endl;
        cout <<"  -timeNewOutDelayStarts       network time wait before starts timeToSetNewOut (defautl:"<<timeNewOutDelayStarts<<")"<<endl;
        cout <<"  --dynamicTopologyArrangement enable free rider slice technique         "<<endl;
        cout <<endl;
        cout <<"  --separatedFreeOutList       share peer per listOut or ListOut_FREE    "<<endl;
        cout <<"  --isolaVirtutalPeerSameIP    permit only different IP partner          "<<endl;

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
        else if (swtc=="-timeNewOutDelayStarts") {
            optind++;
            timeNewOutDelayStarts = atoi(argv[optind]);
            timeNewOutDelayStarts = timeNewOutDelayStarts / 10;
         }
        else if (swtc=="-inCommon") {
            optind++;
            inCommon = atoi(argv[optind]);
         }
        else if (swtc=="-inFree") {
            optind++;
            inFree = atoi(argv[optind]);
         }
        else if (swtc=="-percentPeersInClass") {
            optind++;
            percentPeersInClass = atoi(argv[optind]);
         }
        else if (swtc=="-classAmount") {
            optind++;
            classAmount = atoi(argv[optind]);
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
    Bootstrap bootstrapInstance(myUDPPort, peerlistSelectorStrategy, peerListSharedSize, minimumBandwidth, minimumBandwidth_FREE, timeToSetNewOut, timeNewOutDelayStarts,
    		                    inCommon, inFree, percentPeersInClass, classAmount);
    
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
