#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <bitset>
#include <vector>
#include <math.h>
// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma warning(disable:4996)

#define DEFAULT_BUFLEN 225
#define DEFAULT_PORT 27015 //port tcp


class serwer {
public:

    bool error = false;


    std::bitset<3> poleOperacji;
    std::bitset<4> poleStatusu;
    std::bitset<32> poleDlugosciDanych;
    std::vector<std::string> poleDanych;
    std::bitset<2> identyfikator;
    std::bitset<32> liczba1;
    std::bitset<32> liczba2;

    WSADATA wsaData;
    int iResult;
    std::string s;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    sockaddr_in service;

    int iSendResult;
    char sendbuf[DEFAULT_BUFLEN];
    unsigned int sendbuflen;
    char recvbuf[DEFAULT_BUFLEN];
    unsigned int recvbuflen = DEFAULT_BUFLEN;


    void validation();
    void connectsocket();
    void receive();
    void sending();
    void cleanup();

    void dekompresja();
    void operacje();
    std::string kompresja();

};
std::bitset<4> status(long wynik){
    std::bitset<4> status;
    if(wynik>=2147483647||wynik<=-214783647) {
        status = 1;
    }else{
        status = 0;
    }
    return status;
}
void serwer::operacje() {
    long wynik;
    if (poleOperacji.to_ulong() == 0) {
        std::cout << "+" << std::endl;
        wynik = liczba1.to_ulong() + liczba2.to_ulong();
    }
    if (poleOperacji.to_ulong() == 1) {
        std::cout << "-" << std::endl;
        wynik = liczba1.to_ulong() - liczba2.to_ulong();

    }
    if (poleOperacji.to_ulong() == 2) {
        std::cout << "*" << std::endl;
        wynik = liczba1.to_ulong() * liczba2.to_ulong();
    }
    if (poleOperacji.to_ulong() == 3) {
        std::cout << "/" << std::endl;
        wynik = liczba1.to_ulong() / liczba2.to_ulong();
    }
    if (poleOperacji.to_ulong() == 4) {
        std::cout << "pow" << std::endl;
        poleStatusu = status(pow(liczba1.to_ulong(), liczba2.to_ulong()));
        wynik = pow(liczba1.to_ulong(), liczba2.to_ulong());
    }
    if (poleOperacji.to_ulong() == 5) {
        std::cout << "pow2" << std::endl;
        poleStatusu = status(pow(liczba2.to_ulong(), liczba1.to_ulong()));
        wynik = pow(liczba2.to_ulong(), liczba1.to_ulong());
    }
    if (poleOperacji.to_ulong() == 6) {
        std::cout << "silnia" << std::endl;
        wynik = 1;
        for (int i = 1; i <= liczba1.to_ulong(); i++) {
            wynik *= i;
        }
        if(wynik>=2147483647||wynik<=-214783647) {
            poleStatusu = 1;
        }else{
            poleStatusu = 0;
        }
    }
    if (poleOperacji.to_ulong() == 7) {
        std::cout << "modulo" << std::endl;
        wynik = liczba1.to_ulong() % liczba2.to_ulong();
    }
    std::cout << "Wynik: " << wynik << std::endl;
    liczba1 = wynik;
    poleDanych.resize(1);
    poleDanych[0] = liczba1.to_string();
    poleDlugosciDanych = poleDanych.size() *32;


    std::cout << std::endl;
}
std::string serwer::kompresja() {
    std::string se;
    se.append(poleOperacji.to_string());
    se.append(poleStatusu.to_string());
    se.append(poleDlugosciDanych.to_string());
    se.append(poleDanych[0]);
    se.append(identyfikator.to_string());
    se.append("0000000");
    std::string str;//8bitowe chary;
    std::string temp;//jeden char;
    for (int i = 0; i < se.length(); i++) {
        temp += se[i];
        if (i % 8 == 7) {
            auto a = std::bitset<8>(temp);
            str += a.to_ulong();
            temp.clear();
        }
    }
    std::cout << "Dane zwrotne: " << se << std::endl;
    return str;
}
void serwer::dekompresja() {//dekompresje danych naprawic

    std::string se;//string bitow
    std::string temp;// 8bitowy char w string
    long poleDlugosci;

    for (auto e : s)
        se += std::bitset<8>(e).to_string();

    std::cout << "odkomresowane dane: ";
//wypakowywanie bitow w odpowiednie bitsety
    for (int i = 0; i < se.length(); i++) {
        temp += se[i];

        if (i == 2) {
            poleOperacji = std::bitset<3>(temp);
            std::cout << temp << " ";
            temp = "";
        }
        if (i == 2 + 4) {
            poleStatusu = std::bitset<4>(temp);
            std::cout << temp << " ";
            temp = "";
        }
        if (i == 2 + 4 + 32) {
            poleDlugosciDanych = std::bitset<32>(temp);
            std::cout << temp << " ";
            temp = "";
        }

        if (i ==2 + 4 + 32  + 32) {
            liczba1 = std::bitset<32>(temp);
            std::cout << temp << " ";
            temp = "";
        }
        if (i == 2 + 4 + 32 + 32 + 32) {
            liczba2 = std::bitset<32>(temp);
            std::cout << temp << " ";
            temp = "";
        }
        if (i == 2 + 4 + 32 +32 + 32 + 2) {
            identyfikator = std::bitset<2>(temp);
            std::cout << temp << " ";
            temp = "";
        }
    }
    std::cout<<std::endl;
    std::cout << "O: " << poleOperacji.to_string() << std::endl;
    std::cout << "S: " << poleStatusu.to_string() << std::endl;
    std::cout << "DD: " << poleDlugosciDanych.to_string() << std::endl;
    std::cout << "ID: " << identyfikator.to_string() << std::endl;
    std::cout << "L1: " << liczba1 << std::endl;
    std::cout << "L2: " << liczba2 << std::endl;
}
void serwer::validation()
{
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        error = true;
    }

    ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    memset(&service, 0, sizeof(service));
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY;
    service.sin_port = DEFAULT_PORT;

    if (bind(ListenSocket, (SOCKADDR *)& service, sizeof(service)) == SOCKET_ERROR)
    {
        printf("bind() failed.\n");
        closesocket(ListenSocket);
        error = true;
    }

    if (!error)printf("validation completed\n");
}
void serwer::connectsocket() {
    if (listen(ListenSocket, 1) == SOCKET_ERROR)
        printf("Error listening on socket.\n");

    printf("Waiting for a client to connect...\n");

    while (ClientSocket == SOCKET_ERROR)
    {
        ClientSocket = accept(ListenSocket, NULL, NULL);
    }

    printf("Client connected.\n");
    ListenSocket = ClientSocket;

    if (!error)printf("connecting completed\n\n");
}
void serwer::receive()
{
    // Receive until the peer shuts down the connection
    do {
        //otrzymanie
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            std::cout << recvbuf << std::endl;
            s.resize(105);
            for (int i = 0; i < 14; i++) {
                s[i] = recvbuf[i];
            }
            printf("Bytes received: %d\n", iResult);
            dekompresja();
            sending();


        }
        else if (iResult == 0)
            printf("Connection closing...\n");
        else {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            error = true;
        }



    } while (iResult > 0);


    if (!error)printf("receiving completed\n\n");
}
void serwer::sending()
{
    //wysylanie
    operacje();
    std::string str = kompresja();


    sendbuflen = str.size();
    char sendb[sendbuflen];
    for (int i = 0; i < str.length(); i++)
        sendb[i] = str[i];


    // send the buffer back to the sender
    iSendResult = send(ClientSocket, sendb, sendbuflen, 0);
    if (iSendResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        error = true;
    }
    std::cout << "wyslane slowo: " << s << '\n';
    printf("Bytes sent: %d\n", iSendResult);
}
void serwer::cleanup()
{
    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);



    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        error = true;
    }


    // cleanup
    closesocket(ClientSocket);
    WSACleanup();



    if (!error)printf("sending completed\n\n");
}
int  main() {
    serwer s;

    if (!s.error)
        s.validation();
    if (!s.error)
        s.connectsocket();
    if (!s.error)
        s.receive();
    if (!s.error)
        s.cleanup();

    if (s.error) { std::cout << "error"; }


    return 0;
}