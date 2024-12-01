#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <fstream> 
#include <algorithm>
#include <chrono>
#include <iomanip>

using namespace std;

// Function to read orders from CSV file 
vector<vector<string>> Read_Orders_From_CSV(const string &filename)
{
    vector<vector<string>> inputOrders;
    ifstream file(filename);
    string line, word;

    // Check if the file is opened successfully
    if (file.is_open())
    {
        // Skip the header if present
        bool firstLine = true;
        
        while (getline(file, line))
        {
            // Skip the first line (header)
            if (firstLine)
            {
                firstLine = false;
                continue;
            }

            vector<string> row;
            stringstream s(line);

            // Extract individual values separated by commas
            while (getline(s, word, ','))
            {
                row.push_back(word);
            }

            // Push each row into the inputOrders vector
            inputOrders.push_back(row);
        }

        file.close();
    }
    else
    {
        cout << "Unable to open file." << endl;
    }

    return inputOrders;
}


struct Order
{
    // Input fields
    string Client_Order_ID;
    string Instrument_Name;
    int Side;
    double Price;
    int Quantity;

    // Output specific fields
    string Order_ID;
    int Status;
    string Reason;

    // Other necessary attributes
    int Remaining_Quantity;
    static int Current_Order_ID;

    string Instrument_List[5] = {"Rose", "Lavender", "Lotus", "Tulip", "Orchid"};

    // Constructors
    Order() {}
    Order(string _Client_Order_ID, string _Instrument_Name, string _Side, string _Quantity, string _Price)
    {
        Client_Order_ID = _Client_Order_ID;
        Order_ID = "ord" + to_string(Current_Order_ID++); // Sequential order ID
        Instrument_Name = _Instrument_Name;
        Side = stoi(_Side);
        Price = stod(_Price);
        Quantity = stoi(_Quantity);

        Check_Order_Validity();
        Remaining_Quantity = Quantity;
    }

    // Check order validity and set Status & Reason.
    void Check_Order_Validity()
    {
        if (Client_Order_ID.empty() || Instrument_Name.empty() || to_string(Side).empty() || to_string(Price).empty() || to_string(Quantity).empty())
        {
            Status = 1;
            Reason = "Invalid Fields";
            return;
        }
        if (find(begin(Instrument_List), end(Instrument_List), Instrument_Name) == end(Instrument_List))
        {
            Status = 1;
            Reason = "Invalid Instrument";
            return;
        }
        if (!(Side == 1 || Side == 2))
        {
            Status = 1;
            Reason = "Invalid Side";
            return;
        }
        if (!(Price > 0.0))
        {
            Status = 1;
            Reason = "Invalid Price";
            return;
        }
        if (Quantity % 10 != 0 || Quantity < 10 || Quantity > 1000)
        {
            Status = 1;
            Reason = "Invalid Size";
            return;
        }
        Status = 0;
        Reason = "Accepted";
    }

    // Check if the order is not rejected
    bool Check_If_Not_Rejected()
    {
        return Status == 0;
    }

    // Write order execution details to CSV file
    void execution_rep(int Exec_Quantity, double matched_price, ofstream &outputFile)
    {
        // Get the current time
        auto current_time = std::chrono::system_clock::now();
        
        // Convert it to time_t to extract year, month, day
        std::time_t time_t_now = std::chrono::system_clock::to_time_t(current_time);
        auto ms_part = std::chrono::duration_cast<std::chrono::milliseconds>(current_time.time_since_epoch()) % 1000;
        
        // Format the time to the YYYYMMDD-HHMMSS.sss format
        std::tm* tm_struct = std::localtime(&time_t_now);
        std::stringstream time_ss;
        time_ss << std::put_time(tm_struct, "%Y%m%d-%H%M%S") << '.' << std::setw(3) << std::setfill('0') << ms_part.count();

        string Status_String = (Status == 0) ? "New" : (Status == 1) ? "Reject" : (Status == 2) ? "Fill" : "PFill";

        // Output the matched price instead of the order's price
        outputFile << Order_ID << "," << Client_Order_ID << "," << Instrument_Name << "," << Side
             << "," << Status_String << "," << Exec_Quantity << "," << matched_price << "," << Reason << "," << time_ss.str() << "\n";
    }

    // Execute order, writing to CSV
    void Execute(int Exec_Quantity, double Price, ofstream &outputFile)
    {
        execution_rep(Exec_Quantity, Price, outputFile);
    }
};

int Order::Current_Order_ID = 1;

// Insert new order into order book based on price priority
void Insert_Order_into_OB(vector<Order> &Order_Book_Arg, Order New_Order, int Side)
{
    auto it = begin(Order_Book_Arg);
    while (it != end(Order_Book_Arg))
    {
        if ((it->Price < New_Order.Price && Side == 1) || (it->Price > New_Order.Price && Side == 2))
        {
            break;
        }
        ++it;
    }
    if (it == end(Order_Book_Arg))
    {
        Order_Book_Arg.push_back(New_Order);
    }
    else
    {
        Order_Book_Arg.insert(it, New_Order);
    }
}

