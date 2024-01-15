/*****************************************************************
ファイル名	: server_main.c
機能		: サーバーのメインルーチン
*****************************************************************/

#include "server_func.h"

/***********************************
                変数                
***********************************/

int 	move_flag[CHARANUM]	= { 0 };	// キャラ毎の移動フラグ
int		ct_flag[CHARANUM]	= { 0 };	// キャラ毎のクールタイムフラグ
int		Item_flag 			= 1; 		// アイテム用フラグ
int 	Game_flag			= 0;		// ゲーム開始用フラグ
float 	cooltime[CHARANUM]	= { 0 };	// キャラ毎のクールタイム

/* static */
static int 	i;							// for文用
static int  itemtime;					// アイテム発生時間
static int 	pasttime = 0;				// 経過時間

/***********************************
                関数                
***********************************/

static Uint32 MCTimer(Uint32 interval, void* param);
static Uint32 PITimer(Uint32 interval, void* param);

int main(int argc, char *argv[])
{
	int	num 	= CLIENTNUM;	// クライアント2人限定
	int	endFlag = 1;			// プログラム終了フラグ

	/** 引き数チェック **/
	if(argc != 1){
		fprintf(stderr, "only ./server\n");
		exit(-1);
	}

	/** クライアントとの接続 **/
	if(SetUpServer(num) == -1){
		fprintf(stderr, "main error : SetUpServer\n");
		exit(-1);
	}

	/** 初期化 **/
	/* システム */
	if(InitSystem() == -1){
		fprintf(stderr, "main error : InitSystem\n");
		exit(-1);
	}

	/* 全キャラ情報読み込み */
	ReadAllChara("data/chara_data_server");
    
	/** タイマー作成 **/
	/* 移動時間とクールタイム */
	SDL_TimerID mctimer = SDL_AddTimer(MOVE_TIME * 1000, MCTimer, NULL);
    if(mctimer == 0){
        fprintf(stderr, "main error : mctimer\n");
        exit(-1);
    }

	/* 経過時間とアイテムタイム */
	SDL_TimerID pitimer = SDL_AddTimer(1000, PITimer, NULL);
    if(pitimer == 0){
        fprintf(stderr, "main error : pitimer\n");
        exit(-1);
    }

	/** メインイベントループ **/
	while(endFlag){
		endFlag = SendRecvManager();
	};

    /* 終了処理 */
    SDL_RemoveTimer(mctimer);
	SDL_RemoveTimer(pitimer);
    free(gChara);

	return 0;
}

/* タイマー処理1(移動時間の更新) */
Uint32 MCTimer(Uint32 interval, void* param)
{
    for(i = 0; i < CHARANUM; i++){
		if(gChara[i].aod == CS_Alive){
			/* キャラ座標更新 */
        	if(gChara[i].dir != -1){
        	    MoveCharaPoint(i);
        	}

			/* cooltime更新 */
			if(ct_flag[i]){
        	    cooltime[i] += MOVE_TIME;
				SendCoolCommand(i);
				/* クールタイムが3秒経った場合 */
				if(cooltime[i] > 2.95){
        	    	cooltime[i] = 0;
					ct_flag[i] 	= 0;
        		}
        	}
    	}
	}
    return interval;
}

/* タイマー処理2(経過時間とアイテムタイマー) */
Uint32 PITimer(Uint32 interval, void* param)
{
	/* クライアントがゲームループに入っている場合 */
	if(Game_flag){
		pasttime++;
		SendTimeCommand(pasttime);
		/* イベント発生送信処理 */
		switch(pasttime){
			case EVENT_TIME:
				SendEventCutinCommand();
				break;
			case EVENT_TIME + 3:
				MakeEvent();
				break;
			case FINISH_TIME:
				TimeUp();
				break;
			default:
				break;
		}
	}

	/* アイテムが出現していない場合 */
    if(!Item_flag){
		itemtime++;
		/* アイテム5秒以上出現していない場合 */
		if(itemtime >= 5){
			ItemRand();
			itemtime = 0;
		}
	}
    return interval;
}