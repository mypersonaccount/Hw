#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<netinet/in.h>
#include <arpa/inet.h>
#include<time.h>
#include <pcap.h>
#include <ctype.h>

#define BUFSIZE 10240
#define STRSIZE 10240

typedef int bpf_int32;
typedef unsigned int bpf_u_int32;
typedef unsigned short u_short;
typedef unsigned int u_int32;
typedef unsigned short u_int16;
typedef unsigned char u_int8;
// typedef unsigned char   u_char;

typedef struct IcmpHeader{
  u_int8_t type;		/* message type */
  u_int8_t code;		/* type sub-code */
  u_int16_t checksum;
  union
  {
    struct
    {
      u_int16_t	id;
      u_int16_t	sequence;
    } echo;			/* echo datagram */
    u_int32_t	gateway;	/* gateway address */
    struct
    {
      u_int16_t	__unused;
      u_int16_t	mtu;
    } frag;			/* path mtu discovery */
  } un;
} ICMPHeader_t;

typedef struct FrameHeader_t{
	unsigned char  DstMAC[6];
	unsigned char  SrcMAC[6];
	u_short FrameType;
} FrameHeader_t;

typedef struct IPHeader_t{
	u_int8 Ver_HLen;
	u_int8 TOS;
	u_int16 TotalLen;
	u_int16 ID;
	u_int16 Flag_Segment;
	u_int8 TTL;
	u_int8 Protocol;
	u_int16 Checksum;
	u_int32 SrcIP;
	u_int32 DstIP;
} IPHeader_t;

typedef struct TCPHeader_t{
	u_int16 SrcPort;
	u_int16 DstPort;
	u_int32 SeqNO;
	u_int32 AckNO;
	u_int8 HeaderLen;
	u_int8 Flags;
	u_int16 Window;
	u_int16 Checksum;
	u_int16 UrgentPointer;
} TCPHeader_t;

typedef struct UDPHeader_t{  
    unsigned short SrcPort;
    unsigned short DstPort; 
    unsigned short HeaderLen;
    unsigned short Checksum;
} UDPHeader_t;

void print_hex_ascii_line(const u_char *payload, int len, int offset){

	int i;
	int gap;
	const u_char *ch;

	printf("%05d   ", offset);

	ch = payload;
	for(i = 0; i < len; i++) {
		printf("%02x ", *ch);
		ch++;
		if (i == 7)
			printf(" ");
	}

	if (len < 8)
		printf(" ");

	if (len < 16) {
		gap = 16 - len;
		for (i = 0; i < gap; i++) {
			printf("   ");
		}
	}
	printf("   ");

	ch = payload;
	for(i = 0; i < len; i++) {
		if (isprint(*ch))
			printf("%c", *ch);
		else
			printf(".");
		ch++;
	}

	printf("\n");
	return;
}

void print_payload(const u_char *payload, int len){

	int len_rem = len;
	int line_width = 16;
	int line_len;
	int offset = 0;
	const u_char *ch = payload;

	if (len <= 0)
		return;

	if (len <= line_width) {
		print_hex_ascii_line(ch, len, offset);
		return;
	}

	for ( ;; ) {

		line_len = line_width % len_rem;

		print_hex_ascii_line(ch, line_len, offset);

		len_rem = len_rem - line_len;
		ch = ch + line_len;
		offset = offset + line_width;

		if (len_rem <= line_width) {
			print_hex_ascii_line(ch, len_rem, offset);
			break;
		}
	}

	return;
}

void PROTOCOL(int pro){

	if(pro == 1)	    
		printf(" |___ Protocol : ICMP (%d)\n", pro);
	else if(pro == 4)	    
		printf(" |___ Protocol : IPv4 (%d)\n", pro);
	else if(pro == 41)	    
		printf(" |___ Protocol : IPv6 (%d)\n", pro);	
	else if(pro == 6)	    
		printf(" |___ Protocol : TCP (%d)\n", pro);
	else if(pro == 17)	    
		printf(" |___ Protocol : UDP (%d)\n", pro);
	else if(pro == 136)	    
		printf(" |___ Protocol : UDPLite (%d)\n", pro);
	else	    
		printf(" |___ Protocol : %d\n", pro);

}

