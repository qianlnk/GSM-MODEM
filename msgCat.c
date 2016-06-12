
/*****************************************************************************
 *	filename: msgCat.c
 *	interface1:
 *	    1. scan device
	    int getDeviceName(char *pcDev)
 	    pcDev is device name buf.if return -1 then no device be finded
 *	example:
 *		char acDevName[30];
		if(getDeviceName(acDevName) == -1)
		{
			fprintf(stderr,"setterm error %s\n",strerror(errno));
			return(1);
		}
		else
		{
			printf("the modem name is %s\n",acDevName);
		}
 *  interface2.send message
        int sendMsg(char *pcPhnu,char *pcMsg)
		pcPhnu is the phone number you want to send message,
	 	you can enter multiple number and separate them with ','.
	 	pcMsg is your message.the interface will return how many message send failed.
 *  example:
 *  	char acPhone[] = "1895049XXXX,1895049XXXX";
		char acMsg[] = "xzj，你好！这是短信猫测试信息：！@#￥…，请查收~";
		sendMsg(acPhone,acMsg);
 ****************************************************************************/
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <termio.h>
#include <signal.h>
#include <time.h>
#include <limits.h>             /* CHAR_MAX */
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

typedef struct tagMyMessage_T{
	char acCentreNum[16];
	char acPhoneNum[16];
	char message[4096];
}MyMessage_T;

struct pdu_info {
	char cnswap[32];
	char phswap[32];
};

typedef struct tagConfig_T{
	char acDevName[64];
	char acCentreNum[12];
}Config_T;

int fd;

static struct termio oterm_attr;
static fd_set   fs_read, fs_write;
static struct timeval tv_timeout;
Config_T stConfig;

int write_n(int fd, void *buf, int len,int usec);
int read_n(int fd, void *buf, int len,int usec);
int openport(char *devname);

