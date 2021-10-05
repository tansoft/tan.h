#include <stdio.h>
#include <windows.h>
#include <direct.h>

#include "Inetinc.h"
#pragma comment(lib,"ws2_32.lib")

void DumpBuf( unsigned char *buf, int Len )
{
	int i, l, t;
	char Tmp[ 0x11 ];
	unsigned char c;

	if( !Len ){
		return;
	}

	printf ("Offset=%p\n",buf);
	l = ( ( Len + 0xf ) >> 4 ) << 4;
	for( i = 0; i < l; i++ ){
		if( ( i & 0xfffffff0 ) == (unsigned)i ){
			Tmp[ 0x10 ] = 0;
			printf( "%08X: ", i );
		}
		t = i & 0xf;
		if( i < Len ){
			c = buf[ i ];
			printf( "%02X ", c );
#ifdef PRINT_ORGDATA
			Tmp[ t ] = ( c > 0x1f) ? c : 0x2e;
#else
			Tmp[ t ] = ( c > 0x1f && c < 0x80 ) ? c : 0x2e;
#endif
		}else{
			printf( "   " );
			Tmp[ t ] = 0x20;
		}
		if( t == 0x7 ) printf( "- " );
		if( t == 0xf ) printf( "  %s\n", Tmp );
	}
}

static unsigned char ts [0x10000];

unsigned char * GetPacketFromMiniSnifferFile (FILE *file,int *n,int *nts)
{
	unsigned int len;
    if (ftell(file)==0)
	{
		//the frist byte is data count
        fseek (file,4,SEEK_CUR);
		//not have ethhdr part
		memset(ts,0,sizeof(struct ethhdr));
	}
	if (fread(&len,1,sizeof(unsigned int),file)!=sizeof(unsigned int))
		return NULL;
    if (fread (ts+sizeof(struct ethhdr),1,len,file)!=len)
        return NULL;
    *n=len+sizeof(struct ethhdr);
    return ts;
}

unsigned char * GetPacketFromSpyNetFile (FILE *file,int *n,int *nts)
{
	unsigned char slen;
	unsigned int len;
    if (ftell(file)==0)
	{
		fread (&slen,1,sizeof (unsigned char),file);
        fseek (file,slen,SEEK_CUR);
	}

	if (fread(&len,1,sizeof(unsigned int),file)!=sizeof(unsigned int))
		return NULL;
	if (nts)//nts have some problem
		fread(nts,1,sizeof(unsigned int),file);
	else
		fseek(file,4,SEEK_CUR);
    if (fread (ts,1,len,file)!=len)
        return NULL;
    *n=len;
    return ts;
}

unsigned char * GetPacketFromSnifferCapFile(FILE * file,int * n,int *nts)
{
    unsigned short len;
    int r;

    if (ftell(file)==0)
        fseek (file,0x80,SEEK_CUR);

	if (nts)
	{
		fread (nts,1,sizeof (int),file);
		fseek(file,0x4,SEEK_CUR);
	}
	else
		fseek (file,0x8,SEEK_CUR);
    r=(int)fread (&len,1,sizeof (unsigned short),file);
    if (r!=sizeof(unsigned short))
        return NULL;

    fseek (file,0x1e,SEEK_CUR);
    if (fread (ts,1,len,file)!=len)
        return NULL;
    *n=len;
    return ts;
}

typedef unsigned char * (*CapFileHander)(FILE * file,int * n,int *nts);
//imtest.exe c:\a.cap
//#include <THString.h>
//#include <THSysMisc.h>

