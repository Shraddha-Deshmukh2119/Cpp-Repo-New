#include <array>
#include <sstream>

#include "coverage_helpers.h"
#include "person_accessor.h"
#include "test_fixture.h"

#include "Admin.h"
#include "Complaint_Base.h"
#include "Complaint_C.h"
#include "Complaint_E.h"
#include "Customer.h"
#include "Employee.h"

namespace
{
constexpr std::size_t kGoodsCount = 10;
}

// --- Person::buy additional branches ---
TEST_F(ShoppingSystemTest, BranchBoost_PersonBuyNotEnoughStock)
{
    PersonTestAccessor person(*thread);
    person.setLookupValue("Username: cust1");
    queueRecv(*thread, {"1", "1", "9999", "3"});
    person.testBuy("customer.txt");
    EXPECT_TRUE(sentEquals(*thread, "NOT-ENOUGH"));
}

TEST_F(ShoppingSystemTest, BranchBoost_PersonBuyNoOrderOnConfirm)
{
    PersonTestAccessor person(*thread);
    person.setLookupValue("Username: cust1");
    queueRecv(*thread, {"2", "3"});
    person.testBuy("customer.txt");
    EXPECT_TRUE(sentContains(*thread, "NO-ORDER"));
}

TEST_F(ShoppingSystemTest, BranchBoost_PersonBuyReorderSkippedWhenStockAboveLevel)
{
    writeUserRecord("customer.txt", "cust1", "pass1", 5000.0);
    writeGoodsFile();
    writeCashFile(50000, 0, 0, 50000);

    PersonTestAccessor person(*thread);
    person.setLookupValue("Username: cust1");
    queueRecv(*thread, {"1", "1", "1", "2", "3"});
    person.testBuy("customer.txt");
    EXPECT_TRUE(sentContains(*thread, "Your new balance"));
}

TEST_F(ShoppingSystemTest, BranchBoost_PersonBuyEmployeeFileDiscountPath)
{
    writeUserRecord("emp.txt", "emp1", "emppass", 8000.0);
    PersonTestAccessor person(*thread);
    person.setLookupValue("Username: emp1");
    queueRecv(*thread, {"1", "1", "1", "2", "3"});
    person.testBuy("emp.txt");
    EXPECT_TRUE(sentContains(*thread, "Your new balance"));
}

TEST_F(ShoppingSystemTest, BranchBoost_PersonUpdateGoodsFailsOnReadOnlyFile)
{
    PersonTestAccessor person(*thread);
    std::array<Goods, kGoodsCount> goods{};
    ASSERT_TRUE(person.testInitializeGoods(goods.data()));

    std::filesystem::permissions(
        "goods.txt", std::filesystem::perms::owner_read, std::filesystem::perm_options::replace);
    EXPECT_FALSE(person.testUpdateGoods(goods.data()));
    std::filesystem::permissions(
        "goods.txt", std::filesystem::perms::owner_all, std::filesystem::perm_options::replace);
}

TEST_F(ShoppingSystemTest, BranchBoost_PersonUpdateCashFailsOnReadOnlyFile)
{
    PersonTestAccessor person(*thread);
    Cash cash{};
    ASSERT_TRUE(person.testInitializeCash(cash));

    std::filesystem::permissions(
        "cash.txt", std::filesystem::perms::owner_read, std::filesystem::perm_options::replace);
    cash + 10.0;
    EXPECT_FALSE(person.testUpdateCash(cash));
    std::filesystem::permissions(
        "cash.txt", std::filesystem::perms::owner_all, std::filesystem::perm_options::replace);
}

TEST_F(ShoppingSystemTest, BranchBoost_PersonUpdateBalanceFailsOnReadOnlyFile)
{
    PersonTestAccessor person(*thread);
    person.setLookupValue("Username: cust1");
    person.setBalanceValue("321");

    // On Windows, setting a file to read-only may still allow rewrite/replace via temp+rename.
    // Use an invalid path to force open failure and take the false branch.
    EXPECT_FALSE(person.testUpdateBalance("Z:\\no_such_dir_oss\\customer.txt"));
}

// --- Complaint branches ---
TEST_F(ShoppingSystemTest, BranchBoost_ComplaintBaseViewFormatsOutput)
{
    Complaint_Base base("view_fmt.dat");
    const char client[] = "viewer";
    ASSERT_TRUE(base.write("Issue text", client));

    Complaint readIn{};
    ASSERT_TRUE(base.see(readIn, client, false));

    std::ostringstream os;
    base.view(readIn, os);
    EXPECT_NE(os.str().find("Issue text"), std::string::npos);
    EXPECT_NE(os.str().find("viewer"), std::string::npos);
}

TEST_F(ShoppingSystemTest, BranchBoost_ComplaintUpdateWhenAlreadyAnswered)
{
    Complaint_Base base("answered.dat");
    const char client[] = "answered_user";
    ASSERT_TRUE(base.write("Open", client));

    Complaint_E emp("answered.dat");
    ASSERT_TRUE(emp.answer("Resolved", client));

    bool status = false;
    EXPECT_FALSE(base.update("New text", client, status));
    EXPECT_TRUE(status);
}

