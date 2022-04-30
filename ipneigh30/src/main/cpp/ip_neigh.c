#include <jni.h>
#include <android/log.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <android/log.h>
#include <linux/rtnetlink.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <malloc.h>
#include <stdbool.h>

#define NUD_INCOMPLETE        0x01
#define NUD_REACHABLE        0x02
#define NUD_STALE        0x04
#define NUD_DELAY        0x08
#define NUD_PROBE        0x10
#define NUD_FAILED        0x20
#define LOGTAG  "ip-neigh"
#define TYPEMSG 30
/* Dummy states */
#define NUD_NOARP        0x40
#define NUD_PERMANENT        0x80
#define NUD_NONE        0x00


int do_route_dump_request(int sock)
{
    struct {
        struct nlmsghdr nlh;
        struct rtmsg rtm;
    } nl_request;

    nl_request.nlh.nlmsg_type = TYPEMSG;
    nl_request.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    nl_request.nlh.nlmsg_pid = getpid();
    nl_request.nlh.nlmsg_len = sizeof(nl_request);
    nl_request.nlh.nlmsg_seq = 0;
    nl_request.rtm.rtm_family = AF_INET;
    int sent = send(sock, &nl_request, sizeof(nl_request), 0);

    if(sent > 0)
        __android_log_print(ANDROID_LOG_ERROR, LOGTAG,  "Message sent");


    return sent;
}
int rtnl_receive(int fd, struct msghdr *msg, int flags)
{
    int len;

    do {
        len = recvmsg(fd, msg, flags);
    } while (len < 0 && (errno == EINTR || errno == EAGAIN));

    if (len < 0) {
        __android_log_write(ANDROID_LOG_ERROR, LOGTAG,  strerror(errno));
        perror("Netlink receive failed");
        return -errno;
    }

    if (len == 0) {
        __android_log_write(ANDROID_LOG_ERROR, LOGTAG,  strerror(errno));
        perror("EOF on netlink");
        return -ENODATA;
    }

    return len;
}
static int rtnl_recvmsg(int fd, struct msghdr *msg, char **answer)
{
    struct iovec *iov = msg->msg_iov;
    char *buf;
    int len;

    iov->iov_base = NULL;
    iov->iov_len = 0;

    len = rtnl_receive(fd, msg, MSG_PEEK | MSG_TRUNC);

    if (len < 0) {
        __android_log_write(ANDROID_LOG_ERROR, LOGTAG,  "recv_len < 0");
        return len;
    }

    buf = malloc(len);

    if (!buf) {
        __android_log_write(ANDROID_LOG_ERROR, LOGTAG,  "malloc failed");

        perror("malloc failed");
        return -ENOMEM;
    }

    iov->iov_base = buf;
    iov->iov_len = len;

    len = rtnl_receive(fd, msg, 0);

    if (len < 0) {
        __android_log_write(ANDROID_LOG_ERROR, LOGTAG,  "recv_len < 0");
        free(buf);
        return len;
    }

    *answer = buf;

    return len;
}