int getDeviceName(char *pcDev)
{
	char acDevList[9][15] = {	//9 usb ports
		"/dev/ttyUSB0","/dev/ttyUSB1","/dev/ttyUSB2",
		"/dev/ttyUSB3","/dev/ttyUSB4","/dev/ttyUSB5",
		"/dev/ttyUSB6","/dev/ttyUSB7","/dev/ttyUSB8"
	};
	char acATCmd[] = "AT\r";
	FILE *fp;
	int i = 0;
	int j = 0;
	int nWrite = 0;
	int nRead = 0;
	char acReply[1024];
	char acOK[3];
	int nFindFlag = 0;

	for(i = 0; i < 9; i++)
	{
		if(openport(acDevList[i]) == -1)	//can't open.continue
		{
			continue;
		}

		nWrite = write_n(fd,acATCmd,strlen(acATCmd),20000000);

		if(nWrite <= 0)	//can't write, continue
		{
			close(fd);
			continue;
		}
		memset(acReply,0,sizeof(acReply));
		sleep(1);
		nRead = read_n(fd,acReply,sizeof(acReply),20000000);

		if(nRead <= 0)	//can't read,continue
		{
			close(fd);
			continue;
		}
		j = 0;

		while(acReply[j] != '\0')
		{
			memset(acOK,0,sizeof(acOK));
			acOK[0] = acReply[j];
			acOK[1] = acReply[j+1];

			if(strcmp(acOK,"OK") == 0)
			{
				nFindFlag = 1;
				break;
			}
			j++;
		}

		if(nFindFlag == 1)
		{
			close(fd);   //don't close it;
			break;
		}

		close(fd);
	}

	if(i <= 8)
	{
		strcpy(pcDev,acDevList[i]);
	}
	else
	{
		pcDev[0] = '\0';
		return -1;
	}
#if 0
	if((fp = fopen("usbModem.conf", "w+")) == NULL)
	{
		printf("can not open usbModem.conf!\n");
		return -1;
	}
	fputs(pcDev,fp);
	fputs("\n",fp);
	fputs(stConfig.acCentreNum,fp);
	fputs("\n",fp);
	fclose(fp);
#endif
	return 0;
}
void getCentreNumber(char *pcres)
{
	int nwrite = 0;
	int nread = 0;
	int i = 0;
	int j = 0;
	int nFlag = 0;
	FILE *fp;
	char acATCmd[]="AT+CSCA?\r";
	char acReply[1024];
	nwrite = write_n(fd,acATCmd,strlen(acATCmd),20000000);

	memset(acReply,0,sizeof(acReply));
	sleep(1);
	nread = read_n(fd,acReply,sizeof(acReply),20000000);

	while(acReply[i] != '\0')
	{
		if(nFlag != 0)
		{
			nFlag++;
		}

		if(acReply[i] == '"' && nFlag == 0)
		{
			nFlag++;
		}

		if(acReply[i] == '"' && nFlag > 10)
		{
			break;
		}

		if(nFlag >= 5)
		{
			pcres[j] = acReply[i];
			j++;
		}
		i++;
	}
#if 0
	if((fp = fopen("usbModem.conf", "w+")) == NULL)
	{
		printf("can not open usbModem.conf!\n");
		return;
	}
	fputs(stConfig.acDevName,fp);
	fputs("\n",fp);
	fputs(pcres,fp);
	fputs("\n",fp);
	fclose(fp);
#endif
}
#if 0
int readConfigFile()
{
	FILE *fp;
	int i = 0;
	char acBuf[64];
	if((fp = fopen("usbModem.conf", "w")) == NULL)
	{
		return -1;
	}
	memset(acBuf, 0, sizeof(acBuf));
	fgets(acBuf,sizeof(acBuf), fp);
	strcpy(stConfig.acDevName, acBuf);
	while(stConfig.acDevName[i] != '\n' && stConfig.acDevName[i] != '\0')
	{
		i++;
	}
	stConfig.acDevName[i] = '\0';
	i = 0;
	memset(acBuf, 0, sizeof(acBuf));
	fgets(acBuf, sizeof(acBuf), fp);
	strcpy(stConfig.acCentreNum, acBuf);
	while(stConfig.acCentreNum[i] != '\n' && stConfig.acCentreNum[i] != '\0')
	{
		i++;
	}
	stConfig.acCentreNum[i] = '\0';
	fclose(fp);
	return 0;
}
#endif
int openport(char *devname)
{
	fd=open(devname,O_RDWR | O_NOCTTY ,0);
        fcntl(fd,F_SETFL,0);
	if(fd<0)
	{
	      return -1;
	}
        else
              return 0;
}


int setTerm(int fd)
{
	struct termio term_attr;
	if(ioctl(fd,TCGETA,&oterm_attr)<0) return -1;
	if(ioctl(fd,TCGETA,&term_attr)<0) return -1;

	term_attr.c_iflag &=~(IXON|IXOFF|IXANY|INLCR|IGNCR|ICRNL|ISTRIP);
	term_attr.c_lflag &=~(ISIG|ECHO|ICANON|NOFLSH);
	//term_attr.c_lflag &=~(ISIG|ECHO|ICANON|NOFLSH|XCLUDE);
	term_attr.c_cflag &=~CBAUD;
	term_attr.c_cflag |=B9600;
	term_attr.c_oflag &=~(OPOST|ONLCR|OCRNL);

	term_attr.c_cc[VMIN]=0;
	term_attr.c_cc[VTIME]=20;

	if(ioctl(fd,TCSETAW,&term_attr)<0) return(-1);
	if(ioctl(fd,TCFLSH,2)<0) return(-1);

	return 0;
}

int resetTerm(int fd)
{
	if(ioctl(fd,TCSETAW,&oterm_attr)<0) return -1;
	return 0;
}


