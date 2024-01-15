/*****************************************************************
ファイル名	: server_func.h
機能		: サーバーで使用する定数の宣言と外部関数の定義
*****************************************************************/

#ifndef _SERVER_FUNC_H_
#define _SERVER_FUNC_H_

#include "common.h"

#define ALL_CLIENTS	-1      /* 全クライアントにデータを送る時に使用する */

#define MOVE_TIME   0.05    /* 1動作の時間 */
#define EVENT_TIME  60      /* イベントまでの時間 */

/*****************************************************************
                    キャラ情報に関する構造体など
*****************************************************************/

/* キャラクターの属性 */
typedef enum{
    fire,   /* 火属性 */
    aqua,   /* 水属性 */
    plant   /* 木属性 */
} ElementType;

/* キャラのステータス */
typedef struct{
    ElementType elem;   /* 属性 */
    int         r;      /* 半径 */
    float       m;      /* 基準重量 */
    float       vel;    /* 基礎初速度 */
    float       accl;   /* 基礎加速度 */
    int         life;   /* 基礎体力 */
    int         atk;    /* 基礎攻撃力 */
} CharaStatus;

/* サーバーが保持するキャラクターの情報 */
typedef struct{
    CharaStatus basestts;   /* キャラの基礎ステータス */
    CharaStatus stts;       /* キャラの実用ステータス */
    CharaAoD    aod;        /* キャラの状態 */
    SkillInfo   skill;      /* スキル */
    FloatPoint  point;      /* 座標 */
    int         dir;        /* 移動方向 */
} ServerCharaInfo;

/***********************************
                変数                
***********************************/

/* server_main.c */
extern int              move_flag[CHARANUM];
extern int              ct_flag[CHARANUM];
extern int              Item_flag;
extern int              Game_flag;
extern float            cooltime[CHARANUM];

/* server_command.c */
extern int              stageID;

/* server_system.c */
extern ServerCharaInfo  *gChara;
extern ServerCharaInfo  *aChara;
extern int              AtkUp_turn[CHARANUM];
extern int              AtkRestore_flag[CHARANUM];
extern int              Poison_turn[CHARANUM];

/***********************************
                関数                
***********************************/

/* server_net.c */
extern int  SetUpServer(int num);
extern void SendData(int pos, void *data, int dataSize);
extern int  SendRecvManager();
extern int  RecvData(int pos, void *data, int dataSize);
extern int  RecvIntData(int pos, int *intData);
extern int  RecvFloatData(int pos, float *floatData);

/* server_command.c */
extern void SendLifeCommand(int cnum, int life);
extern void SendDeathCommand(int cnum);
extern void SendPointCommand();
extern void SendTurnCommand(int cnum);
extern void SendCoolCommand(int cnum);
extern void SendItemCommand(int Item_type, int Item_x, int Item_y);
extern void SendGetCommand(int cnum, int Item_type);
extern void SendPoisonCommand(int cnum, int turn);
extern void SendDownCommand(int cnum);
extern void SendBoundCommand();
extern void SendEventCommand();
extern void SendTimeCommand(int pasttime);
extern void SendEventCutinCommand(void);
extern void SendSkyEventCommand(int *plus_flga);
extern void SendHellEventCommand(int *minus_flag);
extern void SendResultCommand(int pos, int result);
extern int  ExecuteCommand(char command,int pos);

/* server_system.c */
extern int  InitSystem();
extern void ReadAllChara(const char *chara_data_file);
extern void ReadChara(int *CID, int i);
extern void MoveCharaPoint(int cnum);
extern void Skill_CD(int pos, int cnum);
extern void Skill_CA(int cnum);
extern void Skill_RO(int pos);
extern void ItemRand();
extern void PoisonCondition(int cnum);
extern void MakeEvent();
extern void TimeUp();

#endif