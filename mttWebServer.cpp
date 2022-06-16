#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h> 
#include <arpa/inet.h>
#include <string>

using namespace std;

#define MAX_WORKER_THERAD_COUNT 100

struct params{
    int client_socket;
    sockaddr_in commSock;
    socklen_t commlen;
};

void * workingThread (void *param){
    struct params * p = (struct params *) param;
    
    char buffer[8096];
    int num_byte_recvd = recv(p->client_socket,buffer,8096,0);
    cout<<string(buffer,0,num_byte_recvd);

    // send the webpage data to the client 
    // with http header info

    string header ;
    string data ;

    data = string("<head></head> <body> <h1> Hello User</h1></body> ");
    
    header = string("HTTP/1.1 200 ok\r\n") + string("Cache-Control: no-cache, private\r\n") \
                + string("Content-Type: text/html\r\n")+ string("Content-Length: ") +to_string(data.size()) + string("\r\n\r\n"); 

    data  = header + data;

    send(p->client_socket,data.c_str(),data.size(),0);

    return 0;   
}

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


    int * wt_tid = (int *) malloc(sizeof(int));
    pthread_t  * wt_pth = (pthread_t*) malloc(sizeof(int));

    int wt_itr =0;

    while(wt_itr < MAX_WORKER_THERAD_COUNT){
        struct params * commPrm = (struct params *)malloc(sizeof(struct params));
        commPrm->commlen = sizeof(sockaddr_in);
        commPrm->client_socket = accept(listening_sock,(sockaddr*)&commPrm->commSock,&commPrm->commlen);
        if (commPrm->client_socket !=-1){
            cout<<"Connected to client on port "<<commPrm->client_socket<<endl; 
            wt_tid[wt_itr]=pthread_create(&wt_pth[wt_itr],0,workingThread,(void*)commPrm);
            wt_itr++;
        }       
        else{
            cerr<<"Connection not established";
        }
    } 

    for (int i;i<MAX_WORKER_THERAD_COUNT;i++)   
        pthread_join(wt_pth[i],0);

    // close listening socket
    close (listening_sock);
    return 0;
}