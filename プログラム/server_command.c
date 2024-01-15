/*****************************************************************
ファイル名	: server_command.c
機能		: サーバーのコマンド処理
*****************************************************************/

#include "server_func.h"

#include <netinet/in.h>

/***********************************
                変数                
***********************************/

int stageID;                // ステージID

/* static */
static int Ready0_flag = 0; // クライアント 0 のREADY_COMMAND受信フラグ
static int Ready1_flag = 0; // クライアント 0 のREADY_COMMAND受信フラグ

/***********************************
                関数                
***********************************/

static void SetIntData2DataBlock(void *data, int intData, int *dataSize);
static void SetCharData2DataBlock(void *data, char charData, int *dataSize);
static void SetFloatData2DataBlock(void *data, float floatData, int *dataSize);

/*****************************************************************
関数名  : SendLifeCommand
機能    : キャラの体力を共有するために，クライアントにデータを送る
引数    : int cnum  : キャラ番号
          int life  : 体力
出力    : なし
*****************************************************************/
void SendLifeCommand(int cnum, int life)
{
    unsigned char   data[MAX_DATA];
    int             dataSize;

    /* 引き数チェック */
    assert(0 <= cnum && cnum < CHARANUM);
    assert(0 <= life && life <= gChara[cnum].basestts.life);

    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data, LIFE_COMMAND, &dataSize);
    /* キャラ番号 */
    SetIntData2DataBlock(data, cnum, &dataSize);
    /* 体力 */
    SetIntData2DataBlock(data, life, &dataSize);

    /* 全クライアントに送る */
    SendData(ALL_CLIENTS, data, dataSize);
}

/*****************************************************************
関数名  : SendDeathCommand
機能    : 死亡キャラを共有するために，クライアントにデータを送る
引数    : int cnum  : キャラ番号
出力    : なし
*****************************************************************/
void SendDeathCommand(int cnum)
{
    unsigned char   data[MAX_DATA];
    int             dataSize;

    /* 引き数チェック */
    assert(0 <= cnum && cnum <CHARANUM);

    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data, DEATH_COMMAND, &dataSize);
    /* キャラ番号 */
    SetIntData2DataBlock(data, cnum, &dataSize);

    /* 全クライアントに送る */
    SendData(ALL_CLIENTS, data, dataSize);
}

/*****************************************************************
関数名  : SendPointCommand
機能    : キャラ座標を共有するために，クライアントにデータを送る
引数    : int cnum  : キャラ番号
出力    : なし
*****************************************************************/
void SendPointCommand(int cnum)
{
    unsigned char   data[MAX_DATA];
    int             dataSize;

    /* 引き数チェック */
    assert(0 <= cnum && cnum <CHARANUM);

    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data, POINT_COMMAND, &dataSize);
    /* キャラ番号 */
    SetIntData2DataBlock(data, cnum, &dataSize);
    /* 速度 */
    SetFloatData2DataBlock(data, gChara[cnum].stts.vel, &dataSize);
    /* キャラx座標のセット */
    SetFloatData2DataBlock(data, gChara[cnum].point.x, &dataSize);
    /* キャラy座標のセット */
    SetFloatData2DataBlock(data, gChara[cnum].point.y, &dataSize);

    /* クライアント番号 0 に送る */
    SendData(0, data, dataSize);

    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data, POINT_COMMAND, &dataSize);
    /* キャラ番号 */
    SetIntData2DataBlock(data, cnum, &dataSize);
    /* 速度 */
    SetFloatData2DataBlock(data, gChara[cnum].stts.vel, &dataSize);
    /* キャラx座標のセット */
    SetFloatData2DataBlock(data, WINDOWWIDE - gChara[cnum].point.x, &dataSize);
    /* キャラy座標のセット */
    SetFloatData2DataBlock(data, WINDOWHEIGHT - gChara[cnum].point.y, &dataSize);

    /* クライアント番号 1 に送る */
    SendData(1, data, dataSize);
}

