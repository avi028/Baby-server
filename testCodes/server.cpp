#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <string>

using namespace std;

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
    host_addr.sin_port = htons(55003);

    // add address to the sockaddr_in while converting string to ipv4 address
    inet_pton(AF_INET,"0.0.0.0",&host_addr.sin_addr);

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

    int result = getnameinfo((sockaddr*)&comm_sock,comm_sock_len,host,NI_MAXHOST,serve,NI_MAXSERV,0);

    if(result ==0 ){ //getnameinfo is success
        cout<< "host "<< host<< " , service " << serve <<endl;
    }
    else{
        inet_ntop(AF_INET,&comm_sock.sin_addr,host,NI_MAXHOST);
        cout<< "host "<< host<< " , service " << ntohs(comm_sock.sin_port) <<endl;    
    }

    char buffer[4096];
    int num_byte_recvd;
    while(true){
        //clear the buffer
        memset(buffer,0,4096);
        // receive message
        num_byte_recvd =  recv(client_socket,buffer,4096,0);
        // display message
        cout<<string(buffer ,0 ,num_byte_recvd);
        // send echo back        
        send(client_socket,buffer,num_byte_recvd,0);
    }
    // close the socket 
    close(client_socket);
    close (listening_sock);
    return 0;
}

