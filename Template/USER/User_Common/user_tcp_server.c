#include <opt.h>
#include <lwip/arch.h>
#include "tcpip.h"

#include "lwip/init.h"
#include "lwip/netif.h"
#include "ethernetif.h"
#include "netif/ethernet.h"
#include "lwip/def.h"
#include "lwip/stats.h"
#include "lwip/etharp.h"
#include "lwip/ip.h"
#include "lwip/snmp.h"
#include "lwip/timeouts.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "api.h"
#include "user_tcp_server.h"
#include "tcp.h"

#if LWIP_SOCKET
    #include <lwip/sockets.h>
    #define RECV_DATA         (1024)
#endif

struct netconn *remote_netconn; // ���ڱ���Զ����Ϣ

// �Ƿ�Ӧ�ý�һ���ṹ��ʹ��ָ���������棿

extern ip4_addr_t ipaddr;

#if LWIP_SOCKET
/******************************************************************
�������ƣ�
��������:������̫���߳�
���������
���������
����ֵ��
ע�⣺
******************************************************************/
/******************************************************************
����������ʷ��¼��
******************************************************************/
TaskHandle_t TCP_ServerSocket_handle;
void TCP_ServerSocket_Init(void)
{
    // sys_thread_new("TCP_Server_Thread",TCP_Server_Thread,NULL,1024,5);//���ȼ�5
    xTaskCreate(TCP_ServerSocket_Thread,
                "TCP_Listen_Thread",
                1024,
                NULL,
                6,
                &TCP_ServerSocket_handle);
}

void TCP_ServerSocket_Thread (void *p)
{
    int sock = -1, connected;
    char *recv_data;
    struct sockaddr_in server_addr, client_addr;
    socklen_t sin_size;
    int recv_data_len;
    //  printf("���ض˿ں���%d\n\n",LOCAL_PORT);
    recv_data = (char *)pvPortMalloc(RECV_DATA);

    if (recv_data == NULL)
    {
        //      printf("No memory\n");
        goto __exit;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        //      printf("Socket error\n");
        goto __exit;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(LOCAL_PORT);
    memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        //      printf("Unable to bind\n");
        goto __exit;
    }

    if (listen(sock, 5) == -1)
    {
        //      printf("Listen error\n");
        goto __exit;
    }

    while (1)
    {
        sin_size = sizeof(struct sockaddr_in);
        connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);
        //    printf("new client connected from (%s, %d)\n",
        //            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        {
            int flag = 1;
            setsockopt(connected,
                       IPPROTO_TCP,     /* set option at TCP level */
                       TCP_NODELAY,     /* name of option */
                       (void *) &flag,  /* the cast is historical cruft */
                       sizeof(int));    /* length of option value */
        }

        while (1)
        {
            recv_data_len = recv(connected, recv_data, RECV_DATA, 0);

            if (recv_data_len <= 0)
            {
                break;
            }

            //      printf("recv %d len data\n",recv_data_len);
            //      write(connected,recv_data,recv_data_len);
        }

        if (connected >= 0)
        {
            closesocket(connected);
        }

        connected = -1;
    }

__exit:

    if (sock >= 0)
    {
        closesocket(sock);
    }

    if (recv_data)
    {
        free(recv_data);
    }
}

#endif

/******************************************************************
�������ƣ�
��������:������̫���߳�
���������
���������
����ֵ��
ע�⣺
******************************************************************/
/******************************************************************
����������ʷ��¼��
******************************************************************/
TaskHandle_t TCP_Server_handle;
void TCP_Server_Init(void)
{
    xTaskCreate(TCP_Server_Thread,
                "TCP_Server_Thread",
                1024,
                NULL,
                6,
                &TCP_Server_handle);
}

