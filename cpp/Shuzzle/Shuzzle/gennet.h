
//
// The General Networking system
//
// A simple layer over BSD sockets to simplify things
//

#include <stdio.h>

#define NET_ON_WIN32 0
#define NET_ON_UNIX  1

#define net_buffersize    400
char net_textbuffer[net_buffersize];


class Net_Transport;

typedef void (*Net_Callback)(Net_Transport* trans, void* extra);


#if NET_ON_UNIX

bool Net_InitNetworking() { return true; }

class Net_Transport
{
public:
    bool mOpen, mIsHost, mHadError;
    char mErrorBuffer[net_buffersize];
    
    void Send(char* data, int bytes) {}
    bool DataToRecieve() { return false; }
    int Recieve(char* data, int max, bool* waserror=0) { return 0; }
    void Close() {}
    
    void SendString(char* txt) {}
    bool RecieveString(char* to, int max) {return false;}
    
    //Host methods
    bool Host_Setup() {return false;}
    bool Host_WaitForClient_Blocking() {return false;}
    void Host_WaitForClient_Begin(Net_Callback whendone, void* extra) {}
    void Host_WaitForClient_Cancel() {}
    
    //Client Methods
    bool Client_Setup(char* ip) {return false;}
    
    ~Net_Transport() {Close();};
    Net_Transport() {mOpen=false; mErrorBuffer[0]=0;
        mHadError=false; mIsHost=false;};
    
    //THESE ARE PRIVATE: Don't use them!
    void _HadError(char* mess) {printf("%s",mess);strcpy(mErrorBuffer,mess);mHadError=true;};
    
    unsigned long _NB_ID;
    void* _NB_Extra;
    bool _NB_ShouldCancel;
    Net_Callback _NB_WhenDone;
};


#else

#include <windows.h>
#include <winsock.h>


#define NET_FAILED			false
#define NET_DEFAULTSOCKET		2345

#define NET_IP_DESKTOP		"192.168.0.2"
#define NET_IP_RECURSIVE	"127.0.0.1"



bool Net_InitNetworking()
{
#if NET_ON_WIN32
	WORD ver = MAKEWORD(1,2);
	WSADATA wsaData;
	return (WSAStartup(ver, &wsaData)==0);
#else
    return true;
#endif
}

bool Net_GetMyLANIP(char* ipname)
{
#if NET_ON_WIN32
	hostent* localHost = gethostbyname("");
	char* f = inet_ntoa (*(in_addr *)localHost->h_addr_list[0]);
	strcpy(ipname, f);
#else
    return false;
#endif
}

bool Net_GetMyIP(char* ipname)
{
#if NET_ON_WIN32
//	ipname[0]=0;
//	return true;

	hostent* localHost = gethostbyname("");
	if(localHost->h_addr_list[1])
	{
		char* j = inet_ntoa(*(in_addr *)localHost->h_addr_list[1]);
		strcpy(ipname, j);
		return true;
	}

	char* f = inet_ntoa (*(in_addr *)localHost->h_addr_list[0]);
	strcpy(ipname, f);
	return true;
#else
    return false;
#endif

	/*
	if (gethostname(net_textbuffer, net_buffersize) == SOCKET_ERROR)
	{
		return false;
	}
	hostent *phe = gethostbyname(net_textbuffer);
	if (phe->h_addr_list[0]==0)
		return false;
	in_addr* addr = (in_addr*)phe->h_addr_list[0];
	strcpy(ipname, inet_ntoa(*addr));
	return true;
	*/
}

#if NET_ON_WIN32
bool Net_SetBlockingMode(SOCKET s, bool isblocking)
{
	unsigned long notblocking = !isblocking;
	if (ioctlsocket(s, FIONBIO, &notblocking)==SOCKET_ERROR)
	  return false;
	return true;
}

void neterror2(char* text)
{
//	ShowMessage(text, "Net Error");
	MessageBox(0, text, "Net Error:", MB_OK);
}
#endif


class Net_Transport
{
public:
#if NET_ON_WIN32
	SOCKET mSocket, mHostSocket;
#endif
	bool mOpen, mIsHost, mHadError;
	char mErrorBuffer[net_buffersize];

	void Send(char* data, int bytes);
	bool DataToRecieve();
	int Recieve(char* data, int max, bool* waserror=0);
	void Close();

	void SendString(char* txt);
	bool RecieveString(char* to, int max);

	//Host methods
	bool Host_Setup();
	bool Host_WaitForClient_Blocking();
	void Host_WaitForClient_Begin(Net_Callback whendone, void* extra);
	void Host_WaitForClient_Cancel();

	//Client Methods
	bool Client_Setup(char* ip);

	~Net_Transport() {Close();};
	Net_Transport() {mOpen=false; mErrorBuffer[0]=0;
		mHostSocket=mSocket=INVALID_SOCKET; mHadError=false; mIsHost=false;};

//THESE ARE PRIVATE: Don't use them!
	void _HadError(char* mess) {neterror2(mess);strcpy(mErrorBuffer,mess);mHadError=true;};

	unsigned long _NB_ID;
	HANDLE _NB_Handle;
	void* _NB_Extra;
	bool _NB_ShouldCancel;
	Net_Callback _NB_WhenDone;
};