int main (int argc , void *argv[])
{
	FILE *f,*of;
	CapFileHander handler=NULL;
	unsigned char *buf;
	int buflen;
	int ti=0;
	int ui=0;
	char outf[1024];
	char outp[1024];
	char outpo[1024];
	char FileFormat[100];
	char infile[1024];

	//THString sfile=THSysDialog::FileDialogOpenExistOne();
	const char *fn=NULL;
	if (argc>=2)
		fn=(const char *)argv[1];
	else
	{
		printf("input the filename:");
		scanf("%s",infile);
		fn=infile;
	}
	//else
	//	fn=sfile;
	f=fopen (fn,"rb");

	//f=fopen ((const char *)argv[1],"rb");
	if (!f)
	{
		printf("out file error");
		return -1;
	}

	fread(FileFormat,1,100,f);
	fseek(f,0,SEEK_SET);
	if (strncmp(FileFormat,"XCP",3)==0)
	{
		handler=GetPacketFromSnifferCapFile;
		printf("Cap File: %s\r\nFile Format: Sniffer",fn);
	}
	else if (strncmp(&FileFormat[1],"SpyNet",6)==0)
	{
		handler=GetPacketFromSpyNetFile;
		printf("Cap File: %s\r\nFile Format: SpyNet",fn);
	}
	else if (strstr(fn,".sfd")!=NULL)
	{
		handler=GetPacketFromMiniSnifferFile;
		printf("Cap File: %s\r\nFile Format: MiniSniffer",fn);
	}
	else
	{
		printf("Cap File: %s\r\nFile Format: Unknown",fn);
		return -1;
	}

	strcpy(outp,fn);
	strcat(outp,"_out");
	mkdir(outp);
	strcpy(outpo,outp);
	strcat(outpo,".txt");

	int tts=0;
	int ts=0;
	while (buf=handler(f,&buflen,&ts))
	{
		if (tts==0) tts=ts;
		ts-=tts;
		ts/=1000;
		struct ethhdr *ethhdr;
		struct ip *iphdr;
		struct udphdr *udphdr;
		unsigned char *data;
		//int direction; //1 from pc 0 from server.
		int datalen,datalen1;
		struct tcphdr *tcphdr;

		ethhdr=(struct ethhdr *)buf;
		//direction=( (ethhdr->h_source[1]==0xe0));// || ((ethhdr->h_source[1]==0x11)));//eth card address 00XX00000000
		//printf("hsoucre:%X\n",ethhdr->h_source[1]);
		iphdr=(struct ip *)(buf+sizeof(struct ethhdr));

		if (iphdr->ip_p==IPPROTO_TCP)
		{
			tcphdr=(struct tcphdr *)(buf+sizeof(struct ethhdr)+sizeof (struct ip));
			datalen1=buflen-(sizeof(struct ethhdr)+sizeof (struct ip)+tcphdr->th_off*4);
			datalen=ntohs(iphdr->ip_len)-sizeof (struct ip)-tcphdr->th_off*4;
			if (datalen<2)
				continue;
#ifdef ONLY_1863
			if (!(ntohs(tcphdr->th_dport)==1863	||ntohs(tcphdr->th_sport)==1863))
				continue;
#endif
			data=(buf+buflen-datalen1);
			printf("(%u)%d.%d.%d.%d:%d->%d.%d.%d.%d:%d(tcp)\n",
				ts,
				iphdr->ip_src.S_un.S_un_b.s_b1,
				iphdr->ip_src.S_un.S_un_b.s_b2,
				iphdr->ip_src.S_un.S_un_b.s_b3,
				iphdr->ip_src.S_un.S_un_b.s_b4,
				ntohs(tcphdr->th_sport),
				iphdr->ip_dst.S_un.S_un_b.s_b1,
				iphdr->ip_dst.S_un.S_un_b.s_b2,
				iphdr->ip_dst.S_un.S_un_b.s_b3,
				iphdr->ip_dst.S_un.S_un_b.s_b4,
				ntohs(tcphdr->th_dport));
			DumpBuf(data,datalen);
			sprintf(outf,"%s\\tcp_%03d_%03u_%d.%d.%d.%d_%d-%d.%d.%d.%d_%d_%03d.txt",outp,ti++,ts,
				iphdr->ip_src.S_un.S_un_b.s_b1,
				iphdr->ip_src.S_un.S_un_b.s_b2,
				iphdr->ip_src.S_un.S_un_b.s_b3,
				iphdr->ip_src.S_un.S_un_b.s_b4,
				ntohs(tcphdr->th_sport),
				iphdr->ip_dst.S_un.S_un_b.s_b1,
				iphdr->ip_dst.S_un.S_un_b.s_b2,
				iphdr->ip_dst.S_un.S_un_b.s_b3,
				iphdr->ip_dst.S_un.S_un_b.s_b4,
				ntohs(tcphdr->th_dport),
				datalen);
			of=fopen(outf,"wb");
			fwrite(data,datalen,1,of);
			fclose(of);
			of=fopen(outpo,"ab");
			fseek(of,0,SEEK_END);
			fwrite(data,datalen,1,of);
			fwrite("\r\n\r\n",4,1,of);
			fclose(of);
		}
		else if (iphdr->ip_p==IPPROTO_UDP)
		{
			udphdr=(struct udphdr *)(buf+sizeof(struct ethhdr)+sizeof (struct ip));
			datalen=buflen-(sizeof(struct ethhdr)+sizeof (struct ip)+sizeof(struct udphdr));
			data=(buf+buflen-datalen);
			datalen=ntohs(udphdr->uh_ulen)-8;
#ifdef ONLY_1863
			if (!(ntohs(udphdr->uh_dport)==1863	||ntohs(udphdr->uh_sport)==1863))
				continue;
#endif
			printf("(%u)%d.%d.%d.%d:%d->%d.%d.%d.%d:%d(udp)\n",
				ts,
				iphdr->ip_src.S_un.S_un_b.s_b1,
				iphdr->ip_src.S_un.S_un_b.s_b2,
				iphdr->ip_src.S_un.S_un_b.s_b3,
				iphdr->ip_src.S_un.S_un_b.s_b4,
				ntohs(udphdr->uh_sport),
				iphdr->ip_dst.S_un.S_un_b.s_b1,
				iphdr->ip_dst.S_un.S_un_b.s_b2,
				iphdr->ip_dst.S_un.S_un_b.s_b3,
				iphdr->ip_dst.S_un.S_un_b.s_b4,
				ntohs(udphdr->uh_dport));
			DumpBuf(data,datalen);
			sprintf(outf,"%s\\udp_%03d_%03u_%d.%d.%d.%d_%d-%d.%d.%d.%d_%d_%03d.txt",outp,ui++,ts,
				iphdr->ip_src.S_un.S_un_b.s_b1,
				iphdr->ip_src.S_un.S_un_b.s_b2,
				iphdr->ip_src.S_un.S_un_b.s_b3,
				iphdr->ip_src.S_un.S_un_b.s_b4,
				ntohs(udphdr->uh_sport),
				iphdr->ip_dst.S_un.S_un_b.s_b1,
				iphdr->ip_dst.S_un.S_un_b.s_b2,
				iphdr->ip_dst.S_un.S_un_b.s_b3,
				iphdr->ip_dst.S_un.S_un_b.s_b4,
				ntohs(udphdr->uh_dport),
				datalen);
			of=fopen(outf,"wb");
			fwrite(data,datalen,1,of);
			fclose(of);
			of=fopen(outpo,"ab");
			fseek(of,0,SEEK_END);
			fwrite(data,datalen,1,of);
			fwrite("\r\n\r\n",4,1,of);
			fclose(of);
		}
	}
	return 0;
}