int read_n(int fd, void *buf, int len,int usec)
{
	int	ret=0,cou=0;
    int retval = 0;
    unsigned long startime=time(NULL);

    FD_ZERO (&fs_read);
    FD_SET (fd, &fs_read);
    tv_timeout.tv_sec = 1;
    tv_timeout.tv_usec = 0;
    retval = select (fd + 1, &fs_read, NULL, NULL, &tv_timeout);
    if (retval < 0)
    {
    	fprintf(stderr,"\nSelect Error");
    	return -1;
    }
    else if (retval)
    {
    	if (FD_ISSET(fd,&fs_read))
    	{
				ret=read(fd,(char *)buf+cou, len);
				cou+=ret;
    	}
	}
    else
    {
    	return -1;
    }
  return cou;
}

int write_n(int fd, void *buf, int len,int usec)
{
	int ret=0,cou=0;
	int retval = 0;
	unsigned long startime=time(NULL);

	FD_ZERO (&fs_write);
	FD_SET (fd, &fs_write);
	tv_timeout.tv_sec = 0;
	tv_timeout.tv_usec = usec;
	retval = select (fd + 1, NULL, &fs_write, NULL, &tv_timeout);

    if (retval < 0)
    {
    	printf("\nSelect Error");
    	return -1;
    }
    else if (retval)
    {
       if (FD_ISSET(fd,&fs_write))
       {
    	   while(len>0)
    	   {
    		   if(time(NULL)>startime+4)
    		   {
    			   break;
    		   }
    		   ret=write(fd,(char*)buf+cou,len);
    		   if(ret < 0)
    		   {
    			   return -1;
    		   }
    		   cou+=ret;
    		   len-=ret;
    	   }
       }
    }
    else
	{
    	return -1;
	}
    return cou;
}

//write data to port and  judge receive information
int send(int fd,char *cmgf,char *cmgs,char *message)
{
	int nread,nwrite;
	char buff[4096];
	char reply[4096];

	memset(buff,0,sizeof(buff));
	strcpy(buff,"at\r");
	nwrite = write_n(fd,buff,strlen(buff),20000000);

	memset(reply,0,sizeof(reply));
	sleep(1);

	nread = read_n(fd,reply,sizeof(reply),20000000);

    memset(buff,0,sizeof(buff));
	strcpy(buff,"AT+CMGF=");
	strcat(buff,cmgf);
	strcat(buff,"\r");
	nwrite = write_n(fd,buff,strlen(buff),20000000);

    memset(reply,0,sizeof(reply));
	sleep(1);
	nread = read_n(fd,reply,sizeof(reply),20000000);

    memset(buff,0,sizeof(buff));
	strcpy(buff,"AT+CMGS=");
	strcat(buff,cmgs);
    strcat(buff,"\r");
    nwrite = write_n(fd,buff,strlen(buff),20000000);

    memset(reply,0,sizeof(reply));
    sleep(1);
    nread = read_n(fd,reply,sizeof(reply),20000000);

    memset(buff,0,sizeof(buff));
    strcpy(buff,message);
    nwrite = write_n(fd,buff,strlen(buff),20000000);

    memset(reply,0,sizeof(reply));
    sleep(1);
    nread = read_n(fd,reply,sizeof(reply),20000000);
}

//processing phone
void swap(char number[],char swap[])
{
	char ch1[32];// = "86";
    char tmp[32];
    int i;

	memset(swap,0,32);
    memset(tmp,0,16);
    memset(tmp,0,32);
    strcpy(ch1,"86");
    strcpy(swap,number);
    strcat(swap,"f");
    strcat(ch1,swap);
    strcpy(swap,ch1);

	for(i = 0;i <= strlen(swap) - 1;i += 2)
	{
		tmp[i + 1] = swap[i];
		tmp[i] = swap[i + 1];
	}
	strcpy(swap,tmp);
}

