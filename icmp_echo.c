#include <netinet/ip_icmp.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

static int nsent;
unsigned short in_check(void* buf, size_t len);
void readloop();

int main () {

    struct icmp* icmp;
    time_t t;
    nsent = 0;
    char *str_date;
    unsigned short check_sum;
    int sock;
    int len;

    struct sockaddr_in sa;

    icmp = calloc(1024, 0);
    icmp->icmp_type = ICMP_ECHO;
    icmp->icmp_code = 0;
    icmp->icmp_id = getpid();
    icmp->icmp_seq = ++nsent;

    t = time(NULL);
    str_date = ctime(&t);

    strcpy(((char*)icmp->icmp_data), str_date);

    printf("strdate: %s\n", str_date);

    len = ICMP_MINLEN + strlen(str_date) + 1;
    icmp->icmp_cksum = in_check(icmp, len);

    sock = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);

    memset(&sa, sizeof(sa), 0);
    sa.sin_family = PF_INET;
    inet_aton("192.168.1.1", &sa.sin_addr.s_addr);

    if( sendto(sock, icmp, len, 0, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
        perror("");
    }

    readloop(sock);
    return 0;
}


void readloop(int sock) {
	
	struct sockaddr_in sa;
	char data[1500];

	struct ip *ip;
	struct icmp *icmp;
	int len = sizeof(sa);

	while(1) {
		recvfrom(sock, data, 1500, 0, (struct sockaddr *)&sa, &len);
		ip   = (struct ip*)data;
		icmp = (struct icmp*)(data + (ip->ip_hl << 2));


		if( icmp->icmp_type == ICMP_ECHOREPLY
			&& icmp->icmp_code == 0 ) {
			if ( icmp->icmp_id != getpid() ) {
				continue;
			}

			printf("reply message: %s\n", (char*)icmp->icmp_data);
		}
	}
}

unsigned short in_check(void* buf, size_t len){

    int val = 0;
    unsigned short *sp = buf;
    
    while( len >= 2 ) {
        val += *sp++;
        len -= 2;
    }

    if( len == 1 ) {
        val += *(unsigned char *)sp;
    }

    val = (val >> 16) + (val & 0xffff);
    val += (val >> 16);


    return ~(unsigned short)val;
}
