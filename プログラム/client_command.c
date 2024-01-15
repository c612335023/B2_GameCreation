/*****************************************************************
ファイル名	: client_command.c
機能		: クライアントのコマンド処理
*****************************************************************/

#include "client_func.h"

#include <netinet/in.h>

/***********************************
                変数                
***********************************/

int itemFlag            = 0;            // 1の時アイテム描画フラグ
int wallFlag            = 0;            // 地上階イベントフラグ
int upFlag[CHARANUM]    = { 0 };        // イベントフラグ
int result              = No_results;   // 勝敗

/* static */
static int stageID;                     // ステージID

/***********************************
                関数                
***********************************/

static void SetCharData2DataBlock(void *data, char charData, int *dataSize);
static void SetIntData2DataBlock(void *data, int intData, int *dataSize);
static void SetFloatData2DataBlock(void *data, float floatData, int *dataSize);
static void RecvStageData();
static void RecvCharaData();
static void RecvLifeData();
static void RecvDeathData();
static void RecvPointData();
static void RecvTurnData();
static void RecvCoolData();
static void RecvSkillData();
static void RecvItemData();
static void RecvGetData();
static void RecvPoisonData();
static void RecvDownData();
static void RecvEventData();
static void RecvTimeData();
static void RecvResultData();

/*****************************************************************
関数名  : SendEndCommand
機能    : 終了を知らせるために，サーバーにデータを送る
引数    : なし
出力    : なし
*****************************************************************/
void SendEndCommand()
{
    unsigned char   data[MAX_DATA];
    int             dataSize;

    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data, END_COMMAND, &dataSize);

    /* データの送信 */
    SendData(data, dataSize);
}

/*****************************************************************
関数名	: SendTitleCommand
機能	: クライアントがタイトル画面から進行する際，
		  サーバーにデータを送る
引数	: なし
出力	: なし
*****************************************************************/
void SendTitleCommand()
{
    unsigned char	data[MAX_DATA];
    int			    dataSize;

    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data, TITLE_COMMAND, &dataSize);

    /* データの送信 */
    SendData(data, dataSize);
}

/*****************************************************************
関数名	: SendStageCommand
機能	: クライアントに決定されたステージを表示させるために，
		  サーバーにデータを送る
引数	: int stageID   : 表示させるステージID
出力	: なし
*****************************************************************/
void SendStageCommand(int stageID)
{
    unsigned char	data[MAX_DATA];
    int			    dataSize;

    /* 引き数チェック */
    assert(0 <= stageID && stageID < ALL_STAGES);

    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data, STAGE_COMMAND, &dataSize);
    /* ステージIDのセット */
    SetIntData2DataBlock(data, stageID, &dataSize);

    /* データの送信 */
    SendData(data, dataSize);
}

/*****************************************************************
関数名  : SendReadyCommand
機能    : 決定された選択キャラのIDと選択キャラの数を表示させるために，
          サーバーにデータを送る
引数    : int *CID      : 選択キャラのID
          int num_sc    : 選択キャラ数
出力    : なし
*****************************************************************/
void SendReadyCommand(int *CID, int num_sc)
{
    unsigned char   data[MAX_DATA];
    int             dataSize;
    int             i;

    /* 引数チェック */
    assert(num_sc == 4);
    for(i = 0 ; i < num_sc; i++){
        assert(0 <= *(CID + i) && *(CID + i) < ALL_CHARAS);
    }

    dataSize = 0;
    /* コマンドのセット */ 
    SetCharData2DataBlock(data, READY_COMMAND, &dataSize);
    /* キャラIDのセット */
    for(i = 0 ; i < num_sc ; i++){
        SetIntData2DataBlock(data, *(CID + i), &dataSize);
    }

    /* データの送信 */
    SendData(data, dataSize);
}

