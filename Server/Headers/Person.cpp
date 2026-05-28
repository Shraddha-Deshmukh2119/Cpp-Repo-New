#include "Person.h"
#include "Goods.h"
#include "Cash.h"

bool Person::input(const string& fileName)
{
    (*server).Rec(name);
    (*server).Rec(age);
    (*server).Rec(sex);
    (*server).Rec(DOB.month);
    (*server).Rec(DOB.day);
    (*server).Rec(DOB.year);
    (*server).Rec(CNIC);
    (*server).Rec(email);
    (*server).Rec(phone_num);
    (*server).Rec(username);
    (*server).Rec(password);
    bool error = transfer_to_file(fileName);

    if (error)
        (*server).Send("true");
    else
        (*server).Send("false");
    return error;
}

bool Person::transfer_to_file(const string& fileName)
{
    ofstream file(fileName, ios::app); // Opening the file in append mode

    if (!file.is_open()) // Ensuring that the file opened successfully
        return false; // Indicates error

    // Writing to file
    file << "Username: " << username << endl
        << "Password: " << password << endl 
        << "Name: " << name << endl 
        << "Age: " << age << endl 
        << "Sex: " << sex << endl
        << "Date of birth: " << DOB.month 
        << "/" << DOB.day 
        << "/" << DOB.year << endl 
        << "CNIC: " << CNIC << endl 
        << "Email: " << email << endl 
        << "Phone number: " << phone_num << endl 
        << "Balance: " << 0 << endl 
        << endl;

    // Ensuring that the writing operation was carried out successfully
    if (file.fail())
    {
        file.close();
        return false; // Indicates error
    }
    else
    {
        file.close();
        return true; // Indicates successful termination
    }
}

void Person::login(const string& fileName)
{   
    (*server).Rec(username);
    (*server).Rec(password);

    ifstream file(fileName); // Opening the file
    string line;
    bool userFound = false, passFound = false;
    
    if (!file.is_open()) // Ensuring that the file opened successfully
    {
        cout << endl << "\nSorry! An error occured while processing the data.\n";
        getch();
        return;
    }
    
    while(!file.eof())
    {
        getline(file, line);
        if(line == ("Username: " + username))
        {
            userFound = true;
            getline(file, line);
            if(line == ("Password: " + password))
            {
                passFound = true;
            }

        }
    }
    file.close();

    if (userFound && passFound)
    {
        this->lookup = "Username: " + username;
        (*server).Send("Correct");
        home();
        return;
    }
    if (!userFound && !passFound)
        (*server).Send("Incorrect-User");

    if (userFound && !passFound)
        (*server).Send("Incorrect-Pass");

    login(fileName);
}

void Person::profile(const string& fileName, const string& Lookup)
{
    ifstream file(fileName); // Opens the file
    if (!file.is_open()) // Ensuring that the file opened successfully
    {
        cout << "Sorry! An error occured while processing the data.\n";
        getch();
        (*server).endServer();
        return;
    }

    // Reading and displaying the required record
    string line;
    ostringstream record;
    while (!file.eof())
    {
        getline(file, line);
        if (line == Lookup)
        {
            while(true)
            {
                record << " --> " << line << endl;
                getline(file, line);
                if (line == "")
                    break;
            }
            break;
        }
    }
    (*server).Send(record.str());
}

bool Person::initialize_goods(Goods* goods) // Initializes array of goods from file
{
    ifstream file("goods.txt"); // Opening the file

    if (!file.is_open()) // Ensuring that the file opened correctly
        return false; // Indicates error

    string str;
    for(int i = 0; i < N; i++) // Populating array of goods
    {
        file >> str;
        file >> str;
        goods[i].name = str;

        file >> str;
        file >> str;
        goods[i].stock = stoi(str);

        file >> str;
        file >> str;
        goods[i].cost = stod(str);

        file >> str;
        file >> str;
        goods[i].price = stod(str);        

        file >> str;
        file >> str;
        file >> str;
        goods[i].reorder_level = stoi(str);
        
        file >> str;
        file >> str;
        file >> str;
        goods[i].reorder_amount = stoi(str);

        goods[i].bought = false;
    }
    file.close();
    return true; // Indicates successful termination
}

bool Person::initialize_balance(string fileName) // Initializes balance from file
{
    ifstream file(fileName); // Opening the file
    
    if (!file.is_open()) // Ensuring that the file opened correctly
        return false; // Indicates error

    string str;
    while (!file.eof()) // Intializing balance
    {
        getline(file, str);
        if (str == this->lookup)
        {
            while(getline(file, str))
            {
                if (str.substr(0, 7) == "Balance")
                {
                    this->balance = str.substr(9);
                    break;
                }
            }
            break;
        }
    }
    file.close();
    return true;
}

