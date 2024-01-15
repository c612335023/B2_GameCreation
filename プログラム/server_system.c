/*****************************************************************
ファイル名	: server_system.c
機能		: クライアントのゲームシステム処理
*****************************************************************/

#include "server_func.h"

#include <time.h>

/***********************************
                変数                
***********************************/

ServerCharaInfo *gChara, *aChara;                       // キャラの情報

int AtkUp_turn[CHARANUM]        = { 0 };                // 攻撃アップしているキャラ判別
int AtkRestore_flag[CHARANUM]   = { 0 };                // 攻撃力をもとに戻すフラグ
int Poison_turn[CHARANUM]       = { 0 };                // 毒状態の残存ターン数

/* static */
static FloatPoint   prepoint[CHARANUM];                 // 直前の座標

static ItemInfo     Item;                               // アイテムの情報

static int          i, j;                               // for文用

static float        rad;                                // 移動角度のラジアン

static float        dx, dy;                             // キャラ同士のx, y座標の差

static float        distance;                           // 重なり部分の長さ
static float        bvx0, bvy0, bvx1, bvy1;             // x, y方向の速度
static float        t;                                  // よくわからないやつ
static float        mx0, my0, mx1, my1;                 // 移動運動
static float        rx0, ry0, rx1, ry1;                 // 回転運動
static float        avx0, avy0, avx1, avy1;             // 衝突後速度
static float        dir0, dir1;                         // 衝突後角度
static float        dif;                                // 衝突キャラ半径の合計
static float        len;                                // キャラの中心座標の距離

static int 	        Road_flag = 0;                      // 地上界イベントフラグ

static float        CharaItemdx, CharaItemdy;           // キャラとアイテムのx, y座標の差

static int          Sword_flag[CHARANUM]    = { 0 };    // 剣所持状態フラグ
static int          Scythe_flag[CHARANUM]   = { 0 };    // 死神の鎌所持状態フラグ
static int          SD_flag[CHARANUM]       = { 0 };    // SDスキル使用フラグ

/***********************************
                関数                
***********************************/

static void AdjustCharaLife(int cnum0, int cnum1);
static void AdjustCharaAngle(int cnum1, int cnum2);
static void AdjustOverlapBlock(int cnum);
static void IncreaseSkillTurn(int intData);
static void DecreaseSkillTurn(int cnum);
static void GetItem(int cnum);
static void GetBansouko(int cnum);
static void GetBat(int cnum);
static void GetBike(int cnum);
static void GetGoldApple(int cnum);
static void GetSword(int cnum);
static void GetPlane(int cnum);
static void GetPoison(int cnum);
static void GetScythe(int cnum);
static void GetBalance(int cnum);
static void JudgeResult(void);
static void MakeSkyEvent(void);
static void MakeHellEvent(void);

/*****************************************************************
関数名  : InitSystem
機能    : ゲームシステム初期化
引数    : キャラデータ、マップデータ　←後で考える
出力    : 正常終了  0
          エラー    負数
*****************************************************************/
int InitSystem()
{
    /* 領域確保 */
    gChara = (ServerCharaInfo *)malloc(sizeof(ServerCharaInfo) * CHARANUM);
    if(gChara == NULL){
        fprintf(stderr, "system error : gChara malloc failed. (InitSystem)\n");
        exit(-1);
    }

    /* キャラの基本設定 */
    for(i = 0; i < 4; i++){
        gChara[i].point.x   = ((WINDOWWIDE - STAGEWIDE) / 2) + 180 + 180 * i;
        gChara[i].point.y   = 850 - SHIFTCHARA;
		prepoint[i].x       = gChara[i].point.x;
		prepoint[i].y       = gChara[i].point.y;
        gChara[i].aod       = CS_Alive;
        gChara[i].dir       = -1;
    }
    for(i = 4; i < CHARANUM; i++){
        gChara[i].point.x   = ((WINDOWWIDE - STAGEWIDE) / 2) + 720 - 180 * (i - 4);
        gChara[i].point.y   = 50 + SHIFTCHARA;
		prepoint[i].x       = gChara[i].point.x;
        prepoint[i].y       = gChara[i].point.y;
        gChara[i].aod       = CS_Alive;
        gChara[i].dir       = -1;
    }
    return 0;
}

/******************************************************************
関数名  : ReadAllChara
機能    : 全キャラ情報の読み込み
補足    : chara.data.serverは初速,速度,加速度,半径,基礎体力,基礎攻撃力を記載
******************************************************************/
void ReadAllChara(const char *chara_data_file)
{
    /* 領域確保 */
    aChara = (ServerCharaInfo *)malloc(sizeof(ServerCharaInfo) * ALL_CHARAS);
    if(aChara == NULL){
        fprintf(stderr, "system error : aChara malloc failed. (ReadAllChara)\n");
        exit(-1);
    }
    /* ファイルを開く */
    FILE *fp = fopen(chara_data_file, "r");
    if(fp == NULL){
        fprintf(stderr,"system error : failed to open chara data file. (ReadAllChara)\n");
        exit(-1);
    }
    /* ファイル情報読込 */
    for (i = 0; i < ALL_CHARAS; i++){
        if(8 != fscanf(fp, "%d%d%f%f%u%f%u%u", &(aChara[i].basestts.life), &(aChara[i].basestts.atk), &(aChara[i].basestts.vel), &(aChara[i].basestts.accl), &(aChara[i].basestts.r), &(aChara[i].basestts.m), &(aChara[i].skill.type), &(aChara[i].basestts.elem))){
            fprintf(stderr, "system error : failed to read the chara status. (ReadAllChara)\n");
            exit(-1);
        }
    }
    fclose(fp);
}

/******************************************************************
関数名  : ReadChara
機能    : 使用するキャラ情報の読み込み
引数    : int *CID  : 選んだキャラID
          int cnum  : キャラ番号
出力    : なし
******************************************************************/
void ReadChara(int *CID, int cnum)
{
    /* キャラ情報読込 */
    gChara[cnum].basestts.elem  = aChara[*CID].basestts.elem;
    gChara[cnum].basestts.r     = aChara[*CID].basestts.r;
    gChara[cnum].basestts.m     = aChara[*CID].basestts.m;
    gChara[cnum].basestts.vel   = aChara[*CID].basestts.vel;
    gChara[cnum].basestts.accl  = aChara[*CID].basestts.accl;
    gChara[cnum].basestts.life  = aChara[*CID].basestts.life;
    gChara[cnum].basestts.atk   = aChara[*CID].basestts.atk;
    gChara[cnum].skill.type     = aChara[*CID].skill.type;
    gChara[cnum].skill.turn     = 0;
    
    /* 読込情報を元にステータス生成 */
    gChara[cnum].stts.elem  = gChara[cnum].basestts.elem;
    gChara[cnum].stts.r     = gChara[cnum].basestts.r;
    gChara[cnum].stts.m     = gChara[cnum].basestts.m;
    gChara[cnum].stts.vel   = 0;
    gChara[cnum].stts.accl  = gChara[cnum].basestts.accl;
    gChara[cnum].stts.life  = gChara[cnum].basestts.life;
    gChara[cnum].stts.atk   = gChara[cnum].basestts.atk;
}

