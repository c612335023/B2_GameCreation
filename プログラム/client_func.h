/*****************************************************************
ファイル名	: client_func.h
機能		: クライアントで使用する定数の宣言と外部関数の定義
*****************************************************************/

#ifndef _CLIENT_FUNC_H_
#define _CLIENT_FUNC_H_

#include "common.h"

#define ALL_STAGES  3   /* ステージの種類 */
#define ALL_ITEMS   9   /* アイテムの種類 */

/*****************************************************************
                ウィンドウ表示に関する構造体など
*****************************************************************/

/* マップの種類 */
typedef enum {
    map_Sky,    /* 天界 */
    map_Hell,   /* 冥界 */
    map_Normal  /* 地上界 */
} MapType;

/* マップ情報 */
typedef struct{
    MapType type;
    char    path[128];
} StageImgInfo;

/* ゲームマップ　*/
typedef struct {
    SDL_Rect    src;    /* マップ転送領域 */
    SDL_Rect    dst;
    SDL_Texture *tex;
    char path[128];
} MapInfo;

/* 汎用的画像情報 */
typedef struct {
    SDL_Rect    src;
    SDL_Rect    dst;
    SDL_Texture *tex;
} GeneralTexInfo;

typedef struct {
    int         w;  /* キャラ画像の幅 */
    int         h;  /* 高さ */
    SDL_Texture *tex;
} ArrowIMG;

/*****************************************************************
                    システムに関する構造体など
*****************************************************************/

/* 効果音の種類 */
typedef enum {
    Select,
    Decision,
    Cancel,
    Shoot,
    Bound,
    Damage,
    Heal,
    Poison,
    Kill
} SEType;

/*****************************************************************
                    キャラ情報に関する構造体など
*****************************************************************/

/* クライアントが保持するキャラクターの情報 */
typedef struct{
    CharaAoD    aod;        /* キャラの生死 */
    CharaID     pos;        /* キャラの番号 */
    SkillInfo   skill;      /* スキル */
    FloatPoint  point;      /* 座標 */
    float       velocity;   /* 速度 */
    int         basiclife;  /* 基礎体力 */
    int         life;       /* 体力 */
    SDL_Rect    src_hp;     /* 基礎ライフゲージの幅 */
    SDL_Rect    dst_hp;     /* 現在ライフゲージの幅 */
    SDL_Rect    src;        /* キャラ元画像のRECT */
    SDL_Rect    dst;        /* キャラ画像の貼り付け位置のRECT */ 
    SDL_Texture *tex;       /* テクスチャー*/
    char        path[128];  /* パス */
} ClientCharaInfo;

/*****************************************************************
                    アイテムに関する構造体など
*****************************************************************/

/* アイテムの画像情報 */
typedef struct{
    char        path[128];  /* パス */
    SDL_Rect    src;
    SDL_Texture *tex;
} ItemIMG;

/***********************************
                変数                
***********************************/

static wiimote_t wiimote = WIIMOTE_INIT; // Wiiリモコンの状態格納用

/* client_main.c */
extern int              waitFlag;

/* client_command.c */
extern int              itemFlag;
extern int              wallFlag;
extern int              upFlag[CHARANUM];
extern int              result;

/* client_system.c */
extern ClientCharaInfo  *gChara;
extern ClientCharaInfo  *aChara;
extern ItemIMG          *gItem;
extern int              beforeLife[CHARANUM];

/* client_win.c */
extern MapInfo          gMap;
extern ItemInfo         Item;
extern int              input_home;
extern int              input_up;
extern int              input_right;
extern int              input_left;
extern int              input_down;
extern int              input_one;
extern int              input_two;
extern int              cutinFlag;
extern int              angle;
extern float            cooltime[CHARANUM];
extern int              atkFlag[CHARANUM];
extern int              poisonFlag[CHARANUM];
extern int              pasttime;

/***********************************
                関数                
***********************************/

/* client_net.c */
extern int  SetUpClient(char* hostName, int *clientID, int *num, char clientName[][MAX_NAME_SIZE]);
extern void SendData(void *data, int dataSize);
extern int  RecvIntData(int *intData);
extern int  RecvFloatData(float *floatData);
extern int  SendRecvManager();
extern void CloseSoc();

/* client_command.c */
extern void SendEndCommand();
extern void SendTitleCommand();
extern void SendStageCommand(int stageID);
extern void SendReadyCommand(int *CID, int cnum);
extern void SendActionCommand(int cnum);
extern void SendSkillCommand(int cnum);
extern int  ExecuteCommand(char command);

/* client_system.c */
extern int  InitSystem(int pos);
extern void ReadAllStage(const char *stage_data_file);
extern void ReadAllChara(const char *chara_dada_file);
extern void ReadAllItem(const char *Item_dada_fail);
extern void ReadStage(int num);
extern void ReadChara(int *CID, int i);
extern void ReadItem(int stageID);
extern void GetMusic();
extern void SoundEffect(int act);
extern void CloseMusic();

/* client_win.c */
extern int  InitWindows(int num);
extern void InitChara(int pos);
extern void MakeTitleIMG();
extern void MakeSSIMG();
extern void MakeStageIMG();
extern void MakeItemIMG();
extern void MakeEventIMG(int stageID);
extern void MakeCSIMG();
extern void MakeCharaIMG();
extern void MakeEffectIMG();
extern void MakeCNIMG();
extern void MakeSGIMG(int cnum);
extern int  DrawTitle();
extern int  StageSelect();
extern int  CharaSelect();
extern void DestroyTitleTex();
extern void DestroySSTex();
extern void DestroyCSTex();
extern void WindowEvent();
extern void WinDraw(int pos);
extern void DrawSSWait();
extern void DrawWait();
extern void DrawResult();
extern void DestroyAllTexture();

#endif