/******************************************************************
�������ƣ�
��������:��̫���������ݽӿ�
���������
���������
����ֵ��
ע�⣺
******************************************************************/
/******************************************************************
����������ʷ��¼��
******************************************************************/
void TCP_Server_Thread(void *p)
{
    struct netbuf *buf;
    //	err_t err,err_rev;
    struct netconn *user_netconn;   // ����һ��netconn���ӵĽṹ��ָ��
    LWIP_UNUSED_ARG(p);                        // ��������
    user_netconn = netconn_new(NETCONN_TCP);   // ����һ��������tcp netconn�ṹ��
    netconn_bind(user_netconn, &ipaddr, 5000); // �󶨱���ip�Ͷ˿ں�
    netconn_listen(user_netconn);              // ��������״̬

    // ��ʼ�������ӣ����û��������������ֱ�ӵ��ú����������߳����߳���������Ҫ����𣿻���˵������һ���̲߳���Ҫ���̵߳ľ������
    while (netconn_accept(user_netconn, &remote_netconn) ==
            ERR_OK) // ���δ�����ϣ�����������Ӱ�������̹߳����������Զ���������ֹر����ӻ���β�����
    {
        //		struct netbuf *buf;
        if (user_netconn != NULL)
        {
            remote_netconn->pcb.tcp->so_options |= SOF_KEEPALIVE;    //����keep_live����
        }

        while ((netconn_recv(remote_netconn,
                             &buf)) == ERR_OK) // �������ϣ��ȴ�����һ�����ݰ���1500�ֽڣ����������������� ���������֮��ִ�г���
        {
            // TODO ʹ��len���ж��Ƿ��������Ϊ������һ������С��1460��Ҳ�п�����1448����������ʱ���ж�д�������Խ�����һ�����ܵ���1460
            SysCtrl.Ethernet.ETH_Length.data = netbuf_copy(buf, &SysCtrl.Ethernet.Data[0],
                                               1536); // ���մ˰�ȫ�����ݵ����������һ�ȡ�ð��ֽ���
            EthernetInterfaceHandler();
            netbuf_delete(buf);                                // ����buf��Ϻ��ͷ�ȫ�������ڴ�
        }

        // ��Զ����������ʱ�ﵽ��
        netconn_close(remote_netconn);  // ��Զ�˻���
        netconn_delete(remote_netconn); // ��Զ�˹رգ��ȴ��´�����
    }
}

#if 0

/*-----------------------------------------------------------------------------------*/
static void
tcpecho_thread(void *arg)
{
    struct netconn *conn, *newconn;
    err_t err;
    LWIP_UNUSED_ARG(arg);
    /* Create a new connection identifier. */
    /* Bind connection to well known port number 7. */
#if LWIP_IPV6
    conn = netconn_new(NETCONN_TCP_IPV6);
    netconn_bind(conn, IP6_ADDR_ANY, LOCAL_PORT);
#else  /* LWIP_IPV6 */
    conn = netconn_new(NETCONN_TCP);
    netconn_bind(conn, IP_ADDR_ANY, LOCAL_PORT);
#endif /* LWIP_IPV6 */
    LWIP_ERROR("tcpecho: invalid conn", (conn != NULL), return;);
    printf("���ض˿ں���%d\n\n", LOCAL_PORT);
    /* Tell connection to go into listening mode. */
    netconn_listen(conn);

    while (1)
    {
        /* Grab new connection. */
        err = netconn_accept(conn, &newconn);//�������ȴ�����,ֻ����һ���µ�����newconn���ڿ���ɾ��������������

        /*printf("accepted new connection %p\n", newconn);*/
        /* Process the new connection. */
        if (err == ERR_OK)
        {
            struct netbuf *buf;
            void *data;//һ��voidָ��
            u16_t len;

            while ((err = netconn_recv(newconn, &buf)) == ERR_OK)  //һ�����ݽ������(һ�����ݰ�����һ�����ݻ���һ��buf������)
            {
                //���ݴ洢��netbuf���� �������Ϊbuf�Ķ���ָ�룬��netbuf���׵�ַ���ݸ�buf

                /*printf("Recved\n");*/
                do
                {
                    netbuf_data(buf, &data, &len);//��ȡһ��netbuf ��������ָ��ͳ���
                    memcpy(recv_arr, data, len); //��һ��ָ��ָ��ÿ�ν���Ҫƫ��recv_arr��ָ�뼴��
                    /*
                     �ڴ�copy���������ٽ��д���
                    */
                    err = netconn_write(newconn, data, len, NETCONN_COPY);//ֱ�Ӵ�bufԴ��ַ�ٷ���ȥ�ٷ���ȥ
#if 0

                    if (err != ERR_OK)
                    {
                        printf("tcpecho: netconn_write: error \"%s\"\n", lwip_strerr(err));
                    }

#endif
                } while (netbuf_next(buf) >= 0);//�����NEXTָ����������ָ����һ��buf��������

                netbuf_delete(buf);//�ͷ�ȫ��buf���ڴ�
            }

            /*printf("Got EOF, looping\n");*/
            /* Close connection and discard connection identifier. */
            netconn_close(newconn);
            netconn_delete(newconn);
        }
    }
}

/*-----------------------------------------------------------------------------------*/
void
tcpecho_init(void)
{
    sys_thread_new("tcpecho_thread", tcpecho_thread, NULL, 512, 8);
}

#endif