/*****************************************************************
関数名  : SendActionCommand
機能    : キャラの動作を描画するために，サーバーにデータを送る
引数    : int cnum  : キャラ番号
出力    : なし
*****************************************************************/
void SendActionCommand(int cnum)
{
    unsigned char   data[MAX_DATA];
    int             dataSize;

    /* 引数チェック */
    assert(0 <= cnum && cnum < CHARANUM);

    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data, ACTION_COMMAND, &dataSize);
    /* キャラ番号のセット */
    SetIntData2DataBlock(data, cnum, &dataSize);
    /* 発射角度のセット */
    SetIntData2DataBlock(data, angle, &dataSize);

    /* データの送信 */
    SendData(data, dataSize);
}

/*****************************************************************
関数名  : SendSkillCommand
機能    : キャラのスキルを描画させるために，サーバーにデータを送る
引数    : int cnum  : キャラ番号
出力    : なし
*****************************************************************/
void SendSkillCommand(int cnum)
{
    unsigned char   data[MAX_DATA];
    int             dataSize;

    /* 引数チェック */
    assert(0 <= cnum && cnum < CHARANUM);

    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data, SKILL_COMMAND, &dataSize);
    /* キャラ番号のセット */
    SetIntData2DataBlock(data, cnum, &dataSize);
    
    /* データの送信 */
    SendData(data, dataSize);
}

/*****************************************************************
関数名	: ExecuteCommand
機能	: サーバーから送られてきたコマンドを元に，
		  引き数を受信し，実行する
引数	: char command  : コマンド
出力	: プログラム終了コマンドがおくられてきた時には0を返す．
		  それ以外は1を返す
*****************************************************************/
int ExecuteCommand(char command)
{
    int endFlag = 1;
/*
#ifdef DEBUG_PRINT
    printf("#####\n");
    printf("ExecuteCommand()\n");
    printf("command = %c\n",command);
#endif
*/
    switch(command){
        case END_COMMAND:
            endFlag = 0;
            break;
        case TITLE_COMMAND:
            waitFlag = 0;
            break;
        case STAGE_COMMAND:
            RecvStageData();
            break;
        case READY_COMMAND:
            RecvCharaData();
            break;
        case LIFE_COMMAND:
            RecvLifeData();
            break;
        case ACTION_COMMAND:
            SoundEffect(Shoot);
            break;
        case SKILL_COMMAND:
            RecvSkillData();
            break;
        case DEATH_COMMAND:
            RecvDeathData();
            break;
        case POINT_COMMAND:
            RecvPointData();
            break;
        case TURN_COMMAND:
            RecvTurnData();
            break;
        case COOL_COMMAND:
            RecvCoolData();
            break;
        case ITEM_COMMAND:
            RecvItemData();
            break;
        case GET_COMMAND:
            RecvGetData();
            break;
        case POISON_COMMAND:
            RecvPoisonData();
            break;
        case DOWN_COMMAND:
            RecvDownData();
            break;
        case BOUND_COMMAND:
            SoundEffect(Bound);
            break;
        case EVENT_CUTIN_COMMAND:
            cutinFlag = 1;
            break;
        case EVENT_COMMAND:
            RecvEventData();
            break;
        case TIME_COMMAND:
            RecvTimeData();
            break;
        case RESULT_COMMAND:
            RecvResultData();
            break;
        default:
            /* 未知のコマンドが送られてきた */
            fprintf(stderr, "command error : Unknown command. (ExecuteCommand)\n");
    }
    return endFlag;
}

 /***************
 *   static    *
***************/
/*****************************************************************
関数名	: SetCharData2DataBlock
機能	: char 型のデータを送信用データの最後にセットする
引数	: void  *data		: 送信用データ
		  int   intData		: セットするデータ
		  int   *dataSize   : 送信用データの現在のサイズ
出力	: なし
*****************************************************************/
static void SetCharData2DataBlock(void *data, char charData, int *dataSize)
{
    /* 引き数チェック */
    assert(data != NULL);
    assert(0 <= (*dataSize));

    /* char 型のデータを送信用データの最後にコピーする */
    *(char *)(data + (*dataSize)) = charData;
    /* データサイズを増やす */
    (*dataSize) += sizeof(char);
}

