/*****************************************************************
ファイル名	: client_win.c
機能		: クライアントのユーザーインターフェース処理
*****************************************************************/

#include "client_func.h"

#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL2_gfxPrimitives.h>

/***********************************
                変数                
***********************************/

MapInfo     gMap;
ItemInfo    Item;

/* 入力変数 */
int     input_home;
int     input_up;
int     input_right;
int     input_left;
int     input_down;
int     input_one;
int     input_two;

int     angle                   = 0;        // 発射角度

float   cooltime[CHARANUM]      = { 0 };    // クールタイムカウント
int     atkFlag[CHARANUM]       = { 0 };    // 攻撃アップ描画フラグ
int     poisonFlag[CHARANUM]    = { 0 };    // 毒状態描画フラグ
int     cutinFlag               = 0;        // イベントカットインフラグ

int     pasttime                = 0;        // 経過時間

/* static */
static SDL_Window       *gMainWindow;
static SDL_Renderer     *gMainRenderer;
static SDL_Texture      *NSTex, *ItemTex;
static GeneralTexInfo   cName;
static ArrowIMG         Arrow;
static GeneralTexInfo   Title, ToBF, Quit;
static GeneralTexInfo   DamageEffect, HealEffect;
static GeneralTexInfo   Vector_l, Vector_r, Chara_sel, Ready, Chara_all, Chara_mini;
static SDL_Rect         ns_src, item_src;
static SDL_Rect         ns_dst, item_dst;
static SDL_Rect         dst_attack[CHARANUM], dst_poison[CHARANUM], dst_event[CHARANUM];

static GeneralTexInfo   Event;
static GeneralTexInfo   Event_chara;

static int scnum;                               // 選択キャラの番号
static int iscnum;                              // 初期選択キャラ番号

static int GorQ         = 0;                    // 戦場へ行くか否か
static int dt_frame     = 380;                  // タイトル選択枠のx座標
static int dt_frame_b;                          // 前のタイトル選択枠のx座標

static int stage        = 0;                    // ステージID
static int ss_frame     = 130;                  // ステージ選択枠のx座標
static int ss_frame_b;                          // 前のステージ選択枠のx座標

static int win_num      = 0;                    // 表示中のウィンドウ番号
static int readyFlag   = 0;                    // 準備完了フラグ
static int num_sc       = 0;                    // 現在の選択キャラ数
static int CID[4]       = { -1, -1, -1, -1 };   // サーバに送るためのキャラ情報を格納する配列

static int i;                                   // for文用

static float argf;                              // 扇形描画の角度

static float time_arg   = 0;                    // 経過時間の角度

static int HPbar[CHARANUM];                     // 現在のライフゲージの長さ

static int de_count[CHARANUM] = { 0 };          // ダメージエフェクト表示時間
static int he_count[CHARANUM] = { 0 };          // ヒールエフェクト表示時間

/***********************************
                関数                
***********************************/

static void DrawCharaName(int pos);
static void DrawSelect(int pos, int cnum);
static void DrawArrow(int cnum);
static void DrawChara(int pos, int cnum);
static void DrawLife(int pos, int cnum);
static void DrawCooltime(int cnum);
static void DrawMap();
static void DrawDamageEffect(int cnum);
static void DrawHealEffect(int cnum);
static void DrawSkillGauge(int pos);
static void DrawAttack(int pos, int cnum);
static void DrawPoison(int pos, int cnum);
static void DrawItem();
static void DrawTime();
static void DrawDraw();
static void DrawLose();
static void DrawWin();
static void DrawEvent(int pos, int cnum);

