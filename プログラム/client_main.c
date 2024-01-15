/*****************************************************************
ファイル名	: client_main.c
機能		: クライアントのメインルーティン
*****************************************************************/

#include "client_func.h"

/***********************************
                変数                
***********************************/

int waitFlag  = 1;                  // 待ち画面フラグ

/* static */
static SDL_mutex*   gLock;          // 排他制御用mutex

static int          gameFlag = 1;   // ゲームループフラグ

/***********************************
                関数                
***********************************/

static int wii_func(void *args);
static int SRM(void *args);
static int WinEve(void *args);

int main(int argc, char *argv[])
{
    int	 num, clientID;
    char name[CLIENTNUM][MAX_NAME_SIZE];
    char localHostName[] = "localhost";
    char *serverName;
    
#ifdef DEBUG_PRINT
    fprintf(stderr, "connecting...\n");
#endif

    /** 初期化 **/
    /* SDL */
    if (SDL_Init(SDL_INIT_TIMER) < 0) {
        fprintf(stderr, "main error : SDL_Init failed.\n");
        exit(-1);
    }

    /* ウインドウ */
	if(InitWindows(num) == -1){
		fprintf(stderr, "main error : InitWindows failed.\n");
		exit(-1);
	}

    DrawWait();

    /** 引数チェック **/
    switch(argc){
        case 2:
            /* Wiiリモコンとの接続 */
    	    if(wiimote_connect(&wiimote, argv[1]) < 0){
                fprintf(stderr, "main error : wiimote_connect failed.\n");
                exit(-1);
            }

            serverName = localHostName;

            break;
        case 3:
            /* Wiiリモコンとの接続 */
            if(wiimote_connect(&wiimote, argv[1]) < 0){
                fprintf(stderr, "main error : wiimote_connect failed.\n");
                exit(-1);
            }

    	    serverName = argv[2];

            break;
        default:
            fprintf(stderr, "%s Wiiリモコン識別情報 serverName\n", argv[0]);
		    exit(-1);
    }

    /** サーバーとの接続 **/
    if(SetUpClient(serverName, &clientID, &num, name) == -1){
		fprintf(stderr, "main error : SetUpClient failed.\n");
		exit(-1);
	}

    /* システムの初期化 */
	if(InitSystem(clientID) == -1){
		fprintf(stderr, "main error : InitSystem failed.\n");
		exit(-1);
	}

    /* mutex */
    gLock = SDL_CreateMutex();
    if (gLock == NULL) {
        exit(-1);
    }

    /* 効果音読み込み */
    GetMusic();

    /* 全キャラ情報読込 */
    ReadAllChara("data/chara_data_client");
    /* 全ステージ情報読み込み */
    ReadAllStage("data/stage_data_client");
    /* 全アイテム読み込み */
    ReadAllItem("data/item_data_client");

    /* wiiリモコン入力処理スレッド作成 */
    SDL_Thread* wii_thread = SDL_CreateThread(wii_func, "wii_thread", NULL);
    if (wii_thread == NULL) {
        fprintf(stderr, "main error : wii_thread failed.\n");
        exit(-1);
    }
    wiimote.led.one = 1; // WiiリモコンのLEDの一番左を点灯させる（接続を知らせるために）
    SDL_DetachThread(wii_thread);

    /* データ受信処理スレッド作成 */
    SDL_Thread* srm_thread = SDL_CreateThread(SRM, "srm_thread", NULL); // ウィンドウ描画のスレッド
    if (srm_thread == NULL) {
        fprintf(stderr, "main error : srm_thread failed.\n");
        exit(-1);
    }
    SDL_DetachThread(srm_thread);

    /* タイトル表示 */
    MakeTitleIMG();

    int titleFlag = 1;
    waitFlag = 1;

    while(titleFlag && gameFlag){
        SDL_PumpEvents();
        titleFlag = DrawTitle();
        SDL_Delay(125);
    }
    DestroyTitleTex();

    while(waitFlag && gameFlag){
        SDL_PumpEvents();
        DrawSSWait();
    }

    /* ステージ選択 */
    if(clientID == 0){
        MakeSSIMG();

        int stageFlag = 1;    // ステージ選択フラグ

        while(stageFlag && gameFlag){
            SDL_PumpEvents();
            stageFlag = StageSelect();
            SDL_Delay(125);
        }
        DestroySSTex();
    }else if(clientID == 1){
        waitFlag = 1;

        while(waitFlag && gameFlag){
            SDL_PumpEvents();
            DrawSSWait();
        }
    }

    /* キャラ選択 */
    MakeCSIMG();

    int charaFlag = 1;  // キャラ選択フラグ
    waitFlag = 1;

    while(charaFlag && gameFlag){
        SDL_PumpEvents();
        charaFlag = CharaSelect();
        SDL_Delay(125);
    }

    while(waitFlag && gameFlag){
        SDL_PumpEvents();
        DrawWait();
    }
    DestroyCSTex();

    /* ゲーム時入力処理スレッド作成 */
    SDL_Thread* WinEve_thread = SDL_CreateThread(WinEve, "WinEve_thread", NULL);
    if (WinEve_thread == NULL) {
        fprintf(stderr, "main error : WinEve_thread failed\n");
        exit(-1);
    }
    SDL_DetachThread(WinEve_thread);

    /* ゲームループ */
    while(gameFlag){
        SDL_PumpEvents();
        WinDraw(clientID);
    }

    /* 勝敗判定 */
    DrawResult();

    /** 終了処理 **/
	DestroyAllTexture();
	CloseSoc();
    free(gChara);
    free(gItem);
    SDL_DestroyMutex(gLock);
    CloseMusic();

    return 0;
}

/* スレッド処理1(wii入力受付) */
static int wii_func(void *args)
{
    while(wiimote_is_open(&wiimote)){
        if(wiimote_update(&wiimote)){
            /* ホームボタンが押された時 */
            if(wiimote.keys.home){
                input_home  = 1;
            }else{
                input_home  = 0;
            }
            /* 横持ち上入力された時 */
            if(wiimote.keys.right){
                input_up    = 1;
            }else{
                input_up    = 0;
            }
            /* 横持ち右入力された時 */
            if(wiimote.keys.down){
                input_right = 1;
            }else{
                input_right = 0;
            }
            /* 横持ち左入力された時 */
            if(wiimote.keys.up){
                input_left  = 1;
            }else{
                input_left  = 0;
            }
            /* 横持ち下入力された時 */
            if(wiimote.keys.left){
                input_down  = 1;
            }else{
                input_down  = 0;
            }
            /* 1ボタンが入力された時 */
            if(wiimote.keys.one){
                input_one   = 1;
            }else{
                input_one   = 0;
            }
            /* 2ボタンが入力された時 */
            if(wiimote.keys.two){
                input_two   = 1;
            }else{
                input_two   = 0;
            }
        }else{
            fprintf(stderr, "main error : wii_func failed.\n");
            exit(-1);
        }
    }
    return 0;
}

/* スレッド処理2(データ受信) */
static int SRM(void *args)
{
    while(gameFlag){
        gameFlag = SendRecvManager();
    }
    return 0;
}

/* スレッド処理3(入力をゲーム操作に変換) */
static int WinEve(void *args)
{
    while(gameFlag){
        WindowEvent();
        SDL_Delay(25);
    }
    return 0;
}