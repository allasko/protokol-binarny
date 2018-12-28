#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <bitset>
#include <vector>
#include <string>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma warning(disable:4996)

#define DEFAULT_BUFLEN 225
#define DEFAULT_PORT 27015 //port tcp
//pole operacji
//000 +
//001 -
//010 *
//011 /
//jeszcze 4
//i silnia
//wynik przekroczy zakres ma zwracac kod bledu
//identyfikator sesji w trakcie komunikacji
//1 dla wyjscia poza zakres dlugosci danych 0 jesli ok

class klient {
public:
    bool error = false;

    std::bitset<3> poleOperacji;
    std::bitset<4> poleStatusu;
    std::bitset<32> poleDlugosciDanych;
    std::vector<std::string> poleDanych;
    std::bitset<2> identyfikator;
    std::bitset<32> wynik;

    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    sockaddr_in service;

    char sendbuf[DEFAULT_BUFLEN];
    int sendbuflen;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    int iResult;
    std::string s;

    klient() {}

    void validation();
    void connectsocket();
    void sending();
    void receive();
    void cleanup();

    std::string kompresja();
    void dekompresja();
};
void klient::validation() {
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        error = true;
    }

    ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    std::cout << "Podaj adres IP serwera: ";
    std::string IP;
    std::cin >> IP;

    memset(&service, 0, sizeof(service));
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr(IP.c_str());
    service.sin_port = DEFAULT_PORT;

    if (!error)printf("validation completed\n");
}
void klient::connectsocket() {
    if (connect(ConnectSocket, (SOCKADDR *)& service, sizeof(service)) == SOCKET_ERROR)
    {
        printf("Failed to connect.\n");
        WSACleanup();
        error = true;
    }
}
void klient::cleanup()
{
    // shutdown the connection since no more data will be sent
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        error = true;
    }

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

}
std::string klient::kompresja() {
    std::string se;
    se.append(poleOperacji.to_string());
    se.append(poleStatusu.to_string());
    se.append(poleDlugosciDanych.to_string());
    se.append(poleDanych[0]);
    se.append(poleDanych[1]);
    identyfikator = 01;
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

    std::cout << "Dane: " << se << std::endl;
    return str;
}
void klient::dekompresja() {
    std::string se;//string bitow
    std::string temp;
    for (auto e : s)
        se += std::bitset<8>(e).to_string();

    std::cout << "Odkompresowane dane: ";
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
        if (i == 2+4+32+32) {
            wynik = std::bitset<32>(temp);
            std::cout << temp << " ";
            temp = "";
        }
        if (i == 2 + 4 + 32 +32  + 2) {
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
    if(poleStatusu.to_ulong()==0) {
        std::cout << "Wynik: " << wynik.to_ulong();
    }else {
        std::cout<<"Liczba wyszla poza zakres wiec wynik jest bledny."<<std::endl;
    }


}
void klient::sending()
{
    // Send a buffer            (int)strlen(sendbuf)        //strcpy_s(sendbuf, s.c_str());     s.c_str()
    std::string str = kompresja();
    sendbuflen = str.size();
    char sendb[sendbuflen];//policzyc ile moze być paczek
    for (int i = 0; i < str.length(); i++)
        sendb[i] = str[i];

    std::cout<<"sendb: " <<sendb<<std::endl;
    iResult = send(ConnectSocket, sendb, sendbuflen, 0);
    std::cout << "Bytes Sent: " << iResult << "\n";
    std::cout << "String sent: " << str << '\n';


    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        error = true;
    }
    else
    {
        printf("sending completed\n");
        receive();
    }

}
void klient::receive()
{
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            for (int i = 0; i < 10; i++) {
                s[i] = recvbuf[i];
            }
            std::cout << "Received: " << recvbuf << std::endl;
            printf("\nBytes received: %d\n", iResult);

        } else if (iResult == 0)
            printf("\nConnection closed\n");
        else
            printf("recv failed with error: %d\n", WSAGetLastError());
    dekompresja();
    if (!error)printf("\nreceiving completed\n");
}
int main() {
    klient k;
    if (!k.error)
        k.validation();
    if (!k.error)
        k.connectsocket();
    if (!k.error) {
        int ok = 1;
        std::cout << "000 +,001 -,010 *,011 /,100 potega L1^L2,101 potega L2^L1,110 silnia L1, 111 L1%L2" << std::endl;
        std::cout << "Operacja: ";
        std::string temp;
        std::cin >> temp;
        k.poleOperacji = std::bitset<3>(temp);
        //   std::cout<<k.poleOperacji<<std::endl;
        unsigned long liczba1;
        unsigned long liczba2;
        std::cout << "Podaj liczbe 1: ";
        std::cin >> liczba1;
        std::string L1B = std::bitset<32>(liczba1).to_string();
        std::cout << L1B << std::endl;
        k.poleDanych.push_back(L1B);

        //unsigned long decimal = std::bitset<32>(binary).to_ulong();
        std::cout << std::endl;
        std::cout << "Podaj liczbe 2: ";
        std::cin >> liczba2;
        std::string L2B = std::bitset<32>(liczba2).to_string();
        k.poleDanych.push_back(L2B);
        std::cout << L2B << std::endl;
        std::cout << std::endl;
        if (liczba1 > 21474836478 || liczba2 > 21474836478) {
            k.poleStatusu = 0001;
            std::cout << "Liczba poza zakresem" << std::endl;
        }
        else {
            k.poleDlugosciDanych = k.poleDanych.size() * 32;
            k.poleStatusu = 0000;
        }
        k.poleStatusu;
        k.s = k.kompresja();
        k.sending();
        k.cleanup();
    }
    if (k.error) { std::cout << "Error" << std::endl; }
    return 0;
}