/******************************************************************
関数名  : ItemRand
機能    : 乱数によりアイテムの種類と座標を決定する
引数    : なし
出力    : なし
******************************************************************/
void ItemRand(void)
{
    unsigned int seed;
    seed = (unsigned int)time(NULL);
    srand(seed);

    /* アイテムの種類決定 */
    Item.type       = (rand() % 3);
    /* アイテムの座標決定 */
    Item.point.x    = (rand() % 8 * 100) + 500;
    Item.point.y    = (rand() % 6 * 100) + 200;
    Item.r          = 25;

    /* キャラとの重なり確認 */
    for(i = 0; i < CHARANUM; i++){
        CharaItemdx = Item.point.x - gChara[i].point.x;
        CharaItemdy = Item.point.y - gChara[i].point.y;
        if(sqrt(CharaItemdx * CharaItemdx + CharaItemdy * CharaItemdy) <= Item.r + gChara[i].stts.r){
            break;
        }
    }
    /* 誰とも重なっていない場合 */
    if(i == CHARANUM){

#ifdef DEBUG_PRINT
    fprintf(stderr,"Coordinate determination\n");
#endif

        SendItemCommand(Item.type, Item.point.x, Item.point.y);
        Item_flag = 1;
    }
}

/******************************************************************
関数名  : MoveCharaPoint
機能    : キャラを動かす
引数    : int   cnum    : キャラ番号
出力    : なし
******************************************************************/
void MoveCharaPoint(int cnum)
{
    /* 動作前の座標を保持 */
    prepoint[cnum] = gChara[cnum].point;

    /* 初動の場合 */
    if(!move_flag[cnum]){
        /* 初速を与える */
		gChara[cnum].stts.vel = gChara[cnum].basestts.vel;
		move_flag[cnum] = 1;
	}

    /* キャラ速度変化 */
    gChara[cnum].stts.vel += gChara[cnum].stts.accl * MOVE_TIME;

    /* 速度 0 で処理終了 */
    if(gChara[cnum].stts.vel <= 0){
        /* フラグ変化 */
        move_flag[cnum] = 0;
        ct_flag[cnum]   = 1;
        SD_flag[cnum]   = 0;
        if(AtkRestore_flag[cnum]){
            AtkUp_turn[cnum]--;
            AtkRestore_flag[cnum]   = 0;
            Sword_flag[cnum]        = 0;
            Scythe_flag[cnum]       = 0;
        }
        /* 能力値初期化 */
        gChara[cnum].stts.accl  = gChara[cnum].basestts.accl;
        gChara[cnum].stts.m     = gChara[cnum].basestts.m;
        gChara[cnum].dir        = -1;
        gChara[cnum].stts.vel   = 0;
        if(AtkUp_turn[cnum] == 0){
            gChara[cnum].stts.atk = gChara[cnum].basestts.atk;
            SendDownCommand(cnum);
        }
        /* スキルターン増加 */
        IncreaseSkillTurn(cnum);

#ifdef DEBUG_PRINT
    fprintf(stderr, "End of calculation\n");
#endif

    }else{
        /* ラジアンの計算 */
        rad = gChara[cnum].dir * M_PI / 180.0;

        /* 座標更新 */
        gChara[cnum].point.x = prepoint[cnum].x + gChara[cnum].stts.vel * MOVE_TIME * cosf(rad);
        gChara[cnum].point.y = prepoint[cnum].y + gChara[cnum].stts.vel * MOVE_TIME * sinf(rad);

        /* 他キャラとの重なり判定 */
        for(i = 0; i < CHARANUM; i++){
            if(cnum != i && gChara[i].aod == CS_Alive){
                /* x, y座標の差 */
                dx = gChara[cnum].point.x - gChara[i].point.x;
                dy = gChara[cnum].point.y - gChara[i].point.y;

                if(sqrt(dx * dx + dy * dy) <= gChara[cnum].stts.r + gChara[i].stts.r){
                    /* 味方に衝突の場合 */
                    if(((cnum >= 0 && cnum < 4) && (i >= 0 && i < 4))
                    || ((cnum >= 4 && cnum < CHARANUM) && (i >= 4 && i < CHARANUM))){
                        SendBoundCommand();
                    }else{
                        /* 剣（アイテム）効果 */
                        if(Sword_flag[cnum]){
                            IncreaseSkillTurn(cnum);
                        }

                        /* 鎌（アイテム）効果 */
                        if(Scythe_flag[cnum]){
                            DecreaseSkillTurn(i);
                        }

                        /* ダメージ計算 */
                        AdjustCharaLife(cnum, i);
                    }
                    /* 毒状態効果 */
                    if(Poison_turn[cnum] != 0){
                        if(--Poison_turn[cnum] == 0){
                            SendPoisonCommand(cnum, Poison_turn[cnum]);
                        }
                        SendPoisonCommand(i, ++Poison_turn[i]);
                    }

                    /* 反射角度計算 */
                    AdjustCharaAngle(cnum, i);
                }
            }
        }
        /* アイテムとの重なり判定 */
        if(Item_flag){
            GetItem(cnum);
        }

        /* 壁との重なり判定 */
        AdjustOverlapBlock(cnum);
    }
    SendPointCommand(cnum);
}

/******************************************************************
関数名  : Skill_CA
機能    : 次回行動のキャラ重量減少
          及び衝突時加速効果フラグ付与(Collision acceleration)

          スキル効果:
            重量 1/5
            衝突時加速

引数    : int cnum  : キャラ番号
出力    : なし
******************************************************************/
void Skill_CA(int cnum)
{
    /* 重量減少 */
    gChara[cnum].stts.m *= 0.2;
    /* フラグ */
    SD_flag[cnum]       = 1;
}

/******************************************************************
関数名  : Skill_CD
機能    : 最も近い敵キャラにダメージを与える(Cause damage)
          
          スキル効果:
            100ダメージ

引数    : int pos   : クライアントID
          int cnum  : キャラ番号
出力    : なし
******************************************************************/
void Skill_CD(int pos, int cnum)
{
    float   CD_dx, CD_dy;                   // 敵キャラとのx,y座標の距離
    float   min_dif = STAGEWIDE * sqrt(2);  // 敵キャラとの中心座標の距離（ステージの対角線の長さで初期化）
    int     nearchara;                      // 最も近い敵キャラの番号

    /* 最も近い敵キャラ番号を探索 */
    if(pos == 0){
        for(i = 4; i < CHARANUM; i++){
            if(gChara[i].aod == CS_Alive){
                CD_dx = gChara[cnum].point.x - gChara[i].point.x;
                CD_dy = gChara[cnum].point.y - gChara[i].point.y;
                if(min_dif > sqrt(CD_dx * CD_dx + CD_dy * CD_dy)){
                    min_dif = sqrt(CD_dx * CD_dx + CD_dy * CD_dy);
                    nearchara = i;
                }
            }
        }
    }else if(pos == 1){
        for(i = 0; i < 4; i++){
            if(gChara[i].aod == CS_Alive){
                CD_dx = gChara[cnum].point.x - gChara[i].point.x;
                CD_dy = gChara[cnum].point.y - gChara[i].point.y;
                if(min_dif > sqrt(CD_dx * CD_dx + CD_dy * CD_dy)){
                    min_dif = sqrt(CD_dx * CD_dx + CD_dy * CD_dy);
                    nearchara = i;
                }
            }
        }
    }

    /* 最も近い敵キャラにダメージ処理 */
    gChara[nearchara].stts.life -= 100;
    if(gChara[nearchara].stts.life <= 0){
        gChara[nearchara].stts.life  = 0;
        gChara[nearchara].aod   = CS_Death;
        SendDeathCommand(nearchara);
        JudgeResult();
    }
    SendLifeCommand(nearchara, gChara[nearchara].stts.life);
}

