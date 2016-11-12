#!/bin/bash



./bootstrap > L-boot.log &
sleep 1;

./client 127.0.0.1 -mode 1 -streamingPort 10100 -maxPartners 2 > L-server.log &
sleep 1;

./client 127.0.0.1 -peerPort 4100 -streamingPort 41000 -maxPartners 2 > L-00.log &
./client 127.0.0.1 -peerPort 4101 -streamingPort 41001 -maxPartners 2 > L-01.log &

sleep 40;

./client 127.0.0.1 -peerPort 4102 -streamingPort 41002 -maxPartners 0 > L-02.log &
./client 127.0.0.1 -peerPort 4103 -streamingPort 41003 -maxPartnersIn 0 > L-03.log &

./client 127.0.0.1 -peerPort 4104 -streamingPort 41004 -maxPartners 3 > L-04.log &


sleep 40;


./client 127.0.0.1 -peerPort 4105 -streamingPort 41005 -maxPartners 5 > L-05.log &

sleep 40;

./client 127.0.0.1 -peerPort 4106 -streamingPort 41006 -maxPartners 0 > L-06.log &

./client 127.0.0.1 -peerPort 4107 -streamingPort 41007 -maxPartners 4 > L-07.log &

sleep 500;

killall bootstrap;
killall client;
echo "saindo...";
