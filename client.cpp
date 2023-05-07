#include <stdio.h>
#include "networkstructures.h"
#include <stdlib.h>
#include <string.h>
#include "fileutils.h"
#include <thread>
#include <mutex>
#include <functional>
#include <iostream>
#include "setNetwork.h"
#include "userId.h"
#include <fstream>
#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <ctime>

int CHUNKSIZE = 4096;

using namespace std;
using namespace std::chrono;

std::mutex mtx;

struct Buffet
{
    string buffer;
    int size;
    ~Buffet()
    {
        // delete[] buffer;
    }
};

void combine_files(string fname, int num)
{
    string name = "output/" + fname;
    ofstream file(name, ios::binary);
    for (int i = 0; i < num; i++)
    {
        string fname1 = fname;
        fname1 = "output/"+ to_string(i) +fname1;
        ifstream file1(fname1, ios::binary);
        file << file1.rdbuf();
        // remove(fname1.c_str());
        file1.close();
    }
    file.close();
}

bool is_file_empty(std::fstream &pFile)
{
    return pFile.peek() == std::ifstream::traits_type::eof();
}

void listenLoop(int socket_id, int port)
{
    FILE *fp = fopen("abc.jpg", "rb");

    while (1)
    {
        struct sockaddr_in *client_address;
        int len = sizeof(sockaddr_in);
        int client_socket = accept(socket_id, (struct sockaddr *)&client_address, (socklen_t *)&len);
        printf("CONNECTED\n");
        int f; // temp, used to see which part to recieve, first six or last six
        // read(client_socket, &f, sizeof(f));
        printf("BEGINNING WRITE\n");
        if (port)
        {
            cout << "here1" << endl;

            // final size and name
            int fileNameSize = 0;
            char filename[100] = {0};
            int a1 = read(client_socket, &fileNameSize, sizeof(fileNameSize));
            cout << "a1 value: " << a1 << endl;
            if (a1 == -1)
            {
                std::cerr << "Read error: " << strerror(errno) << std::endl;
                // handle the error
            }
            cout << "filename size: " << fileNameSize << endl;

            int a2 = read(client_socket, filename, fileNameSize);
            cout << "a2 value: " << a2 << endl;
            cout << "filename: " << filename << endl;

            // parts
            float parts = 0;
            read(client_socket, &parts, sizeof(float));
            cout << "parts value: " << parts << endl;

            // position i value
            int pos = 0;
            read(client_socket, &pos, sizeof(pos));
            cout << "position value: " << pos << endl;

            // i max value, total number
            int imax = 0;
            read(client_socket, &imax, sizeof(imax));
            cout << "imax value: " << imax << endl;

            std::ifstream file(filename, std::ios::binary);
            if (!file.is_open())
            {
                printf("Failed to open file.\n");
                return;
            }

            // Get the file size
            file.seekg(0, std::ios::end);
            unsigned long long int size = file.tellg();
            file.seekg(0, std::ios::beg);

            unsigned long long int start = 0;
            double diff = (double)size * parts;

            for (int i = 0; i < pos; i++)
            {
                start += (unsigned long long int)diff;
            }
            start = start + 1;

            cout << "pos value: " << pos << endl;
            cout << "start value: " << start << endl;
            cout << "diff value: " << diff << endl;
            cout << "size value: " << size << endl;
            unsigned long long int buffersize = 0;
            if (imax != pos + 1)
            {
                buffersize = (unsigned long long int)diff;
            }
            else
            {
                buffersize = size - start + 1;
            }

            cout << "buffer size: " << buffersize << endl;

            // Allocate a buffer to hold the section
            char *buffer = new char[buffersize];
            if (buffer == NULL)
            {
                printf("Failed to allocate memory for file buffer.\n");
                file.close();
                return;
            }

            // Read the file into the buffer
            if (start == 0)
            {
                file.seekg(start, std::ios::beg); // check this -1
            }
            else
            {
                file.seekg(start - 1, std::ios::beg); // check this -1
            }
            // file.seekg(start, std::ios::beg); //check this -1

            cout << "here1" << endl;

            file.read(buffer, buffersize);
            if (!file)
            {
                printf("Failed to read file into buffer.\n");
                delete[] buffer;
                file.close();
                return;
            }

            // Send the file segment size over the socket
            int err = send(client_socket, &buffersize, sizeof(buffersize), 0);
            if (err < 0)
            {
                printf("Failed to send file segment size.\n");
                delete[] buffer;
                file.close();
                return;
            }
            cout << "size sent: " << buffersize << endl;

            int i = 0;
            while (i < buffersize)
            {
                const int l = send(client_socket, &buffer[i], std::min((unsigned long long int)CHUNKSIZE, buffersize - i), 0);
                if (l < 0)
                {
                    return;
                }
                i += l;
            }

            // Send the file buffer over the socket
            // err = send(client_socket, buffer, size, 0);
            // if (err < 0) {
            //     printf("Failed to send file.\n");
            //     delete[] buffer;
            //     file.close();
            //     return;
            // }

            printf("File segment sent successfully.\n");

            // Clean up
            delete[] buffer;
            file.close();
        }
        else
        {
            cout << "in the else, meaning port error\n";
        }
        cout << "here3" << endl;
    }
    fclose(fp);
    cout << "here4" << endl;
}

