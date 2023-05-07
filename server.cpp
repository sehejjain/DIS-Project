#include <stdio.h>
#include <string.h>
#include <sstream>
#include <map>
#include <iostream>
#include <fstream>
#include <cmath>

#include "networkstructures.h"
#include "setNetwork.h"

template <typename T>
void sendVector(vector<T> vec, int sock)
{
    int size = vec.size();
    write(sock, &size, sizeof(int));
    for (int i = 0; i < vec.size(); i++)
    {
        write(sock, &vec[i], sizeof(T));
    }
}

bool findInVector(int id, vector<int> vec)
{
    for (int i = 0; i < vec.size(); i++)
    {
        if (vec[i] == id)
        {
            return true;
        }
    }

    return false;
}

map<string, User> users;
map<int32_t, string> filesMap;

void readID(int client_socket, char **ID)
{
    *ID = (char *)malloc(ID_SIZE);
    read(client_socket, *ID, ID_SIZE);
}

int main()
{
    int socket_fd = setUpNetwork(PORT);

    while (1)
    {

        struct sockaddr_in client_address;
        int addrlen = sizeof(client_address);
        int client_socket = accept(socket_fd, (sockaddr *)&client_address, (socklen_t *)&(addrlen));
        if (client_socket < 0)
        {
            cout << "ACCEPT FAILURE\n";
        }
        int mid;

        int32_t r = read(client_socket, &mid, sizeof(r));
        switch (mid)
        {
        case 0:
        {
            cout << "HANDSHAKE INITIATED\n";
            Handshake handshake;
            int r = read(client_socket, &handshake.port, sizeof(handshake.port));
            User user;
            user.address = client_address;
            user.address.sin_port = htons(handshake.port);

            readID(client_socket, &user.id);
            cout << "User ID: " << user.id << endl;
            users.insert(make_pair(user.id, user));

            break;
        }

        case 1:
        {
            cout << "REGISTERING FILE INFO\n";
            FileInfo fileInfo;

            int namesize;
            read(client_socket, &namesize, sizeof(namesize));
            char name[100] = {0};
            read(client_socket, name, namesize);

            cout << "File name: " << name << endl;
            string message;

            bool contains = false;
            for (auto i = filesMap.begin(); i != filesMap.end(); i++)
            {
                if (std::strcmp((i->second).c_str(), name) == 0)
                {

                    contains = true;
                }
            }

            ifstream file(name, ios::binary);
            if (file)
            {
                int lastID = filesMap.empty() ? 0 : filesMap.rbegin()->first;

                if (!contains)
                {
                    filesMap.insert(make_pair(lastID + 1, name));
                    fileInfo.file_id = lastID + 1;
                }

                readID(client_socket, &fileInfo.user_id);
                cout << "\nUser ID: " << fileInfo.user_id << endl;
                users[fileInfo.user_id].files.push_back(fileInfo.file_id);
                if (!contains)
                {
                    message = "File registered successfully\n";
                }
                else
                {
                    message = "File already exists but registered for this user\n";
                }
            }
            else
            {
                cout << "File not found\n";
                message = "File not found\n";
            }
            char *msg = (char *)message.c_str();
            int size = strlen(msg);
            write(client_socket, &size, sizeof(size));
            // cout<<"Sending file name: "<<file_name<<endl;
            if (send(client_socket, msg, strlen(msg), 0) == -1)
            {
                std::cerr << "Failed to send the file name" << std::endl;
            }

            break;
        }

        case 2:
        {
            // file request

            cout << "REQUESTING FILE\n";
            FileRequest fileRequest;
            read(client_socket, &fileRequest.file_id, sizeof(fileRequest.file_id));
            readID(client_socket, &fileRequest.user_id);

            // printf("FILE ID is: %d\n", fileRequest.file_id);
            cout << "FILE ID is: " << fileRequest.file_id << endl;
            FileRequestResponse response;
            for (auto i = users.begin(); i != users.end(); i++)
            {
                cout << i->first.c_str() << " " << i->second.files.size() << endl;
                // printf("%s %ld\n", i->first.c_str(), i->second.files.size());
                bool found = findInVector(fileRequest.file_id, i->second.files);
                if (found)
                {
                    response.clients.push_back(i->second.address);
                }
            }

            float num_of_peers_w_file = response.clients.size();
            float decimal = (float)1 / (float)num_of_peers_w_file;
            cout << "Num of peers with file: " << num_of_peers_w_file << endl;
            cout << "Parts: " << decimal << endl;
            // decimal = (float)((int)(decimal*10))/(float)10;
            // cout<<"Parts rounded "<< decimal<<endl;

            char *filename = (char *)filesMap[fileRequest.file_id].c_str();

            send(client_socket, &decimal, sizeof(decimal), 0);
            int namesize = strlen(filename);
            write(client_socket, &namesize, sizeof(namesize));
            write(client_socket, filename, namesize);

            sendVector(response.clients, client_socket);

            break;
        }

        case 3:
        {
            int num = filesMap.size();
            vector<int> idarr;
            vector<string> names;
            cout << "Get available files";
            for (auto i = filesMap.begin(); i != filesMap.end(); i++)
            {
                idarr.push_back(i->first);
                names.push_back(i->second);
            }
            write(client_socket, &num, sizeof(num));
            for (int i = 0; i < num; i++)
            {
                write(client_socket, &idarr[i], sizeof(idarr[i]));
                int namesize = names[i].size();
                write(client_socket, &namesize, sizeof(namesize));
                write(client_socket, names[i].c_str(), namesize);
            }
            break;
        }
        case 4:
        {
            close(socket_fd);
            exit(0);
        }
        }

        close(client_socket);
    }
}