//test send character
int send_en_message(MyMessage_T info)
{
    char cmgf[] = "1";
    int conter = 0;
    char cmgs[16] = {'\0'};
	strcpy(info.acPhoneNum,"18950498839");

	/*
	gets(info.phnu);
	while(strlen(info.phnu) != 11){
		if(conter >= 3){
			printf("conter out !\n");
			return -1;
		}
		printf("number shuld be --11-- bits ! enter agin :\n");
		gets(info.phnu);
		conter ++;
    }*/

	printf("enter you message !\n");
	strcat(info.message,"\x1a");
	strcat(cmgs,info.acPhoneNum);
	send(fd,cmgf,cmgs,info.message);
}

//send message to port
int send_zh_message(MyMessage_T info)
{
	char cmgf[] = "0";
    char cmgs[4] = {'\0'};
    char ch2[] = "0891";
    char ch3[] = "1100";
    char ch4[] = "000800";
    char ch5[] = "0d91";
    char final[128];
    struct pdu_info pdu;
    int conter = 0,flag = 1,len;
    memset(final,0,128);

    swap(info.acPhoneNum,pdu.phswap);
    swap(info.acCentreNum,pdu.cnswap);

	strcpy(final,ch2);
    strcat(final,pdu.cnswap);
    strcat(final,ch3);
    strcat(final,ch5);
    strcat(final,pdu.phswap);
    strcat(final,ch4);
    strcat(final,info.message);
    strcat(final,"\x1a");

    len = strlen(ch3)+ strlen(ch4)+ strlen(ch5)+strlen(pdu.phswap)+ strlen(info.message);
    //puts(final);
    sprintf(cmgs,"%d",len/2);
    //puts(final);
    send(fd,cmgf,cmgs,final);
}

//put a single Chinese character into utf8 encoding
void OneGBToUTF8(char *pcHZ, char *pcRes)
{
	unsigned short usDst;
	usDst = (pcHZ[0] & 0X1F) << 12;
	usDst |= (pcHZ[1] & 0x3F) << 6;
	usDst |= (pcHZ[2] & 0x3F);
	sprintf(pcRes,"%X",usDst);
}
//string in both chinese and english --->utf8 encoding
void GBToUTF8(char *pcHZ, char *pcRes)
{
	char acTmp[3];
	char acDst[4] = {'\0'};
	char acDstTmp[4] = {'\0'};
	int i = 0;
	strcpy(pcRes,"00");//reserve 2char for msg length
	while(pcHZ[i] != '\0')
	{
		if(pcHZ[i] >= 0)
		{
			strcpy(acDst,"00");
			sprintf(acDstTmp,"%X",pcHZ[i]);
			strcat(acDst,acDstTmp);
            //printf("EN-->%s\n",acDst);
		}
		else
		{
			acTmp[0] = pcHZ[i];
			i++;
			acTmp[1] = pcHZ[i];
			i++;
			acTmp[2] = pcHZ[i];
			OneGBToUTF8(acTmp,acDst);
            //printf("ZH-->%s\n",acDst);
		}
		strcat(pcRes,acDst);
		i++;
	}
}

//add message length in front of utf8 encoding string
void GBToUTF8WithMsgLen(char *pcHZ, char *pcRes)
{
	char acLen[2] = {'\0'};
	GBToUTF8(pcHZ,pcRes);
	sprintf(acLen,"%X",strlen(pcRes)/2);
	if(acLen[1] == '\0')
	{
		pcRes[1] = acLen[0];
	}
	else
	{
		pcRes[0] = acLen[0];
		pcRes[1] = acLen[1];
	}

}

