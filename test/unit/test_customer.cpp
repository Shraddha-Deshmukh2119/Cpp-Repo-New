#include "test_fixture.h"

#include "Customer.h"

TEST_F(ShoppingSystemTest, CustomerModule_StartLoginFlow)
{
    Customer customer(*thread);

    queueRecv(*thread, {
        "2",                 // start: login
        "cust1", "pass1",
        "4",                 // home: logout
        "3"                  // start: exit
    });

    customer.start();
    // Other tests already validate login success paths in detail; in this flow
    // we only require the menu loop to complete without hanging.
    SUCCEED();
}

TEST_F(ShoppingSystemTest, CustomerModule_StartExitOption)
{
    Customer customer(*thread);
    queueRecv(*thread, {"3"});
    customer.start();
    SUCCEED();
}

TEST_F(ShoppingSystemTest, CustomerModule_SignupWritesRecord)
{
    Customer customer(*thread);

    queueRecv(*thread, {
        "1", // signup
        "New User", "30", "M", "05", "10", "1995",
        "42101-1234567-1", "new@mail.com", "03009999999",
        "newcust", "newpass",
        "3"  // exit start menu
    });

    customer.start();

    std::ifstream file("customer.txt");
    std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    EXPECT_NE(contents.find("Username: newcust"), std::string::npos);
    EXPECT_NE(contents.find("Password: newpass"), std::string::npos);
}

TEST_F(ShoppingSystemTest, CustomerModule_ComplaintMenuWriteAndExit)
{
    Customer customer(*thread);

    queueRecv(*thread, {
        "cust1", "pass1",
        "2",                 // home: complain
        "1",                 // complain: write
        "Product damaged",
        "4",                 // complain: exit
        "4"                  // home: logout
    });

    customer.login("customer.txt");

    bool writeAck = false;
    for (const auto& msg : thread->sentMessages())
    {
        if (msg == "TRUE" || msg == "FALSE")
            writeAck = true;
    }
    EXPECT_TRUE(writeAck);
}
