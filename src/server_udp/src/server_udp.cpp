#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>

#include <chrono>
using namespace std::chrono;

#define PRINT_LEVEL 5
#include <utils_printlog.h>
#include <utils_shm.h>

#define MAX_CLIENTS 4

typedef struct
{
    int sockfd;
    struct sockaddr_in client_addr;
    time_t t_last;
} ClientInfo;

ClientInfo clients[MAX_CLIENTS];

int main()
{

    void *shm_ptr = shm_create(SHM_VARS_STREAM_KEY, SHM_VARS_STREAM_SIZE);
    shm_struct_vars_stream *var_stream = (shm_struct_vars_stream *)shm_ptr;

    void *shm_ptr_in = shm_create(SHM_VARS_IN_KEY, SHM_VARS_IN_SIZE);
    shm_struct_vars_in *var_in = (shm_struct_vars_in *)shm_ptr_in;

    PRINT_LOG(5, PRINT_GREEN "REROBAPP SERVER UDP" PRINT_RESET);

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clients[i].sockfd = -1;
    }

    int port = 1515;
    int sockfd;
    struct sockaddr_in si_me, si_other;
    char buffer[1024];
    socklen_t addr_size;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
               (const void *)&optval, sizeof(int));

    memset(&si_me, '\0', sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(port);
    si_me.sin_addr.s_addr = inet_addr("0.0.0.0");

    if (bind(sockfd, (struct sockaddr *)&si_me, sizeof(si_me)) < 0)
    {
        perror("Error al enlazar el socket");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    addr_size = sizeof(si_other);

    int nClients = 0;

    fcntl(sockfd, F_SETFL, O_NONBLOCK);
    int nsz, idx_client;

    time_t tNclient = time(NULL);
    auto t0_test = high_resolution_clock::now();

    int nClientDisp = -1;
    bool nClientFound = false;

    float aux_f[10];

    bool printRec = false;
    while (1)
    {

        fflush(stdout);

        if ((nsz = recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr *)&si_other, &addr_size)) >= 0)
        {
            printRec = false;
            if (nsz == 4)
            {
                if (strncmp("ACK\n", buffer, 4) == 0)
                {
                    nClientDisp = -1;
                    nClientFound = false;
                    for (idx_client = 0; idx_client < MAX_CLIENTS; idx_client++)
                    {
                        if (clients[idx_client].sockfd != -1)
                        {
                            if (memcmp(&clients[idx_client].client_addr, &si_other, addr_size) == 0)
                            {
                                nClientFound = true;
                                nClientDisp = idx_client;
                                break;
                            }
                        }
                        else
                        {
                            nClientDisp = idx_client;
                        }
                    }

                    if (nClientFound)
                    {
                        PRINT_LOG(5, "Alive %s:%d", inet_ntoa(clients[nClientDisp].client_addr.sin_addr),
                                  clients[nClientDisp].client_addr.sin_port);
                        clients[nClientDisp].t_last = time(NULL);
                    }
                    else if (nClientDisp != -1)
                    {
                        PRINT_LOG(4, PRINT_GREEN "Acepted %s:%d", inet_ntoa(si_other.sin_addr), si_other.sin_port);

                        clients[nClientDisp].sockfd = sockfd;
                        clients[nClientDisp].t_last = time(NULL);
                        memcpy(&clients[nClientDisp].client_addr, &si_other, addr_size);

                        // sprintf(buffer, "Hola\n");
                        // sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&clients[nClientDisp].client_addr, addr_size);
                    }
                    else
                    {
                        PRINT_LOG(5, "Buffer lleno");
                    }

                    printRec = false;
                }
            }
            else if (strncmp("pos_d:", buffer, 6) == 0)
            {
                if (sscanf(buffer, "pos_d:%f\n", &aux_f[0]) >= 0)
                {
                    var_in->pos_d = aux_f[0];
                    printRec = false;
                }
            }
            else if (strncmp("KV:", buffer, 3) == 0)
            {
                if (sscanf(buffer, "KV:%f\n", &aux_f[0]) >= 0)
                {
                    PRINT_LOG(4, PRINT_YELLOW "KV: %f", aux_f[0]);
                    var_in->KV = aux_f[0];
                    printRec = false;
                }
            }
            else
            {
                printRec = true;
            }

            if (printRec)
            {
                buffer[nsz] = '\0';
                PRINT_LOG(5, PRINT_BLUE "%d:%s", nsz, buffer);
            }
        }

        if (tNclient < time(NULL))
        {
            static bool type_sym = false;
            if (type_sym)
                printf("\r\\");
            else
                printf("\r/");
            type_sym = !type_sym;

            for (idx_client = 0; idx_client < MAX_CLIENTS; idx_client++)
            {
                if (clients[idx_client].sockfd != -1)
                {
                    if ((clients[idx_client].t_last + 5) < tNclient)
                    {
                        PRINT_LOG(4, PRINT_YELLOW "Disconnected %s:%d", inet_ntoa(clients[idx_client].client_addr.sin_addr),
                                  clients[idx_client].client_addr.sin_port);
                        clients[idx_client].sockfd = -1;
                    }
                }
            }
            time(&tNclient);
        }

        static auto t0_send = high_resolution_clock::now();
        long dt = duration_cast<milliseconds>(high_resolution_clock::now() - t0_send).count();
        if (dt >= 20)
        {
            t0_send = high_resolution_clock::now();

            // var_stream->t_s = duration_cast<milliseconds>(high_resolution_clock::now() - t0_test).count()/1000.0f;
            // var_stream->exo_hip_rigth_pos_in = sin(2*M_PI*1*var_stream->t_s);

            for (idx_client = 0; idx_client < MAX_CLIENTS; idx_client++)
            {

                if (clients[idx_client].sockfd != -1)
                {
                    // sendto(clients[idx_client].sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&clients[idx_client].client_addr, addr_size);
                    sendto(clients[idx_client].sockfd, var_stream, sizeof(shm_struct_vars_stream), 0, (struct sockaddr *)&clients[idx_client].client_addr, addr_size);
                }
            }
        }
        usleep(15000);
    }

    shmdt(shm_ptr);
    return 0;
}