/******************************************************************
関数名  : Skill_RO
機能    : 味方キャラ全員を回復する(recover overall)
          
          スキル効果:
            全体50回復

引数    : int pos   : クライアントID
出力    : なし
******************************************************************/
void Skill_RO(int pos)
{
    /* 味方キャラ全員回復 */
    if(pos == 0){
        for(i = 0; i < 4; i++){
            if(gChara[i].aod == CS_Alive){
                gChara[i].stts.life += 50;
                if(gChara[i].stts.life > gChara[i].basestts.life){
                    gChara[i].stts.life = gChara[i].basestts.life;
                }
                SendLifeCommand(i, gChara[i].stts.life);
            }
        }
    }else if(pos == 1){
        for(i = 4; i < CHARANUM; i++){
            if(gChara[i].aod == CS_Alive){
                gChara[i].stts.life += 50;
                if(gChara[i].stts.life > gChara[i].basestts.life){
                    gChara[i].stts.life = gChara[i].basestts.life;
                }
                SendLifeCommand(i, gChara[i].stts.life);
            }
        }
    }
}

/******************************************************************
関数名  : PoisonCondition
機能    : 毒状態のダメージ計算
引数    : int cnum  : キャラ番号
出力    : なし
******************************************************************/
void PoisonCondition(int cnum)
{
    /* ダメージ計算 */
    gChara[cnum].stts.life -= 30;
    if(gChara[cnum].stts.life <= 0){
        gChara[cnum].stts.life  = 0;
        gChara[cnum].aod        = CS_Death;
        SendDeathCommand(cnum);
        JudgeResult();
    }
    SendLifeCommand(cnum, gChara[cnum].stts.life);
    if(--Poison_turn[cnum] == 0){
        SendPoisonCommand(cnum, Poison_turn[cnum]);
    }
}

/******************************************************************
関数名  : MakeEvent
機能    : イベント処理
引数    : なし
出力    : なし
******************************************************************/
void MakeEvent(void)
{
    switch(stageID){
        case 0:
            MakeSkyEvent();
            break;
        case 1:
            MakeHellEvent();
            break;
        case 2:
            Road_flag = 1;
            SendEventCommand();
            break;
        default:
            fprintf(stderr, "system error : MakeEvent failed.\n");
    }
}

/******************************************************************
関数名  : TimeUp
機能    : 時間切れで結果を表示
引数    : なし
出力    : なし
******************************************************************/
void TimeUp(void)
{
    int     count_num_0 = 0;        // 残存キャラ数
    int     count_num_1 = 0;
    float   count_percent_0 = 0;    // 残存HP割合の和
    float   count_percent_1 = 0;
    int     count_hp_0 = 0;         // 残存HPの和
    int     count_hp_1 = 0;

    /* 残存キャラ数を合計 */
    for(i = 0; i < 4; i++){
        if(gChara[i].aod == CS_Alive){
            count_num_0++;
        }
    }
    for(i = 4; i < CHARANUM; i++){
        if(gChara[i].aod == CS_Alive){
            count_num_1++;
        }
    }

    /* 勝敗判定 */
    if(count_num_0 > count_num_1){
        SendResultCommand(0, Win);
        SendResultCommand(1, Lose);
    }else if(count_num_0 < count_num_1){
        SendResultCommand(0, Lose);
        SendResultCommand(1, Win);

    /* 残存キャラ数が等しい場合 */
    }else{
        /* 残存HP割合を合計 */
        for(i = 0; i < 4; i++){
            if(gChara[i].aod == CS_Alive){
                count_percent_0 += gChara[i].stts.life * 100 / gChara[i].basestts.life;
            }
        }
        for(i = 4; i < CHARANUM; i++){
            if(gChara[i].aod == CS_Alive){
                count_percent_1 += gChara[i].stts.life * 100 / gChara[i].basestts.life;
            }
        }

        /* 勝敗判定 */
        if(count_percent_0 > count_percent_1){
            SendResultCommand(0, Win);
            SendResultCommand(1, Lose);
        }else if(count_percent_0 < count_percent_1){
            SendResultCommand(0, Lose);
            SendResultCommand(1, Win);

        /* 残存HP割合が等しい場合 */
        }else{
            /* 残存HPを合計 */
            for(i = 0; i < 4; i++){
                if(gChara[i].aod == CS_Alive){
                    count_hp_0 += gChara[i].stts.life;
                }
            }
            for(i = 4; i < CHARANUM; i++){
                if(gChara[i].aod == CS_Alive){
                    count_hp_1 += gChara[i].stts.life;
                }
            }

            /* 勝敗判定 */
            if(count_hp_0 > count_hp_1){
                SendResultCommand(0, Win);
                SendResultCommand(1, Lose);
            }else if(count_hp_0 < count_hp_1){
                SendResultCommand(0, Lose);
                SendResultCommand(1, Win);

            /* 全て等しい場合 */
            }else SendResultCommand(ALL_CLIENTS, Draw);
        }
    }
}

 /***************
 *   static    *
***************/
/******************************************************************
関数名  : AdjustCharaAngle
機能    : 反射後の移動角度を計算する
引数    : int cnum0 : キャラ番号
          int cnum1 : キャラ番号
出力    : なし
******************************************************************/
static void AdjustCharaAngle(int cnum0, int cnum1)
{
    /* x, y方向の速度 */
    bvx0 = gChara[cnum0].stts.vel * cosf(gChara[cnum0].dir * M_PI / 180);
    bvx1 = gChara[cnum1].stts.vel * cosf(gChara[cnum1].dir * M_PI / 180);
    bvy0 = gChara[cnum0].stts.vel * sinf(gChara[cnum0].dir * M_PI / 180);
    bvy1 = gChara[cnum1].stts.vel * sinf(gChara[cnum1].dir * M_PI / 180);

    /* 重心, 垂直方向の速度 */
    t   = -(dx * bvx0 + dy * bvy0) / (dx * dx + dy * dy);
    rx0 = bvx0 + dx * t;
    ry0 = bvy0 + dy * t;

    t   = -(-dy * bvx0 + dx * bvy0) / (dy * dy + dx * dx);
    mx0 = bvx0 - dy * t;
    my0 = bvy0 + dx * t;

    t   = -(dx * bvx1 + dy * bvy1) / (dx * dx + dy * dy);
    rx1 = bvx1 + dx * t;
    ry1 = bvy1 + dy * t;

    t   = -(-dy * bvx1 + dx * bvy1) / (dy * dy + dx * dx);
    mx1 = bvx1 - dy * t;
    my1 = bvy1 + dx * t;

    /* x, y方向の衝突後の速度 */
    avx0 = (mx0 * (gChara[cnum0].stts.m - gChara[cnum1].stts.m) + 2 * gChara[cnum1].stts.m * mx1) / (gChara[cnum0].stts.m + gChara[cnum1].stts.m);
    avy0 = (my0 * (gChara[cnum0].stts.m - gChara[cnum1].stts.m) + 2 * gChara[cnum1].stts.m * my1) / (gChara[cnum0].stts.m + gChara[cnum1].stts.m);
    avx1 = - (mx1 - mx0) + avx0;
    avy1 = - (my1 - my0) + avy0;

    bvx0 = avx0 + rx0;
    bvy0 = avy0 + ry0;
    bvx1 = avx1 + rx1;
    bvy1 = avy1 + ry1;

    /* 角度変更 */
    dir0 = atan2f(bvy0, bvx0) * 180 / M_PI;
    dir1 = atan2f(bvy1, bvx1) * 180 / M_PI;

    if(dir0 < 0){
        dir0 += 360;
    }
    if(dir1 < 0){
        dir1 += 360;
    }

    gChara[cnum0].dir = (int)dir0;
    gChara[cnum1].dir = (int)dir1;

    /* 速度変更 */
    gChara[cnum0].stts.vel = sqrt(bvx0 * bvx0 + bvy0 * bvy0);
    gChara[cnum1].stts.vel = sqrt(bvx1 * bvx1 + bvy1 * bvy1);

    if(SD_flag[cnum0]){
        gChara[cnum0].stts.vel += 150;
    }
    if(SD_flag[cnum1]){
        gChara[cnum1].stts.vel += 150;
    }

    /* キャラの中心座標の距離 */
    dif = gChara[cnum0].stts.r + gChara[cnum1].stts.r;
    len = sqrt(dx * dx + dy * dy);

    /* 座標修正 */
    distance = dif - len;
    if(len > 0){
        len = 1 / len;
    }
    dx *= len;
    dy *= len;
    distance /= 2;
    gChara[cnum0].point.x += dx * distance;
    gChara[cnum0].point.y += dy * distance;
    gChara[cnum1].point.x -= dx * distance;
    gChara[cnum1].point.y -= dy * distance;

    /* フラグを立てる */
    move_flag[cnum0] = 1;
    move_flag[cnum1] = 1;
}