/*****************************************************************
関数名	: SetIntData2DataBlock
機能	: int 型のデータを送信用データの最後にセットする
引数	: void  *data		: 送信用データ
		  int   intData		: セットするデータ
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
    memcpy(data + (*dataSize),&tmp,sizeof(int));
    /* データサイズを増やす */
    (*dataSize) += sizeof(int);
}

/*****************************************************************
関数名  : SetFloatData2DataBlock
機能    : float 型のデータを送信用データの最後にセットする
引数    : void  *data       : 送信用データ
          float floatData   : セットするデータ
          int   *dataSize   : 送信用データの現在のサイズ
出力    : なし
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

/*****************************************************************
関数名	: RecvStageData
機能	: 決定されたステージを表示するためのデータを受信し，表示する
引数	: なし
出力	: なし
*****************************************************************/
static void RecvStageData()
{
    /* STAGE_COMMANDに対する引き数を受信する */
    RecvIntData(&stageID);

    /* 選択ステージのパスを代入　*/
    ReadStage(stageID);
    /* アイテムデータコピー */
    ReadItem(stageID);

    MakeStageIMG();
    MakeItemIMG();
    MakeEventIMG(stageID);

    waitFlag = 0;
}

/*****************************************************************
関数名	: RecvCharaData
機能	: 決定された選択キャラを表示するためのデータを受信し，表示する
引数	: なし
出力	: なし
*****************************************************************/
static void RecvCharaData()
{
    int CID[CHARANUM];

    /* READY_COMMANDに対する引き数を受信する */
    for(int i = 0; i < CHARANUM; i++){
        RecvIntData(&CID[i]);
        ReadChara(&CID[i], i);  // 使用するキャラ情報を読み取る
    }
    free(aChara);

    MakeCharaIMG();
    MakeEffectIMG();

    waitFlag = 0;
}

/*****************************************************************
関数名	: RecvLifeData
機能	: キャラの体力を表示するためのデータを受信し，表示する
引数	: なし
出力	: なし
*****************************************************************/
static void RecvLifeData()
{
    int cnum, life;

    /* LIFE_COMMANDに対する引き数を受信する */
    RecvIntData(&cnum);
    RecvIntData(&life);

    if(gChara[cnum].life > life){
        SoundEffect(Damage);
    }else if(gChara[cnum].life < life){
        SoundEffect(Heal);
    }

    beforeLife[cnum] = gChara[cnum].life;

    gChara[cnum].life = life;
}

/*****************************************************************
関数名	: RecvDeathData
機能	: キャラの体力を表示するためのデータを受信し，表示する
引数	: なし
出力	: なし
*****************************************************************/
static void RecvDeathData()
{
    int cnum;

    /* DEATH_COMMANDに対する引き数を受信する */
    RecvIntData(&cnum);

    gChara[cnum].aod = CS_Death;

    SoundEffect(Kill);
}

/*****************************************************************
関数名	: RecvPointData
機能	: キャラの位置を表示するためのデータを受信し，表示する
引数	: なし
出力	: なし
*****************************************************************/
static void RecvPointData()
{
    int     cnum;
    float   v, x, y;

    /* POINT_COMMANDに対する引き数を受信する */
    RecvIntData(&cnum);
    RecvFloatData(&v);
    RecvFloatData(&x);
    RecvFloatData(&y);
    
    gChara[cnum].velocity   = v;
    gChara[cnum].point.x    = x;
    gChara[cnum].point.y    = y;
}

/*****************************************************************
関数名	: RecvTurnData
機能	: キャラのスキルゲージを表示するためのデータを受信し，表示する
引数	: なし
出力	: なし
*****************************************************************/
static void RecvTurnData()
{
    int cnum, turn;

    /* TURN_COMMANDに対する引き数を受信する */
    RecvIntData(&cnum);
    RecvIntData(&turn);

    gChara[cnum].skill.turn = turn;
}