bool Person::initialize_cash(Cash& cash) // Initializes cash from file
{
    ifstream file("cash.txt"); // Opening the file

    if (!file.is_open()) // Ensuring that the file opened correctly
        return false; // Indicates error

    string str;

    file >> str;
    file >> str;
    file >> str;
    cash.initial = stod(str);
    
    file >> str;
    file >> str;
    file >> str;
    cash.in = stod(str);
    
    file >> str;
    file >> str;
    file >> str;
    cash.out = stod(str);
    
    file >> str;
    file >> str;
    file >> str;
    cash.finl = stod(str);

    file.close();
    return true; // Indicates successful termination
}

bool Person::update_goods(Goods* goods) // Updates goods.txt
{
    ofstream file("goods.txt", ios::trunc); // deletes old file's content
    
    if (!file.is_open()) // Ensuring that the file opened correctly
        return false; // Indicates error
    
    for (int i = 0; i < N; i++) // Writing new content to file
    {
        file << "Name: " << goods[i].name << endl 
            << "Stock: " << goods[i].stock << endl 
            << "Cost: " << goods[i].cost << endl 
            << "Price: " << goods[i].price << endl 
            << "Reorder Level: " << goods[i].reorder_level << endl 
            << "Reorder Amount: " << goods[i].reorder_amount << endl 
            << endl;
    }
    file.close();
    return true; // Indicates successful termination
}

bool Person::update_balance(string fileName) // Updates balance to file passed as argument
{
    ifstream old_file(fileName); // Opening the files
    ofstream new_file("tmp.txt");

    if (!old_file.is_open() || !new_file.is_open()) // Ensuring that the files opened correctly
        return false; // Indicates error

    string str;
    while (!old_file.eof())
    {
        getline(old_file, str);
        if (str == this->lookup)
        {
            new_file << str << endl;
            while(getline(old_file, str))
            {
                if (str.substr(0, 7) == "Balance")
                {
                    new_file << "Balance: " << stod(balance) << endl;
                    break;
                }
                new_file << str << endl;
            }
            continue;
        }
        new_file << str << endl;
    }

    old_file.close();
    new_file.close();
    remove(fileName.c_str());
    rename("tmp.txt", fileName.c_str());
    return true; // Indicates successful termination
}

bool Person::update_cash(Cash& cash) // Updates cash.txt
{
    ofstream file("cash.txt", ios::trunc); // Opens the file and deletes old file

    if (!file.is_open()) // Ensuring that the file opened correctly
        return false; // Indicates error
    
    file << "Initial cash: " << cash.initial << endl 
        << "Cash in: " << cash.in << endl 
        << "Cash out: " << cash.out << endl 
        << "Final cash: " << cash.finl;
    file.close();
    return true; // Indicates successful termination
}

