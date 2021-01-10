//
// Created by talha on 01/01/2021.
//

#ifndef BOOST_ECHO_CLIENT_USER_H
#define BOOST_ECHO_CLIENT_USER_H

#include <string>
#include <thread>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <condition_variable>
#include <boost/asio/ip/tcp.hpp>
#include <connectionHandler.h>


using namespace std;
class Client {

public:

    static short OpCode(string opCode) {
        short s;
        if (opCode == "ADMINREG") {
            s = 1;

        } else if (opCode == "STUDENTREG") {
            s = 2;
        } else if (opCode == "LOGIN") {
            s = 3;
        } else if (opCode == "LOGOUT") {
            s = 4;
        } else if (opCode == "COURSEREG") {
            s = 5;
        } else if (opCode == "KDAMCHECK") {
            s = 6;
        } else if (opCode == "COURSESTAT") {
            s = 7;
        } else if (opCode == "STUDENTSTAT") {
            s = 8;
        } else if (opCode == "ISREGISTERED") {
            s = 9;
        } else if (opCode == "UNREGISTER") {
            s = 14;
        } else if (opCode == "MYCOURSES") {
            s = 11;
        }
        return s;
    }
    static string updateOutPut(string output, short opCode) {
            if (opCode == 1)
                output = output+" "+ "1"+" ";
            if (opCode == 2)
                output = output+" "+ "2"+" ";
            if (opCode == 3)
                output = output+" "+ "3"+" ";
            if (opCode == 4)
                output = output+" "+ "4"+" ";
            if (opCode == 5)
                output = output+" "+ "5"+" ";
            if (opCode == 6)
                output = output+" "+ "6"+" ";
            if (opCode == 7)
                output = output+" "+ "7"+" ";
            if (opCode == 8)
                output = output+" "+ "8"+" ";
            if (opCode == 9)
                output = output+" "+ "9"+" ";
            if (opCode == 14)
                output = output+" "+ "10"+" ";
            if (opCode == 11)
                output = output+" "+ "11"+" ";
            return output;
        }

};