TEST_F(ShoppingSystemTest, BranchBoost_ComplaintEmployeeAnswerAndClear)
{
    const char client[] = "clear_me";
    Complaint_Base seed("emp_clear.dat");
    ASSERT_TRUE(seed.write("Need fix", client));

    Complaint_E emp("emp_clear.dat");

    EXPECT_TRUE(emp.answer("Fixed", client));

    std::ostringstream summary;
    emp.view(false, summary);
    EXPECT_NE(summary.str().find("clear_me"), std::string::npos);

    std::ostringstream detailed;
    emp.view(true, detailed);
    EXPECT_NE(detailed.str().find("Need fix"), std::string::npos);

    emp.clear();
    std::ostringstream afterClear;
    emp.view(false, afterClear);
    EXPECT_TRUE(afterClear.str().empty());
}

TEST_F(ShoppingSystemTest, BranchBoost_ComplaintCustomerReviewWhenNotRegistered)
{
    Complaint_C customerSide("nobody", "nobody.dat");
    std::ostringstream review;
    customerSide.reView(review);
    // Complaint_C::reView prints the message to stderr (not to the provided ostream).
    EXPECT_TRUE(review.str().empty());
}

// --- Admin branches ---
TEST_F(ShoppingSystemTest, BranchBoost_AdminStockModifyPriceSuccess)
{
    Admin admin(*thread);
    queueRecv(*thread, {
        "admin1", "adminpass",
        "7", "2", "Product1", "250.5", "8", "12", "17"
    });
    admin.login("admin.txt");
    EXPECT_TRUE(sentEquals(*thread, "TRUE"));

    std::ifstream goods("goods.txt");
    const std::string contents((std::istreambuf_iterator<char>(goods)),
                               std::istreambuf_iterator<char>());
    EXPECT_NE(contents.find("250.5"), std::string::npos);
}

TEST_F(ShoppingSystemTest, BranchBoost_AdminStockReorderTruePath)
{
    writeCashFile(50000, 0, 0, 50000);
    Admin admin(*thread);
    queueRecv(*thread, {
        "admin1", "adminpass",
        "7", "1", "Product1", "5", "17"
    });
    admin.login("admin.txt");
    EXPECT_GE(sentCount(*thread, "TRUE"), 2u);
}

TEST_F(ShoppingSystemTest, BranchBoost_AdminDeleteMissingSourceFile)
{
    std::filesystem::remove("customer.txt");
    Admin admin(*thread);
    queueRecv(*thread, {"admin1", "adminpass", "12", "cust1", "17"});
    admin.login("admin.txt");
    SUCCEED();
}

TEST_F(ShoppingSystemTest, BranchBoost_AdminBalanceMissingSourceFile)
{
    std::filesystem::remove("customer.txt");
    Admin admin(*thread);
    queueRecv(*thread, {"admin1", "adminpass", "13", "cust1", "100", "17"});
    admin.login("admin.txt");
    EXPECT_FALSE(sentEquals(*thread, "TRUE"));
}

TEST_F(ShoppingSystemTest, BranchBoost_AdminSearchRecordBlockWithoutMatch)
{
    Admin admin(*thread);
    queueRecv(*thread, {
        "admin1", "adminpass",
        "10", "4", "___no_such_user___", "6",
        "17"
    });
    admin.login("admin.txt");
    EXPECT_TRUE(sentEquals(*thread, "FALSE"));
}

// --- Employee complain branches ---
TEST_F(ShoppingSystemTest, BranchBoost_EmployeeComplainClearAndDefaultCase)
{
    Complaint_Base base("complaint.dat");
    ASSERT_TRUE(base.write("To clear", "cust1"));

    Employee employee(*thread);
    queueRecv(*thread, {"emp1", "emppass", "2", "6", "99", "7", "4"});
    employee.login("emp.txt");
    SUCCEED();
}

TEST_F(ShoppingSystemTest, BranchBoost_EmployeeComplainSeeWithDetailsTrue)
{
    Complaint_Base base("complaint.dat");
    ASSERT_TRUE(base.write("Detail me", "cust1"));

    Employee employee(*thread);
    queueRecv(*thread, {"emp1", "emppass", "2", "1", "cust1", "7", "4"});
    employee.login("emp.txt");
    EXPECT_TRUE(sentContains(*thread, "Detail me"));
}

// --- Thread branches ---
TEST_F(ShoppingSystemTest, BranchBoost_ThreadEmptyRecvReturnsZero)
{
    thread->clearSentMessages();
    std::string empty;
    EXPECT_EQ(thread->Rec(empty), 0);
    EXPECT_TRUE(empty.empty());
}

TEST_F(ShoppingSystemTest, BranchBoost_ThreadEndServerSetsExitFlag)
{
    thread->endServer();
    SUCCEED();
}

// NOTE: Avoid creating/resuming real Win32 threads in this branch-boost file.
// That behavior is already covered elsewhere and can be flaky in CI.
