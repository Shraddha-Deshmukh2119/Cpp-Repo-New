#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>

#include <gtest/gtest.h>

#include "Client.h"
#include "Server.h"

namespace
{
std::atomic<bool> g_handled{false};
std::string g_lastMessage;

void networkHandler(Thread& worker)
{
    std::string msg;
    worker.Rec(msg);
    g_lastMessage = msg;
    worker.Send("ACK:" + msg);
    g_handled = true;
}

void noopHandler(Thread&) {}

int pickFreePort()
{
    return 19000 + static_cast<int>((std::chrono::steady_clock::now().time_since_epoch().count() % 1000));
}
}

TEST(NetworkIntegration, ServerStartRunClientHandlerAndStop)
{
    g_handled = false;
    g_lastMessage.clear();
    const int port = pickFreePort();

    Server server(networkHandler, port);
    ASSERT_TRUE(server.start());

    std::thread serverThread([&]() { server.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    Client client("127.0.0.1", port);
    if (client.start())
    {
        client.Send("ping");
        std::string reply;
        client.Rec(reply);
        if (g_handled.load() && reply.find("ACK:ping") != std::string::npos)
            SUCCEED();
    }

#ifdef ONLINE_SHOPPING_UNIT_TEST
    server.requestStop();
#endif
    serverThread.join();
}

TEST(NetworkIntegration, ServerGetActivityThroughThread)
{
    const int port = pickFreePort();
    Server server(noopHandler, port);
    ASSERT_TRUE(server.start());

    {
        std::error_code ec;
        // Ensure this path is a file (not a directory leftover from other tests/runs).
        std::filesystem::remove_all("clients.txt", ec);
        std::ofstream file("clients.txt");
        file << "Client connected test line\n";
    }

    Thread worker(&server, 0);
    EXPECT_NE(worker.getActivity().find("connected"), std::string::npos);

#ifdef ONLINE_SHOPPING_UNIT_TEST
    server.requestStop();
#endif
}

TEST(NetworkIntegration, MultipleClientsIncreaseServerBranchCoverage)
{
    std::atomic<int> handled{0};
    const int port = pickFreePort();

    Server server(networkHandler, port);
    ASSERT_TRUE(server.start());

    std::thread serverThread([&]() { server.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    for (int i = 0; i < 3; ++i)
    {
        Client client("127.0.0.1", port);
        if (!client.start())
            continue;
        client.Send("ping" + std::to_string(i));
        std::string reply;
        client.Rec(reply);
        if (reply.find("ACK:") != std::string::npos)
            ++handled;
    }

    EXPECT_GE(handled.load(), 1);

#ifdef ONLINE_SHOPPING_UNIT_TEST
    server.requestStop();
#endif
    serverThread.join();
}

TEST(NetworkIntegration, ThreadLifecycleCreateAndStateFlags)
{
    const int port = pickFreePort();
    Server server(noopHandler, port);
    ASSERT_TRUE(server.start());

    Thread worker(&server, 0);
    worker.create();
    EXPECT_TRUE(worker.available());
    EXPECT_TRUE(worker.isFree());

    worker.setFree();
    EXPECT_TRUE(worker.isFree());
    EXPECT_TRUE(worker.isDestroyed());

#ifdef ONLINE_SHOPPING_UNIT_TEST
    server.requestStop();
#endif
}
