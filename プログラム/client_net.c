/*****************************************************************
ファイル名	: client_net.c
機能		    : クライアントのネットワーク処理
*****************************************************************/

#include "client_func.h"

#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define	BUF_SIZE 100

/** 変数 **/
/* static */
static int	  gSocket;	/* ソケット */
static fd_set	gMask;	  /* select()用のマスク */
static int	  gWidth;		/* gMask中ののチェックすべきビット数 */

/* 関数 */
static void GetAllName(int *clientID, int *num, char clientNames[][MAX_NAME_SIZE]);
static int  RecvData(void *data, int dataSize);
static void SetMask(void);

/*****************************************************************
関数名	: SetUpClient
機能	  : サーバーとのコネクションを設立し，
		      ユーザーの名前の送受信を行う
引数	  : char  *hostName		    : ホスト
          int   *clientID       : クライアント番号
		      int	  *num			      : 全クライアント数
		      char  clientNames[][] : 全クライアントのユーザー名
出力	  : コネクションに失敗した時-1, 成功した時0
*****************************************************************/
int SetUpClient(char *hostName, int *clientID, int *num, char clientNames[][MAX_NAME_SIZE])
{
  struct hostent	    *servHost;
  struct sockaddr_in	server;
  int			            len;
  char		            str[BUF_SIZE] = "a";

  /* ホスト名からホスト情報を得る */
  if((servHost = gethostbyname(hostName)) == NULL){
		fprintf(stderr, "net error : gethostbyname failed. (SetUpClient)\n");
		return -1;
  }

  bzero((char*)&server,sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  bcopy(servHost->h_addr,(char*)&server.sin_addr,servHost->h_length);

  /* ソケットを作成する */
  if((gSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		fprintf(stderr, "net error : socket failed. (SetUpClient)\n");
		return -1;
  }

  /* サーバーと接続する */
  if(connect(gSocket, (struct sockaddr*)&server, sizeof(server)) == -1){
		fprintf(stderr, "net error : connect failed. (SetUpClient)\n");
		close(gSocket);
		return -1;
  }

  SendData(str, MAX_NAME_SIZE);

  /* 全クライアントのユーザー名を得る */
  GetAllName(clientID, num, clientNames);

  /* select()のためのマスク値を設定する */
  SetMask();

#ifdef DEBUG_PRINT
    fprintf(stderr, "Connection completed\n");
#endif
    
  return 0;
}

/*****************************************************************
関数名	: SendData
機能	  : サーバーにデータを送る
引数	  : void  *data		  : 送るデータ
		      int	  dataSize  : 送るデータのサイズ
出力	  : なし
*****************************************************************/
void SendData(void *data, int dataSize)
{
  /* 引き数チェック */
  assert(data != NULL);
  assert(0 < dataSize);

  write(gSocket, data, dataSize);
}

/*****************************************************************
関数名	: RecvIntData
機能	  : サーバーからint型のデータを受け取る
引数	  : int	*intData  : 受信したデータ
出力	  : 受け取ったバイト数
*****************************************************************/
int RecvIntData(int *intData)
{
  int n, tmp;

  /* 引き数チェック */
  assert(intData != NULL);

  n = RecvData(&tmp, sizeof(int));
  (*intData) = ntohl(tmp);

  return n;
}

/*****************************************************************
関数名   : RecvFloatData
機能     : サーバーからfloat型のデータを受け取る
引数     : float *floatData : 受信したデータ
出力     : 受け取ったバイト数
*****************************************************************/
int RecvFloatData(float *floatData)
{
  int   n; 
  char  tmp[16];

  /* 引き数チェック */
  assert(*floatData != NULL);

  n = RecvData(&tmp, sizeof(tmp));
  (*floatData) = strtof(tmp, NULL);

  return n;
}

/*****************************************************************
関数名	: SendRecvManager
機能	  : サーバーから送られてきたデータを処理する
引数	  : なし
出力	  : プログラム終了コマンドが送られてきた時0を返す．
		      それ以外は1を返す
*****************************************************************/
int SendRecvManager(void)
{
  fd_set	readOK;
  char	command;
  int		i;
  int		endFlag = 1;
  struct timeval	timeout;

  /* select()の待ち時間を設定する */
  timeout.tv_sec = 0;
  timeout.tv_usec = 20;

  readOK = gMask;
  /* サーバーからデータが届いているか調べる */
  select(gWidth, &readOK, NULL, NULL, &timeout);
  if(FD_ISSET(gSocket, &readOK)){
	  /* サーバーからデータが届いていた */
  	/* コマンドを読み込む */
	  RecvData(&command, sizeof(char));
  	/* コマンドに対する処理を行う */
	  endFlag = ExecuteCommand(command);
  }
  return endFlag;
}

/*****************************************************************
関数名	: CloseSoc
機能	  : サーバーとのコネクションを切断する
引数	  : なし
出力	  : なし
*****************************************************************/
void CloseSoc(void)
{
#ifdef DEBUG_PRINT
    fprintf(stderr, "...Connection closed\n");
#endif
  close(gSocket);
}

 /***************
 *   static    *
***************/
/*****************************************************************
関数名	: GetAllName
機能	  : サーバーから全クライアントのユーザー名を受信する
引数	  : int   *clientID       : クライアントID
          int   *num			      : クライアント数
		      char  clientNames[][] : 全クライアントのユーザー名
出力	  : なし
*****************************************************************/
static void GetAllName(int *clientID, int *num, char clientNames[][MAX_NAME_SIZE])
{
  int	i;

  /* クライアント番号の読み込み */
  RecvIntData(clientID);
  /* クライアント数の読み込み */
  RecvIntData(num);

  /* 全クライアントのユーザー名を読み込む */
  for(i = 0; i < (*num); i++){
		RecvData(clientNames[i], MAX_NAME_SIZE);
  }
}

/*****************************************************************
関数名	: RecvData
機能	  : サーバーからデータを受け取る
引数	  : void  *data		  : 受信したデータ
		      int	  dataSize  : 受信するデータのサイズ
出力	  : 受け取ったバイト数
*****************************************************************/
static int RecvData(void *data, int dataSize)
{
  /* 引き数チェック */
  assert(data != NULL);
  assert(0 < dataSize);

  return read(gSocket, data, dataSize);
}

/*****************************************************************
関数名	: SetMask
機能	  : select()のためのマスク値を設定する
引数	  : なし
出力	  : なし
*****************************************************************/
static void SetMask(void)
{
  int	i;

  FD_ZERO(&gMask);
  FD_SET(gSocket, &gMask);

  gWidth = gSocket + 1;
}