/*****************************************************************
関数名  : SendTurnCommand
機能    : キャラのスキルターンを共有するために，
          クライアントにデータを送る
引数    : int cnum  : キャラ番号
出力    : なし
*****************************************************************/
void SendTurnCommand(int cnum)
{
    unsigned char   data[MAX_DATA];
    int             dataSize;

    /* 引き数チェック */
    assert(0 <= cnum && cnum <CHARANUM);

    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data, TURN_COMMAND, &dataSize);
    /* キャラ番号 */
    SetIntData2DataBlock(data, cnum, &dataSize);
    /* スキルターン */
    SetIntData2DataBlock(data, gChara[cnum].skill.turn, &dataSize);

    /* 全クライアントに送る */
    SendData(ALL_CLIENTS, data, dataSize);
}

/*****************************************************************
関数名  : SendItemCommand
機能    : アイテム情報を共有するために，クライアントにデータを送る
引数    : int Item_type : アイテムの種類
        : int Item_x    : アイテムのx座標
          int Item_y    : アイテムのy座標
出力    : なし
*****************************************************************/
void SendItemCommand(int Item_type, int Item_x, int Item_y)
{
    unsigned char   data[MAX_DATA];
    int             dataSize;

    /* 引き数チェック */
    assert(0 <= Item_type && Item_type < ITEMNUM);
    assert(500 <= Item_x && Item_x <= 1200);
    assert(200 <= Item_y && Item_y <= 700);

    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data, ITEM_COMMAND, &dataSize);
    /* アイテム番号 */
    SetIntData2DataBlock(data, Item_type, &dataSize);
    /* アイテムx座標のセット */
    SetIntData2DataBlock(data,Item_x, &dataSize);
    /* アイテムy座標のセット */
    SetIntData2DataBlock(data, Item_y, &dataSize);
    
    /* クライアント番号 0 に送る */
    SendData(0, data, dataSize);

    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data, ITEM_COMMAND, &dataSize);
    /* アイテム番号 */
    SetIntData2DataBlock(data, Item_type, &dataSize);
    /* アイテムx座標のセット */
    SetIntData2DataBlock(data, WINDOWWIDE - Item_x,&dataSize);
    /* アイテムy座標のセット */
    SetIntData2DataBlock(data, WINDOWHEIGHT - Item_y ,&dataSize);

    /* クライアント番号 1 に送る */
    SendData(1, data, dataSize);
}

/*****************************************************************
関数名  : SendGetCommand
機能    : アイテム獲得を共有するために，クライアントにデータを送る
引数    : int cnum      : キャラ番号
          int Item_type : アイテムの種類
*****************************************************************/
void SendGetCommand(int cnum, int Item_type)
{
    unsigned char   data[MAX_DATA];
    int             dataSize;

    /* 引き数チェック */
    assert(0 <= cnum && cnum <CHARANUM);
    assert(0 <= Item_type && Item_type < ITEMNUM);

    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data, GET_COMMAND, &dataSize);
    /* キャラ番号 */
    SetIntData2DataBlock(data, cnum, &dataSize);
    /* アイテムタイプのセット */
    SetIntData2DataBlock(data, Item_type, &dataSize);

    /* 全クライアントに送る */
    SendData(ALL_CLIENTS, data, dataSize);

    Item_flag = 0;
}

/*****************************************************************
関数名  : SendPoisonCommand
機能    : 毒状態のキャラを共有するために，
          クライアントにデータを送る
引数    : int cnum  : キャラ番号
          int turn  : 毒状態の残りターン数
出力    : なし
*****************************************************************/
void SendPoisonCommand(int cnum, int turn)
{
    unsigned char   data[MAX_DATA];
    int             dataSize;

    /* 引き数チェック */
    assert(0 <= cnum && cnum <CHARANUM);
    assert(0 <= turn);

    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data, POISON_COMMAND, &dataSize);
    /* キャラ番号 */
    SetIntData2DataBlock(data, cnum, &dataSize);
    /* 毒状態の残りターン数 */
    SetIntData2DataBlock(data, turn, &dataSize);

    /* 全クライアントに送る */
    SendData(ALL_CLIENTS, data, dataSize);
}