//divide message,let every message'length is not more then 27
void DivideMsg(char *pcMsg, char acMsg[][82])
{
	if(NULL == pcMsg || NULL == acMsg)
	{
		return;
	}
	int nOneMsgLen = 0;
	int i = 0;
	int j = 0;
	int k = 0;
	while(pcMsg[i] != '\0')
	{
		if(pcMsg[i] >= 0)
		{
			acMsg[j][k] = pcMsg[i];
			nOneMsgLen++;
		}
		else
		{
			acMsg[j][k] = pcMsg[i];
			i++;k++;
			acMsg[j][k] = pcMsg[i];
			i++;k++;
			acMsg[j][k] = pcMsg[i];
			nOneMsgLen++;
		}
		i++;k++;
		if(nOneMsgLen == 27)
		{
			j++;
			k = 0;
			nOneMsgLen = 0;
		}
	}
}

//the interface for send message
int sendMsg(char *pcPhnu,char *pcMsg)
{
	if(NULL == pcPhnu || NULL == pcMsg)
	{
		return -1;
	}
	MyMessage_T info;
	char acOneNum[12] = {'\0'};	//one phnoe number
	char acMsg[10][82] = {'\0'};//27*3=81
	int nSendFail = 0;			//send failed count
	int i = 0; 					//for loop
	int j = 0;					//for Phone number loop
	int k = 0;					//for msg loop 13800591500
#if 0
	if(readConfigFile() == -1)
	{
		printf("error:can't open file usbModem.conf!\n");
		return(1);
	}
#endif

		//printf("asdasd\n");
	if(getDeviceName(stConfig.acDevName) == -1)
	{
		fprintf(stderr,"setterm error %s\n",strerror(errno));
		return(1);
	}
	//printf("device name:%s\n",stConfig.acDevName);

	if(openport(stConfig.acDevName) == -1)
	{
		fprintf(stderr,"setterm error %s\n",strerror(errno));
		return(1);
	}
  	printf("getting centre number,please wait...\n");
  	if(stConfig.acCentreNum[0] == '\0')
  	{
  		getCentreNumber(stConfig.acCentreNum);
  		printf("centre number:%s\n",stConfig.acCentreNum);
  	}
  	printf("sending message,it may take more time,please wait...\n");

	if(setTerm(fd))
	{
        fprintf(stderr,"setterm error %s\n",strerror(errno));
		return(1);
	}


	if((fcntl(fd, F_SETFL, FNDELAY))==-1)
		return -1;

    strcpy(info.acCentreNum,stConfig.acCentreNum);
    DivideMsg(pcMsg,acMsg);
    while(pcPhnu[i] != '\0')
    {
    	while(pcPhnu[i] != ',' && pcPhnu[i] != '\0')
    	{
    		acOneNum[j] = pcPhnu[i];
    		i++;
    		j++;
    	}
    	if(j != 11)	//phone number error
    	{
    		nSendFail++;
    	}
    	else
    	{
    		strcpy(info.acPhoneNum,acOneNum);

    		for(k = 0; k < 10; k++)
    		{
    			if(acMsg[k][0] == '\0')
    			{
					break;
    			}
    			GBToUTF8WithMsgLen(acMsg[k],info.message);
    			send_zh_message(info);
    			sleep(3);//give usb port some time for rest
    		}
    		printf("send message to phone %s is OK!\n",info.acPhoneNum);

    	}
    	if(pcPhnu[i] == '\0')//no more phone number
    	{
    		break;
    	}
		j = 0;
		i++;
    }

    close(fd);
    return nSendFail;
}

int main(int argc, char *argv[])
{
	char buf[20];
	char cc[]="at\r";
	memset(buf,0,20);
	char acDevName[30];
	char acPhone[] = "1895049XXXX,1895049XXXX";
	char acMsg[] = "月下独狼";
	//write_n(fd,cc,3,2000000);
	//sleep(2);
	//read_n(fd,buf,20,20000000);
	//printf("buf:%s\n",buf);
	printf("looking for modem,please wait...\n");
	if(getDeviceName(acDevName) == -1)
	{
		fprintf(stderr,"setterm error %s\n",strerror(errno));
		return(1);
	}
	else
	{
		printf("the modem name is %s\n",acDevName);
	}
	sendMsg(acPhone,acMsg);

	return 0;
}


