/*****************************************************************
ファイル名	: common.h
機能		: サーバーとクライアントで使用する定数の宣言を行う
*****************************************************************/

#ifndef _COMMON_H_
#define _COMMON_H_

#include <assert.h>
#include <SDL2/SDL.h>

/* Wiiリモコンを用いるために必要なヘッダファイル */
#include "wiimote.h"
#include "wiimote_api.h"

#define DEBUG_PRINT                         /* コメントアウトで出力非表示 */

#define PORT			    (u_short)8888   /* ポート番号 */
#define CLIENTNUM		    2	            /* クライアント数の最大値 */
#define MAX_NAME_SIZE       10 	            /* ユーザー名の最大値 */
#define MAX_DATA		    300		        /* 送受信するデータの最大値 */

#define WINDOWWIDE          1700            /* ウィンドウの横サイズ */
#define WINDOWHEIGHT        900             /* ウィンドウの縦サイズ */
#define STAGEWIDE           900             /* ステージの横サイズ */
#define STAGEHEIGHT         900             /* ステージの縦サイズ */
#define SHIFTCHARA          50              /* キャラの座標ずらす */

#define ALL_CHARAS          9               /* キャラの種類 */
#define CHARANUM            8               /* 操作されるキャラの数 */
#define ITEMNUM             3               /* 使用されるアイテムの数 */
#define FINISH_TIME         120             /* ゲーム終了までの時間 */

#define END_COMMAND         'A'             /* プログラム終了コマンド */
#define TITLE_COMMAND       'B'             /* タイトル終了コマンド */
#define LIFE_COMMAND        'C'             /* 体力共有コマンド */
#define POINT_COMMAND       'D'             /* 座標共有コマンド */
#define COOL_COMMAND        'E'             /* クールタイム共有コマンド */
#define STAGE_COMMAND       'F'             /* ステージ決定コマンド */
#define READY_COMMAND       'G'             /* 準備完了コマンド */
#define ACTION_COMMAND      'H'             /* 移動コマンド */
#define SKILL_COMMAND       'I'             /* スキルコマンド */
#define TURN_COMMAND        'J'             /* スキルターンコマンド */
#define DEATH_COMMAND       'K'             /* 死亡コマンド */
#define ITEM_COMMAND        'L'             /* アイテムコマンド */
#define GET_COMMAND         'M'             /* アイテム入手コマンド */
#define POISON_COMMAND      'N'             /* 毒状態コマンド */
#define DOWN_COMMAND        'O'             /* 攻撃アイテム使用コマンド */
#define BOUND_COMMAND       'P'             /* キャラ衝突コマンド */
#define EVENT_COMMAND       'Q'             /* イベント発生コマンド */
#define TIME_COMMAND        'R'             /* 経過時間共有コマンド */
#define RESULT_COMMAND      'S'             /* 勝敗コマンド */
#define EVENT_CUTIN_COMMAND 'T'             /* 〜参戦の描画 */

/*****************************************************************
                    システムに関する構造体など
*****************************************************************/

/* 勝敗の種類 */
typedef enum {
    Win,
    Lose,
    Draw,
    No_results
} ResultType;

/*****************************************************************
                    キャラ情報に関する構造体など
*****************************************************************/

/* キャラクターの種類 */
typedef enum{
    Zeus,           /* キャラID 0 */
    Amateras,       /* キャラID 1 */
    Bishamonten,    /* キャラID 2 */
    Odin,           /* キャラID 3 */
    Anubis,         /* キャラID 4 */
    Hercules,       /* キャラID 5 */
    Shiva,          /* キャラID 6 */
    Hades,          /* キャラID 7 */
    Susano          /* キャラID 8 */
} CharaID;

/* キャラクターの状態 */
typedef enum {
    CS_Alive,   /* 生きる */
    CS_Death    /* 死ぬ */
} CharaAoD;

/* スキル */
typedef struct {
    int type;
    int turn;
    SDL_Rect    src;
    SDL_Rect    dst;
    SDL_Texture *tex;
} SkillInfo;

/* 実数座標 */
typedef struct{
    float x;
    float y;
} FloatPoint;

/*****************************************************************
                    アイテムに関する構造体など
*****************************************************************/

/* アイテムの種類 */
typedef enum {
    Life,   /* ライフ変動系 */
    Attack, /* 攻撃上昇系 */
    etc     /* 速度等 */
} ItemType;

/* アイテムの情報 */
typedef struct {
    ItemType    type;   /* 種類 */
    FloatPoint  point;  /* 座標 */
    int         r;      /* 半径(25) */
    SDL_Rect    dst;
} ItemInfo;

#endif