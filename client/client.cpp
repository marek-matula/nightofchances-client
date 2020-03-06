#include <iostream>
#include <windows.h> 
#include <vector>
#include <string>
#include <json/json.h>

#include "ClientConnection.h"

void JsonSample()
{
    Json::CharReaderBuilder builder;
    Json::CharReader* reader = builder.newCharReader();

    std::string input = R"({"Event": "Night of Chances", "Year": 2020})";
    Json::Value output;
    std::string errors;

    if (!reader->parse(input.c_str(), input.c_str() + input.length(), &output, &errors)) {
        return;
    }

    std::cout << "Event: " << output["Event"].asString() << "\n";
    std::cout << "Year:  " << output["Year"].asInt() << "\n";
}

int main()
{
    JsonSample();

    ClientConnection connection;
    
    if (connection.ConnectToServer() == false)
        return 1;    

    std::string tmpStr = "Test message from client.";
    std::vector<char> inputBuffer(tmpStr.begin(), tmpStr.end());
    std::vector<char> reply = connection.SendAndReceive(inputBuffer);
    std::cout << reply.data() << "\n";

    return 0;
}