bool Net_Transport::RecieveString(char* to, int max)
{
	int len;
	bool err;
	Net_SetBlockingMode(mSocket, true);
	Recieve( (char*)&len, sizeof(int), &err);
	if (err)
		return false;
	if (len > max)
	{
		_HadError("String buffer too small");
		return false;
	}
	Recieve( to, len, &err );
	return !err;
}

void Net_Transport::SendString(char* text)
{
	int len = strlen(text);
	len+=2;
	Send( (char*)&len, sizeof(int) );
	Send( text, len );
}

unsigned long _stdcall_net_NoBlockingConnect(void* param)
{
#if NET_ON_WIN32
	Net_Transport* trans = (Net_Transport*)param;

	Net_SetBlockingMode(trans->mHostSocket, false);
	SOCKET work;

	while(!trans->_NB_ShouldCancel)
	{
		work = accept( trans->mHostSocket, 0, 0 );
		if (work != INVALID_SOCKET)
		{
			trans->mSocket = work;
			Net_SetBlockingMode(trans->mHostSocket, true);
			if (trans->_NB_WhenDone)
				(*trans->_NB_WhenDone)(trans, trans->_NB_Extra);
			return 0;
		}
		else
			Sleep(200);
	}

	trans->Close();

	return 1;
#else
    return false;
#endif
}

void Net_Transport::Host_WaitForClient_Cancel()
{
	_NB_ShouldCancel = true;
}

void Net_Transport::Host_WaitForClient_Begin(Net_Callback whendone, void* extra)
{
	_NB_Extra = extra;
	_NB_WhenDone = whendone;
	_NB_ShouldCancel = false;

	_NB_Handle=CreateThread(0,0,net_NoBlockingConnect,this,0,&_NB_ID);
}

int Net_Transport::Recieve(char* data, int max, bool* waserror)
{
	if (*waserror)
		*waserror = false;
	int len = recv(mSocket, data, max, 0);
	if (len <= 0)
	{
		mHadError = true;
		if (waserror)
		{
			*waserror = true;
		}
	}
	return len;
}

void Net_Transport::Send(char* data, int bytes)
{
	int err = send(mSocket, data, bytes, 0);
	if (err == SOCKET_ERROR)
		_HadError("Couldn't send data");
}

bool Net_Transport::DataToRecieve()
{
	fd_set input_set, exc_set; // Create Input and Error sets for the select function
	
	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	FD_ZERO(&input_set);
	FD_ZERO(&exc_set);

	FD_SET(mSocket,&input_set);
    FD_SET(mSocket,&exc_set);

	int s = select(0,&input_set,NULL,&exc_set,&timeout);

	if(s > 0)
	{
		if (FD_ISSET(mSocket,&exc_set)) 
		{
			_HadError("Error on socket in select()");
			return false;
		}
		return true;
	}
	return false;
}

bool Net_Transport::Client_Setup(char* ip)
{
  int sd;

  struct sockaddr_in cliAddr;
  //char line[MAX_MSG];


  /* create socket */
  sd = socket(AF_INET, SOCK_STREAM, 0);
  if(sd<0) 
  {
    _HadError("cannot open socket ");
    return false;
  }


  cliAddr.sin_family = AF_INET;
  cliAddr.sin_addr.s_addr = inet_addr( ip );
  cliAddr.sin_port = htons( NET_DEFAULTSOCKET );


  int servsoc = connect(sd, (sockaddr*)&cliAddr, sizeof(cliAddr) );
  if ( servsoc < 0 ) 
  {
	_HadError("Error calling connect()\n");
	return false;
  }

  listen(sd,5);

  mSocket = sd;
  mHostSocket = servsoc;
  mOpen = true;
  mIsHost = false;

  return true;
}

bool Net_Transport::Host_Setup()
{
  int sd;

  struct sockaddr_in servAddr;
  //char line[MAX_MSG];


  /* create socket */
  sd = socket(AF_INET, SOCK_STREAM, 0);
  if(sd<0) 
  {
    _HadError("cannot open socket");
    return false;
  }
  
  /* bind server port */
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(NET_DEFAULTSOCKET);
  
  if(bind(sd, (sockaddr*)&servAddr, sizeof(servAddr))<0) 
  {
    _HadError("cannot bind port ");
    return false;
  }

  listen(sd,5);

  mHostSocket = sd;
  mOpen = true;
  mIsHost = true;

  return true;
}

bool Net_Transport::Host_WaitForClient_Blocking()
{
	Net_SetBlockingMode(mHostSocket, true);
  SOCKET work = accept( mHostSocket, 0, 0 );
  if (work == INVALID_SOCKET)
  {
	  _HadError("Didn't 'accept' properly");
	  return false;
  }

  mSocket = work;
  return true;
}

void Net_Transport::Close()
{
	if (!mOpen)
		return;

	if (mHostSocket!=INVALID_SOCKET)
		closesocket(mHostSocket);
	if (mSocket!=INVALID_SOCKET)
		closesocket(mSocket);

	mErrorBuffer[0]=0;
	mHadError = false;
	mIsHost = false;
	mOpen = false;
	mSocket = INVALID_SOCKET;
	mHostSocket = INVALID_SOCKET;
}

#endif // NET_ON_WIN32

