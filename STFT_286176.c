////////////////////////////////////////////////////////
//                                                    //
//  振幅スペクトルの描画                                 //
//                                                    //
//                                         2022. 4.23.//
////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define  N 512                                              // フレーム長
#define  pi 3.1415926535897932384626433832795               // 円周率

int main(int argc, char **argv){
    ///////////////////////////////
    //  ファイル読み出し用変数   //
    ///////////////////////////////
    FILE *f1;
    unsigned short tmp1;                                    // 2バイト変数
    unsigned int   tmp2;                                    // 4バイト変数
    int Fs;                                                 // 入力サンプリング周波数
    int ch;                                                 // 入力チャネル数
    int len;                                                // 入力信号の長さ
    
    ///////////////////////////////
    //  ファイル書き込み用変数   //
    ///////////////////////////////
    FILE *f2;
    
    char outname[256]={0};                                 // 出力ファイル名
    unsigned int  file_size;                               // ファイルサイズ 4Byte
    unsigned int  width;                                   // 画像の幅
    unsigned int  height;                                  // 画像の高さ
    unsigned int  zero  = 0;                               // '0'書き込み用
    unsigned int  one   = 1;                               // '1'書き込み用
    unsigned int  FileHeaderSize=54;                       // 全ヘッダーサイズ(カラー画像用)
    unsigned int  InfoHeaderSize=40;                       // 情報ヘッダーサイズ
    unsigned int  data_len;                                // 波形データのサンプル数
    unsigned int  color = 24;                              // 1画素あたりの色数
    unsigned int  image_size;                              // 出力画像サイズ
    unsigned char  Rout, Gout, Bout;                       // RGB要素の出力用
    int            k, n;                                   // 座標の変数(k行n列)
    int            DataWritePoint;                         // データ書き込み位置

//************************************************************************//

    ///////////////////////////////
    //      信号処理用変数       //
    ///////////////////////////////
    int      t      = 0;                                    // 時刻の変数
    unsigned int t_out  = 0;                                // 終了時刻計測用の変数
    short    input;                                         // 読込み変数と書出し変数

    unsigned int l,i;                                       // ループ用変数
    double   x[N+1] ={0};                                   // 入力信号
    double   Xamp[N+1];                                     // 振幅スペクトル
    double   spec;                                          // スペクトル(画像の輝度)
    double cos_sum;
    double sin_sum;

//************************************************************************//

    //////////////////////////////////////
    // 実行ファイルの使い方に関する警告 //
    //////////////////////////////////////
    if( argc != 2 ){                                        // 使用方法が間違った場合の警告
        fprintf( stderr, "Usage: ./a input.wav \n" );
        exit(-1);
    }
    if( (f1 = fopen(argv[1], "rb")) == NULL ){              // 出力ファイルが開けない場合の警告
        fprintf( stderr, "Cannot open %s\n", argv[1] );
        exit(-2);
    }

    ////////////////////////////////////////////
    //  入力waveファイルのヘッダ情報読み込み  //
    ////////////////////////////////////////////
    printf("Wave data is\n");
    fseek(f1, 22L, SEEK_SET);                               // チャネル情報位置に移動
    fread ( &tmp1, sizeof(unsigned short), 1, f1);          // チャネル情報読込 2Byte
    ch=tmp1;                                                // 入力チャネル数の記録
    fread ( &tmp2, sizeof(unsigned int), 1, f1);            // サンプリング周波数の読込 4Byte
    Fs = tmp2;                                              // サンプリング周波数の記録
    fseek(f1, 40L, SEEK_SET);                               // サンプル数情報位置に移動
    fread ( &tmp2, sizeof(unsigned int), 1, f1);            // データのサンプル数取得 4Byte
    len=tmp2/2/ch;                                          // 音声の長さの記録 (2Byteで1サンプル)

    printf("Channel       = %d ch\n",  ch);                 // 入力チャネル数の表示
    printf("Sample rate   = %d Hz \n", Fs);                 // 入力サンプリング周波数の表示
    printf("Sample number = %d\n",     len);                // 入力信号の長さの表示

    //////////////////////////////////////////
    //      書き込みbmpファイルの設定      //
    //////////////////////////////////////////
    height          = N/2;                                  // 画像の高さDFTサイズ）
    width           = len/N;                                // 画像の幅 (フレーム長)
    if(width%4  != 0)width = width  - width%4;              // 幅を4の倍数に調整
    image_size     = width * height * 3;                    // 画像サイズ(3色)
    file_size      = image_size + FileHeaderSize;           // 全体ファイルサイズ(byte)

    //////////////////////////////////////
    //      ヘッダ書き込み              //
    //////////////////////////////////////
    f2=fopen("output.bmp","wb");                                       // 出力ファイルオープン．存在しない場合は作成される

    fprintf(f2, "BM");                                                 // 'BM'
    fwrite(&file_size,      sizeof(unsigned int ), 1, f2);             // ファイルサイズ
    fwrite(&zero,           sizeof(unsigned short), 1, f2);            // 0(予約領域)
    fwrite(&zero,           sizeof(unsigned short), 1, f2);            // 0(予約領域)
    fwrite(&FileHeaderSize, sizeof(unsigned int ), 1, f2);             // 画像データまでのオフセット
    fwrite(&InfoHeaderSize, sizeof(unsigned int ), 1, f2);             // 情報ヘッダサイズ
    fwrite(&width,          sizeof(unsigned int ), 1, f2);             // 画像の幅
    fwrite(&height,         sizeof(unsigned int ), 1, f2);             // 画像の長さ
    fwrite(&one,            sizeof(unsigned short), 1, f2);            // プレーン数(常に1)
    fwrite(&color,          sizeof(unsigned short), 1, f2);            // 1画素あたりの色数
    fwrite(&zero,           sizeof(unsigned int ), 1, f2);             // 0(無圧縮)
    fwrite(&image_size,     sizeof(unsigned int ), 1, f2);             // 画像サイズ
    fwrite(&zero,           sizeof(unsigned int ), 1, f2);             // 水平解像度
    fwrite(&zero,           sizeof(unsigned int ), 1, f2);             // 垂直解像度
    fwrite(&zero,           sizeof(unsigned int ), 1, f2);             // パレットの色の数
    fwrite(&zero,           sizeof(unsigned int ), 1, f2);             // 重要パレットのインデックス

    printf("\nOutput BMP data is\n");
    printf("Width         = %u \n", width );
    printf("Height        = %u \n", height);
    printf("Image Size    = %u\n",  image_size/3);

    ///////////////////////////////
    //      変数の初期設定       //
    ///////////////////////////////
    l = 0;                                                  // 画像の横座標(フレーム番号に対応)

    ///////////////////////////////////
    //        メインループ           //
    ///////////////////////////////////
    fseek(f1, 44L, SEEK_SET);                               // 音声データ開始位置に移動
    while(1){                                               // メインループ
        if(fread( &input, sizeof(short), 1,f1) < 1){        // 音声を input に読込み
            if( t_out > len ) break;                        // ループ終了判定
            else input=0;                                   // ループ継続かつ音声読込み終了なら input=0
        }
        x[t] = input/32768.0;                               // 音声の最大値を1とする(正規化)

//************************************************************************//
        //////////////////////////////////////////////
        //                                          //
        //  Signal Processing                       //
        //  ※ tは0からN-1までをループ                  //
        //                                          //
        //  x[t] (t=0, ..., N-1)を利用して，          //
        //  Nサンプルごとに振幅スペクトルを作成する        //
        //                                          //
        //////////////////////////////////////////////
        if( t==N-1 ){                                      // フレームごとに振幅スペクトルを取得
        
        /***** ここから *****/

            for(k=0;k<N;k++){                              // 周波数番号kのループ
            cos_sum = 0.0;
            sin_sum = 0.0;
            for (i = 0; i < N; i++) {
                cos_sum = cos_sum + x[i] * cos((-2 * pi * k * i) / N); // 実部の総和
                sin_sum = sin_sum + x[i] * sin((-2 * pi * k * i) / N); // 虚部の総和
            }
                Xamp[k] = pow(cos_sum, 2.0) + pow(sin_sum, 2.0);     // 振幅スペクトル
                // printf("%f\n", Xamp[k]);
            }

        /***** ここまで *****/
        
            /////////////////////////////////////////////
            // 振幅スペクトルを画像の1列として書き出し //
            /////////////////////////////////////////////
            for(k=0;k<N/2;k++){
                if(Xamp[k]<=0.000001)Xamp[k]=0.000001;      // 振幅の最小値を制限する
                spec = Xamp[k]*255;                         // 振幅スペクトルの大きさ調整
                if(spec>255)spec=255;                       // 輝度の最大値を255とする
                if(spec<0)  spec=0;                         // 輝度の最小値を0とする
                Bout=Gout=Rout = spec;                      // 輝度の設定（3色同値）
                DataWritePoint = FileHeaderSize + ( k*(int)width + l )*3;// 3は色の数
                fseek(f2, DataWritePoint, SEEK_SET);        // データ書き込み位置に移動
                fwrite(&Bout, sizeof(unsigned char), 1, f2);// B結果の書き込み
                fwrite(&Gout, sizeof(unsigned char), 1, f2);// G結果の書き込み
                fwrite(&Rout, sizeof(unsigned char), 1, f2);// R結果の書き込み
            }
            l++;
        }
//************************************************************************//
        t=(t+1)%N;                                          // 時刻 t の更新
        t_out++;                                            // 終了時刻の管理
    }
    fclose(f1);                                             // 入力ファイルを閉じる
    fclose(f2);                                             // 出力ファイルを閉じる
    return 0;                                               // メイン関数の終了
}
