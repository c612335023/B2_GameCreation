/*****************************************************************
ファイル名	: client_system.c
機能		: クライアントのゲームシステム処理
*****************************************************************/

#include "client_func.h"

#include <SDL2/SDL_mixer.h>

/***********************************
                変数                
***********************************/

ClientCharaInfo *gChara;            // 操作キャラ情報読込用
ClientCharaInfo *aChara;            // 全キャラ情報読込用
ItemIMG         *gItem;             // ゲームに使用する構造体

int beforeLife[CHARANUM];           // 各キャラの以前のライフゲージ

/* static */
static StageImgInfo *StageIMG;      // 全ステージ情報読込用
static ItemIMG      *aItem;         // 全てのアイテム情報入れる

static Mix_Chunk *chunk_select;     // 選択音
static Mix_Chunk *chunk_decision;   // 決定音
static Mix_Chunk *chunk_cancel;     // 取消音
static Mix_Chunk *chunk_shoot;      // 発射音
static Mix_Chunk *chunk_bound;      // 衝突音
static Mix_Chunk *chunk_damage;     // 被攻撃音
static Mix_Chunk *chunk_heal;       // 回復音
static Mix_Chunk *chunk_poison;     // 被毒音
static Mix_Chunk *chunk_kill;       // 死亡音

/***********************************
                関数                
***********************************/

static void Chomp(char *str);

/*****************************************************************
関数名  : InitSystem
機能    : ゲームシステム初期化
引数    : int pos   : クライアントID
返値    : 正常終了  0
          エラー    負数
*****************************************************************/
int InitSystem(int pos)
{
    /* 領域確保 */
    gChara = (ClientCharaInfo *)malloc(sizeof(ClientCharaInfo) * CHARANUM);
    if(gChara == NULL){
        fprintf(stderr, "system error : gChara malloc failed. (InitSystem)\n");
        exit (-1);
    }

    /* キャラの基本設定 */
    int i;
    for(i = 0; i < 4; i++){
        gChara[i].point.x   = ((WINDOWWIDE - STAGEWIDE) / 2) + 180 + 540 * pos + (180 - 360 * pos) * i;
        gChara[i].point.y   = 850 - 800 * pos - (SHIFTCHARA - 2 * SHIFTCHARA * pos);
        gChara[i].velocity  = 0;
        gChara[i].aod       = CS_Alive;
        gChara[i].dst.w     = 100;
        gChara[i].dst.h     = 100;
        gChara[i].src_hp.w  = 100;
        gChara[i].src_hp.h  = 10;
    }
    for(i = 4; i < CHARANUM; i++){
        gChara[i].point.x   = ((WINDOWWIDE - STAGEWIDE) / 2) + 720 - 540 * pos - (180 - 360 * pos) * (i - 4);
        gChara[i].point.y   = 50 + 800 * pos + (SHIFTCHARA - 2 * SHIFTCHARA * pos);
        gChara[i].velocity  = 0;
        gChara[i].aod       = CS_Alive;
        gChara[i].dst.w     = 100;
        gChara[i].dst.h     = 100;
        gChara[i].src_hp.w  = 100;
        gChara[i].src_hp.h  = 10;
    }

    InitChara(pos);

    return 0;
}

/**********************************************************************
関数名  : ReadAllStage
機能    : 全ステージ画像読み込み
引数    : ステージ画像パスデータ
出力    : なし
***********************************************************************/
void ReadAllStage(const char *stage_data_file)
{
    /* 領域確保 */
    StageIMG = (StageImgInfo *)malloc(sizeof(StageImgInfo) * ALL_STAGES);
    if(StageIMG == NULL){
        fprintf(stderr, "system error : StageIMG malloc failed. (ReadAllStage)\n");
        exit (-1);
    }
    
    FILE *fp = fopen(stage_data_file, "r");
    if(fp == NULL){
        fprintf(stderr,"system error : failed to open stage_data_file. (ReadAllStage)\n");
        exit(-1);
    }

    for(int i = 0; i < ALL_STAGES; i++){
        /* ステージの画像のみ読み取り*/
        if(NULL == fgets(StageIMG[i].path, 128, fp)){
            fprintf(stderr, "system error : failed to read the stage path. (ReadAllStage)");
            exit(-1);
        }
        Chomp(StageIMG[i].path);
        StageIMG[i].type = i;
    }
    fclose(fp);
}

