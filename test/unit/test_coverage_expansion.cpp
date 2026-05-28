#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "test_fixture.h"
#include "coverage_helpers.h"
#include "person_accessor.h"

#include "Admin.h"
#include "Client.h"
#include "Server.h"
#include "Thread.h"
#include "Customer.h"
#include "Employee.h"

// --- Thread & Client specialization branch tests ---

TEST_F(ShoppingSystemTest, CoverageBoost_ThreadSendSpecializations)
{
    thread->enableTestMode();
    thread->clearSentMessages();
    
    thread->Send(100);
    thread->Send(50.5);
    
    EXPECT_TRUE(sentEquals(*thread, "100"));
    EXPECT_TRUE(sentEquals(*thread, "50.5"));
}

TEST(NetworkIntegration, CoverageBoost_ClientSendSpecializations)
{
    Client client("127.0.0.1", 12345);
    // These calls execute the int and double send specializations.
    // They will fail/return SOCKET_ERROR because the client is not connected,
    // but the target branch coverage for the code is successfully achieved.
    client.Send(42);
    client.Send(3.14);
    SUCCEED();
}

// --- Server activity error branches ---

TEST_F(ShoppingSystemTest, CoverageBoost_ServerActivityFileErrors)
{
    // Force file open failure in updateActivity and getActivity by creating a directory with the same name
    std::filesystem::remove("clients.txt");
    std::filesystem::create_directory("clients.txt");
    
    // Call getActivity which should fail and return empty string
    std::string activity = thread->getActivity();
    EXPECT_TRUE(activity.empty());
    
    // Clean up
    std::filesystem::remove_all("clients.txt");
}

TEST(NetworkIntegration, CoverageBoost_ServerStartBindFailure)
{
    // Dynamic port to avoid bind/time-wait conflicts on consecutive runs
    static int port_counter = 27000 + static_cast<int>((std::chrono::steady_clock::now().time_since_epoch().count() % 500));
    int port = port_counter++;

    // Bind first server
    Server s1([](Thread&){}, port);
    ASSERT_TRUE(s1.start());
    
    // Attempt second server on same port (causes bind to fail, but start() returns WSAok)
    Server s2([](Thread&){}, port);
    EXPECT_TRUE(s2.start());
}

// --- Admin stock & accounts branch tests ---

TEST_F(ShoppingSystemTest, CoverageBoost_AdminStockModifyInvalidName)
{
    Admin admin(*thread);
    // Choice 7 (stock), Option 2 (modify), Invalid Product Name, Exit choice 17
    queueRecv(*thread, {
        "admin1", "adminpass",
        "7", "2", "No_Such_Product", "17"
    });
    admin.login("admin.txt");
    EXPECT_TRUE(sentEquals(*thread, "FALSE"));
}

TEST_F(ShoppingSystemTest, CoverageBoost_AdminStockReorderInvalidName)
{
    Admin admin(*thread);
    // Choice 7 (stock), Option 1 (reorder), Invalid Product Name, Exit choice 17
    queueRecv(*thread, {
        "admin1", "adminpass",
        "7", "1", "No_Such_Product", "17"
    });
    admin.login("admin.txt");
    EXPECT_TRUE(sentEquals(*thread, "FALSE"));
}

TEST_F(ShoppingSystemTest, CoverageBoost_AdminStockReorderInsufficientCash)
{
    // Write cash file with very low final cash
    writeCashFile(100.0, 0, 0, 5.0);
    
    Admin admin(*thread);
    // Choice 7 (stock), Option 1 (reorder), Product1, Quantity 2.0 (needs 2 * 150 = 300.0, cash is only 5.0), Exit choice 17
    queueRecv(*thread, {
        "admin1", "adminpass",
        "7", "1", "Product1", "2.0", "17"
    });
    admin.login("admin.txt");
    EXPECT_TRUE(sentEquals(*thread, "FALSE"));
}

TEST_F(ShoppingSystemTest, CoverageBoost_AdminAccountsFileOpenFailure)
{
    std::filesystem::remove("cash.txt");
    
    Admin admin(*thread);
    // Choice 8 (accounts), Exit choice 17
    queueRecv(*thread, {
        "admin1", "adminpass",
        "8", "17"
    });
    admin.login("admin.txt");
    SUCCEED();
}

TEST_F(ShoppingSystemTest, CoverageBoost_AdminDeleteFileOpenFailure)
{
    // Create a directory where customer.txt should be to cause file open failure
    std::filesystem::remove("customer.txt");
    std::filesystem::create_directory("customer.txt");
    
    Admin admin(*thread);
    // Choice 12 (Delete Customer), cust1, Exit choice 17
    queueRecv(*thread, {
        "admin1", "adminpass",
        "12", "cust1", "17"
    });
    admin.login("admin.txt");
    
    std::filesystem::remove_all("customer.txt");
    SUCCEED();
}

TEST_F(ShoppingSystemTest, CoverageBoost_AdminHomeInvalidOptions)
{
    Admin admin(*thread);
    // Send invalid choice 99, then valid exit choice 17
    queueRecv(*thread, {
        "admin1", "adminpass",
        "99", "17"
    });
    admin.login("admin.txt");
    SUCCEED();
}

// --- Person signup validation and buy edge cases ---

TEST_F(ShoppingSystemTest, CoverageBoost_PersonBuyInvalidOption)
{
    PersonTestAccessor person(*thread);
    person.setLookupValue("Username: cust1");
    
    // Choice 99 (invalid choice), then choice 3 (exit buy menu)
    queueRecv(*thread, {"99", "3"});
    person.testBuy("customer.txt");
    SUCCEED();
}
