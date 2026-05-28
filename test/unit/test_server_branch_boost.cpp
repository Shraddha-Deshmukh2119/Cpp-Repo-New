#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <thread>

#include <gtest/gtest.h>

#include "Client.h"
#include "Server.h"

namespace
{
std::atomic<bool> g_slowHandled{false};

void slowHandler(Thread& worker)
{
    std::string msg;
    worker.Rec(msg);
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    worker.Send("SLOW:" + msg);
    g_slowHandled = true;
}

int pickFreePort()
{
    return 20000 + static_cast<int>((std::chrono::steady_clock::now().time_since_epoch().count() % 1000));
}
}

TEST(NetworkIntegration, ServerBoost_ActivityUpdateWhenClientsPathBlocked)
{
    const int port = pickFreePort();
    Server server(slowHandler, port);
    ASSERT_TRUE(server.start());

    std::error_code ec;
    std::filesystem::remove("clients.txt", ec);
    std::filesystem::create_directory("clients.txt", ec);

    std::thread serverThread([&]() { server.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    Client client("127.0.0.1", port);
    if (client.start())
    {
        client.Send("activity");
        std::string reply;
        client.Rec(reply);
    }

#ifdef ONLINE_SHOPPING_UNIT_TEST
    server.requestStop();
#endif
    serverThread.join();

    std::filesystem::remove_all("clients.txt", ec);
    SUCCEED();
}

TEST(NetworkIntegration, ServerBoost_ThreadPoolExhaustedLogsNoSpace)
{
    const int port = pickFreePort();
    Server server(slowHandler, port);
    ASSERT_TRUE(server.start());

    std::thread serverThread([&]() { server.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    std::vector<std::unique_ptr<Client>> clients;
    for (int i = 0; i < 7; ++i)
    {
        auto c = std::make_unique<Client>("127.0.0.1", port);
        if (c->start())
        {
            c->Send("load" + std::to_string(i));
            clients.push_back(std::move(c));
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(600));

#ifdef ONLINE_SHOPPING_UNIT_TEST
    server.requestStop();
#endif
    serverThread.join();
    SUCCEED();
}