/*****************************************************************
関数名  : SendCoolCommand
機能    : キャラのクールタイムを共有するために，
          クライアントにデータを送る
引数    : int cnum  : キャラ番号
出力    : なし
*****************************************************************/
void SendCoolCommand(int cnum)
{
    unsigned char   data[MAX_DATA];
    int             dataSize;

    /* 引き数チェック */
    assert(0 <= cnum && cnum <CHARANUM);

    if(cnum >= 0 && cnum < 4){
        dataSize = 0;
        /* コマンドのセット */
        SetCharData2DataBlock(data, COOL_COMMAND, &dataSize);
        /* キャラ番号 */
        SetIntData2DataBlock(data, cnum, &dataSize);
        /* クールタイムのセット */
        SetFloatData2DataBlock(data, cooltime[cnum], &dataSize);

        /* クライアント番号 0 に送る */
        SendData(0, data, dataSize);
    }else if(cnum >= 4 && cnum < CHARANUM){
        dataSize = 0;
        /* コマンドのセット */
        SetCharData2DataBlock(data, COOL_COMMAND, &dataSize);
        /* キャラ番号 */
        SetIntData2DataBlock(data, cnum, &dataSize);
        /* クールタイムのセット */
        SetFloatData2DataBlock(data, cooltime[cnum], &dataSize);

        /* クライアント番号 1 に送る */
        SendData(1, data, dataSize);
    }
}

/*****************************************************************
関数名  : SendDownCommand
機能    : 攻撃アップ効果が切れることを知らせるために，
          クライアントにデータを送る
引数    : int cnum  : キャラID
出力    : なし
*****************************************************************/
void SendDownCommand(int cnum)
{
    unsigned char   data[MAX_DATA];
    int             dataSize;

    /* 引き数チェック */
    assert(0 <= cnum && cnum <CHARANUM);

    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data, DOWN_COMMAND, &dataSize);
    /* キャラ番号 */
    SetIntData2DataBlock(data, cnum, &dataSize);

    /* 全クライアントに送る */
    SendData(ALL_CLIENTS, data, dataSize);
}

/*****************************************************************
関数名  : SendBoundCommand
機能    : キャラ同士の衝突を伝える
引数    : なし
出力    : なし
*****************************************************************/
void SendBoundCommand(void)
{
    unsigned char   data[MAX_DATA];
    int             dataSize;

    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data, BOUND_COMMAND, &dataSize);

    /* 全クライアントに送る */
    SendData(ALL_CLIENTS, data, dataSize);
}

/*****************************************************************
関数名  : SendEventCutinCommand
機能    : イベント発生(-参戦の描画)
引数    : なし
出力    : なし
*****************************************************************/
void SendEventCutinCommand(void)
{
    unsigned char   data[MAX_DATA];
    int             dataSize;

    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data, EVENT_CUTIN_COMMAND, &dataSize);
    /* クライアントに送る */
    SendData(ALL_CLIENTS, data, dataSize);
}

/*****************************************************************
関数名  : SendEventCommand
機能    : 冥界イベント発生
引数    : ステータスダウン用フラグ
出力    : なし
*****************************************************************/
void SendEventCommand(int *flag)
{
    unsigned char   data[MAX_DATA];
    int             dataSize, intData;

    /* 引き数チェック */
    assert(*flag == NULL || *flag == 0 || *flag == 1);

    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data, EVENT_COMMAND, &dataSize);
    switch(stageID){
        case 0:
        case 1:
            /* キャラIDセット */
            for(int i = 0; i < CHARANUM; i++){
                SetIntData2DataBlock(data, *(flag + i), &dataSize);
            }
            break;
        case 2:
            break;
        default:
            fprintf(stderr, "command error : SendEventCommand\n");
    }
    /* 全クライアントに送る */
    SendData(ALL_CLIENTS, data, dataSize);
}

/*****************************************************************
関数名  : SendTimeCommand
機能    : 残り時間を知らせる
引数    : なし
出力    : なし
*****************************************************************/
void SendTimeCommand(int pasttime)
{
    unsigned char   data[MAX_DATA];
    int             dataSize;

    /* 引き数チェック */
    assert(0 <= pasttime && pasttime <= FINISH_TIME);

    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data, TIME_COMMAND, &dataSize);
    /* 経過時間のセット */
    SetIntData2DataBlock(data, pasttime, &dataSize);

    /* 全クライアントに送る */
    SendData(ALL_CLIENTS, data, dataSize);
}

/*****************************************************************
関数名  : SendResultCommand
機能    : 結果を知らせる
引数    : int pos       : クライアントID
          int result    : 勝敗の結果
出力    : なし
*****************************************************************/
void SendResultCommand(int pos, int result)
{
    unsigned char   data[MAX_DATA];
    int             dataSize;

    /* 引き数チェック */
    assert(pos == 0 || pos == 1);
    assert(0 <= result && result <= 3);

    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data, RESULT_COMMAND, &dataSize);
    /* 経過時間のセット */
    SetIntData2DataBlock(data, result, &dataSize);

    /* 全クライアントに送る */
    SendData(pos, data, dataSize);
}