void background_listen(int port)
{

    int socket_id = setUpNetwork(port);
    listenLoop(socket_id, port);
}

vector<sockaddr_in> readVector(int sock)
{
    int size;
    read(sock, &size, sizeof(int));
    vector<sockaddr_in> out(size);
    for (int i = 0; i < size; i++)
    {
        read(sock, &out[i], sizeof(sockaddr));
    }

    return out;
}

void recieve_section(sockaddr_in address, map<int, Buffet> &sections, float parts, string filename, int pos, int client_size)
{
    //mtx.lock();
    int socket_id = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(socket_id, (sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("RECIEVE ERROR");
    };

    // inform the client
    char *fname = (char *)filename.c_str();
    int namesize = strlen(fname);
    write(socket_id, &namesize, sizeof(namesize));
    cout << "here name size: " << namesize << endl;
    write(socket_id, fname, namesize);
    cout << "here name: " << fname << endl;
    cout << "parts: " << parts << endl;

    send(socket_id, &parts, sizeof(parts), 0);
    send(socket_id, &pos, sizeof(pos), 0);
    send(socket_id, &client_size, sizeof(client_size), 0);

    std::cout << "READING: " << socket_id << std::endl;

    // file segment size
    unsigned long long int filesize = 0;
    int err = recv(socket_id, (char *)&filesize, sizeof(filesize), 0);
    if (err <= 0)
    {
        cout << "recv: " << err << endl;
    }
    cout << "recv bytes [OK]: " << err << endl;
    cout << "received size: " << filesize << endl;

    // getting the file content
    char *buffer = new char[filesize];
    memset(buffer, 0, filesize);
    int i = 0;
    while (i < filesize)
    {
        const int l = recv(socket_id, &buffer[i], std::min((unsigned long long)CHUNKSIZE, filesize - i), 0);
        if (l < 0)
        {
            return;
        } // this is an error
        i += l;
    }

    // add the file segment data to the vector
    // sections.push_back(buffer);
    Buffet temp;
    temp.buffer = string(buffer);
    temp.size = filesize;

    sections.insert(make_pair(pos, temp));

    string fname1 = filename;
    fname1 = "output/" + to_string(pos) + fname1;

    ofstream file(fname1, std::ios::binary | std::ios::app);
    file.write(buffer, filesize);
    file.close();

    delete[] buffer;

    //mtx.unlock();
}

int sendID(int socket_id, char *ID)
{
    int ID_written = write(socket_id, ID, strlen(ID));
    return ID_written;
}

int sendHandshake(int socket_id, char *ID, int32_t port)
{
    int written = write(socket_id, &port, sizeof(port));
    written += sendID(socket_id, ID);
    return written;
}

void sendFileInfo(int socket_id, char *ID, char *file_name)
{
    // uint32_t dataLength = htonl(file_name.size());
    // cout<<dataLength<<endl;
    // if (send(socket_id, &dataLength, sizeof(uint32_t), 0) == -1) {
    //     std::cerr << "Failed to send the file size" << std::endl;
    // }
    int size = strlen(file_name);
    write(socket_id, &size, sizeof(size));
    // cout<<"Sending file name: "<<file_name<<endl;
    if (send(socket_id, file_name, strlen(file_name), 0) == -1)
    {
        std::cerr << "Failed to send the file name" << std::endl;
    }

    // write(socket_id, &file_id, sizeof(file_id));
    sendID(socket_id, ID);
}

void requestForFile(int socket_id, char *id, int32_t file_id)
{
    // vector<char *> recieved;
    map<int, Buffet> rec;
    // int32_t file_id = 1;
    int w = write(socket_id, &file_id, sizeof(file_id));

    sendID(socket_id, id);

    // parts decimal value
    float parts = 0;
    read(socket_id, &parts, sizeof(float));
    cout << "parts value: " << parts << endl;

    // final size and name
    int fileNameSize = 0;
    char filename[100] = {0};
    read(socket_id, &fileNameSize, sizeof(fileNameSize));
    cout << "filename size: " << fileNameSize << endl;

    read(socket_id, filename, fileNameSize);
    cout << "filename: " << filename << endl;

    FileRequestResponse response;
    response.clients = readVector(socket_id);
    // int ports[] = {2012, 2014};
    vector<thread> recieve_threads;
    printf("Receiving from Port %d", htons(response.clients[0].sin_port));
    printf("\nSize: %ld\n", response.clients.size());
    int client_size = response.clients.size();

    auto start = high_resolution_clock::now();
    for (int i = 0; i < response.clients.size(); i++)
    {
        // recieve_threads.push_back(std::thread(recieve_section, response.clients[i], std::ref(recieved),
        //                                       parts, filename, i, client_size));
        recieve_threads.push_back(std::thread(recieve_section, response.clients[i], std::ref(rec),
                                              parts, filename, i, client_size));
        // recieve_threads.push_back(recieve);
    }

    for (int i = 0; i < recieve_threads.size(); i++)
    {
        recieve_threads[i].join();
    }
    cout << "RECEIVED ALL SECTIONS\n";
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);

    cout << "Time taken by function: "
         << duration.count() << " microseconds" << endl;

    // Add time to results table
    fstream csv = fstream("results/timer.csv", ios::out | ios::app);
    // if(is_file_empty(csv)){
    //     cout<<"File is empty\n";
    //     string header = "File Name, Number of Seeders, Time Taken (microseconds)\n";
    //     csv.write(header.c_str(), header.size());
    // }
    // FILE *fp = fopen(filename, "wb");

    csv << filename << "," << to_string(client_size) << "," << duration.count() << "\n";
    csv.close();
    // fclose(fp);

    // for(int i = 0; i<client_size; i++){
    //     cout<<"Section Size : "<<rec[i].size<<endl;
    //     string fname1 = filename;
    //     fname1 = "output/" + fname1+to_string(i);
    //     cout<<rec[i].size<<endl;
    //     ofstream file(fname1, std::ios::binary | std::ios::app);
    //     // file.write((rec[i].buffer).c_str(), rec[i].size);
    //     file.close();
    // }

    combine_files(filename, client_size);

    // string fname1 = filename;
    // fname1 = "output/" + fname1;

    // ofstream file(fname1, std::ios::binary | std::ios::app);
    // file.write(buffer, filesize);
    // file.close();

    // if (!file)
    // {
    //     cout << "File not opened" << endl;
    // }
    // else
    // {

    // for (auto i = rec.begin(); i != rec.end(); i++)
    // {
    //     cout<<"Size: "<<i->second.size<<endl;
    //     fstream file(fname1, std::ios::binary | std::ios::app);
    //     file.write((i->second.buffer).c_str(), i->second.size);
    //     file.close();
    // }
    // }

    // cout << "size of received: " << recieved.size() << endl;

    // ofstream file("rec.jpg", ios::binary);
    // for(auto i= rec.begin(); i!=rec.end();i++){
    //     file.write(i->second, fileNameSize);
    // }
    // file.close();

    // reconstruct_from_sections(recieved, recieved.size());
    // rec.clear();
}