/****************************************************************
関数名  : ReadAllChara
機能    : 全てのキャラの情報を読み取る
引数    : const char *chara_data_file : chara.dataのポインタ
出力    : なし
補足    : chara.data.clientはbasiclife, 画像パスを記載
****************************************************************/
void ReadAllChara(const char *chara_data_file)
{
    /* 領域確保 */
    aChara = (ClientCharaInfo *)malloc(sizeof(ClientCharaInfo) * ALL_CHARAS);
    if(aChara == NULL){
        fprintf(stderr, "system error : aChara malloc failed. (ReadAllChara)\n");
        exit (-1);
    }

    /* ファイル読み込み */
    FILE *fp = fopen(chara_data_file, "r");
    if(fp == NULL){
        fprintf(stderr,"system error : failed to open chara_data_file. (ReadAllChara)\n");
        exit(-1);
    }

    for(int i = 0; i < ALL_CHARAS; i++){
        /* basiclife, skilltypeの読み込み */
        if(2 != fscanf(fp, "%d%d", &(aChara[i].basiclife), &(aChara[i].skill.type))){
            fprintf(stderr, "system error : failed to read the chara status. (ReadAllChara)");
            exit(-1);
        }
        fgetc(fp);
        /* キャラの画像のパス読み取り */
        if(NULL == fgets(aChara[i].path, 128, fp)){
            fprintf(stderr, "system error : failed to read the chara path. (ReadAllChara)");
            exit(-1);
        }
        Chomp(aChara[i].path);
    }
    fclose(fp);    
    MakeCNIMG();
}

/****************************************************************
関数名  : ReadAllItem
機能    : 全てのアイテムの情報を読み取る
引数    : Item.dataのポインタ
出力    : なし
補足    : Item.data.はパスのみ記載
****************************************************************/
void ReadAllItem(const char *Item_data_fail)
{
    /* 領域確保 */
    aItem = (ItemIMG *)malloc(sizeof(ItemIMG) * ALL_ITEMS);
    if(aItem == NULL){
        fprintf(stderr, "system error : aItem malloc failed. (ReadAllItem)\n");
        exit (-1);
    }

    FILE *fp = fopen(Item_data_fail, "r");
    if(fp == NULL){
        fprintf(stderr,"system error : failed to open Item_data_file. (ReadAllItem)\n");
        exit(-1);
    }
    
    for(int i = 0; i < ALL_ITEMS; i++){
        if(NULL == fgets(aItem[i].path, 128, fp)){
            fprintf(stderr, "system error : failed to read the item path. (ReadAllItem)\n");
            exit(-1);
        }
        Chomp(aItem[i].path);
    }
    fclose(fp);
}

/*******************************************************************
関数名  : ReadStage
機能    : 選択ステージの画像パスを挿入
引数    : int snum   : 選択ステージ番号
出力    : なし
*******************************************************************/
void ReadStage(int snum)
{
    strncpy(gMap.path, StageIMG[snum].path, strlen(StageIMG[snum].path) + 1);

#ifdef DEBUG_PRINT
    fprintf(stderr,"copy ");
    fprintf(stderr,"%s\n", StageIMG[snum].path);
#endif

    free(StageIMG);
}

/****************************************************************
関数名  : ReadChara
機能    : 使用するキャラの情報を獲得する
引数    : int *CID    : 選んだキャラのID
          int cnum    : キャラ番号
出力    : なし
****************************************************************/
void ReadChara(int *CID, int cnum)
{
    /* 描画に必要なデータをコピー */
    gChara[cnum].basiclife  = aChara[*CID].basiclife;
    gChara[cnum].life       = gChara[cnum].basiclife;
    beforeLife[cnum]        = gChara[cnum].life;
    gChara[cnum].skill.type = aChara[*CID].skill.type;
    gChara[cnum].skill.turn = 0;

    /* キャラ画像パスをコピー */
    strcpy(gChara[cnum].path, aChara[*CID].path);
    /* キャラ番号をコピー */
    gChara[cnum].pos = *CID;
    
    MakeSGIMG(cnum);
}

/****************************************************************
関数名  : ReadItem
機能    : 使用するアイテム画像のパスを獲得する
引数    : int stageID   : 選択されたステージID
出力    : なし
****************************************************************/
void ReadItem(int stageID)
{
    /* 領域確保 */
    gItem = (ItemIMG *)malloc(sizeof(ItemIMG) * ITEMNUM);
    if(gItem == NULL){
        fprintf(stderr, "system error : gItem malloc failed. (ReadItem)\n");
        exit(-1);
    }

    /* アイテム画像パスをコピー */
    for(int i = 0; i < ITEMNUM; i++){
        strcpy(gItem[i].path, aItem[(stageID * ITEMNUM) + i].path);
    }

    free(aItem);
}