/*****************************************************************
関数名	: ExecuteCommand
機能	: クライアントから送られてきたコマンドを元に
		  引き数を受信し，実行する
引数	: char  command : コマンド
		  int   pos		: コマンドを送ったクライアント番号
出力	: プログラム終了コマンドが送られてきた時には0を返す．
		  それ以外は1を返す
*****************************************************************/
int ExecuteCommand(char command, int pos)
{
    unsigned char	data[MAX_DATA];
    int			    dataSize, intData;
    int			    endFlag = 1;
    int             i;
    int             CID[CHARANUM];  // キャラID格納配列
    int             angle;          // 発射角度

    /* 引き数チェック */
    assert(0 <= pos && pos < CLIENTNUM);

    switch(command){
        case END_COMMAND:
            endFlag = 0;

			dataSize = 0;
			/* コマンドのセット */
			SetCharData2DataBlock(data, command, &dataSize);

			/* 全ユーザーに送る */
			SendData(ALL_CLIENTS, data, dataSize);
            break;
        case TITLE_COMMAND:
            /* クライアント 0 からのデータの場合 */
            if(pos == 0 && !Ready0_flag){
                Ready0_flag = 1;
            /* クライアント 1 からのデータの場合 */
            }else if(pos == 1 && !Ready1_flag){
                Ready1_flag = 1;
            }

            /* 両クライアントからデータを受け取った場合 */
            if(Ready0_flag && Ready1_flag){
                Ready0_flag = 0;
                Ready1_flag = 0;

                dataSize = 0;
                /* コマンドのセット */
                SetCharData2DataBlock(data, command, &dataSize);

                /* 全ユーザーに送る */
                SendData(ALL_CLIENTS, data, dataSize);
            }
            break;
        case STAGE_COMMAND:
            /* 表示するステージIDを受信する */
            RecvIntData(pos, &stageID);

            dataSize = 0;
            /* コマンドのセット */
            SetCharData2DataBlock(data, command, &dataSize);
            /* ステージIDをセット */
            SetIntData2DataBlock(data, stageID, &dataSize);

            /* 全ユーザーに送る */
            SendData(ALL_CLIENTS, data, dataSize);
            break;
        case READY_COMMAND:
            /* クライアント 0 からのデータの場合 */
            if(pos == 0 && !Ready0_flag){
                /* キャラIDを受信する */
                for(i = 0; i < 4; i++){
                    RecvIntData(pos, &CID[i]);
                }
                /* クライアント 0 からのREADY_COMMAND受信フラグ*/ 
                Ready0_flag = 1;
            /* クライアント 1 からのデータの場合 */
            }else if(pos == 1 && !Ready1_flag){
                /* キャラIDを受信する */
                for(i = 4; i < CHARANUM; i++){
                    RecvIntData(pos, &CID[i]);
                }
                /* クライアント 1 からのREADY_COMMAND受信フラグ*/ 
                Ready1_flag = 1;
            }

            /* 両クライアントからデータを受け取った場合 */
            if(Ready0_flag && Ready1_flag){
                dataSize = 0;
                /* コマンドのセット */
                SetCharData2DataBlock(data, command, &dataSize);
                /* キャラIDのセット & データ読み込み */
                for(i = 0; i < CHARANUM; i++){
                    SetIntData2DataBlock(data, CID[i], &dataSize);
                    ReadChara(&CID[i],i);
                }
                free(aChara);

                /* 全ユーザーに送る */
                SendData(ALL_CLIENTS, data, dataSize);

                /* フラグ */
                Item_flag = 0;
                Game_flag = 1;
            }
            break;
        case ACTION_COMMAND:
            /* キャラ番号を取得 */
            RecvIntData(pos, &intData);
            /* 発射角度を取得 */
            RecvIntData(pos, &angle);

            /* 移動中でないかつクールタイム中でない場合 */
            if(!move_flag[intData] && !ct_flag[intData] && gChara[intData].aod == CS_Alive){
                /* クライアント 1 からのデータの場合 */
                if(pos == 1){
                    if(angle + 180 >= 360){
                        angle -= 180;
                    }else{
                        angle += 180;
                    }
                }

                /* 移動方向 */
                gChara[intData].dir = angle;
                
                /* 攻撃アップ状態の場合 */
                if(AtkUp_turn[intData] != 0){
                    /* 攻撃力をもとに戻すフラグ */
                    AtkRestore_flag[intData] = 1;
                }
                /* 毒状態の場合 */
                if(Poison_turn[intData] != 0){
                    /* 毒ダメージ処理 */
                    PoisonCondition(intData);
                }

                dataSize = 0;
                /* コマンドのセット */
                SetCharData2DataBlock(data, command, &dataSize);
                
                /* 全クライアントへ送る */
                SendData(ALL_CLIENTS, data, dataSize);
            }
            break;
        case SKILL_COMMAND:
            /* キャラ番号を取得 */
            RecvIntData(pos, &intData);

            switch(gChara[intData].skill.type){
                case 0:
                    if(gChara[intData].skill.turn == 3){
                        gChara[intData].skill.turn = 0;
                        Skill_CA(intData);
                    }
                    break;
                case 1:
                    if(gChara[intData].skill.turn == 5){
                        gChara[intData].skill.turn = 0;
                        Skill_CD(pos, intData);
                    }
                    break;
                case 2:
                    if(gChara[intData].skill.turn == 4){
                        gChara[intData].skill.turn = 0;
                        Skill_RO(pos);
                    }
                    break;
                default:
                    fprintf(stderr, "command error : Unknown skilltype. (ExecuteCommand)\n");
            }

            dataSize = 0;
            /* コマンドセット */
            SetCharData2DataBlock(data, command, &dataSize);
            /* キャラ番号 */
            SetIntData2DataBlock(data, intData, &dataSize);
            
            /* 全ユーザーに送る */
            SendData(ALL_CLIENTS, data, dataSize);
            break;
        default:
            /* 未知のコマンドが送られてきた */
            //fprintf(stderr, "0x%02x is not command!\n", command);
            fprintf(stderr, "command error : Unknown command. (ExecuteCommand)\n");
    }
    return endFlag;
}

 /***************
 *   static    *
***************/
/*****************************************************************
関数名	: SetIntData2DataBlock
機能	: int 型のデータを送信用データの最後にセットする
引数	: void  *data       : 送信用データ
		  int   intData     : セットするデータ
		  int   *dataSize   : 送信用データの現在のサイズ
出力	: なし
*****************************************************************/
static void SetIntData2DataBlock(void *data, int intData, int *dataSize)
{
    int tmp;

    /* 引き数チェック */
    assert(data != NULL);
    assert(0 <= (*dataSize));

    tmp = htonl(intData);

    /* int 型のデータを送信用データの最後にコピーする */
    memcpy(data + (*dataSize), &tmp, sizeof(int));
    /* データサイズを増やす */
    (*dataSize) += sizeof(int);
}