void Person::buy(const string& fileName)
{
    Goods goods[N]; // Array of goods
    Cash cash;
    string str1, str2, str3;
    ostringstream ss;
    int itemNo, quantity, ans;
    bool go = true;
    double Balance;

    // #region agent log
    {
        std::ofstream log("C:\\Users\\shraddha\\OneDrive\\Desktop\\test\\debug-206870.log", std::ios::app);
        log << "{\"sessionId\":\"206870\",\"runId\":\"pre-fix-1\",\"hypothesisId\":\"H1\",\"location\":\"Person.cpp:buy:entry\",\"message\":\"enter buy\",\"data\":{\"fileName\":\"" << fileName << "\"},\"timestamp\":" << std::time(nullptr) << "}" << std::endl;
    }
    // #endregion

    // Initializing
    if (!initialize_goods(goods) || !initialize_balance(fileName) || !initialize_cash(cash))
    {
        cout << "Sorry! An error occured while processing the data.\n";
        getch();
        return;
    }

    while (go)
    {
        ss.str("");
        ss << left << setw(3) << " " << setw(15) << "Name" << "Price($)" << endl << endl;
        for (int i = 0; i < N; i++)
            ss << setw(3) << i + 1 << setw(15) << goods[i].name << goods[i].price  << " -- " << goods[i].stock << endl;
        ss << endl << "Your balance: $" << balance << right << endl;
        
        (*server).Send(ss.str());
        ss.str("");
        Balance = stod(this->balance);

        (*server).Rec(str1);
        ans = stoi(str1);

        // #region agent log
        {
            std::ofstream log("C:\\Users\\shraddha\\OneDrive\\Desktop\\test\\debug-206870.log", std::ios::app);
            log << "{\"sessionId\":\"206870\",\"runId\":\"pre-fix-1\",\"hypothesisId\":\"H2\",\"location\":\"Person.cpp:buy:switch\",\"message\":\"switch choice\",\"data\":{\"ans\":" << ans << "},\"timestamp\":" << std::time(nullptr) << "}" << std::endl;
        }
        // #endregion

        switch(ans)
        {
            case 1:
            {   
                (*server).Rec(str2);
                (*server).Rec(str3);

                itemNo = stoi(str2);
                quantity = stoi(str3);
    
                if (quantity > goods[itemNo - 1].stock)
                {
                    // #region agent log
                    {
                        std::ofstream log("C:\\Users\\shraddha\\OneDrive\\Desktop\\test\\debug-206870.log", std::ios::app);
                        log << "{\"sessionId\":\"206870\",\"runId\":\"pre-fix-1\",\"hypothesisId\":\"H3\",\"location\":\"Person.cpp:buy:case1\",\"message\":\"not enough stock\""
                            << ",\"data\":{\"itemNo\":" << itemNo << ",\"quantity\":" << quantity << "},\"timestamp\":" << std::time(nullptr) << "}" << std::endl;
                    }
                    // #endregion
                    (*server).Send("NOT-ENOUGH");
                }
                else
                {
                    (*server).Send("ENOUGH");
                    goods[itemNo - 1].bought = true;
                    goods[itemNo - 1].quantity = quantity;
                }
                break;
            }

            case 2:
            {
                bool order = false; // false if no order has been placed yet, true otherwise
                double ordercost = 0; // stores the cost of the order

                for (int i = 0; i < N; i++)
                {
                    if (goods[i].bought)
                    {
                        order = true;
                        if (fileName == "customer.txt")
                            ordercost += goods[i].price * goods[i].quantity;
                        else
                            ordercost += goods[i].price * goods[i].quantity * 0.80; // employees and admin avail a 20% discount
                    }
                }

                ss.str("");
                if (!order)
                {
                    // #region agent log
                    {
                        std::ofstream log("C:\\Users\\shraddha\\OneDrive\\Desktop\\test\\debug-206870.log", std::ios::app);
                        log << "{\"sessionId\":\"206870\",\"runId\":\"pre-fix-1\",\"hypothesisId\":\"H4\",\"location\":\"Person.cpp:buy:case2\",\"message\":\"no order placed\",\"data\":{},\"timestamp\":" << std::time(nullptr) << "}" << std::endl;
                    }
                    // #endregion
                    ss << "NO-ORDER";
                }
                else
                {
                    if (ordercost > Balance) // Display error if cost exceeds balance
                    {
                        // #region agent log
                        {
                            std::ofstream log("C:\\Users\\shraddha\\OneDrive\\Desktop\\test\\debug-206870.log", std::ios::app);
                            log << "{\"sessionId\":\"206870\",\"runId\":\"pre-fix-1\",\"hypothesisId\":\"H5\",\"location\":\"Person.cpp:buy:case2\",\"message\":\"ordercost exceeds balance\""
                                << ",\"data\":{\"ordercost\":" << ordercost << ",\"balance\":" << Balance << "},\"timestamp\":" << std::time(nullptr) << "}" << std::endl;
                        }
                        // #endregion
                        ss  << "Your balance: $" << balance << endl 
                            << "Cost of the order: $" << ordercost << endl;
                    }
                    else
                    {
                        // #region agent log
                        {
                            std::ofstream log("C:\\Users\\shraddha\\OneDrive\\Desktop\\test\\debug-206870.log", std::ios::app);
                            log << "{\"sessionId\":\"206870\",\"runId\":\"pre-fix-1\",\"hypothesisId\":\"H6\",\"location\":\"Person.cpp:buy:case2\",\"message\":\"successful order and potential reorder\""
                                << ",\"data\":{\"ordercost\":" << ordercost << ",\"balance\":" << Balance << "},\"timestamp\":" << std::time(nullptr) << "}" << std::endl;
                        }
                        // #endregion

                        Balance -= ordercost; // Deducting balance
                        this->balance = to_string(Balance);
                        cash + ordercost; // Increasing cash

                        update_cash(cash);
                        initialize_cash(cash);
                        // DO ERROR HANDLING

                        // Automatic reorder logic
                        for (int i = 0; i < N; i++)
                        {
                            if (goods[i].bought)
                            {
                                goods[i].stock -= goods[i].quantity;

                                if ((goods[i].stock <= goods[i].reorder_level) && ((goods[i].cost * goods[i].reorder_amount) <= cash.get_final_cash()))
                                {
                                    goods[i].stock += goods[i].reorder_amount; // Stock increases
                                    cash - (goods[i].cost * goods[i].reorder_amount); // Cash decrease
                                    
                                    update_cash(cash);
                                    initialize_cash(cash);
                                    // DO ERROR HANDLING
                                }
                            }
                        }
                        update_balance(fileName);
                        update_goods(goods);
                        // DO ERROR HANDLING
                        
                        ss << "Cost of the order: $" << ordercost << endl 
                            << "Your new balance: $" << balance << endl;
                    }
                }
            }
            (*server).Send(ss.str());

            // clearing all orders:
            for (int i = 0; i < N; i++)
                goods[i].bought = false;
            break;       
            
            case 3:
                go = false;
                break;
        }
    }
}