// Process orders and print execution results to CSV
void Process_Orders_and_Update_Books(vector<vector<string>> inputOrders)
{
    // Setup output file for writing
    ofstream outputFile("test3.csv");
    outputFile << "Order ID,Client Order ID,Instrument,Side,Exec Status,Quantity,Price,Reason,Transaction Time\n"; // CSV header

    vector<vector<Order>> Order_Book_Library[5] = {
        vector<vector<Order>>(2), vector<vector<Order>>(2), vector<vector<Order>>(2),
        vector<vector<Order>>(2), vector<vector<Order>>(2)};
    string Instrument_List[5] = {"Rose", "Lavender", "Lotus", "Tulip", "Orchid"};

    for (const auto &row : inputOrders)
    {
        Order New_Order(row[0], row[1], row[2], row[3], row[4]);

        if (New_Order.Check_If_Not_Rejected())
        {
            int Order_Book_Index = (int)(find(begin(Instrument_List), end(Instrument_List), New_Order.Instrument_Name) - begin(Instrument_List));
            vector<vector<Order>> Order_Book = Order_Book_Library[Order_Book_Index];

            if (New_Order.Side == 1) // Buy side processing
            {
                while (!(Order_Book[1].empty()) && (Order_Book[1][0].Price <= New_Order.Price))
                {
                    if (New_Order.Remaining_Quantity == Order_Book[1][0].Remaining_Quantity)
                    {
                        New_Order.Status = 2;
                        Order_Book[1][0].Status = 2;
                        New_Order.Execute(New_Order.Remaining_Quantity, Order_Book[1][0].Price, outputFile);
                        Order_Book[1][0].Execute(New_Order.Remaining_Quantity, Order_Book[1][0].Price, outputFile);
                        Order_Book[1].erase(begin(Order_Book[1]));
                        break;
                    }
                    else if (New_Order.Remaining_Quantity > Order_Book[1][0].Remaining_Quantity)
                    {
                        New_Order.Status = 3;
                        Order_Book[1][0].Status = 2;
                        New_Order.Execute(Order_Book[1][0].Remaining_Quantity, Order_Book[1][0].Price, outputFile);
                        Order_Book[1][0].Execute(Order_Book[1][0].Remaining_Quantity, Order_Book[1][0].Price, outputFile);
                        New_Order.Remaining_Quantity -= Order_Book[1][0].Remaining_Quantity;
                        Order_Book[1].erase(begin(Order_Book[1]));
                    }
                    else
                    {
                        New_Order.Status = 2;
                        Order_Book[1][0].Status = 3;
                        New_Order.Execute(Order_Book[1][0].Remaining_Quantity, Order_Book[1][0].Price, outputFile);
                        Order_Book[1][0].Execute(New_Order.Remaining_Quantity, Order_Book[1][0].Price, outputFile);
                        Order_Book[1][0].Remaining_Quantity -= New_Order.Remaining_Quantity;
                        break;
                    }
                }

                if (New_Order.Status == 0)
                {
                    New_Order.Execute(New_Order.Quantity, New_Order.Price, outputFile);
                }

                if (New_Order.Remaining_Quantity > 0)
                {
                    Insert_Order_into_OB(Order_Book[0], New_Order, 1);
                }
            }
            else if (New_Order.Side == 2) // Sell side processing
            {
                while (!(Order_Book[0].empty()) && (Order_Book[0][0].Price >= New_Order.Price))
                {
                    if (New_Order.Remaining_Quantity == Order_Book[0][0].Remaining_Quantity)
                    {
                        New_Order.Status = 2;
                        Order_Book[0][0].Status = 2;
                        New_Order.Execute(Order_Book[0][0].Remaining_Quantity,Order_Book[0][0].Price, outputFile);
                        Order_Book[0][0].Execute(Order_Book[0][0].Remaining_Quantity,Order_Book[0][0].Price, outputFile);
                        Order_Book[0].erase(begin(Order_Book[0]));
                        break;
                    }
                    else if (New_Order.Remaining_Quantity > Order_Book[0][0].Remaining_Quantity)
                    {
                        New_Order.Status = 3;
                        Order_Book[0][0].Status = 2;
                        New_Order.Execute(Order_Book[0][0].Remaining_Quantity,Order_Book[0][0].Price, outputFile);
                        New_Order.Remaining_Quantity -= Order_Book[0][0].Remaining_Quantity;
                        Order_Book[0][0].Execute(Order_Book[0][0].Remaining_Quantity,Order_Book[0][0].Price, outputFile);
                        Order_Book[0].erase(begin(Order_Book[0]));
                    }
                    else
                    {
                        New_Order.Status = 2;
                        Order_Book[0][0].Status = 3;
                        Order_Book[0][0].Remaining_Quantity -= New_Order.Remaining_Quantity;
                        New_Order.Execute(Order_Book[0][0].Remaining_Quantity,Order_Book[0][0].Price, outputFile);
                        Order_Book[0][0].Execute(New_Order.Remaining_Quantity,Order_Book[0][0].Price, outputFile);
        
                        break;
                    }
                }

                if (New_Order.Status == 0)
                {
                    New_Order.Execute(New_Order.Quantity, New_Order.Price, outputFile);
                }

                if (New_Order.Remaining_Quantity > 0)
                {
                    Insert_Order_into_OB(Order_Book[1], New_Order, 2);
                }
            }

            Order_Book_Library[Order_Book_Index] = Order_Book;
        }
        else
        {
            New_Order.Execute(New_Order.Quantity, New_Order.Price, outputFile);
        }
    }

    outputFile.close();
}

int main()
{
    // Measure the start time
    auto start = std::chrono::high_resolution_clock::now();

    // Read orders from a CSV file
    string inputFilename = "ex3_input_two_orderbooks_5.csv";
    vector<vector<string>> inputOrders = Read_Orders_From_CSV(inputFilename);

    Process_Orders_and_Update_Books(inputOrders);
    
    // Measure the end time
    auto end = std::chrono::high_resolution_clock::now();
    
    // Calculate the execution time in milliseconds
    auto executionTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    cout << "Execution time: " << executionTime << endl;

    return 0;
}



    