/*****************************************************************
関数名	: SetCharData2DataBlock
機能	: char 型のデータを送信用データの最後にセットする
引数	: void  *data		: 送信用データ
		  int   intData		: セットするデータ
		  int   *dataSize	: 送信用データの現在のサイズ
出力	: なし
*****************************************************************/
static void SetCharData2DataBlock(void *data, char charData, int *dataSize)
{
    /* 引き数チェック */
    assert(data != NULL);
    assert(0 <= (*dataSize));

    /* int 型のデータを送信用データの最後にコピーする */
    *(char *)(data + (*dataSize)) = charData;
    /* データサイズを増やす */
    (*dataSize) += sizeof(char);
}

/*****************************************************************
関数名	: SetFloatData2DataBlock
機能	: float 型のデータを送信用データの最後にセットする
引数	: void  *data       : 送信用データ
		  float floatData   : セットするデータ
		  int   *dataSize   : 送信用データの現在のサイズ
出力	: なし
*****************************************************************/
static void SetFloatData2DataBlock(void *data, float floatData, int *dataSize)
{
    char charData[16]; 

    /* 引き数チェック */
    assert(data != NULL);
    assert(0 <= (*dataSize));

    snprintf(charData, sizeof(charData), "%f", floatData);

    /* char 型のデータを送信用データの最後にコピーする */
    memcpy(data + (*dataSize), &charData, sizeof(charData));
    /* データサイズを増やす */
    (*dataSize) += sizeof(charData);
}