/******************************************************************
関数名  : AdjustCharaLife
機能    : 当たったキャラが敵の場合ダメージを与える
引数    : int cnum0 : キャラ番号
          int cnum1 : キャラ番号
出力    : なし
******************************************************************/
static void AdjustCharaLife(int cnum0, int cnum1)
{
    /* 不利属性の場合 */
    if((gChara[cnum0].stts.elem == fire && gChara[cnum1].stts.elem == aqua)
    || (gChara[cnum0].stts.elem == aqua && gChara[cnum1].stts.elem == plant)
    || (gChara[cnum0].stts.elem == plant && gChara[cnum1].stts.elem == fire)){
        /* 相手に自攻撃3/4ダメージ */
        gChara[cnum1].stts.life -= gChara[cnum0].stts.atk * 0.75;
        /* 当たったキャラが動いている場合 */
        if(gChara[cnum1].stts.vel > 0){
            /* 自分に相手攻撃1.5倍ダメージ */
            gChara[cnum0].stts.life -= gChara[cnum1].stts.atk * 1.5;
        }
    /* 有利属性の場合 */
    }else if((gChara[cnum0].stts.elem == aqua && gChara[cnum1].stts.elem == fire)
    || (gChara[cnum0].stts.elem == plant && gChara[cnum1].stts.elem == aqua)
    || (gChara[cnum0].stts.elem == fire && gChara[cnum1].stts.elem == plant)){
        /* 相手に自攻撃1.5倍ダメージ */
        gChara[cnum1].stts.life -= gChara[cnum0].stts.atk * 1.5;
        /* 当たったキャラが動いている場合 */
        if(gChara[cnum1].stts.vel > 0){
            /* 自分に相手攻撃3/4ダメージ */
            gChara[cnum0].stts.life -= gChara[cnum1].stts.atk * 0.75;
        }
    /* 同属性の場合 */
    }else{
        /* 相手に自攻撃等倍ダメージ */
        gChara[cnum1].stts.life -= gChara[cnum0].stts.atk;
        /* 当たったキャラが動いている場合 */
        if(gChara[cnum1].stts.vel > 0){
            /* 自分に相手攻撃等倍ダメージ */
            gChara[cnum0].stts.life -= gChara[cnum1].stts.atk;
        }
    }

    /* 自体力が0以下の時 */
    if(gChara[cnum0].stts.life <= 0){
        gChara[cnum0].stts.life = 0;
        gChara[cnum0].aod       = CS_Death;
        SendDeathCommand(cnum0);
        JudgeResult();
    }
    SendLifeCommand(cnum0, gChara[cnum0].stts.life);

    /* 相手体力が0以下の時 */
    if(gChara[cnum1].stts.life <= 0){
        gChara[cnum1].stts.life = 0;
        gChara[cnum1].aod       = CS_Death;
        SendDeathCommand(cnum1);
        JudgeResult();
    }
    SendLifeCommand(cnum1, gChara[cnum1].stts.life);
}