int main(int argc, char **argv){

	FrameHeader_t *frameheader;
	IPHeader_t *ip_header;
	TCPHeader_t *tcp_header;
	UDPHeader_t *udp_header;
	ICMPHeader_t *icmp_header;

	int ip_proto;
	int src_port, dst_port;
	char buf[BUFSIZE], Time[STRSIZE];
	char src_ip[STRSIZE], dst_ip[STRSIZE];

	char str[BUFSIZE], File[BUFSIZE];
	char *filter = str;
	int count = 1;
	memset(str, 0, sizeof(str));
	memset(File, 0, sizeof(File));
	
	if(argc >= 2)
		strcat(File, argv[count++]);
	while(argc != count) {
		//filter = argv[1];
		strcat(str, argv[count++]);
		strcat(str, " ");
	}

	memset(buf, 0, sizeof(buf));
	pcap_t * pcap = pcap_open_offline(File, buf);

	struct bpf_program fcode;
	if(-1 == pcap_compile(pcap, &fcode, filter, 1, PCAP_NETMASK_UNKNOWN)) {
		fprintf(stderr, "pcap_compile(): %s\n", pcap_geterr(pcap));
		pcap_close(pcap);
		exit(1);
	}

	struct pcap_pkthdr *header;
    const u_char *data;

	int returnValue;
    u_int packetCount = 0, total = 0;
    while (returnValue = pcap_next_ex(pcap, &header, &data) >= 0){

		packetCount++;
		if(pcap_offline_filter(&fcode, header, data) == 0)
			continue;
		total++;

		printf("   ----------------------------------------   \n\n");
        printf(" < Packet #%i >\n\n", packetCount);
 
        //printf(" Packet size: %u bytes\n", header->len);
		printf(" Frame %i - %u bytes\n", packetCount, header->len);

		/* Frame */
		struct tm *info;
		info = localtime( &header->ts.tv_sec );
		strftime(Time, sizeof(Time), "%b %d , %Y %X", info);
		printf(" |___ Arrival Time : %s.%ld\n",Time, header->ts.tv_usec);
        printf(" |___ Epoch Time : %ld.%ld seconds\n", header->ts.tv_sec, header->ts.tv_usec);
		printf(" |___ Frame Length : %u bytes ( %u bits )\n", header->len, header->len*8);
		printf(" |___ Capture Length : %u bytes ( %u bits )\n", header->caplen, header->caplen*8);

        for (u_int i=0; (i < header->caplen); i++){
            if ((i % 16) == 0) printf("\n");
            printf(" %.2x", data[i]);
        }

		const char *ptr = data;
		
		/* Ethernet 14B */
		frameheader=(struct FrameHeader_t *)ptr;
		printf("\n\n Ethernet\n");
		printf(" |___ Destination address : %.02x:", frameheader->DstMAC[0]); 	// 6位變1位
		printf("%.02x:", frameheader->DstMAC[1]);
		printf("%.02x:", frameheader->DstMAC[2]);
		printf("%.02x:", frameheader->DstMAC[3]);
		printf("%.02x:", frameheader->DstMAC[4]);
		printf("%.02x\n", frameheader->DstMAC[5]);	  
		printf(" |___    Source address   : %02x:", frameheader->SrcMAC[0]);	// 6位變1位
		printf("%02x:", frameheader->SrcMAC[1]);
		printf("%02x:", frameheader->SrcMAC[2]);
		printf("%02x:", frameheader->SrcMAC[3]);
		printf("%02x:", frameheader->SrcMAC[4]);
		printf("%02x\n", frameheader->SrcMAC[5]);
		if( ntohs(frameheader->FrameType) == 0x800 )
			printf(" |___ Type : IPv4 (0x%04x)\n", ntohs(frameheader->FrameType));
		else if( ntohs(frameheader->FrameType) == 0x86DD )
			printf(" |___ Type : IPv6 (0x%04x)\n", ntohs(frameheader->FrameType));
		else
			printf(" |___ Type : 0x%04x\n", ntohs(frameheader->FrameType));
		ptr +=  14;

		/* IP 20B */
		ip_header = (IPHeader_t *)ptr;
		printf("\n\n Internet Protocol\n");
	    printf(" |___ Version & Header Length : %x\n", ip_header->Ver_HLen);
	    printf(" |___ Differentiated Services Field : 0x%02x\n", ntohs(ip_header->TOS));
	    printf(" |___ Total Length : %d\n", ntohs(ip_header->TotalLen));
	    printf(" |___ Identification : 0x%04x (%d)\n", ntohs(ip_header->ID), ntohs(ip_header->ID));
	    printf(" |___ Flags : 0x%04x\n", ntohs(ip_header->Flag_Segment));
	    printf(" |___ Time to live : %d\n", ip_header->TTL);
		PROTOCOL(ip_header->Protocol);
	    printf(" |___ Header checksum : 0x%04x\n", ntohs(ip_header->Checksum));

		int size_payload = ntohs(ip_header->TotalLen);

		inet_ntop(AF_INET,(void *)&(ip_header->SrcIP),src_ip,16);
		inet_ntop(AF_INET,(void *)&(ip_header->DstIP),dst_ip,16);
		ip_proto=ip_header->Protocol;
		
		printf(" |___ Source : %s\n |___ Destination : %s\n", src_ip, dst_ip);
		ptr += 20;
		size_payload = size_payload - 20;

		//printf("ip_proto = %d\n",ip_proto);
		/* UDP 8B */
		if(ip_proto == 17 || ip_proto == 136){		//	判斷是不是 UDP 協議
		
			udp_header=(UDPHeader_t *)ptr;
			ptr += 8;

			printf("\n\n User Datagram Protocol\n");
			printf(" |___ Source Port : %d\n", ntohs(udp_header->SrcPort));
			printf(" |___ Destination port : %d\n", ntohs(udp_header->DstPort));
			//printf(" |___ Header Length : %d\n", udp_header->HeaderLen);
			printf(" |___ Checksum = 0x%04x\n", ntohs(udp_header->Checksum));

			size_payload = size_payload - 8;
		}

		/* TCP 20B */
		else if(ip_proto == 6){

			tcp_header = (TCPHeader_t *)ptr;
			ptr += 20;

			src_port = ntohs(tcp_header->SrcPort);
			dst_port = ntohs(tcp_header->DstPort);
			printf("\n\n Transmission Control Protocol\n");
			printf(" |___ Source Port : %d\n", ntohs(tcp_header->SrcPort));
			printf(" |___ Destination port : %d\n", ntohs(tcp_header->DstPort));
			//printf(" |___ Sequence number : %u\n", ntohl(tcp_header->SeqNO));
			//printf(" |___ Acknowledgment number : %u\n", ntohl(tcp_header->AckNO));
			//printf(" |___ Header Length : 0x%02x\n", tcp_header->HeaderLen);
			printf(" |___ Flags : 0x%03x\n", tcp_header->Flags);
			printf(" |___ Window size value = %d\n", ntohs(tcp_header->Window));
			printf(" |___ Checksum = 0x%04x\n", ntohs(tcp_header->Checksum));
			printf(" |___ Urgent pointer = %d\n", tcp_header->UrgentPointer);

			size_payload = size_payload - 20;
		}

		/* ICMP 8B */
		else if(ip_proto == 1){

			icmp_header = (ICMPHeader_t *)ptr;
			ptr += 8;

			printf("\n\n Internet Control Message Protocol\n");
			printf(" |___ Type = %d\n", ntohs(icmp_header->type));
			printf(" |___ Code = %d\n", ntohs(icmp_header->code));
			printf(" |___ Checksum = 0x%04x\n", ntohs(icmp_header->checksum));

			size_payload = size_payload - 8;
		}

		//printf("pay = %d\n",size_payload);
		printf("\n");
		print_payload(ptr, size_payload);
        printf("\n");

		printf("   ----------------------------------------   \n");
    }

    if(returnValue == -1)
        fprintf(stderr, "pcap_next_ex(): %s\n", pcap_geterr(pcap));

	if(strlen(filter) != 0) {
		printf("File : %s , filter: %s, argc = %d, str = %s, count = %i\n", File, filter, argc, str, total);
	}

	return 0;
}