//--------------------------------------Read Class--------------------------------
    class TaskRead {
    private:
        ConnectionHandler &connectionHandler;

    public:
        TaskRead(ConnectionHandler &connectionHandler) :
                connectionHandler(connectionHandler) {}

        void operator()() {
            while (!connectionHandler.getTerminate()) {
                bool flag=false;
            
                string msgtoOutput;
                char AckOrErrorBytes[4];
                char AckOrError[2];
                char opCode[2];
                char msg[300];
                for (int j=0;j<300;j++)
                    msg[j]=0;
                // char OPCodeBytes[2];
                connectionHandler.getBytes(AckOrErrorBytes, 4); //getting err\ack and original op code
                AckOrError[0] = AckOrErrorBytes[0];
                AckOrError[1] = AckOrErrorBytes[1];
                opCode[0] = AckOrErrorBytes[2];
                opCode[1] = AckOrErrorBytes[3];

                short opCodes = connectionHandler.bytesToShort(AckOrError);
                short originalOP = connectionHandler.bytesToShort(opCode);
                char nextByte[1];
                int i=0;
                connectionHandler.getBytes(nextByte, 1);
                while (nextByte[0] != '\0') {
                    msg[i] =nextByte[0];
                    i++;
                    connectionHandler.getBytes(nextByte, 1);
                }
                char newMsg[i];
                for (int j=0;j<i;j++)
                    newMsg[j]=0;
                for (int j=0;j<i;j++)
                    newMsg[j]=msg[i];

                string endmsg (newMsg);

                if (opCodes == 12){
                    msgtoOutput = "ACK";
                    if (originalOP == 3)
                        connectionHandler.setLogIN(true);
                    if (originalOP == 4) {
                        connectionHandler.setReadAs(true);
                        connectionHandler.setWriteAs(true);
                        connectionHandler.setTerminate(true);
                        msgtoOutput=Client::updateOutPut(msgtoOutput,originalOP);
                        cout<<msgtoOutput<<" "<<msg<<endl;
                        return;
                    }
                    if ((originalOP == 7)|(originalOP == 8)) {
                        flag=true;// PRINT HERE
                        vector<string> newMsg;
                        msgtoOutput=Client::updateOutPut(msgtoOutput,originalOP);
                        cout<<msgtoOutput<<endl; // OUT PRINT OP CODES
                        boost::split(newMsg, msg, boost::is_any_of("&"));
			int size1 = newMsg.size();
                        for (int i=0;i<size1;i++)
                            cout<<newMsg[i]<<endl; // OUT PRINT THE MSG
                    }
                }
                else if (opCodes==13)
                    msgtoOutput = "ERROR";
                connectionHandler.setReadAs(false);
                if (!flag) {
                    msgtoOutput = Client::updateOutPut(msgtoOutput, originalOP);
                    cout << msgtoOutput << " " << msg << endl;
                }
            }
        }
    };



    //--------------------------------------Main Class--------------------------------
    int main(int argc, char *argv[]) {
        if (argc < 3) {
            std::cerr << "Usage: " << argv[0] << " host port" << std::endl << std::endl;
            return -1;
        }
        std::string host = argv[1];
        short port = atoi(argv[2]);
        ConnectionHandler *connectionHandler=new ConnectionHandler(host, port);
        bool Connected = connectionHandler->connect();
        if (!Connected) {
            std::cerr << "Cannot connect to " << host << ":" << port << std::endl;
            return 1;
        }


        TaskRead taskRead(*connectionHandler); //Start read from server
        std::thread thRead(std::ref(taskRead)); // RUN ! RUN ! RUN !
	// assuming users can't register on the same terminal
        while ((!connectionHandler->getTerminate())&&(!connectionHandler->getIsWriteable())) {

        //Reading line from the command:
            const short bufsize = 1024;
            char buf[bufsize];

            //Checking signal
            if (connectionHandler->getIsWriteable())
                break;
            std::cin.getline(buf, bufsize);
            std::string line(buf); // new string from the argument buf .
            vector<string> lineVector;
            bool x;
            x = (line.find(" ") != std::string::npos); // More then 1 word
            if (x != 0) {
                boost::split(lineVector, line, boost::is_any_of(" "));
                short opCode = Client::OpCode(lineVector[0]);
                short sizeOfSending;

                if ((opCode == 1) | (opCode == 2) | (opCode == 3)) {
                    std::vector<char> bytes1(lineVector[1].begin(), lineVector[1].end());//USER-NAME
                    std::vector<char> bytes2(lineVector[2].begin(), lineVector[2].end());//PASSWORD
                    sizeOfSending = 2 + bytes1.size() + 1 + bytes2.size() + 2;
                    char bytesToSend[sizeOfSending];
                    for (int i=0;i<sizeOfSending;i++)
                        bytesToSend[i]=0; // CLEANING
                    connectionHandler->shortToBytes(opCode, bytesToSend, 0);
		int size2 = bytes1.size();
                    for (int i = 0; i < size2; i++)
                        bytesToSend[2 + i] = bytes1[i]; // Set our buffer !
                    bytesToSend[2 + bytes1.size()] = '\0';
		int size3 = bytes2.size();
                    for (int j = 0; j < size3; j++)
                        bytesToSend[2 + bytes1.size() + 1 + j] = bytes2[j];// Continue set our buffer !
                    bytesToSend[2 + bytes1.size() + 1 + bytes2.size()] = '\0';
                    bytesToSend[2 + bytes1.size() + 1 + bytes2.size() + 1] = '\n';
                    connectionHandler->sendBytes(bytesToSend, sizeOfSending);
                }

                if ((opCode == 5) | (opCode == 6) | (opCode == 7) | (opCode == 9)| (opCode == 14)) {

                    sizeOfSending = 5;
                    char bytesToSend[sizeOfSending];
                    connectionHandler->shortToBytes(opCode, bytesToSend, 0);
                    short opCodes = boost::lexical_cast<short>(lineVector[1]);
                    connectionHandler->shortToBytes(opCodes, bytesToSend, 2);
                    bytesToSend[4] = '\n';
                    connectionHandler->sendBytes(bytesToSend, sizeOfSending);
                }
                if (opCode == 8) {
                    //       opCode+String+"\0"+"\n".
                    std::vector<char> bytes3(lineVector[1].begin(), lineVector[1].end());
                    sizeOfSending = 2 + bytes3.size() + 1 + 1;
                    char bytesToSend[sizeOfSending];
                    for (int i=0;i<sizeOfSending;i++)
                        bytesToSend[i]=0;
                    connectionHandler->shortToBytes(opCode, bytesToSend, 0);
		int size4 = bytes3.size();
                    for (int i = 0; i < size4; i++)
                        bytesToSend[2 + i] = bytes3[i];
                    bytesToSend[2 + bytes3.size()] = '\0';
                    bytesToSend[2 + bytes3.size() + 1 ] = '\n';
                    connectionHandler->sendBytes(bytesToSend, sizeOfSending);
                }
                connectionHandler->setReadAs(true);
            }
            else
            {     //MYCOURSES / LOGOUT

                short opCode = Client::OpCode(line);

                if ((opCode == 4) | (opCode == 11)) {
                    //                 opCode
                    if (connectionHandler->getLogIN()&&opCode==4) // Assumption that LOGOUT will get ack if we are logged in.
                        connectionHandler->setTerminate(true);
                    int sizeOfSending = 3;
                    char bytesToSend[sizeOfSending];
                    for (int i=0;i<sizeOfSending;i++)
                        bytesToSend[i]=0;
                    connectionHandler->shortToBytes(opCode, bytesToSend, 0);
                    bytesToSend[2] = '\n';
                    connectionHandler->sendBytes(bytesToSend, sizeOfSending);
                }
            }
        }
        cout<< "CLOSE!" <<endl;

        thRead.join();
        connectionHandler->close();
        delete connectionHandler;
        return 0;
    }


#endif //BOOST_ECHO_CLIENT_USER_H