/******************************************************************
関数名  : AdjustOverlapBlock
機能    : 壁との重なりを調べて重なっていたら反射する
引数    : int cnum   : キャラ番号
出力    : なし
******************************************************************/
static void AdjustOverlapBlock(int cnum)
{
    /* 上の壁に当たった場合 */
    if(gChara[cnum].point.y < gChara[cnum].stts.r + SHIFTCHARA){
        /* 座標修正 */
        gChara[cnum].point.y = gChara[cnum].stts.r + SHIFTCHARA;

        /* 地上界イベントが発生している場合 */
        if(Road_flag){
            /* ダメージ処理 */
            gChara[cnum].stts.life -= gChara[cnum].basestts.life / 3;
            if(gChara[cnum].stts.life <= 0 && gChara[cnum].aod == CS_Alive){
                gChara[cnum].stts.life  = 0;
                gChara[cnum].aod        = CS_Death;
                SendDeathCommand(cnum);
                JudgeResult();
            }
            SendLifeCommand(cnum,gChara[cnum].stts.life);
        }

        /* 角度更新 */
        gChara[cnum].dir = 360 - gChara[cnum].dir;

    /* 下の壁に当たった場合 */
    }else if(gChara[cnum].point.y > - gChara[cnum].stts.r + STAGEHEIGHT - SHIFTCHARA){
        /* 座標修正 */
        gChara[cnum].point.y = - gChara[cnum].stts.r + STAGEHEIGHT - SHIFTCHARA;

        /* 地上界イベントが発生している場合 */
        if(Road_flag){
            /* ダメージ処理 */
            gChara[cnum].stts.life -= gChara[cnum].basestts.life / 3;
            if(gChara[cnum].stts.life <= 0 && gChara[cnum].aod == CS_Alive){
                gChara[cnum].stts.life  = 0;
                gChara[cnum].aod        = CS_Death;
                SendDeathCommand(cnum);
                JudgeResult();
            }
            SendLifeCommand(cnum,gChara[cnum].stts.life);
        }

        /* 角度更新 */
        gChara[cnum].dir = 360 - gChara[cnum].dir;
    }

    /* 左の壁に当たった場合 */
    if(gChara[cnum].point.x < gChara[cnum].stts.r + ((WINDOWWIDE - STAGEWIDE) / 2) + SHIFTCHARA){
        /* 座標修正 */
        gChara[cnum].point.x = gChara[cnum].stts.r + ((WINDOWWIDE - STAGEWIDE) / 2) + SHIFTCHARA;

        /* 地上界イベントが発生している場合 */
        if(Road_flag){
            /* ダメージ処理 */
            gChara[cnum].stts.life -= gChara[cnum].basestts.life / 3;
            if(gChara[cnum].stts.life <= 0 && gChara[cnum].aod == CS_Alive){
                gChara[cnum].stts.life  = 0;
                gChara[cnum].aod        = CS_Death;
                SendDeathCommand(cnum);
                JudgeResult();
            }
            SendLifeCommand(cnum,gChara[cnum].stts.life);
        }

        /* 角度更新 */
        gChara[cnum].dir = 540 - gChara[cnum].dir;

    /* 右の壁に当たった場合 */
    }else if(gChara[cnum].point.x > - gChara[cnum].stts.r + ((WINDOWWIDE + STAGEWIDE) / 2) - SHIFTCHARA){
        /* 座標修正 */
        gChara[cnum].point.x = - gChara[cnum].stts.r + ((WINDOWWIDE + STAGEWIDE) / 2) - SHIFTCHARA;

        /* 地上界イベントが発生している場合 */
        if(Road_flag){
            /* ダメージ処理 */
            gChara[cnum].stts.life -= gChara[cnum].basestts.life / 3;
            if(gChara[cnum].stts.life <= 0 && gChara[cnum].aod == CS_Alive){
                gChara[cnum].stts.life  = 0;
                gChara[cnum].aod        = CS_Death;
                SendDeathCommand(cnum);
                JudgeResult();
            }
            SendLifeCommand(cnum,gChara[cnum].stts.life);
        }

        /* 角度更新 */
        gChara[cnum].dir = 180 - gChara[cnum].dir;
    }
}

/***********************************************************
関数名  : IncreaseSkillTurn
機能    : スキルターンを加速する
引数    : int cnum  : キャラ番号
出力    : なし
*************************************************************/
static void IncreaseSkillTurn(int cnum)
{
    /* スキルターン加速 */
    switch(gChara[cnum].skill.type){
        case 0:
            if(gChara[cnum].skill.turn < 3){
                gChara[cnum].skill.turn++;
            }
            break;
        case 1:
            if(gChara[cnum].skill.turn < 5){
                gChara[cnum].skill.turn++;
            }
            break;
        case 2:
            if(gChara[cnum].skill.turn < 4){
                gChara[cnum].skill.turn++;
            }
            break;
        default:
            fprintf(stderr, "system error : IncreaseSkillTurn failed.\n");
    }
    SendTurnCommand(cnum);
}

/***********************************************************
関数名  : DecreaseSkillTurn
機能    : スキルターンを延長する
引数    : int cnum : キャラ番号
出力    : なし
*************************************************************/
static void DecreaseSkillTurn(int cnum)
{
    /* スキルターン延長 */
    if(gChara[cnum].skill.turn > 0){
        gChara[cnum].skill.turn--;
    }
    SendTurnCommand(cnum);
}

/******************************************************************
関数名  : GetItem
機能    : アイテムとの重なりを調べて重なっていたら効果発動
引数    : int cnum  : キャラ番号
出力    : なし
******************************************************************/
static void GetItem(int cnum)
{
    /* アイテムとキャラのx, y座標の差 */
    CharaItemdx = gChara[cnum].point.x - Item.point.x;
    CharaItemdy = gChara[cnum].point.y - Item.point.y;

    /* アイテムとキャラが重なっている場合 */
    if(sqrt(CharaItemdx * CharaItemdx + CharaItemdy * CharaItemdy) <= gChara[cnum].stts.r + Item.r){
        /* アイテムタイプとステージIDに応じてアイテム効果処理 */
        switch(Item.type){
            case Life:
                switch(stageID){
                    case 0:
                        GetGoldApple(cnum);
                        break;
                    case 1:
                        GetPoison(cnum);
                        break;
                    case 2:
                        GetBansouko(cnum);
                        break;
                    default:
                        fprintf(stderr, "system error : Unknown stageID. (GetItem : Life)\n");
                }
                break;
            case Attack:
                switch(stageID){
                    case 0:
                        GetSword(cnum);
                        break;
                    case 1:
                        GetScythe(cnum);
                        break;
                    case 2:
                        GetBat(cnum);
                        break;
                    default:
                        fprintf(stderr, "system error : Unknown stageID. (GetItem : Attack)\n");
                }
                break;
            case etc:
                switch(stageID){
                    case 0:
                        GetPlane(cnum);
                        break;
                    case 1:
                        GetBalance(cnum);
                        break;
                    case 2:
                        GetBike(cnum);
                        break;
                    default:
                        fprintf(stderr, "system error : Unknown stageID. (GetItem : etc)\n");
                }
                break;
            default:
                fprintf(stderr, "system error : Unknown ItemType. (GetItem)\n");
        }
    }
}

/******************************************************************
関数名  : GetGoldApple
機能    : 金のりんご（回復アイテム）の効果：
            200回復
            スキルターン増加
引数    : int cnum   : キャラ番号
出力    : なし
******************************************************************/
static void GetGoldApple(int cnum)
{
    /* 体力回復 */
    gChara[cnum].stts.life += 200;
    if(gChara[cnum].stts.life > gChara[cnum].basestts.life){
        gChara[cnum].stts.life = gChara[cnum].basestts.life;
    }
    /* スキルターン加速 */
    IncreaseSkillTurn(cnum);
    SendLifeCommand(cnum, gChara[cnum].stts.life);
    SendGetCommand(cnum, Life);
}

/******************************************************************
関数名  : GetPoison
機能    : 毒薬（ダメージアイテム）の効果：
            取得時30ダメージ
            移動させた時30ダメージ
引数    : int cnum   : キャラ番号
出力    : なし
******************************************************************/
static void GetPoison(int cnum)
{
    /* 毒状態5ターン追加 */
    Poison_turn[cnum] += 5;
    /* 毒ダメージ処理 */
    PoisonCondition(cnum);
    SendGetCommand(cnum, Life);
}

