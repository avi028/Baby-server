#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h> 
#include <arpa/inet.h>
#include <string>

using namespace std;


struct params{
    int client_socket;
    sockaddr_in commAddr;
    socklen_t commlen;
};

int main(int argc,char** argcv){
    // create a socket
    int listening_sock = socket(AF_INET,SOCK_STREAM,0);

    // port check 
    if(listening_sock==-1){
        cerr<<"Unable to open socket"<<endl;
        return -1;
    }

    sockaddr_in host_addr ;
    host_addr.sin_family = AF_INET;

    // reverse the bits of the port fron host number to netwrok number 
    host_addr.sin_port = htons(8080);

    // add address to the sockaddr_in while converting string to ipv4 address
    inet_pton(AF_INET,"127.0.0.1",&host_addr.sin_addr);

    // bind a socket 
    if(bind(listening_sock,(sockaddr*)&host_addr,sizeof(sockaddr_in))==-1){
        cerr<<"Bind unsuccessful : Unable to bond to given socket"<<endl;
    }

    // listen on the socket
    listen(listening_sock,SOMAXCONN);

    
    // create new sockaddr dedicated to a client
    sockaddr_in comm_sock;
    socklen_t comm_sock_len = sizeof(sockaddr_in);
    
    // accept request on socket
    int client_socket = accept(listening_sock,(sockaddr*)&comm_sock,&comm_sock_len);
    if(client_socket == -1){
        cerr<<"connection not established"<<endl;
    }

    char host[NI_MAXHOST];
    char serve[NI_MAXSERV];

    memset(host,0,NI_MAXHOST);
    memset(serve,0,NI_MAXSERV);

    // int result = getnameinfo((sockaddr*)&comm_sock,comm_sock_len,host,NI_MAXHOST,serve,NI_MAXSERV,0);

    // if(result ==0 ){ //getnameinfo is success
    //     cout<< "host "<< host<< " , service " << serve <<endl;
    // }
    // else{
    //     inet_ntop(AF_INET,&comm_sock.sin_addr,host,NI_MAXHOST);
    //     cout<< "host "<< host<< " , service " << ntohs(comm_sock.sin_port) <<endl;    
    // }

    //receive request 

    char buffer[8096];
    int num_byte_recvd = recv(client_socket,buffer,8096,0);
    cout<<string(buffer,0,num_byte_recvd);

    // send the webpage data to the client 
    // with http header info

    string header ;
    string data ;

    data = string("<head></head> <body> <h1> Hello User</h1></body> ");
    
    header = string("HTTP/1.1 200 ok\r\n") + string("Cache-Control: no-cache, private\r\n") \
                + string("Content-Type: text/html\r\n")+ string("Content-Length: ") +to_string(data.size()) + string("\r\n\r\n"); 

    data  = header + data;

    send(client_socket,data.c_str(),data.size(),0);

    // close the socket 

    close(client_socket);
    close (listening_sock);
    return 0;
}