/*****************************************************************
関数名	: RecvCoolData
機能	: キャラのクールタイムを表示するためのデータを受信し，
          表示する
引数	: なし
出力	: なし
*****************************************************************/
static void RecvCoolData()
{
    int     cnum;
    float   ct;

    /* COOL_COMMANDに対する引き数を受信する */
    RecvIntData(&cnum);
    RecvFloatData(&ct);

    cooltime[cnum] = ct;
}

/*****************************************************************
関数名	: RecvSkillData
機能	: キャラのスキルを表示するためのデータを受信し，表示する
引数	: なし
出力	: なし
*****************************************************************/
static void RecvSkillData()
{
    int cnum; // キャラ番号

    /* SKILL_COMMANDに対する引き数を受信する */
    RecvIntData(&cnum);
    gChara[cnum].skill.turn = 0;
}

/*****************************************************************
関数名	: RecvItemData
機能	: アイテム表示するためのデータを受信し，表示する
引数	: なし
出力	: なし
*****************************************************************/
static void RecvItemData()
{
    int item_num, item_x, item_y;
    
    /* ITEM_COMMANDに対する引き数を受信する */
    RecvIntData(&item_num);
    RecvIntData(&item_x);
    RecvIntData(&item_y);

    Item.type       = item_num;
    Item.point.x    = item_x;
    Item.point.y    = item_y;

    itemFlag = 1;
}

/*****************************************************************
関数名	: RecvGetData
機能	: アイテムの効果発動とアイテムを消す
引数	: なし
出力	: なし
*****************************************************************/
static void RecvGetData()
{
    int cnum, item_num;
    
    /* GET_COMMANDに対する引き数を受信する */
    RecvIntData(&cnum);
    RecvIntData(&item_num);

    itemFlag = 0;

    if(item_num == 1){
        atkFlag[cnum] = 1;
    }
}

/*****************************************************************
関数名	: RecvPoisonData
機能	: 毒状態のキャラの番号を受信し，描画フラグを立てる
引数	: なし
出力	: なし
*****************************************************************/
static void RecvPoisonData()
{
    int cnum, turn;

    /* POISON_COMMANDに対する引き数を受信する */
    RecvIntData(&cnum);
    RecvIntData(&turn);

    SoundEffect(Poison);

    if(turn != 0){
        poisonFlag[cnum] = 1;
    }else{
        poisonFlag[cnum] = 0;
    }
}

/*****************************************************************
関数名	: RecvDownData
機能	: 攻撃アップが終わるキャラの番号を受信し，フラグを降ろす
引数	: なし
出力	: なし
*****************************************************************/
static void RecvDownData()
{
    int cnum;

    /* DOWN_COMMANDに対する引き数を受信する */
    RecvIntData(&cnum);

    atkFlag[cnum] = 0;
}

/*****************************************************************
関数名  : RecvEventData
機能	: 天界または冥界イベント発生
引数	: なし
出力	: なし
*****************************************************************/
static void RecvEventData(void)
{
    cutinFlag = 0;
    switch(stageID){
        case 0:
        case 1:
            for(int i = 0; i < CHARANUM; i++){
                RecvIntData(&upFlag[i]);
            }
            break;
        case 2:
            wallFlag = 1;
            break;
        default:
            fprintf(stderr, "command error : RecvEventData failed.\n");
    }
}

/*****************************************************************
関数名	: RecvTimeData
機能	: 経過時間を受け取る
引数	: なし
出力	: なし
*****************************************************************/
static void RecvTimeData(void)
{
    int time;

    /* TIME_COMMANDに対する引き数を受信する */
    RecvIntData(&time);

    pasttime = time;
}

/*****************************************************************
関数名	: RecvResultData
機能	: 勝敗を受け取る
引数	: なし
出力	: なし
*****************************************************************/
static void RecvResultData(void)
{
    int res;

    /* RESULT_COMMANDに対する引き数を受信する */
    RecvIntData(&res);

    result = res;

    SendEndCommand();
}