/******************************************************************
関数名  : GetBansouko
機能    : 絆創膏（回復アイテム）の効果：
            100回復
引数    : int cnum   : キャラ番号
出力    : なし
******************************************************************/
static void GetBansouko(int cnum)
{
    /* 体力回復 */
    gChara[cnum].stts.life += 100;
    if(gChara[cnum].stts.life > gChara[cnum].basestts.life){
        gChara[cnum].stts.life = gChara[cnum].basestts.life;
    }
    SendLifeCommand(cnum, gChara[cnum].stts.life);
    SendGetCommand(cnum, Life);
}

/******************************************************************
関数名  : GetSword
機能    : 剣（攻撃アイテム）の効果：
            攻撃50アップ
            敵に当たる度，自スキルターン加速
引数    : int cnum   : キャラ番号
出力    : なし
******************************************************************/
static void GetSword(int cnum)
{
    /* 攻撃アップ */
    if(AtkUp_turn[cnum] == 0){
        gChara[cnum].stts.atk += 50;
    }
    AtkUp_turn[cnum]++;
    Sword_flag[cnum] = 1;
    SendGetCommand(cnum, Attack);
}

/******************************************************************
関数名  : GetScythe
機能    : 死神の鎌（攻撃アイテム）の効果：
            攻撃50アップ
            衝突した敵のスキルターン延長
引数    : int cnum   : キャラ番号
出力    : なし
******************************************************************/
static void GetScythe(int cnum)
{
    /* 攻撃アップ */
    if(AtkUp_turn[cnum] == 0){
        gChara[cnum].stts.atk += 50;
    }
    AtkUp_turn[cnum]++;
    Scythe_flag[cnum] = 1;
    SendGetCommand(cnum, Attack);
}

/******************************************************************
関数名  : GetBat
機能    : バット（攻撃アイテム）の効果：
            攻撃20アップ
引数    : int cnum   : キャラ番号
出力    : なし
******************************************************************/
static void GetBat(int cnum)
{
    /* 攻撃アップ */
    if(AtkUp_turn[cnum] == 0){
        gChara[cnum].stts.atk += 20;
    }
    AtkUp_turn[cnum]++;
    SendGetCommand(cnum, Attack);
}

/******************************************************************
関数名  : GetPlane
機能    : 飛行機（加速アイテム）の効果：
            速度200アップ
            減速率50ダウン
引数    : int cnum   : キャラ番号
出力    : なし
******************************************************************/
static void GetPlane(int cnum)
{
    /* 速度アップ */
    gChara[cnum].stts.vel   += 200;
    /* 減速率ダウン */
    gChara[cnum].stts.accl  = -50;
    SendGetCommand(cnum, etc);
}

/******************************************************************
関数名  : GetBalance
機能    : 天秤（平均化アイテム）の効果：
            操作キャラ合計体力が多い方が取った場合
            操作キャラ全体70ダメージ
            操作キャラ合計体力が少ない方が取った場合
            操作キャラ全体70回復
            同じ場合全体50ダメージ
引数    : int cnum   : キャラ番号
出力    : なし
******************************************************************/
static void GetBalance(int cnum)
{
    /* 合計体力計算 */
    int TotalLife0 = 0,
        TotalLife1 = 0;
    for(i = 0; i < 4; i++){
        if(gChara[i].aod == CS_Alive){
            TotalLife0 += gChara[i].stts.life;
        }
    }
    for(i = 4; i < CHARANUM; i++){
        if(gChara[i].aod == CS_Alive){
            TotalLife1 += gChara[i].stts.life;
        }
    }
    
    /* クライアント 0 操作キャラの合計体力が多い場合 */
    if(TotalLife0 > TotalLife1){
        /* クライアント 0 操作キャラが取った場合 */
        if(cnum >= 0 && cnum < 4){
            for(i = 0; i < 4; i++){
                if(gChara[i].aod == CS_Alive){
                    gChara[i].stts.life -= 70;   // 操作キャラ全体 70 ダメージ
                    if(gChara[i].stts.life <= 0){
                        gChara[i].stts.life = 0;
                        gChara[i].aod       = CS_Death;
                        SendDeathCommand(i);
                        JudgeResult();
                    }
                    SendLifeCommand(i, gChara[i].stts.life);
                }
            }
        /* クライアント 1 操作キャラが取った場合 */
        }else if(cnum >= 4 && cnum < CHARANUM){
            for(i = 4; i < CHARANUM; i++){
                if(gChara[i].aod == CS_Alive){
                    gChara[i].stts.life += 70;   // 操作キャラ全体 70 回復
                    if(gChara[i].stts.life > gChara[i].basestts.life){
                        gChara[i].stts.life = gChara[i].basestts.life;
                    }
                    SendLifeCommand(i, gChara[i].stts.life);
                }
            }
        }
    /* クライアント 1 操作キャラの合計体力が多い場合 */
    }else if(TotalLife0 < TotalLife1){
        /* クライアント 0 操作キャラが取った場合 */
        if(cnum >= 0 && cnum < 4){
            for(i = 0; i < 4; i++){
                if(gChara[i].aod == CS_Alive){
                    gChara[i].stts.life += 70;   // 操作キャラ全体 70 回復
                    if(gChara[i].stts.life > gChara[i].basestts.life){
                        gChara[i].stts.life = gChara[i].basestts.life;
                    }
                    SendLifeCommand(i, gChara[i].stts.life);
                }
            }
        /* クライアント 1 操作キャラが取った場合 */
        }else if(cnum >= 4 && cnum < CHARANUM){
            for(i = 4; i < CHARANUM; i++){
                if(gChara[i].aod == CS_Alive){
                    gChara[i].stts.life -= 70;   // 操作キャラ全体 70 ダメージ
                    if(gChara[i].stts.life <= 0){
                        gChara[i].stts.life = 0;
                        gChara[i].aod       = CS_Death;
                        SendDeathCommand(i);
                        JudgeResult();
                    }
                    SendLifeCommand(i, gChara[i].stts.life);
                }
            }
        }
    /* 同じ場合 */
    }else{
        for(i = 0; i < CHARANUM; i++){
            if(gChara[i].aod == CS_Alive){
                gChara[i].stts.life -= 50;   // 全体 50 ダメージ
                if(gChara[i].stts.life <= 0){
                    gChara[i].stts.life = 0;
                    gChara[i].aod       = CS_Death;
                    SendDeathCommand(i);
                    JudgeResult();
                }
                SendLifeCommand(i, gChara[i].stts.life);
            }
        }
    }
    SendGetCommand(cnum, etc);
}

/******************************************************************
関数名  : GetBike
機能    : 自転車（加速アイテム）の効果：
            速度200アップ
引数    : int cnum   : キャラ番号
出力    : なし
******************************************************************/
static void GetBike(int cnum)
{
    gChara[cnum].stts.vel += 200;
    SendGetCommand(cnum, etc);
}