void choiceLoop(struct sockaddr_in server_address, char *id, int32_t port)
{
    int socket_id = -1;
    while (1)
    {

        int choice;
        cout << "\n\nEnter choice: \n";
        cout << "0. Handshake\n1. Send File Info\n2. Request File\n3. See all files\n\n";
        cin >> choice;
        if (socket_id > 1)
        {
            close(socket_id);
        }

        socket_id = socket(AF_INET, SOCK_STREAM, 0);

        if (connect(socket_id, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
        {
            perror("connect error: ");
            exit(EXIT_FAILURE);
        };
        write(socket_id, &choice, sizeof(choice));
        switch (choice)
        {
        case 0:
        {
            // send handshake
            sendHandshake(socket_id, id, port);

            break;
        }

        case 1:
        {
            string file_name;
            cout << "Enter file name: ";
            cin >> file_name;
            char *file_name_c = (char *)file_name.c_str();

            sendFileInfo(socket_id, id, file_name_c);

            char msg[100] = {0};
            int msgsize;
            read(socket_id, &msgsize, sizeof(msgsize));
            read(socket_id, msg, 100);
            cout << msg << endl;
            break;
        }

        case 2:
        {
            int32_t file_id;
            cout << "Enter the file ID to be fetched: ";
            cin >> file_id;
            requestForFile(socket_id, id, file_id);

            break;
        }
        case 3:
        {
            // send file
            int num, n, id;
            read(socket_id, &num, sizeof(num));
            for (int i = 0; i < num; i++)
            {
                read(socket_id, &id, sizeof(id));
                read(socket_id, &n, sizeof(n));
                char *buf = (char *)malloc(n);
                read(socket_id, buf, n);
                cout << "ID: " << id << " File: " << buf << endl;
                free(buf);
            }

            break;
        }
        case 4:
        {
            // CLose and exit
            close(socket_id);
            break;
        }
        }
    }
}


int main(int argc, char *argv[])
{

    // printf("%d\n", socket_id);

    int32_t port;

    char ID[128];

    char *IP = (char *)"127.0.0.1";
    generateUniqueID(ID, 128);

    struct sockaddr_in server_address;
    srand(time(0));
    if (argc == 3)
    {
        port = atoi(argv[1]);
        IP = argv[2];
    }

    else
    {
        // std::cout << "Enter port: ";
        // cin >> port;
        port = rand() % 35000 + 10000;
        cout << "Port: " << port << endl;
    }

    cout << endl
         << port;

    std::thread background(background_listen, port);

    server_address.sin_port = htons(PORT);
    server_address.sin_family = AF_INET;
    if (inet_pton(AF_INET, IP, &server_address.sin_addr) < 0)
    {
        cout << "PTON ERRRO\n";
    };
    choiceLoop(server_address, ID, port);
}