jstring JNICALL Java_it_alessangiorgi_ipneigh30_ArpNDK_ARPFromJNI(JNIEnv *env, jclass clazz) {



    jclass cls_StringBuilder = (*env)->FindClass(env,"java/lang/StringBuilder");
    jmethodID ctr_StringBuilder = (*env)->GetMethodID(env,cls_StringBuilder, "<init>", "(I)V");
    jmethodID mid_StringBuilder_append = (*env)->GetMethodID(env,cls_StringBuilder, "append", "(Ljava/lang/String;)Ljava/lang/StringBuilder;");
    jobject  stringBuilder = (*env)->NewObject(env,cls_StringBuilder, ctr_StringBuilder, 1000);


    struct sockaddr_nl saddr;
    int s = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
    if (s < 0) {
        __android_log_write(ANDROID_LOG_ERROR, LOGTAG,  "socket netlink failed");
        return -1;
    }

    char* arp_result = malloc(1024);


    memset(&saddr, 0, sizeof(saddr));
    saddr.nl_family = AF_NETLINK;
    saddr.nl_pid = getpid();

    if(do_route_dump_request(s) < 0){
        __android_log_write(ANDROID_LOG_ERROR, LOGTAG,  "socket send failed");
    }else
        __android_log_write(ANDROID_LOG_ERROR, LOGTAG,  "socket send success");


    struct sockaddr_nl nladdr;
    struct iovec iov;
    struct msghdr msg = {
            .msg_name = &nladdr,
            .msg_namelen = sizeof(nladdr),
            .msg_iov = &iov,
            .msg_iovlen = 1,
    };
    char *buf;

    int len = rtnl_recvmsg(s, &msg, &buf);
    if(len<0)
        __android_log_write(ANDROID_LOG_ERROR, LOGTAG,  "recv_len < 0");

    struct nlmsghdr *h = (struct nlmsghdr *)buf;
    struct  rtmsg *route_entry;  /* This struct represent a route entry \
                                    in the routing table */
    struct  rtattr *route_attribute; //* This struct contain route \
                                            attributes (route type) *//
    int     route_attribute_len = 0;
    unsigned char    route_netmask = 0;
    unsigned char    route_protocol = 0;
    char    destination_address[32];
    char    gateway_address[32];
    unsigned char mac_buf[12];
    struct ifinfomsg *inf_msg_ptr;
    struct rtattr *rta_ptr;
    struct  ndmsg *rtmp ;


    int msglen = len;


    while (NLMSG_OK(h, msglen)) {
        if(h->nlmsg_type == 28) {
            memset(arp_result, 0, 1024);


            if (h->nlmsg_flags & NLM_F_DUMP_INTR) {
                __android_log_write(ANDROID_LOG_ERROR, LOGTAG, "Dump was interrupted");

                fprintf(stderr, "Dump was interrupted\n");
                free(buf);
                return -1;
            }

            if (nladdr.nl_pid != 0) {
                continue;
            }

            if (h->nlmsg_type == NLMSG_ERROR) {
                __android_log_write(ANDROID_LOG_ERROR, LOGTAG, "netlink reported error");

                perror("netlink reported error");
                free(buf);
            }


            route_entry = (struct rtmsg *) NLMSG_DATA(h);


            route_netmask = route_entry->rtm_dst_len;
            route_protocol = route_entry->rtm_protocol;
            route_attribute = (struct rtattr *) RTM_RTA(route_entry);
            /* Get the route attributes len */
            route_attribute_len = RTM_PAYLOAD(h);

            printf("route attribute type: %d\n", route_attribute->rta_type);

            /* Get the destination address */
            if (route_attribute->rta_type == RTA_DST) {
                inet_ntop(AF_INET, RTA_DATA(route_attribute), \
                    destination_address, sizeof(destination_address));
            }
            /* Get the gateway (Next hop) */
            if (route_attribute->rta_type == RTA_GATEWAY) {
                inet_ntop(AF_INET, RTA_DATA(route_attribute), \
                    gateway_address, sizeof(gateway_address));
            }

            printf("route to destination --> %s/%d proto %d and gateway %s\n",
                   destination_address, route_netmask, route_protocol, gateway_address);


            inf_msg_ptr = (struct ifinfomsg *) NLMSG_DATA(h);
            rta_ptr = (struct rtattr *) IFLA_RTA(inf_msg_ptr);

            memcpy(mac_buf, RTA_DATA(rta_ptr), 10);
            rtmp = (struct ndmsg *) NLMSG_DATA(h);

            char ifname[1024];
            if_indextoname(inf_msg_ptr->ifi_index, ifname);


            jstring arp_entry;
            switch (rtmp->ndm_state) {
                case NUD_REACHABLE:
                    __android_log_print(ANDROID_LOG_ERROR, LOGTAG,
                                        "%s dev %s lladdr %02x:%02x:%02x:%02x:%02x:%02x REACHABLE\n",
                                        destination_address, ifname, mac_buf[4], mac_buf[5],
                                        mac_buf[6],
                                        mac_buf[7], mac_buf[8], mac_buf[9]);


                    snprintf(arp_result, 1024,
                             "%s dev %s lladdr %02x:%02x:%02x:%02x:%02x:%02x REACHABLE\n",
                             destination_address, ifname, mac_buf[4], mac_buf[5], mac_buf[6],
                             mac_buf[7], mac_buf[8], mac_buf[9]);
                    arp_entry = (*env)->NewStringUTF(env, arp_result);
                    (*env)->CallObjectMethod(env, stringBuilder, mid_StringBuilder_append,
                                             arp_entry);
                    break;
                case NUD_STALE:
                    __android_log_print(ANDROID_LOG_ERROR, LOGTAG,
                                        "%s dev %s lladdr %02x:%02x:%02x:%02x:%02x:%02x STALE\n",
                                        destination_address, ifname, mac_buf[4], mac_buf[5],
                                        mac_buf[6],
                                        mac_buf[7], mac_buf[8], mac_buf[9]);
                    snprintf(arp_result, 1024,
                             "%s dev %s lladdr %02x:%02x:%02x:%02x:%02x:%02x STALE\n",
                             destination_address, ifname, mac_buf[4], mac_buf[5], mac_buf[6],
                             mac_buf[7], mac_buf[8], mac_buf[9]);
                    arp_entry = (*env)->NewStringUTF(env, arp_result);
                    (*env)->CallObjectMethod(env, stringBuilder, mid_StringBuilder_append,
                                             arp_entry);
                    break;
                case NUD_DELAY:
                    __android_log_print(ANDROID_LOG_ERROR, LOGTAG,
                                        "%s dev %s lladdr %02x:%02x:%02x:%02x:%02x:%02x DELAY\n",
                                        destination_address, ifname, mac_buf[4], mac_buf[5],
                                        mac_buf[6],
                                        mac_buf[7], mac_buf[8], mac_buf[9]);
                    snprintf(arp_result, 1024,
                             "%s dev %s lladdr %02x:%02x:%02x:%02x:%02x:%02x DELAY\n",
                             destination_address, ifname, mac_buf[4], mac_buf[5], mac_buf[6],
                             mac_buf[7], mac_buf[8], mac_buf[9]);
                    arp_entry = (*env)->NewStringUTF(env, arp_result);
                    (*env)->CallObjectMethod(env, stringBuilder, mid_StringBuilder_append,
                                             arp_entry);
                    break;
                default:
                    break;
            }

        }
        h = NLMSG_NEXT(h, msglen);
    }
    __android_log_write(ANDROID_LOG_ERROR, LOGTAG,  "arp-Finish");

    free(buf);
    free(arp_result);


    jmethodID mid_StringBuilder_toString = (*env)->GetMethodID(env,cls_StringBuilder, "toString", "()Ljava/lang/String;");
    jstring final = (jstring) (*env)->CallObjectMethod(env,stringBuilder, mid_StringBuilder_toString);
    return final;
}