/******************************************************************
関数名  : MakeSkyEvent
機能    : 天界のイベント不利な方のHP,ATK増加
引数    : なし
出力    : なし
******************************************************************/
static void MakeSkyEvent(void)
{
    int count_num_0         = 0;        // 残存キャラ数
    int count_num_1         = 0;
    float count_percent_0   = 0;        // 残存HP割合の和
    float count_percent_1   = 0;
    int count_hp_0          = 0;        // 残存HPの和
    int count_hp_1          = 0;
    int plus_flag[8]        = { 0 };    // 被バフフラグ

    /* 残存キャラ数を合計 */
    for(i = 0; i < 4; i++){
        if(gChara[i].aod == CS_Alive){
            count_num_0++;
        }
    }
    for(i = 4; i < CHARANUM; i++){
        if(gChara[i].aod == CS_Alive){
            count_num_1++;
        }
    }

    /* 少ない方にバフ */
    if(count_num_0 < count_num_1){
        /* 操作キャラ全員のHP回復, 攻撃力増加 */
        for(i = 0; i < 4; i++){
            if(gChara[i].aod == CS_Alive){
                gChara[i].stts.life += 100;
                if(gChara[i].stts.life > gChara[i].basestts.life){
                    gChara[i].stts.life = gChara[i].basestts.life;
                }
                SendLifeCommand(i, gChara[i].stts.life);
                gChara[i].basestts.atk  += 15;
                gChara[i].stts.atk      += 15;
                plus_flag[i]            = 1;
            }
        }
    }else if(count_num_0 > count_num_1){
        /* 操作キャラ全員のHP回復, 攻撃力増加 */
        for(i = 4; i < CHARANUM; i++){
            if(gChara[i].aod == CS_Alive){
                gChara[i].stts.life += 100;
                if(gChara[i].stts.life > gChara[i].basestts.life){
                    gChara[i].stts.life = gChara[i].basestts.life;
                }
                SendLifeCommand(i, gChara[i].stts.life);
                gChara[i].basestts.atk  += 15;
                gChara[i].stts.atk      += 15;
                plus_flag[i]            = 1;
            }
        }
    }else{
        /* 残存HP割合を合計 */
        for(i = 0; i < 4; i++){
            if(gChara[i].aod == CS_Alive){
                count_percent_0 += gChara[i].stts.life * 100 / gChara[i].basestts.life;
            }
        }
        for(i = 4; i < CHARANUM; i++){
            if(gChara[i].aod == CS_Alive){
                count_percent_1 += gChara[i].stts.life * 100 / gChara[i].basestts.life;
            }
        }
        /* 少ない方にバフ */
        if(count_percent_0 < count_percent_1){
            /* 操作キャラ全員のHP回復, 攻撃力増加 */
            for(i = 0; i < 4; i++){
                if(gChara[i].aod == CS_Alive){
                    gChara[i].stts.life += 100;
                    if(gChara[i].stts.life > gChara[i].basestts.life){
                        gChara[i].stts.life = gChara[i].basestts.life;
                    }
                    SendLifeCommand(i, gChara[i].stts.life);
                    gChara[i].basestts.atk  += 15;
                    gChara[i].stts.atk      += 15;
                    plus_flag[i]            = 1;
                }
            }
        }else if(count_percent_0 > count_percent_1){
            /* 操作キャラ全員のHP回復, 攻撃力増加 */
            for(i = 4; i < CHARANUM; i++){
                if(gChara[i].aod == CS_Alive){
                    gChara[i].stts.life += 100;
                    if(gChara[i].stts.life > gChara[i].basestts.life){
                        gChara[i].stts.life = gChara[i].basestts.life;
                    }
                    SendLifeCommand(i, gChara[i].stts.life);
                    gChara[i].basestts.atk  += 15;
                    gChara[i].stts.atk      += 15;
                    plus_flag[i]            = 1;
                }
            }
        }else{
            /* 残存HPを合計 */
            for(i = 0; i < 4; i++){
                if(gChara[i].aod == CS_Alive){
                    count_hp_0 += gChara[i].stts.life;
                }
            }
            for(i = 4; i < CHARANUM; i++){
                if(gChara[i].aod == CS_Alive){
                    count_hp_1 += gChara[i].stts.life;
                }
            }
            /* 少ない方にバフ */
            if(count_hp_0 < count_hp_1){
                /* 操作キャラ全員のHP回復, 攻撃力増加 */
                for(i = 0; i < 4; i++){
                    if(gChara[i].aod == CS_Alive){
                        gChara[i].stts.life += 100;
                        if(gChara[i].stts.life > gChara[i].basestts.life){
                            gChara[i].stts.life = gChara[i].basestts.life;
                        }
                        SendLifeCommand(i, gChara[i].stts.life);
                        gChara[i].basestts.atk  += 15;
                        gChara[i].stts.atk      += 15;
                        plus_flag[i]            = 1;
                    }
                }
            }else if(count_hp_0 > count_hp_1){
                /* 操作キャラ全員のHP回復, 攻撃力増加 */
                for(i = 4; i < CHARANUM; i++){
                    if(gChara[i].aod == CS_Alive){
                        gChara[i].stts.life += 100;
                        if(gChara[i].stts.life > gChara[i].basestts.life){
                            gChara[i].stts.life = gChara[i].basestts.life;
                        }
                        SendLifeCommand(i, gChara[i].stts.life);
                        gChara[i].basestts.atk  += 15;
                        gChara[i].stts.atk      += 15;
                        plus_flag[i]            = 1;
                    }
                }
            /* 全て等しい場合 */
            }else{
                /* 全キャラのHP回復, 攻撃力増加 */
                for(i = 0; i < CHARANUM; i++){
                    if(gChara[i].aod == CS_Alive){
                        gChara[i].stts.life += 100;
                        if(gChara[i].stts.life > gChara[i].basestts.life){
                            gChara[i].stts.life = gChara[i].basestts.life;
                        }
                        SendLifeCommand(i, gChara[i].stts.life);
                        gChara[i].basestts.atk  += 15;
                        gChara[i].stts.atk      += 15;
                        plus_flag[i]            = 1;
                    }
                }
            }
        }
    }
    SendEventCommand(plus_flag);
}

