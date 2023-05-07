
#include <vector>
#include <map>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include<arpa/inet.h>
#include<cstdint>
#define PORT 2005
#define ID_SIZE 128


using namespace std;

struct Handshake
{
    int32_t port;
    char* ID;

};

struct FileRequest{
    int32_t file_id;
    char* user_id;

};

struct FileRequestResponse{
    vector<sockaddr_in> clients;
};

struct User
{
    char* id;
    vector<int32_t> files;
    sockaddr_in address;
};

struct FileInfo{
    char* user_id;
    int32_t file_id;
};

