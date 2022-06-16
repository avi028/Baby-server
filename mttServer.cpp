#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <string>
#include <pthread.h>
#include <semaphore.h>

#define MAX_WORKER_THERAD_COUNT 100

using namespace std;

struct wtParam{
    int client_socket;
    sockaddr_in comm_sock;
    socklen_t comm_sock_len;
};


void * workerThread(void * params){
    struct wtParam * conn = (struct wtParam*)params;    
    char buffer[4096];
    int num_byte_recvd;

    char host[NI_MAXHOST];
    char serve[NI_MAXSERV];

    memset(host,0,NI_MAXHOST);
    memset(serve,0,NI_MAXSERV);
    inet_ntop(AF_INET,&conn->comm_sock.sin_addr,host,NI_MAXHOST);
    
    cout<<"connected to "<<host <<" At port "<<ntohs(conn->comm_sock.sin_port)<<" at client port " \
            << conn->client_socket<<endl; 
 
    string stopstr = "bye";
    string welcomeMsg = "************* Welcome to chat room 101 ****************\n ---------------- to exit send *bye* -----------------\n ";
    send(conn->client_socket,welcomeMsg.c_str(),welcomeMsg.size(),0);

    while(true){
        //clear the buffer
        memset(buffer,0,4096);
        // receive message
        num_byte_recvd =  recv(conn->client_socket,buffer,4096,0);
        // display message
        cout<<string(buffer ,0 ,num_byte_recvd);
        // send echo back        
        send(conn->client_socket,buffer,num_byte_recvd,0);
        if(memcmp(stopstr.c_str(),buffer,stopstr.size())==0)
            break;
    }
    // close the socket 
    close(conn->client_socket);
    cout<<"connection to client "<<host<<" closed\n";
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
    host_addr.sin_port = htons(55005);

    // add address to the sockaddr_in while converting string to ipv4 address
    inet_pton(AF_INET,"0.0.0.0",&host_addr.sin_addr);

    const int enable = 1;
    if (setsockopt(listening_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        cerr<<"setsockopt(SO_REUSEADDR) failed";    
    
    // bind a socket 
    if(bind(listening_sock,(sockaddr*)&host_addr,sizeof(sockaddr_in))==-1 ){
        cerr<<"Bind unsuccessful : Unable to bond to given socket"<<endl;
    }
    
    // workerthreadsId's
    int * wt_ids = (int *) malloc(sizeof(int)*MAX_WORKER_THERAD_COUNT);
    pthread_t *wts = (pthread_t*) malloc(sizeof(pthread_t)*MAX_WORKER_THERAD_COUNT);

    // worker thread iterator
    int wt_itr=0;
    // listen on the socket
    listen(listening_sock,SOMAXCONN);

    // accept request on socket
    while(wt_itr<MAX_WORKER_THERAD_COUNT){
        struct  wtParam * param = (struct wtParam *) malloc(sizeof(struct wtParam));

        param->comm_sock_len = sizeof(sockaddr_in);
        param->client_socket = accept(listening_sock,(sockaddr*)&(param->comm_sock),&(param->comm_sock_len));        
        if(param->client_socket == -1){
                cerr<<"connection not established"<<endl;
            }
        else{
                cout<<"connected to port "<<param->client_socket<<endl;
            }
        pthread_create(&wts[wt_itr],NULL,workerThread,(void*)param);
        wt_itr++;
        cout<<"thread "<<wt_itr<< "started\n";

    }

    for (int i;i<MAX_WORKER_THERAD_COUNT;i++)   
        pthread_join(wts[i],0);

    close (listening_sock);
    free(wts);
    free(wt_ids);
    return 0;
}