/******************************************************************
関数名  : MakeHellEvent
機能    : 冥界のイベント有利な方のHP,ATK減少
引数    : なし
出力    : なし
******************************************************************/
static void MakeHellEvent(void)
{
    int count_num_0         = 0;        // 残存キャラ数
    int count_num_1         = 0;
    float count_percent_0   = 0;        // 残存HP割合の和
    float count_percent_1   = 0;
    int count_hp_0          = 0;        // 残存HPの和
    int count_hp_1          = 0;
    int minus_flag[8]       = { 0 };    // 被デバフフラグ
    
    /* 残存キャラ数を合計 */
    for(i = 0; i < 4; i++){
        if(gChara[i].aod == CS_Alive){
            count_num_0++;
        }
    }
    for(i = 4; i < CHARANUM; i++){
        if(gChara[i].aod == CS_Alive){
            count_num_1++;
        }
    }
    /* 多い方にデバフ */
    if(count_num_0 > count_num_1){
        /* 操作キャラ全員にダメージ，攻撃力減少 */
        for(i = 0; i < 4; i++){
            if(gChara[i].aod == CS_Alive){
                gChara[i].stts.life -= 30;
                if(gChara[i].stts.life <= 0){
                    gChara[i].stts.life = 0;
                    gChara[i].aod = CS_Death;
                    SendDeathCommand(i);
                    JudgeResult();
                }            
                SendLifeCommand(i, gChara[i].stts.life);
                gChara[i].basestts.atk  -= 15;
                gChara[i].stts.atk      -= 15;
                minus_flag[i]           = 1;
            }
        }    
    }else if(count_num_0 < count_num_1){
        /* 操作キャラ全員にダメージ，攻撃力減少 */
        for(i = 4; i < CHARANUM; i++){
            if(gChara[i].aod == CS_Alive){
                gChara[i].stts.life -= 30;
                if(gChara[i].stts.life <= 0){
                    gChara[i].stts.life = 0;
                    gChara[i].aod = CS_Death;
                    SendDeathCommand(i);
                    JudgeResult();
                }
                SendLifeCommand(i, gChara[i].stts.life);
                gChara[i].basestts.atk  -= 15;
                gChara[i].stts.atk      -= 15;
                minus_flag[i]           = 1;
            }
        }
    }else{
        /* 残存HP割合を合計 */
        for(i = 0; i < 4; i++){
            if(gChara[i].aod == CS_Alive){
                count_percent_0 += gChara[i].stts.life * 100 / gChara[i].basestts.life;
            }
        }
        for(i = 4; i < CHARANUM; i++){
            if(gChara[i].aod == CS_Alive){
                count_percent_1 += gChara[i].stts.life * 100 / gChara[i].basestts.life;
            }
        }
        /* 多い方にデバフ */
        if(count_percent_0 > count_percent_1){
            /* 操作キャラ全員にダメージ，攻撃力減少 */
            for(i = 0; i < 4; i++){
                if(gChara[i].aod == CS_Alive){
                    gChara[i].stts.life -= 30;
                    if(gChara[i].stts.life <= 0){
                        gChara[i].stts.life = 0;
                        gChara[i].aod = CS_Death;
                        SendDeathCommand(i);
                        JudgeResult();
                    }
                    SendLifeCommand(i, gChara[i].stts.life);
                    gChara[i].basestts.atk  -= 15;
                    gChara[i].stts.atk      -= 15;
                    minus_flag[i]           = 1;
                }
            }
        }else if(count_percent_0 < count_percent_1){
            /* 操作キャラ全員にダメージ，攻撃力減少 */
            for(i = 4; i < CHARANUM; i++){
                if(gChara[i].aod == CS_Alive){
                    gChara[i].stts.life -= 30;
                    if(gChara[i].stts.life <= 0){
                        gChara[i].stts.life = 0;
                        gChara[i].aod = CS_Death;
                        SendDeathCommand(i);
                        JudgeResult();
                    }
                    SendLifeCommand(i, gChara[i].stts.life);
                    gChara[i].basestts.atk  -= 15;
                    gChara[i].stts.atk      -= 15;
                    minus_flag[i]           = 1;
                }
            }
        }else{
            /* 残存HPを合計 */
            for(i = 0; i < 4; i++){
                if(gChara[i].aod == CS_Alive){
                    count_hp_0 += gChara[i].stts.life;
                }
            }
            for(i = 4; i < CHARANUM; i++){
                if(gChara[i].aod == CS_Alive){
                    count_hp_1 += gChara[i].stts.life;
                }
            }
            /* 多い方にデバフ */
            if(count_hp_0 > count_hp_1){
                /* 操作キャラ全員にダメージ，攻撃力減少 */
                for(i = 0; i < 4; i++){
                    if(gChara[i].aod == CS_Alive){
                        gChara[i].stts.life -= 30;
                        if(gChara[i].stts.life <= 0){
                            gChara[i].stts.life = 0;
                            gChara[i].aod = CS_Death;
                            SendDeathCommand(i);
                            JudgeResult();
                        }
                        SendLifeCommand(i, gChara[i].stts.life);
                        gChara[i].basestts.atk  -= 15;
                        gChara[i].stts.atk      -= 15;
                        minus_flag[i]           = 1;
                    }
                }
            }else if(count_hp_0 < count_hp_1){
                /* 操作キャラ全員にダメージ，攻撃力減少 */
                for(i = 4; i < CHARANUM; i++){
                    if(gChara[i].aod == CS_Alive){
                        gChara[i].stts.life -= 30;
                        if(gChara[i].stts.life <= 0){
                            gChara[i].stts.life = 0;
                            gChara[i].aod = CS_Death;
                            SendDeathCommand(i);
                            JudgeResult();
                        }
                        SendLifeCommand(i, gChara[i].stts.life);
                        gChara[i].basestts.atk  -= 15;
                        gChara[i].stts.atk      -= 15;
                        minus_flag[i]           = 1;
                    }
                }
            /* 全て等しい場合 */
            }else{
                /* 全キャラにダメージ，攻撃力減少 */
                for(i = 0; i < CHARANUM; i++){
                    if(gChara[i].aod == CS_Alive){
                        gChara[i].stts.life -= 30;
                        if(gChara[i].stts.life <= 0){
                            gChara[i].stts.life = 0;
                            gChara[i].aod = CS_Death;
                            SendDeathCommand(i);
                            JudgeResult();
                        }
                        SendLifeCommand(i, gChara[i].stts.life);
                        gChara[i].basestts.atk  -= 15;
                        gChara[i].stts.atk      -= 15;
                        minus_flag[i]           = 1;
                    }
                }
            }
        }
    }
    SendEventCommand(minus_flag);
}

/******************************************************************
関数名  : JudgeResult
機能    : 勝利敗北を判定する
引数    : なし
出力    : なし
******************************************************************/
static void JudgeResult(void)
{
    /* 全員倒された場合 */
    if((gChara[0].aod == CS_Death) && (gChara[1].aod == CS_Death) && (gChara[2].aod == CS_Death) && (gChara[3].aod == CS_Death)
    && (gChara[4].aod == CS_Death) && (gChara[5].aod == CS_Death) && (gChara[6].aod == CS_Death) && (gChara[7].aod == CS_Death)){
        SendResultCommand(ALL_CLIENTS, Draw);
    /* クライアント 0 操作キャラが全員倒された場合 */
    }else if(gChara[0].aod == CS_Death && gChara[1].aod == CS_Death && gChara[2].aod == CS_Death && gChara[3].aod == CS_Death){
        SendResultCommand(0, Lose);
        SendResultCommand(1, Win);
    /* クライアント１操作キャラが全員倒された場合 */
    }else if(gChara[4].aod == CS_Death && gChara[5].aod == CS_Death && gChara[6].aod == CS_Death && gChara[7].aod == CS_Death){
        SendResultCommand(0, Win);
        SendResultCommand(1, Lose);
    }
}