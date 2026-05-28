#pragma once

#include <string>
#include <winsock2.h>  // FIX: Explicitly include winsock2.h first to define the SOCKET type
#include <windows.h>

#ifdef ONLINE_SHOPPING_UNIT_TEST
#include <deque>
#include <vector>
#endif

class Server;
struct Data
{
    Server *server;
    SOCKET client;
    int index;
};
void worker(Data *);

class Thread
{
public:
    Thread() = default;
    Thread(Server* server, int index)
    {
        data.server = server;
        data.index = index; // Fixed a minor stray double semicolon here
    }

    void create();
    bool available() const;
    void enter(SOCKET &Client);
    void setFree();
    bool isFree() const;
    bool isDestroyed() const;
    void CloseThread();
    HANDLE getHandle() const;
    SOCKET getClient() const;
    void endServer();

    // extra function:
    std::string getActivity();
    void closeClient();
 
    int Send(const std::string& str) const;
    int Send(const int& var) const;
    int Send(const double& var) const;
    int Rec(std::string& receivingString);

#ifdef ONLINE_SHOPPING_UNIT_TEST
    void enableTestMode();
    void pushRecv(const std::string& message);
    const std::vector<std::string>& sentMessages() const;
    void clearSentMessages();
#endif

private:
    HANDLE handle = nullptr;
    Data data;
    bool free = true;
    bool destroyed = false;

#ifdef ONLINE_SHOPPING_UNIT_TEST
    bool testMode = false;
    std::deque<std::string> recvQueue;
    mutable std::vector<std::string> sendLog;
#endif
};
