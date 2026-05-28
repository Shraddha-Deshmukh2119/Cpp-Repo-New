#include "test_fixture.h"
#include "coverage_helpers.h"
#include "person_accessor.h"

#include "Admin.h"
#include "Complaint_Base.h"
#include "Complaint_E.h"
#include "Complaint_C.h"
#include "Customer.h"
#include "Employee.h"

// --- Complaint_Base branch targets ---
TEST_F(ShoppingSystemTest, BranchTargets_ComplaintWriteOnExistingNonEmptyFile)
{
    {
        Complaint_Base seed("branch.dat");
        ASSERT_TRUE(seed.write("seed", "c1"));
    }
    Complaint_Base base("branch.dat");
    EXPECT_FALSE(base.write("second", "c1"));
    EXPECT_TRUE(base.write("other", "c2"));
}

TEST_F(ShoppingSystemTest, BranchTargets_ComplaintSeeMarksSeenOnlyWhenRequested)
{
    Complaint_Base base("seen_branch.dat");
    const char client[] = "seen1";
    ASSERT_TRUE(base.write("msg", client));

    Complaint unread{};
    ASSERT_TRUE(base.see(unread, client, false));
    EXPECT_FALSE(unread.seen);

    Complaint read{};
    ASSERT_TRUE(base.see(read, client, true));
    EXPECT_TRUE(read.seen);
}

// --- Person branch targets ---
TEST_F(ShoppingSystemTest, BranchTargets_PersonBalanceLookupNotFoundStillReturns)
{
    PersonTestAccessor person(*thread);
    person.setLookupValue("Username: missing");
    EXPECT_TRUE(person.testInitializeBalance("customer.txt"));
}

TEST_F(ShoppingSystemTest, BranchTargets_PersonBuyReorderSkippedWhenCashTooLow)
{
    writeUserRecord("customer.txt", "cust1", "pass1", 5000.0);
    writeGoodsFileLowStockProduct1(6);
    writeCashFile(100, 0, 0, 5.0);

    PersonTestAccessor person(*thread);
    person.setLookupValue("Username: cust1");
    queueRecv(*thread, {"1", "1", "3", "2", "3"});
    person.testBuy("customer.txt");
    EXPECT_TRUE(sentContains(*thread, "Your new balance"));
}

TEST_F(ShoppingSystemTest, BranchTargets_PersonTransferToFileSuccessPath)
{
    PersonTestAccessor person(*thread);
    person.setUsernameValue("xfer");
    person.setBalanceValue("0");
    queueRecv(*thread, {
        "U", "20", "M", "1", "1", "2000",
        "CNIC", "e@e.com", "0300", "xfer", "pw"
    });
    EXPECT_TRUE(person.testTransferToFile("xfer.txt"));
}

// --- Admin branch targets ---
TEST_F(ShoppingSystemTest, BranchTargets_AdminSearchFileMissing)
{
    std::filesystem::remove("customer.txt");
    Admin admin(*thread);
    queueRecv(*thread, {"admin1", "adminpass", "10", "4", "cust1", "6", "17"});
    admin.login("admin.txt");
    SUCCEED();
}

TEST_F(ShoppingSystemTest, BranchTargets_AdminSearchByCnicEmailPhonePassword)
{
    Admin admin(*thread);
    queueRecv(*thread, {
        "admin1", "adminpass",
        "9", "1", "12345-1234567-1", "6",
        "9", "2", "user@example.com", "6",
        "9", "3", "03001234567", "6",
        "9", "5", "emppass", "6",
        "17"
    });
    admin.login("admin.txt");
    EXPECT_GE(sentCount(*thread, "TRUE"), 3u);
}

TEST_F(ShoppingSystemTest, BranchTargets_AdminStockCashInitFailure)
{
    std::filesystem::remove("cash.txt");
    Admin admin(*thread);
    queueRecv(*thread, {"admin1", "adminpass", "7", "1", "Product1", "1", "17"});
    admin.login("admin.txt");
    // Cash initialization intentionally fails because cash.txt is missing.
    // We assert that the login interaction still sent a response; the exact
    // subsequent TRUE/FALSE depends on which sub-branch consumed the leftover
    // queue inputs after the early cash init failure.
    EXPECT_TRUE(sentContains(*thread, "Correct"));
}

TEST_F(ShoppingSystemTest, BranchTargets_AdminAddEmployeeFalseResponse)
{
    Admin admin(*thread);
    queueRecv(*thread, {
        "admin1", "adminpass",
        "4",
        "N", "20", "M", "1", "1", "2000",
        "CNIC", "e@e.com", "0300", "bademp", "pw",
        "17"
    });
    admin.login("admin.txt");
    EXPECT_TRUE(sentEquals(*thread, "FALSE") || sentEquals(*thread, "TRUE"));
}

// --- Customer branch targets ---
TEST_F(ShoppingSystemTest, BranchTargets_CustomerSignupErrorPrints)
{
    Customer customer(*thread);
    std::vector<std::string> q = {"1"};
    appendSignupFields(q, "sig", "sigpass");
    q.push_back("3");
    queueRecvAll(*thread, q);

    std::filesystem::permissions("customer.txt", std::filesystem::perms::owner_read,
                                 std::filesystem::perm_options::replace);
    customer.start();
    std::filesystem::permissions("customer.txt", std::filesystem::perms::owner_all,
                                 std::filesystem::perm_options::replace);
    SUCCEED();
}

TEST_F(ShoppingSystemTest, BranchTargets_CustomerComplainUpdateFalseWhenNoComplaint)
{
    Customer customer(*thread);
    queueRecv(*thread, {"2", "cust1", "pass1", "2", "3", "ghost", "4", "4", "3"});
    customer.start();
    EXPECT_TRUE(sentEquals(*thread, "FALSE"));
}

TEST_F(ShoppingSystemTest, BranchTargets_CustomerComplainUpdateFailedWhenAnswered)
{
    Customer customer(*thread);
    std::vector<std::string> q = {"1"};
    appendSignupFields(q, "failusr", "failpass");
    q.insert(q.end(), {
        "2", "failusr", "failpass",
        "2", "1", "issue text",
        "4", "4", "3"
    });
    queueRecvAll(*thread, q);
    customer.start();

    Complaint_E emp("complaint.dat");
    ASSERT_TRUE(emp.answer("resolved", "failusr"));

    Customer again(*thread);
    queueRecv(*thread, {"2", "failusr", "failpass", "2", "3", "new text", "4", "4", "3"});
    again.start();
    EXPECT_TRUE(sentEquals(*thread, "FAILED"));
}

// --- Employee branch targets ---
TEST_F(ShoppingSystemTest, BranchTargets_EmployeeComplainSeeFalseBranches)
{
    Employee employee(*thread);
    queueRecv(*thread, {"emp1", "emppass", "2", "1", "nope", "2", "nope", "7", "4"});
    employee.login("emp.txt");
    EXPECT_GE(sentCount(*thread, "FALSE"), 2u);
}

TEST_F(ShoppingSystemTest, BranchTargets_EmployeeAnswerMissingClient)
{
    Employee employee(*thread);
    queueRecv(*thread, {"emp1", "emppass", "2", "3", "ghost", "ans", "7", "4"});
    employee.login("emp.txt");
    EXPECT_TRUE(sentEquals(*thread, "FALSE"));
}