/*****************************************************************
関数名	: InitWindows
機能	: メインウインドウの表示，設定を行う
引数	: int   num		    : 全クライアント数
出力	: 正常に設定できたとき0，失敗したとき-1
*****************************************************************/
int InitWindows(int num)
{
	char title[10];

    /* 引き数チェック */
    assert(0 < num && num <= CLIENTNUM);
	
	/** 初期化 **/
    /* SDL */
	if(SDL_Init(SDL_INIT_VIDEO) < 0){
		printf("failed to initialize SDL.\n");
		exit(-1);
	}
    /* TTF */
    if(TTF_Init() < 0){
		printf("failed to initialize TTF.\n");
		exit(-1);
	}
	
	/* メインウインドウ作成 */
	if((gMainWindow = SDL_CreateWindow("My Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOWWIDE, WINDOWHEIGHT, 0)) == NULL){
		printf("failed to initialize videomode.\n");
		exit(-1);
	}

	gMainRenderer = SDL_CreateRenderer(gMainWindow, -1, SDL_RENDERER_SOFTWARE);

	/* ウインドウのタイトルをセット */
	sprintf(title, "God_Impact.exe");
	SDL_SetWindowTitle(gMainWindow, title);
	
	/* 背景を白にする */
	SDL_SetRenderDrawColor(gMainRenderer, 255, 255, 255, 255);
  	SDL_RenderClear(gMainRenderer);
    SDL_RenderPresent(gMainRenderer);
	
	return 0;
}

/**********************************************************************
関数名  : InitChara
機能    : クライアントの選択キャラ番号初期化
引数	: int pos   : クライアントID
出力	: なし
***********************************************************************/
void InitChara(int pos)
{
    scnum   = 4 * pos;
    iscnum  = 4 * pos;
}

/***********************************************************************
関数名	: MakeTitleIMG
機能	: タイトル画面の描画に必要なデータを作成する
引数	: なし
出力	: なし
***********************************************************************/
void MakeTitleIMG()
{
    SDL_SetRenderDrawColor(gMainRenderer, 255, 255, 255, 255); // 白色で画面を塗りつぶす
    SDL_RenderClear(gMainRenderer);

    SDL_Surface *image1 = IMG_Load("win/title.png");    // タイトル画像
    SDL_Surface *image2 = IMG_Load("win/ok.png");       // 「戦場へ」ボタン
    SDL_Surface *image3 = IMG_Load("win/no.png");       // 「やめる」ボタン
    SDL_Surface *image4 = IMG_Load("win/back.png");     // 背景
    if(!(image1 && image2 && image3 && image4)){        // いずれかの画像読み込みに失敗したら
        fprintf(stderr, "win error : MakeTitleIMG failed.\n");
        exit(-1);
	}

    Title.tex               = SDL_CreateTextureFromSurface(gMainRenderer, image1); 
    ToBF.tex                = SDL_CreateTextureFromSurface(gMainRenderer, image2);
    Quit.tex                = SDL_CreateTextureFromSurface(gMainRenderer, image3);
    SDL_Texture *Back_tex   = SDL_CreateTextureFromSurface(gMainRenderer, image4);

    Title.src.x = 0;
    Title.src.y = 0;
    Title.src.w = image1->w;
    Title.src.h = image1->h;

    ToBF.src.x = 0;
    ToBF.src.y = 0;
    ToBF.src.w = image2->w;
    ToBF.src.h = image2->h;

    SDL_Rect Back_src = {0, 0, WINDOWWIDE, WINDOWHEIGHT};

    Title.dst.x = 400;
    Title.dst.y = 50;
    Title.dst.w = 900;
    Title.dst.h = 300;

    ToBF.dst.x = 400;
    ToBF.dst.y = 550;
    ToBF.dst.w = 300;
    ToBF.dst.h = 100;

    Quit.dst.x = 1000;
    Quit.dst.y = 550;
    Quit.dst.w = 300;
    Quit.dst.h = 100;

    SDL_Rect Back_dst = {0, 0, WINDOWWIDE, WINDOWHEIGHT};

    SDL_RenderCopy(gMainRenderer, Title.tex, &Title.src, &Title.dst); 
    SDL_RenderCopy(gMainRenderer, ToBF.tex, &ToBF.src, &ToBF.dst);
    SDL_RenderCopy(gMainRenderer, Quit.tex, &ToBF.src, &Quit.dst);
    SDL_RenderCopy(gMainRenderer, Back_tex, &Back_src, &Back_dst);

    SDL_DestroyTexture(Back_tex);
}

/***********************************************************************
関数名	: MakeSSIMG
機能	: ステージ選択画面の描画に必要なデータを作成する
引数	: なし
出力	: なし
***********************************************************************/
void MakeSSIMG()
{
    SDL_SetRenderDrawColor(gMainRenderer, 255, 255, 255, 255); // 白色で画面を塗りつぶす
    SDL_RenderClear(gMainRenderer);

    SDL_Surface *img_sky    = IMG_Load("win/sky.png");
    SDL_Surface *img_hell   = IMG_Load("win/hell.png");
    SDL_Surface *img_normal = IMG_Load("win/normal.png");
    SDL_Surface *img_ps     = IMG_Load("win/please_select.png");
    SDL_Surface *img_ns     = IMG_Load("win/name_stage.png");
    SDL_Surface *img_item   = IMG_Load("win/item.png");
    if(!(img_sky && img_hell && img_normal && img_ps && img_ns && img_item)){ // いずれかの画像読み込みに失敗したら
        fprintf(stderr, "win error : MakeSSIMG failed.\n");
        exit(-1);
	}

    SDL_Texture *Sky_tex    = SDL_CreateTextureFromSurface(gMainRenderer, img_sky);
    SDL_Texture *Hell_tex   = SDL_CreateTextureFromSurface(gMainRenderer, img_hell);
    SDL_Texture *Normal_tex = SDL_CreateTextureFromSurface(gMainRenderer, img_normal);
    SDL_Texture *PSTex      = SDL_CreateTextureFromSurface(gMainRenderer, img_ps);

    NSTex   = SDL_CreateTextureFromSurface(gMainRenderer, img_ns);
    ItemTex = SDL_CreateTextureFromSurface(gMainRenderer, img_item);

    SDL_QueryTexture(Sky_tex, NULL, NULL, &img_sky->w, &img_sky->h);
    SDL_QueryTexture(Hell_tex, NULL, NULL, &img_hell->w, &img_hell->h);
    SDL_QueryTexture(Normal_tex, NULL, NULL, &img_normal->w, &img_normal->h);
    SDL_QueryTexture(PSTex, NULL, NULL, &img_ps->w, &img_ps->h);
    SDL_QueryTexture(NSTex, NULL, NULL, &img_ns->w, &img_ns->h);
    SDL_QueryTexture(ItemTex, NULL, NULL, &img_item->w, &img_item->h);

    SDL_Rect Sky_src    = {0, 0, img_sky->w, img_sky->h};
    SDL_Rect Hell_src   = {0, 0, img_hell->w, img_hell->h};
    SDL_Rect Normal_src = {0, 0, img_normal->w, img_normal->h};
    SDL_Rect ps_src     = {0, 0, img_ps->w, img_ps->h};

    ns_src.x = 0;
    ns_src.y = 0;
    ns_src.w = 1000;
    ns_src.h = 200;

    item_src.x = 0;
    item_src.y = 0;
    item_src.w = 550;
    item_src.h = 300;

    SDL_Rect Sky_dst    = {150, 140, 400, 400};
    SDL_Rect Hell_dst   = {650, 140, 400, 400};
    SDL_Rect Normal_dst = {1150, 140, 400, 400};
    SDL_Rect ps_dst     = {475, 10, 750, 100};

    ns_dst.x = 50;
    ns_dst.y = 630;
    ns_dst.w = 1000;
    ns_dst.h = 200;

    item_dst.x = 1100;
    item_dst.y = 580;
    item_dst.w = 550;
    item_dst.h = 300;

    SDL_RenderCopy(gMainRenderer, Sky_tex, &Sky_src, &Sky_dst);
    SDL_RenderCopy(gMainRenderer, Hell_tex, &Hell_src, &Hell_dst);
    SDL_RenderCopy(gMainRenderer, Normal_tex, &Normal_src, &Normal_dst);
    SDL_RenderCopy(gMainRenderer, PSTex, &ps_src, &ps_dst);

    SDL_RenderPresent(gMainRenderer); // 描画データを表示

    SDL_FreeSurface(img_sky); // サーフェイス（画像）の解放
    SDL_FreeSurface(img_hell);
    SDL_FreeSurface(img_normal);
    SDL_FreeSurface(img_ps);
    SDL_FreeSurface(img_ns);
    SDL_FreeSurface(img_item);
    SDL_DestroyTexture(Sky_tex);
    SDL_DestroyTexture(Hell_tex);
	SDL_DestroyTexture(Normal_tex);
    SDL_DestroyTexture(PSTex);
}

/****************************************************************
関数名 : MakeStageIMG
機能   : ステージ描画に必要なデータを作成する
引数   : なし
出力   : なし
*****************************************************************/
void MakeStageIMG(void)
{
    SDL_Surface *image = IMG_Load(gMap.path);

    gMap.tex = SDL_CreateTextureFromSurface(gMainRenderer, image);

    SDL_QueryTexture(gMap.tex, NULL, NULL, &image->w, &image->h);

    gMap.src.x = 0;
    gMap.src.y = 0;
    gMap.src.w = image->w;
    gMap.src.h = image->h;

    gMap.dst.x = 0;
    gMap.dst.y = 0;
    gMap.dst.w = WINDOWWIDE;
    gMap.dst.h = WINDOWHEIGHT;

    SDL_FreeSurface(image);
}

/****************************************************************
関数名 : MakeItemIMG
機能   : アイテム描画に必要なデータを作成する
引数   : なし
出力   : なし
*****************************************************************/
void MakeItemIMG(void)
{
    /* アイテム用の描画設定 */
    for(i = 0; i < ITEMNUM; i++){
        SDL_Surface *I = IMG_Load(gItem[i].path);
        if(NULL == I){
            fprintf(stderr, "win error : MakeItemIMG failed.\n");
            exit(-1);
        }
        
        gItem[i].tex = SDL_CreateTextureFromSurface(gMainRenderer, I);

        SDL_QueryTexture(gItem[i].tex, NULL, NULL, &I->w, &I->h);

        gItem[i].src.x = 0;
        gItem[i].src.y = 0;
        gItem[i].src.w = I->w;
        gItem[i].src.h = I->h;

        Item.dst.w = 50;
        Item.dst.h = 50;

        SDL_FreeSurface(I);
    }
    /* 状態表示の描画設定 */
    for(i = 0; i < CHARANUM; i++){
        dst_attack[i].w = 35;
        dst_attack[i].h = 35;
        dst_poison[i].w = 35;
        dst_poison[i].h = 35;
    }
}

/****************************************************************
関数名 : MakeEventIMG
機能   : イベント描画に必要なデータを作成する
引数   : ステージID
出力   : なし
補足   : 追加で画像のロードが必要な場合ここに追記
*****************************************************************/
void MakeEventIMG(int stageID)
{
    SDL_Surface *eve_img;
    SDL_Surface *evecha_img;

    switch(stageID){
        case 0:
            eve_img     = IMG_Load("event/event_sky.png");
            evecha_img  = IMG_Load("event/c_shichihukuzin.png");
            if(!(eve_img && evecha_img)){
                fprintf(stderr, "win error : MakeEventIMG failed.\n");
            }

            Event.tex       = SDL_CreateTextureFromSurface(gMainRenderer, eve_img);
            Event_chara.tex = SDL_CreateTextureFromSurface(gMainRenderer, evecha_img);

            SDL_QueryTexture(Event.tex, NULL, NULL, &eve_img->w, &eve_img->h);
            SDL_QueryTexture(Event_chara.tex, NULL, NULL, &evecha_img->w, &evecha_img->h);

            Event.src.x = 0;
            Event.src.y = 0;
            Event.src.w = eve_img->w;
            Event.src.h = eve_img->h;

            Event_chara.src.x = 0;
            Event_chara.src.y = 0;
            Event_chara.src.w = evecha_img->w;
            Event_chara.src.h = evecha_img->h;

            for(i = 0; i < CHARANUM; i++){
                dst_event[i].w = 50;
                dst_event[i].h = 50;
            }

            SDL_FreeSurface(eve_img);
            SDL_FreeSurface(evecha_img);
            break;
        case 1:
            eve_img     = IMG_Load("event/event_hell.png");
            evecha_img  = IMG_Load("event/c_maou.png");
            if(!(eve_img && evecha_img)){
                fprintf(stderr, "win error : MakeEventIMG failed.\n");
            }

            Event.tex       = SDL_CreateTextureFromSurface(gMainRenderer, eve_img);
            Event_chara.tex = SDL_CreateTextureFromSurface(gMainRenderer, evecha_img);

            SDL_QueryTexture(Event.tex, NULL, NULL, &eve_img->w, &eve_img->h);
            SDL_QueryTexture(Event_chara.tex, NULL, NULL, &evecha_img->w, &evecha_img->h);

            Event.src.x = 0;
            Event.src.y = 0;
            Event.src.w = eve_img->w;
            Event.src.h = eve_img->h;

            Event_chara.src.x = 0;
            Event_chara.src.y = 0;
            Event_chara.src.w = evecha_img->w;
            Event_chara.src.h = evecha_img->h;

            for(i = 0; i < CHARANUM; i++){
                dst_event[i].w = 50;
                dst_event[i].h = 50;
            }

            SDL_FreeSurface(eve_img);
            SDL_FreeSurface(evecha_img);
            break;
        case 2:
            eve_img     = IMG_Load("event/event_normal.png");
            evecha_img  = IMG_Load("event/normal_road.png");
            if(!(eve_img && evecha_img)){
                fprintf(stderr, "win error : MakeEventIMG failed.\n");
            }

            Event.tex       = SDL_CreateTextureFromSurface(gMainRenderer, eve_img);
            Event_chara.tex = SDL_CreateTextureFromSurface(gMainRenderer, evecha_img);

            SDL_QueryTexture(Event.tex, NULL, NULL, &eve_img->w, &eve_img->h);
            SDL_QueryTexture(Event_chara.tex, NULL, NULL, &evecha_img->w, &evecha_img->h);

            Event.src.x = 0;
            Event.src.y = 0;
            Event.src.w = eve_img->w;
            Event.src.h = eve_img->h;

            Event_chara.src.x = 0;
            Event_chara.src.y = 0;
            Event_chara.src.w = evecha_img->w;
            Event_chara.src.h = evecha_img->h;

            Event_chara.dst.x = 400;
            Event_chara.dst.y = 0;
            Event_chara.dst.w = STAGEWIDE;
            Event_chara.dst.h = STAGEHEIGHT;

            SDL_FreeSurface(eve_img);
            SDL_FreeSurface(evecha_img);
            break;
        default:
            fprintf(stderr, "win error : Unknown stageID. (MakeEventIMG)");
    }
}

/***********************************************************************
関数名	: MakeCSIMG
機能	: キャラ選択画面の描画に必要なデータを作成する
引数	: なし
出力	: なし
***********************************************************************/
void MakeCSIMG(void)
{
    SDL_SetRenderDrawColor(gMainRenderer, 255, 255, 255, 255); // 白色で画面を塗りつぶす
    SDL_RenderClear(gMainRenderer);

    /* 画像描画処理 */
    SDL_Surface *image1 = IMG_Load("win/vector_l.png");
    SDL_Surface *image2 = IMG_Load("win/vector_r.png");
    SDL_Surface *image3 = IMG_Load("win/select.png");
    SDL_Surface *image4 = IMG_Load("win/ready.png");
    SDL_Surface *image5 = IMG_Load("win/chara_all.png");
    SDL_Surface *image6 = IMG_Load("win/chara_mini.png");
    /* いずれかの画像読み込みに失敗したら */
    if(!(image1 && image2 && image3 && image4 && image5 && image6)){ 
        fprintf(stderr, "win error : MakeCSIMG failed.\n");
        exit(-1);
    }

    /* 読み込んだ画像からテクスチャを作成 */
    SDL_Texture *lVector_tex    = SDL_CreateTextureFromSurface(gMainRenderer, image1);
    SDL_Texture *rVector_tex    = SDL_CreateTextureFromSurface(gMainRenderer, image2);

    Chara_sel.tex   = SDL_CreateTextureFromSurface(gMainRenderer, image3);
    Ready.tex       = SDL_CreateTextureFromSurface(gMainRenderer, image4);
    Chara_all.tex   = SDL_CreateTextureFromSurface(gMainRenderer, image5);
    Chara_mini.tex  = SDL_CreateTextureFromSurface(gMainRenderer, image6);

    /* 画像（テクスチャ）の情報（サイズなど）を取得 */
    SDL_QueryTexture(lVector_tex, NULL, NULL, &image1->w, &image1->h); 
    SDL_QueryTexture(rVector_tex, NULL, NULL, &image2->w, &image2->h);
    SDL_QueryTexture(Chara_sel.tex, NULL, NULL, &image3->w, &image3->h);
    SDL_QueryTexture(Ready.tex, NULL, NULL, &image4->w, &image4->h);
    SDL_QueryTexture(Chara_all.tex, NULL, NULL, &image5->w, &image5->h);
    SDL_QueryTexture(Chara_mini.tex, NULL, NULL, &image6->w, &image6->h);

    /* 画像描画（表示）のための設定 */
    SDL_Rect lVector_src = {0, 0, image1->w, image1->h};

    Chara_sel.src.x = 0;
    Chara_sel.src.y = 0;
    Chara_sel.src.w = image3->w;
    Chara_sel.src.h = image3->h;

    Chara_all.src.x = 0;
    Chara_all.src.y = 0;
    Chara_all.src.w = 900;
    Chara_all.src.h = 500;

    Chara_mini.src.x = 0;
    Chara_mini.src.y = 0;
    Chara_mini.src.w = 120;
    Chara_mini.src.h = 120;

    SDL_Rect lVector_dst = {150, 400, image1->w, image1->h};
    SDL_Rect rVector_dst = {1450, 400, image2->w, image2->h};

    Chara_sel.dst.x = 750;
    Chara_sel.dst.y = 750;
    Chara_sel.dst.w = image3->w;
    Chara_sel.dst.h = image3->h;

    Ready.dst.x = 1350;
    Ready.dst.y = 50;
    Ready.dst.w = image4->w;
    Ready.dst.h = image4->h;

    Chara_all.dst.x = 400;
    Chara_all.dst.y = 200;
    Chara_all.dst.w = 900;
    Chara_all.dst.h = 500;

    Chara_mini.dst.x = 100;
    Chara_mini.dst.y = 70;
    Chara_mini.dst.w = 120;
    Chara_mini.dst.h = 120;

    SDL_RenderCopy(gMainRenderer, lVector_tex, &lVector_src, &lVector_dst);
    SDL_RenderCopy(gMainRenderer, rVector_tex, &lVector_src, &rVector_dst);

    boxColor(gMainRenderer, 745, 745, 955, 855, 0xff0000ff);
    SDL_RenderCopy(gMainRenderer, Chara_sel.tex, &Chara_sel.src, &Chara_sel.dst);
    boxColor(gMainRenderer, 1345, 45, 1555, 155, 0xff000000);
    SDL_RenderCopy(gMainRenderer, Ready.tex, &Chara_sel.src, &Ready.dst);

    SDL_RenderCopy(gMainRenderer, Chara_all.tex, &Chara_all.src, &Chara_all.dst);

    SDL_RenderPresent(gMainRenderer); // 描画データを表示

    /* サーフェスの解放 */
    SDL_FreeSurface(image1);
    SDL_FreeSurface(image2);
    SDL_FreeSurface(image3);
    SDL_FreeSurface(image4);
    SDL_FreeSurface(image5);
    SDL_FreeSurface(image6);
    /* テクスチャの解放 */
    SDL_DestroyTexture(lVector_tex);
    SDL_DestroyTexture(rVector_tex);
}

/***********************************************************************
関数名	: MakeCharaIMG
機能	: キャラ描画に必要なデータを作成する
引数	: なし
出力	: なし
***********************************************************************/
void MakeCharaIMG()
{
    for(i = 0; i < CHARANUM; i++){
        SDL_Surface *chara = IMG_Load(gChara[i].path);
        if(!chara){
            fprintf(stderr, "win error : failed to load chara_image. (MakeCharaIMG)\n");
            exit(-1);
        }
        
        gChara[i].tex = SDL_CreateTextureFromSurface(gMainRenderer, chara);
        SDL_QueryTexture(gChara[i].tex, NULL, NULL, &chara->w, &chara->h);

        gChara[i].src.x = 0;
        gChara[i].src.y = 0;
        gChara[i].src.w = chara->w;
        gChara[i].src.h = chara->h;

        SDL_FreeSurface(chara);
    }

    SDL_Surface *image = IMG_Load("game/chara_vector.png");
    if(!image){
        fprintf(stderr, "win error : failed to load vector_image. (MakeCharaIMG)\n");
        exit(-1);
    }
    Arrow.w     = image->w;
    Arrow.h     = image->h;
    Arrow.tex   = SDL_CreateTextureFromSurface(gMainRenderer, image);
    SDL_QueryTexture(Arrow.tex, NULL, NULL, &Arrow.w, &Arrow.h);
    SDL_FreeSurface(image);
}

/***********************************************************************
関数名	: MakeEffectIMG
機能	: エフェクト描画に必要なデータを作成する
引数	: なし
出力	: なし
***********************************************************************/
void MakeEffectIMG(void)
{
    SDL_Surface *img_damage = IMG_Load("effect/damage.png");
    SDL_Surface *img_heal   = IMG_Load("effect/heal.png");
    if(!(img_damage && img_heal)){ // 読み込みに失敗したら
        fprintf(stderr, "win error : MakeEffectIMG failed.\n");
        exit(-1);
    }

    DamageEffect.tex    = SDL_CreateTextureFromSurface(gMainRenderer, img_damage);
    HealEffect.tex      = SDL_CreateTextureFromSurface(gMainRenderer, img_heal);

    SDL_QueryTexture(DamageEffect.tex, NULL, NULL, &img_damage->w, &img_damage->h);
    SDL_QueryTexture(HealEffect.tex, NULL, NULL, &img_heal->w, &img_heal->h);

    DamageEffect.src.x = 0;
    DamageEffect.src.y = 0;
    DamageEffect.src.w = img_damage->w;
    DamageEffect.src.h = img_damage->h;

    HealEffect.src.x = 0;
    HealEffect.src.y = 0;
    HealEffect.src.w = img_heal->w;
    HealEffect.src.h = img_heal->h;

    DamageEffect.dst.w = 120;
    DamageEffect.dst.h = 120;

    HealEffect.dst.w = 120;
    HealEffect.dst.h = 120;

    SDL_FreeSurface(img_damage);
    SDL_FreeSurface(img_heal);
}

/***********************************************************************
関数名  : MakeCNIMG
機能	: キャラ名を表示するために必要なデータを作成する
引数	: なし
出力	: なし
***********************************************************************/
void MakeCNIMG(void)
{
    SDL_Surface *img_name = IMG_Load("game/chara_name.png");
    // 画像読み込みに失敗したら
    if(!img_name){ 
        printf("failed to load name_image. (MakeCNIMG)\n");
        exit(-1);
    }
    
    // 読み込んだ画像からテクスチャを作成
    cName.tex = SDL_CreateTextureFromSurface(gMainRenderer, img_name); 
    // 画像（テクスチャ）の情報（サイズなど）を取得
    SDL_QueryTexture(cName.tex, NULL, NULL, &img_name->w, &img_name->h); 

    cName.src.x = 0;
    cName.src.y = 0;
    cName.src.w = 180;
    cName.src.h = 80;

    cName.dst.x = 0;
    cName.dst.y = 0;
    cName.dst.w = 180;
    cName.dst.h = 80;

    SDL_FreeSurface(img_name);
}

/***********************************************************************
関数名  : MakeSGIMG
機能	: スキルゲージを表示するために必要なデータを作成する
引数	: なし
出力	: なし
***********************************************************************/
void MakeSGIMG(int cnum)
{   
    SDL_Surface *img_sg1 = IMG_Load("game/s.png");     //スキル１ （３回の行動で使用可能）
    SDL_Surface *img_sg2 = IMG_Load("game/p.png");     //スキル２ （５回の行動で使用可能）
    SDL_Surface *img_sg3 = IMG_Load("game/h.png");     //スキル３ （４回の行動で使用可能）
    // 画像読み込みに失敗したら
    if(!(img_sg1 && img_sg2 && img_sg3)){ 
        fprintf(stderr, "win error : MakeSGIMG failed.\n");
        exit(-1);
    }

    // 読み込んだ画像からテクスチャを作成
    switch(gChara[cnum].skill.type){
        case 0:
            gChara[cnum].skill.tex = SDL_CreateTextureFromSurface(gMainRenderer, img_sg1);
            SDL_QueryTexture(gChara[cnum].skill.tex, NULL, NULL, &img_sg1->w, &img_sg1->h); 
            break;
        case 1:
            gChara[cnum].skill.tex = SDL_CreateTextureFromSurface(gMainRenderer, img_sg2);
            SDL_QueryTexture(gChara[cnum].skill.tex, NULL, NULL, &img_sg2->w, &img_sg2->h); 
            break;
        case 2:
            gChara[cnum].skill.tex = SDL_CreateTextureFromSurface(gMainRenderer, img_sg3);
            SDL_QueryTexture(gChara[cnum].skill.tex, NULL, NULL, &img_sg3->w, &img_sg3->h); 
            break;
        default:
            fprintf(stderr, "win error : Unknown skill type. (MakeSGIMG)\n");
    }

    // 画像描画（表示）のための設定
    gChara[cnum].skill.src.x = 0;
    gChara[cnum].skill.src.y = 0;
    gChara[cnum].skill.src.w = 90;
    gChara[cnum].skill.src.h = 90;
    gChara[cnum].skill.dst.x = 0;
    gChara[cnum].skill.dst.y = 0;
    gChara[cnum].skill.dst.w = 90;
    gChara[cnum].skill.dst.h = 90;

    SDL_FreeSurface(img_sg1);
    SDL_FreeSurface(img_sg2);
    SDL_FreeSurface(img_sg3);
}

/***********************************************************************
関数名	: DrawTitle
機能	: タイトル画面の表示、続けるかやめるかの選択
引数	: なし
出力	: titleFlag   :タイトル画面のフラグ
***********************************************************************/
int DrawTitle()
{
    int titleFlag = 1;    // タイトル画面フラグ

    if(input_home){
        titleFlag = 0;
        SendEndCommand();
    }
	if(input_right || input_left){
        SoundEffect(Select);

        if(GorQ == 0){
            GorQ = 1;
        }else if(GorQ == 1){
            GorQ = 0;
        }

        dt_frame_b  = dt_frame;
        dt_frame    = 600 * GorQ + 380;

        boxColor(gMainRenderer, dt_frame_b, 530, dt_frame_b + 340, 670, 0xffffffff);    // 白で描画
    }
	if(input_two){
        SoundEffect(Decision);

        if(GorQ == 0){
            titleFlag = 0;
            SendTitleCommand();
        }else if(GorQ == 1){
            titleFlag = 0;
            SendEndCommand();
        }
    }

    boxColor(gMainRenderer, dt_frame, 530, dt_frame + 340, 670, 0xff0000ff);        // 赤で描画

    SDL_RenderCopy(gMainRenderer, Title.tex, &Title.src, &Title.dst);
    SDL_RenderCopy(gMainRenderer, ToBF.tex, &ToBF.src, &ToBF.dst);
    SDL_RenderCopy(gMainRenderer, Quit.tex, &ToBF.src, &Quit.dst);

	SDL_RenderPresent(gMainRenderer);

    return titleFlag;
}

/***********************************************************************
関数名	: StageSelect
機能	: ステージ決定に必要なデータをサーバに送る
引数	: なし
出力	: stageFlag   : ステージ選択のフラグ
***********************************************************************/
int StageSelect()
{
    int stageFlag = 1;    // ステージ選択フラグ

    if(input_home){
        stageFlag = 0;
        SendEndCommand();
    }
    if(input_left){
        SoundEffect(Select);

        if(--stage < 0){
            stage = ALL_STAGES - 1;
        }

        ss_frame_b  = ss_frame;
        ss_frame    = 500 * stage + 130;

        ns_src.y    = stage * 200;
        item_src.y  = stage * 300;

        rectangleColor(gMainRenderer, ss_frame_b, 120, ss_frame_b + 440, 560, 0xffffffff);  // 白で描画
    }
    if(input_right){
        SoundEffect(Select);

        if(++stage > ALL_STAGES - 1){
            stage = 0;
        }

        ss_frame_b  = ss_frame;
        ss_frame    = 500 * stage + 130;

        ns_src.y    = stage * 200;
        item_src.y  = stage * 300;

        rectangleColor(gMainRenderer, ss_frame_b, 120, ss_frame_b + 440, 560, 0xffffffff);  // 白で描画
    }
    if(input_two){
        SoundEffect(Decision);
        SendStageCommand(stage);
        stageFlag = 0;
    }
        
    rectangleColor(gMainRenderer, ss_frame, 120, ss_frame + 440, 560, 0xff0000ff);  // 赤で描画

    SDL_RenderCopy(gMainRenderer, NSTex, &ns_src, &ns_dst);
    SDL_RenderCopy(gMainRenderer, ItemTex, &item_src, &item_dst);

	SDL_RenderPresent(gMainRenderer);

    return stageFlag;
}

/***********************************************************************
関数名	: CharaSelect
機能	: キャラ決定に必要なデータをサーバに送る
引数	: なし
出力	: なし
***********************************************************************/
int CharaSelect()
{
    int charaFlag = 1;

    if(input_home){
        charaFlag = 0;
        SendEndCommand();
    }
    if(input_right){
        SoundEffect(Select);

        if(++win_num > ALL_CHARAS - 1){
            win_num = 0;
        }

        Chara_all.src.y = 500 * win_num;

        SDL_RenderCopy(gMainRenderer, Chara_all.tex, &Chara_all.src, &Chara_all.dst);
    }
    if(input_left){
        SoundEffect(Select);

        if(--win_num < 0){
            win_num = ALL_CHARAS - 1;
        }

        Chara_all.src.y = 500 * win_num;

        SDL_RenderCopy(gMainRenderer, Chara_all.tex, &Chara_all.src, &Chara_all.dst);
    }
    if(input_two){
        SoundEffect(Decision);

        if(!readyFlag){
            CID[num_sc++] = win_num;

            if(num_sc == 4){
                boxColor(gMainRenderer, 745, 745, 955, 855, 0xff000000);
                SDL_RenderCopy(gMainRenderer, Chara_sel.tex, &Chara_sel.src, &Chara_sel.dst);

                boxColor(gMainRenderer, 1345, 45, 1555, 155, 0xff0000ff);
                SDL_RenderCopy(gMainRenderer, Ready.tex, &Chara_sel.src, &Ready.dst);

                readyFlag = 1;
            }

            Chara_mini.src.y = 120 * win_num;
            Chara_mini.dst.x = 120 * num_sc;

            SDL_RenderCopy(gMainRenderer, Chara_mini.tex, &Chara_mini.src, &Chara_mini.dst);
        }else if(readyFlag){
            SendReadyCommand(CID, 4);
            charaFlag = 0;
        }
        SDL_Delay(100);
    }
    if(input_one){
        SoundEffect(Cancel);

        if(1 <= num_sc && num_sc <= 4){
            boxColor(gMainRenderer, 120 * num_sc, 70, 120 * (num_sc + 1), 220, 0xffffffff);

            if(num_sc == 4){
                boxColor(gMainRenderer, 1345, 45, 1555, 155, 0xff000000); 
                SDL_RenderCopy(gMainRenderer, Ready.tex, &Chara_sel.src, &Ready.dst);

                boxColor(gMainRenderer, 745, 745, 955, 855, 0xff0000ff);
                SDL_RenderCopy(gMainRenderer, Chara_sel.tex, &Chara_sel.src, &Chara_sel.dst);

                readyFlag = 0;
            }
            CID[--num_sc] = -1;
        }
        SDL_Delay(100);
    }
    SDL_RenderPresent(gMainRenderer); // 描画データを表示

    return charaFlag;
}

/***********************************************************************
関数名	: DestroyTitleTex
機能	: タイトル画面の描画に使ったテクスチャを解放する
引数	: なし
出力	: なし
***********************************************************************/
void DestroyTitleTex()
{
    SDL_DestroyTexture(Title.tex);
    SDL_DestroyTexture(ToBF.tex);
    SDL_DestroyTexture(Quit.tex);
}

/***********************************************************************
関数名	: DestroySSTex
機能	: ステージ選択画面の描画に使ったテクスチャを解放する
引数	: なし
出力	: なし
***********************************************************************/
void DestroySSTex()
{
    SDL_DestroyTexture(NSTex);
    SDL_DestroyTexture(ItemTex);
}

/***********************************************************************
関数名	: DestroyCSTex
機能	: キャラ選択画面の描画に使ったテクスチャを解放する
引数	: なし
出力	: なし
***********************************************************************/
void DestroyCSTex()
{
    SDL_DestroyTexture(Chara_all.tex);
    SDL_DestroyTexture(Chara_sel.tex);
    SDL_DestroyTexture(Ready.tex);
    SDL_DestroyTexture(Chara_mini.tex);
}

/*****************************************************************
関数名	: WindowEvent
機能	: メインウインドウに対するイベント処理を行う
引数	: なし
出力	: なし
*****************************************************************/
void WindowEvent()
{
    /* ホームボタン入力された場合 */
    if(input_home){
        SendEndCommand();
    }
    /* 右入力された場合 */
    if(input_right){
        angle += 10;
        if(angle >= 360){
            angle = 0;
        }
    }
    /* 左入力された場合 */
    if(input_left){
        angle -= 10;
        if(angle < 0){
            angle = 350;
        }
    }
    /* 下入力された場合 */
    if(input_down){
        SoundEffect(Select);

        if(++scnum > iscnum + 3){
            scnum = iscnum;
        }

        if(gChara[scnum].aod == CS_Death){
            if(gChara[++scnum].aod == CS_Death){
                if(gChara[++scnum].aod == CS_Death){
                    scnum++;
                }
            }

            if(scnum > iscnum + 3){
                scnum = iscnum;
            }
        }
        SDL_Delay(100);
    }
    /* 上入力された場合 */
    if(input_up){
        SoundEffect(Select);

        if(--scnum < iscnum){
            scnum = iscnum + 3;
        }

        if(gChara[scnum].aod == CS_Death){
            if(gChara[--scnum].aod == CS_Death){
                if(gChara[--scnum].aod == CS_Death){
                    scnum--;
                }
            }

            if(scnum < iscnum){
                scnum = iscnum + 3;
            }
        }
        SDL_Delay(100);
    }
    /* 2ボタン入力された場合 */
    if(input_two){
        SendActionCommand(scnum);
        SDL_Delay(100);
    }
    /* 1ボタン入力された場合 */
    if(input_one){
        switch(gChara[scnum].skill.type){
            case 0:
                if(gChara[scnum].skill.turn == 3){
                    SoundEffect(Decision);
                    SendSkillCommand(scnum);
                }
                break;
            case 1:
                if(gChara[scnum].skill.turn == 5){
                    SoundEffect(Decision);
                    SendSkillCommand(scnum);
                }
                break;
            case 2:
                if(gChara[scnum].skill.turn == 4){
                    SoundEffect(Decision);
                    SendSkillCommand(scnum);
                }
                break;
            default:
                fprintf(stderr, "win error : Unknown skilltype. (WindowEvent)\n");
        }
        SDL_Delay(100);
    }
}

/*****************************************************************
関数名	: WinDraw
機能	: ゲームの描画をする
引数	: int pos   : クライアントID
出力	: なし
*****************************************************************/
void WinDraw(int pos)
{
    /* 画面を白色に塗りつぶす */
    SDL_SetRenderDrawColor(gMainRenderer, 255, 255, 255, 255);
    SDL_RenderClear(gMainRenderer);

    /* ステージ，キャラの名前，スキルゲージ，残り時間の描画 */
    DrawMap();
    DrawCharaName(pos);
    DrawSkillGauge(pos);
    DrawTime();

    /* 選択枠, 選択円, 矢印の描画 */
    if(gChara[scnum].aod == CS_Alive){
        DrawSelect(pos, scnum);
        /* 停止していてかつクールタイム中でない場合 */
        if(gChara[scnum].velocity == 0 && cooltime[scnum] == 0){
            DrawArrow(scnum);
        }
    }

    for(i = 0; i < CHARANUM; i++){
        if(gChara[i].aod == CS_Alive){
            /* キャラの描画 */
            DrawChara(pos, i);
            /* ライフゲージの描画 */
            DrawLife(pos, i);
            /* 攻撃アップの描画 */
            if(atkFlag[i]){
                DrawAttack(pos, i);
            }
            /* 毒状態の描画 */
            if(poisonFlag[i]){
                DrawPoison(pos, i);
            }
            /* イベント時の能力アップ描画 */
            if(upFlag[i]){
                DrawEvent(pos, i);
            }
            /* クールタイムの描画 */
            if(cooltime[i] != 0){
                DrawCooltime(i);
            }
            /* ダメージエフェクトの描画 */
            if(beforeLife[i] > gChara[i].life || de_count[i] != 0){
                DrawDamageEffect(i);
                if(de_count[i]++ > 10){
                    de_count[i] = 0;
                }
                beforeLife[i] = gChara[i].life;
            }
            /* ヒールエフェクトの描画 */
            if(beforeLife[i] < gChara[i].life || he_count[i] != 0){
                DrawHealEffect(i);
                if(he_count[i]++ > 60){
                    he_count[i] = 0;
                }
                beforeLife[i] = gChara[i].life;
            }
        }
    }
    /* アイテムの描画 */
    if(itemFlag){
        DrawItem();
    }

    /* イベントカットインの描画 */
    if(cutinFlag){
        SDL_RenderCopy(gMainRenderer, Event.tex, &Event.src, NULL); 
    }
    
    SDL_RenderPresent(gMainRenderer);
}

/*****************************************************************
関数名	: DrawSSWait
機能	: ホストがステージ選択中のゲストの待機画面を表示する
引数	: なし
出力	: なし
*****************************************************************/
void DrawSSWait()
{
    if(input_home){
        SendEndCommand();
    }

    /* 画像描画処理 */
    SDL_Surface *img_wait = IMG_Load("win/waitwindow.png");
    if(!img_wait){ // 読み込みに失敗したら
        fprintf(stderr, "win error : DrawSSWait failed.\n");
        exit(-1);
    }
    /* 画像からテクスチャを作成 */
    SDL_Texture* wait_tex = SDL_CreateTextureFromSurface(gMainRenderer, img_wait);
    /* 画像情報取得 */
    SDL_QueryTexture(wait_tex, NULL, NULL, &img_wait->w, &img_wait->h);

    /* 画像表示設定 */
    SDL_Rect wait_src = {0, 0, img_wait->w, img_wait->h};
    SDL_Rect wait_dst = {0, 0, 1700, 900};

    SDL_RenderCopy(gMainRenderer, wait_tex, &wait_src, &wait_dst);
    /* 描画データの表示 */
    SDL_RenderPresent(gMainRenderer);

    SDL_FreeSurface(img_wait);
    SDL_DestroyTexture(wait_tex);
}

/*****************************************************************
関数名	: DrawWait
機能	: 待機画面を表示する
引数	: なし
出力	: なし
*****************************************************************/
void DrawWait()
{
    if(input_home){
        SendEndCommand();
    }

    /* 画像描画処理 */
    SDL_Surface *img_wait = IMG_Load("win/wait.png");
    if(!(img_wait)){ // 読み込みに失敗したら
        fprintf(stderr, "win error : DrawWait failed.\n");
        exit(-1);
    }
    /* 画像からテクスチャを作成 */
    SDL_Texture* wait_tex = SDL_CreateTextureFromSurface(gMainRenderer, img_wait);
    /* 画像情報取得 */
    SDL_QueryTexture(wait_tex, NULL, NULL, &img_wait->w, &img_wait->h);

    /* 画像表示設定 */
    SDL_Rect wait_src = {0, 0, img_wait->w, img_wait->h};
    SDL_Rect wait_dst = {0, 0, 1700, 900};

    SDL_RenderCopy(gMainRenderer, wait_tex, &wait_src, &wait_dst);
    /* 描画データの表示 */
    SDL_RenderPresent(gMainRenderer);

    SDL_FreeSurface(img_wait);
    SDL_DestroyTexture(wait_tex);
}

/*****************************************************************
関数名	: DrawResult
機能	: 勝敗を表示する
引数	: なし
出力	: なし
*****************************************************************/
void DrawResult()
{
    switch(result){
        case Win:
            DrawWin();
            break;
        case Lose:
            DrawLose();
            break;
        case Draw:
            DrawDraw();
            break;
        case No_results:
            break;
        default:
            fprintf(stderr, "error : DrawResult\n");
    }
}

/*****************************************************************
関数名	: DestroyAllTexture
機能	: テクスチャを全て破棄する
引数	: なし
出力	: なし
*****************************************************************/
void DestroyAllTexture()
{
    SDL_DestroyTexture(gMap.tex);
    SDL_DestroyTexture(cName.tex);
    SDL_DestroyTexture(Event.tex);
    SDL_DestroyTexture(Event_chara.tex);
    SDL_DestroyTexture(Arrow.tex);
    SDL_DestroyTexture(DamageEffect.tex);
    SDL_DestroyTexture(HealEffect.tex);
}

 /***************
 *   static    *
***************/
/**********************************************************************
関数名     : DrawMap
機能       : マップの描画を行う
引数       : なし
出力       : なし
***********************************************************************/
static void DrawMap(void)
{
    /* マップの描画 */
    SDL_RenderCopy(gMainRenderer, gMap.tex, &gMap.src, &gMap.dst);
    if(wallFlag){
        SDL_RenderCopy(gMainRenderer, Event_chara.tex, &Event_chara.src, &Event_chara.dst);
    }
}

/***********************************************************************
関数名     : DrawCharaName
機能       : キャラIDに応じたキャラ名を描画する
引数       : int pos    : クライアントID
出力       : なし
***********************************************************************/
static void DrawCharaName(int pos)
{
    for(i = 0; i < 4; i++){
        if(gChara[i].aod == CS_Alive){
            cName.src.y = 80 * gChara[i].pos;
            cName.dst.x = 1410 - 1300 * pos;
            cName.dst.y = 505 - 400 * pos + 100 * i;
            SDL_RenderCopy(gMainRenderer, cName.tex, &cName.src, &cName.dst);
        }
    }
    for(i = 4; i < CHARANUM; i++){
        if(gChara[i].aod == CS_Alive){
            cName.src.y = 80 * gChara[i].pos;
            cName.dst.x = 110 + 1300 * pos;
            cName.dst.y = 105 + 400 * pos + 100 * (i - 4);
            SDL_RenderCopy(gMainRenderer, cName.tex, &cName.src, &cName.dst);
        }
    }
}

/*****************************************************************
関数名	: DrawSelect
機能	: 選択されているキャラだとわかるような描画する
引数	: int pos   : クライアントID
          int cnum  : キャラ番号
出力	: なし
*****************************************************************/
static void DrawSelect(int pos, int cnum)
{
    /* 選択キャラの名前の背景を黄色にする */
    boxColor(gMainRenderer, 1350, 500 + 100 * (cnum - pos * 4), 1600, 600 + 100 * (cnum - pos * 4), 0x9900ffff);
    /* 選択キャラに黄色の選択円を描画する */
    filledCircleColor(gMainRenderer, gChara[cnum].point.x, gChara[cnum].point.y, 40, 0xff00ffff);
}

/*****************************************************************
関数名	: DrawArrow
機能	: 選択キャラに発射方向を表す矢印を描画する
引数	: int cnum  : キャラ番号
出力	: なし
*****************************************************************/
static void DrawArrow(int cnum)
{
    /* 矢印の描画 */
    SDL_QueryTexture(Arrow.tex, NULL, NULL, &Arrow.w, &Arrow.h);
    SDL_Rect src_rect9 = {0, 0, Arrow.w, Arrow.h};
    SDL_Rect dst_rect9 = {gChara[cnum].point.x - 75, gChara[cnum].point.y - 75, 150, 150}; // キャラの画像より一回り大きい
    SDL_RenderCopyEx(gMainRenderer, Arrow.tex, &src_rect9, &dst_rect9, angle + 90, NULL, SDL_FLIP_HORIZONTAL);
}

/*****************************************************************
関数名	: DrawChara
機能	: キャラを描画する
引数	: int pos   : クライアントID
          int cnum  : キャラ番号
出力	: なし
*****************************************************************/
static void DrawChara(int pos, int cnum)
{
    /* キャラの描画 */
    gChara[cnum].dst.x = gChara[cnum].point.x - 50;
    gChara[cnum].dst.y = gChara[cnum].point.y - 50;

    if(pos == 0){
        /* 自分のキャラは上向き相手は下向き */
        if(cnum < 4){
            SDL_RenderCopy(gMainRenderer, gChara[cnum].tex, &gChara[cnum].src, &gChara[cnum].dst);
        }else{
            SDL_RenderCopyEx(gMainRenderer, gChara[cnum].tex, &gChara[cnum].src, &gChara[cnum].dst, 180, NULL, SDL_FLIP_HORIZONTAL);
        }
    }else if(pos == 1){
        if(cnum >= 4){
            SDL_RenderCopy(gMainRenderer, gChara[cnum].tex, &gChara[cnum].src, &gChara[cnum].dst);
        }else{
            SDL_RenderCopyEx(gMainRenderer, gChara[cnum].tex, &gChara[cnum].src, &gChara[cnum].dst, 180, NULL, SDL_FLIP_HORIZONTAL);
        }
    }
}

/*****************************************************************
関数名	: DrawLife
機能	: キャラ毎の体力を描画する
引数	: int pos   : クライアントID
          int cnum  : キャラ番号
出力	: なし
*****************************************************************/
static void DrawLife(int pos, int cnum)
{
    /* ライフゲージのもとの長さ */
    gChara[cnum].src_hp.x = gChara[cnum].point.x - 50;

    /* 自キャラは下，敵キャラは上にHP表示 */
    if(cnum < 4){
        gChara[cnum].src_hp.y = gChara[cnum].point.y + 33 - pos * 71;   // 71は微調整した値
    }else{
        gChara[cnum].src_hp.y = gChara[cnum].point.y - 38 + pos * 71;   // -38は微調整した値
    }

    /* ライフゲージ下の白の描画 */
    SDL_SetRenderDrawColor(gMainRenderer, 255, 255, 255, 0);
    SDL_RenderFillRect(gMainRenderer, &gChara[cnum].src_hp);
    
    /* 現在ライフのゲージ */
    HPbar[cnum]             = gChara[cnum].life * 100 / gChara[cnum].basiclife;
    gChara[cnum].dst_hp.x   = gChara[cnum].point.x - 50; // ライフゲージの表示範囲

    /* 自キャラは下，敵キャラは上にHP表示 */
    if(cnum < 4){
        gChara[cnum].dst_hp.y = gChara[cnum].point.y + 33 - pos * 71;   // 71は微調整した値
    }else{
        gChara[cnum].dst_hp.y = gChara[cnum].point.y - 38 + pos * 71;   // -38は微調整した値
    }

    gChara[cnum].dst_hp.w = HPbar[cnum];
    gChara[cnum].dst_hp.h = 10;

    /* 体力が0になった場合 */
    if(gChara[cnum].life <= 0){
        SDL_SetRenderDrawColor(gMainRenderer, 255, 255, 255, 0);
    /* 体力が50を切った場合，ゲージを赤色で描画 */
    }else if(gChara[cnum].life <= 50){
        SDL_SetRenderDrawColor(gMainRenderer, 255, 0, 0, 0);
    /* 体力が150を切った場合，を黄色で描画 */
    }else if(gChara[cnum].life <= 150){
        SDL_SetRenderDrawColor(gMainRenderer, 255, 180, 0, 0);
    /* 体力が250を切った場合，バーを緑で描画 */
    }else if(gChara[cnum].life <= 250){
        SDL_SetRenderDrawColor(gMainRenderer, 0, 255, 0, 0);
    /* 体力が250以上の場合，バーを青で描画 */
    }else{
        SDL_SetRenderDrawColor(gMainRenderer, 0, 0, 255, 0);
    }

    SDL_RenderFillRect(gMainRenderer, &gChara[cnum].dst_hp);

    /* ライフゲージの枠 */
    SDL_SetRenderDrawColor(gMainRenderer, 0, 0, 0, 0);
    SDL_RenderDrawRect(gMainRenderer, &gChara[cnum].src_hp);
}

/*****************************************************************
関数名	: DrawCooltime
機能	: クールタイムを描画する
引数	: int cnum  : キャラ番号
出力	: なし
*****************************************************************/
static void DrawCooltime(int cnum)
{
    /* クールタイムの描画 */
    argf = 270 + (360 * cooltime[cnum] / 3);
    if(argf > 360){
        argf -= 360;
    }
    filledPieColor(gMainRenderer, gChara[cnum].point.x, gChara[cnum].point.y, 33, 270, argf, 0xffff0000);
    if(cooltime[cnum] > 2.95 && cooltime[cnum] < 3.05){
        cooltime[cnum] = 0;
    }
}

/*****************************************************************
関数名	: DrawDamageEffect
機能	: ダメージを受けたキャラにエフェクトを表示する
引数	: int cnum  : キャラ番号
出力	: なし
*****************************************************************/
static void DrawDamageEffect(int cnum)
{
    DamageEffect.dst.x = gChara[cnum].point.x - 50;
    DamageEffect.dst.y = gChara[cnum].point.y - 50;

    SDL_RenderCopy(gMainRenderer, DamageEffect.tex, &DamageEffect.src, &DamageEffect.dst);
}

/*****************************************************************
関数名	: DrawHealEffect
機能	: ライフを回復したキャラにエフェクトを表示する
引数	: int cnum  : キャラ番号
出力	: なし
*****************************************************************/
static void DrawHealEffect(int cnum)
{
    HealEffect.dst.x = gChara[cnum].point.x - 50;
    HealEffect.dst.y = gChara[cnum].point.y - 50;

    SDL_RenderCopy(gMainRenderer, HealEffect.tex, &HealEffect.src, &HealEffect.dst);
}

/******************************************************************************
関数名      : DrawSkillGauge
機能        : スキルのゲージの描画
引数        : int pos   : クライアントID
出力        : なし
*******************************************************************************/
static void DrawSkillGauge(int pos)
{
    for(i = 0; i < 4; i++){
        if(gChara[i].aod == CS_Alive){
            gChara[i].skill.src.y = 90 * gChara[i].skill.turn;
            gChara[i].skill.dst.x = 1605 - 1300 * pos;
            gChara[i].skill.dst.y = 505 - 400 * pos + 100 * i;
            SDL_RenderCopy(gMainRenderer, gChara[i].skill.tex, &gChara[i].skill.src, &gChara[i].skill.dst); 
        }
    }
    for(i = 4; i < CHARANUM; i++){
        if(gChara[i].aod == CS_Alive){
            gChara[i].skill.src.y = 90 * gChara[i].skill.turn;
            gChara[i].skill.dst.x = 305 + 1300 * pos;
            gChara[i].skill.dst.y = 105 + 400 * pos + 100 * (i - 4);
            SDL_RenderCopy(gMainRenderer, gChara[i].skill.tex, &gChara[i].skill.src, &gChara[i].skill.dst); 
        }
    }
}

/******************************************************************************
関数名      : DrawItem
機能        : 攻撃力アップの描画
引数        : なし
出力        : なし
*******************************************************************************/
static void DrawItem(void)
{
    Item.dst.x = Item.point.x - 25;
    Item.dst.y = Item.point.y - 25;
        
    SDL_RenderCopy(gMainRenderer, gItem[Item.type].tex, &gItem[Item.type].src, &Item.dst);
}

/******************************************************************************
関数名      : DrawAttack
機能        : 攻撃力アップの描画
引数        : int pos   : クライアントID
              int cnum  : キャラ番号
出力        : なし
*******************************************************************************/
static void DrawAttack(int pos, int cnum)
{
    if(pos == 0){
        if(cnum < 4){
            dst_attack[cnum].x = gChara[cnum].point.x + 40;
            dst_attack[cnum].y = gChara[cnum].point.y;
            SDL_RenderCopy(gMainRenderer, gItem[1].tex, &gItem[1].src, &dst_attack[cnum]);
        }else{
            dst_attack[cnum].x = gChara[cnum].point.x - 65;
            dst_attack[cnum].y = gChara[cnum].point.y;
            SDL_RenderCopyEx(gMainRenderer, gItem[1].tex, &gItem[1].src, &dst_attack[cnum], 180, NULL, SDL_FLIP_NONE);
        }
    }else if(pos == 1){
        if(cnum >= 4){
            dst_attack[cnum].x = gChara[cnum].point.x + 40;
            dst_attack[cnum].y = gChara[cnum].point.y;
            SDL_RenderCopy(gMainRenderer, gItem[1].tex, &gItem[1].src, &dst_attack[cnum]);
        }else{
            dst_attack[cnum].x = gChara[cnum].point.x - 65;
            dst_attack[cnum].y = gChara[cnum].point.y;
            SDL_RenderCopyEx(gMainRenderer, gItem[1].tex, &gItem[1].src, &dst_attack[cnum], 180, NULL, SDL_FLIP_NONE);
        }
    }
}

/******************************************************************************
関数名      : DrawPoison
機能        : 毒状態の描画
引数        : int pos   : クライアントID
              int cnum  : キャラID
出力        : なし
*******************************************************************************/
static void DrawPoison(int pos, int cnum)
{
    if(pos == 0){
        if(cnum < 4){
            dst_poison[cnum].x = gChara[cnum].point.x + 40;
            dst_poison[cnum].y = gChara[cnum].point.y - 30;
            SDL_RenderCopy(gMainRenderer, gItem[0].tex, &gItem[0].src, &dst_poison[cnum]);
        }else{
            dst_poison[cnum].x = gChara[cnum].point.x - 65;
            dst_poison[cnum].y = gChara[cnum].point.y + 30;
            SDL_RenderCopyEx(gMainRenderer, gItem[0].tex, &gItem[0].src, &dst_poison[cnum], 180, NULL, SDL_FLIP_NONE);
        }
    }else if(pos == 1){
        if(cnum >= 4){
            dst_poison[cnum].x = gChara[cnum].point.x + 40;
            dst_poison[cnum].y = gChara[cnum].point.y - 30;
            SDL_RenderCopy(gMainRenderer, gItem[0].tex, &gItem[0].src, &dst_poison[cnum]);
        }else{
            dst_poison[cnum].x = gChara[cnum].point.x - 65;
            dst_poison[cnum].y = gChara[cnum].point.y + 30;
            SDL_RenderCopyEx(gMainRenderer, gItem[0].tex, &gItem[0].src, &dst_poison[cnum], 180, NULL, SDL_FLIP_NONE);
        }
    }
}

/******************************************************************************
関数名      : DrawTime
機能        : 経過時間の描画
引数        : なし
出力        : なし
*******************************************************************************/
static void DrawTime(void)
{
    time_arg = pasttime * 360 / FINISH_TIME - 90;   // -90は扇形を円の上部から描画するため

    filledPieColor(gMainRenderer, 200, 680, 130, -90, 269.9, 0xff000000);       // 黒で描画
    filledPieColor(gMainRenderer, 200, 680, 130, -90, time_arg, 0xffffffff);    // 白で描画
}

/******************************************************************************
関数名      : DrawEvent
機能        : イベントによるステータス変化の描画
引数        : int pos   : クライアントID
              int cnum  : キャラ番号
出力        : なし
*******************************************************************************/
static void DrawEvent(int pos, int cnum)
{
    if(pos == 0){
        if(cnum < 4){
            dst_event[cnum].x = gChara[cnum].point.x - 70;
            dst_event[cnum].y = gChara[cnum].point.y - 10;
            SDL_RenderCopy(gMainRenderer, Event_chara.tex, &Event_chara.src, &dst_event[cnum]);
        }else{
            dst_event[cnum].x = gChara[cnum].point.x + 15;
            dst_event[cnum].y = gChara[cnum].point.y - 40;
            SDL_RenderCopyEx(gMainRenderer, Event_chara.tex, &Event_chara.src, &dst_event[cnum], 180, NULL, SDL_FLIP_NONE);
        }
    }else if(pos == 1){
        if(cnum >= 4){
            dst_event[cnum].x = gChara[cnum].point.x - 70;
            dst_event[cnum].y = gChara[cnum].point.y - 10;
            SDL_RenderCopy(gMainRenderer, Event_chara.tex, &Event_chara.src, &dst_event[cnum]);
        }else{
            dst_event[cnum].x = gChara[cnum].point.x + 15;
            dst_event[cnum].y = gChara[cnum].point.y - 40;
            SDL_RenderCopyEx(gMainRenderer, Event_chara.tex, &Event_chara.src, &dst_event[cnum], 180, NULL, SDL_FLIP_NONE);
        }
    }
}

/*****************************************************************
関数名	: DrawDraw
機能	: 引き分けを表示する
引数	: なし
出力	: なし
*****************************************************************/
static void DrawDraw()
{
    /* 画像描画処理 */
    SDL_Surface *img_draw = IMG_Load("result/draw.png"); 
    if(!img_draw){ // 読み込みに失敗したら
        fprintf(stderr, "win error : DrawDraw failed.\n");
        exit(-1);
    }
    
    /* 画像からテクスチャを作成 */
    SDL_Texture* draw_tex = SDL_CreateTextureFromSurface(gMainRenderer, img_draw);

    /* 画像情報取得 */
    SDL_QueryTexture(draw_tex, NULL, NULL, &img_draw->w, &img_draw->h);

    /* 画像表示設定 */
    SDL_Rect draw_src = {0, 0, img_draw->w, img_draw->h};
    SDL_Rect draw_dst = {0, 0, WINDOWWIDE, WINDOWHEIGHT};

    SDL_RenderCopy(gMainRenderer, draw_tex, &draw_src, &draw_dst);
    /* 描画データの表示 */
    SDL_RenderPresent(gMainRenderer);

    SDL_Delay(3000);

    SDL_FreeSurface(img_draw);
    SDL_DestroyTexture(draw_tex);
}

/*****************************************************************
関数名	: DrawLose
機能	: 敗北を表示する
引数	: なし
出力	: なし
*****************************************************************/
static void DrawLose()
{
    /* 画像描画処理 */
    SDL_Surface *img_lose = IMG_Load("result/lose.png");
    if(!(img_lose)){ // 読み込みに失敗したら
        fprintf(stderr, "win error : DrawLose failed.\n");
        exit(-1);
    }

    /* 画像からテクスチャを作成 */
    SDL_Texture* tex_lose = SDL_CreateTextureFromSurface(gMainRenderer, img_lose);

    /* 画像の情報を取得 */
    SDL_QueryTexture(tex_lose, NULL, NULL, &img_lose->w, &img_lose->h);

    /* 画像表示設定 */
    SDL_Rect lose_src = {0, 0, img_lose->w, img_lose->h};
    SDL_Rect lose_dst = {0, 0, WINDOWWIDE, WINDOWHEIGHT};

    SDL_RenderCopy(gMainRenderer, tex_lose, &lose_src, &lose_dst);
    /* 描画データの表示 */
    SDL_RenderPresent(gMainRenderer);

    SDL_Delay(3000);

    SDL_FreeSurface(img_lose);
    SDL_DestroyTexture(tex_lose);
}

/*****************************************************************
関数名	: DrawWin
機能	: 勝利を表示する
引数	: なし
出力	: なし
*****************************************************************/
static void DrawWin()
{
    /* 画像描画処理 */
    SDL_Surface *img_win = IMG_Load("result/win.png");
    if(!(img_win)){ // 読み込みに失敗したら
        fprintf(stderr, "win error : DrawWin failed.\n");
        exit(-1);
    }

    /* 画像からテクスチャを作成 */
    SDL_Texture* tex_win = SDL_CreateTextureFromSurface(gMainRenderer, img_win); 

    /* 画像情報取得 */
    SDL_QueryTexture(tex_win, NULL, NULL, &img_win->w, &img_win->h); 

    /* 画像表示設定 */
    SDL_Rect win_src = {0, 0, img_win->w, img_win->h}; 
    SDL_Rect win_dst = {0, 0, WINDOWWIDE, WINDOWHEIGHT};

    SDL_RenderCopy(gMainRenderer, tex_win, &win_src, &win_dst);
    /* 描画データの表示 */
    SDL_RenderPresent(gMainRenderer);

    SDL_Delay(3000);

    SDL_FreeSurface(img_win);
    SDL_DestroyTexture(tex_win);
}