/*****************************************************************
関数名	: GetMusic
機能	: 効果音読み込み
引数	: なし
出力	: なし
*****************************************************************/
void GetMusic(void)
{
    /* 初期化 */
    SDL_Init(SDL_INIT_AUDIO);
    Mix_Init(0);
    if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024) < 0){
        fprintf(stderr, "system error : Mix_OpenAudio failed. (GetMusic)\n");
        SDL_Quit();
        exit(-1);
    }

    /* 効果音の読み込み */
    if((chunk_select = Mix_LoadWAV("music/select.wav")) == NULL){
        fprintf(stderr, "system error : failed to load chunk_select. (GetMusic)\n");
        Mix_CloseAudio(); // オーディオデバイスの終了
        SDL_Quit();
        exit(-1);
    }
    if((chunk_decision = Mix_LoadWAV("music/decision.wav")) == NULL){
        fprintf(stderr, "system error : failed to load chunk_decision. (GetMusic)\n");
        Mix_CloseAudio(); // オーディオデバイスの終了
        SDL_Quit();
        exit(-1);
    }
    if((chunk_cancel = Mix_LoadWAV("music/cancel.wav")) == NULL){
        fprintf(stderr, "system error : failed to load chunk_cancel. (GetMusic)\n");
        Mix_CloseAudio(); // オーディオデバイスの終了
        SDL_Quit();
        exit(-1);
    }
    if((chunk_shoot = Mix_LoadWAV("music/shooting.wav")) == NULL){
        fprintf(stderr, "system error : failed to load chunk_shoot. (GetMusic)\n");
        Mix_CloseAudio(); // オーディオデバイスの終了
        SDL_Quit();
        exit(-1);
    }
    if((chunk_bound = Mix_LoadWAV("music/bound.wav")) == NULL){
        fprintf(stderr, "system error : failed to load chunk_bound. (GetMusic)\n");
        Mix_CloseAudio(); // オーディオデバイスの終了
        SDL_Quit();
        exit(-1);
    }
    if((chunk_damage = Mix_LoadWAV("music/damage.wav")) == NULL){
        fprintf(stderr, "system error : failed to load chunk_damage. (GetMusic)\n");
        Mix_CloseAudio(); // オーディオデバイスの終了
        SDL_Quit();
        exit(-1);
    }
    if((chunk_heal = Mix_LoadWAV("music/heal.wav")) == NULL){
        fprintf(stderr, "system error : failed to load chunk_heal. (GetMusic)\n");
        Mix_CloseAudio(); // オーディオデバイスの終了
        SDL_Quit();
        exit(-1);
    }
    if((chunk_poison = Mix_LoadWAV("music/poison.wav")) == NULL){
        fprintf(stderr, "system error : failed to load chunk_poison. (GetMusic)\n");
        Mix_CloseAudio(); // オーディオデバイスの終了
        SDL_Quit();
        exit(-1);
    }
    if((chunk_kill = Mix_LoadWAV("music/kill.wav")) == NULL){
        fprintf(stderr, "system error : failed to load chunk_kill. (GetMusic)\n");
        Mix_CloseAudio(); // オーディオデバイスの終了
        SDL_Quit();
        exit(-1);
    }
}

/******************************************************************************
関数名      : SoundEffect
機能        : 引数に応じた効果音を鳴らす
引数        : int act   : 行動
出力        : なし
*******************************************************************************/
void SoundEffect(int act)
{
    switch(act){
        case Select:
            Mix_PlayChannel(-1, chunk_select, 0);
            break;
        case Decision:
            Mix_PlayChannel(-1, chunk_decision, 0);
            break;
        case Cancel:
            Mix_PlayChannel(-1, chunk_cancel, 0);
            break;
        case Shoot:
            Mix_PlayChannel(-1, chunk_shoot, 0);
            break;
        case Bound:
            Mix_PlayChannel(-1, chunk_bound, 0);
            break;
        case Damage:
            Mix_PlayChannel(-1, chunk_damage, 0);
            break;
        case Heal:
            Mix_PlayChannel(-1, chunk_heal, 0);
            break;
        case Poison:
            Mix_PlayChannel(-1, chunk_poison, 0);
            break;
        case Kill:
            Mix_PlayChannel(-1, chunk_kill, 0);
            break;
        default:
            fprintf(stderr, "system error : SoundEffect failed.");
    }
}

/******************************************************************************
関数名      : CloseMusic
機能        : 効果音関連開放
引数        : なし
出力        : なし
*******************************************************************************/
void CloseMusic(void)
{
    Mix_FreeChunk(chunk_select);
    Mix_FreeChunk(chunk_decision);
    Mix_FreeChunk(chunk_cancel);
    Mix_FreeChunk(chunk_shoot);
    Mix_FreeChunk(chunk_bound);
    Mix_FreeChunk(chunk_damage);
    Mix_FreeChunk(chunk_heal);
    Mix_FreeChunk(chunk_poison);
    Mix_FreeChunk(chunk_kill);

    Mix_CloseAudio();
    Mix_Quit();
}

 /***************
 *   static    *
***************/
/****************************************************************
関数名  : Chomp
機能    : 文字列の改行部分をなくす
引数    : char *str   : 文字列
出力    : なし
****************************************************************/
static void Chomp(char *str)
{
    int i;
    for(i = 0; i < 128; i++){
        if(str[i] == '\0'){
            break;
        }
    }
    if(i > 0 && str[i - 1] == '\n'){
        str[i - 1] = '\0';
    }
}