# -*- coding: utf-8 -*-

import numpy as np
import cv2
import soundfile as sf


def main():
    img = cv2.imread('MrsGREENAPPLE.jpg') # 画像の読み込み
    gray_img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY) # グレイスケールに変換
    M, L = gray_img.shape # 画像サイズの取得(M:縦，L:横)
    # print(M , L) # 画像サイズの出力
    y = np.zeros((2*M*L)) # 出力波形を格納するベクトル


    ##################
      # 画像 → 音 変換 #
    ##################

    for i in range(L):               # 画像を1列ごとに波形にする
        Y = np.flipud(gray_img[:,i]) # 画像のi列目を取り出して反転する
        Y = np.append(Y, 0)          # i 列目の最後に0を足す(rfftへの対応)
        y[i * 2 * M : (i+1) * 2 * M] = np.fft.irfft(Y) # # IFFT(逆高速フーリエ変換)で波形を記録
        
    y = y / np.max(np.abs(y)) * 0.8
    sf.write("out.wav", y, 16000, format = "WAV", subtype = "PCM_16") # 作成した波形の書き出し


    #############
     # 画像の復元 #
    #############
    
    ExImg = np.zeros( (M, L) )       # 復元画像を格納する行列
    for i in range(L):               # STFTループ
        X = np.fft.rfft( y[ i * 2 * M : (i+1) * 2 * M ] )  # 音信号のFFT（2Mサンプルずつ）
        Xamp = np.abs(X)             # 振幅スペクトル
        ExImg[:,i] = Xamp[0:M]       # 振幅スペクトルを画像の1列として記録
        
    ExImg = np.flipud(ExImg) * 255   # 画像の上下反転と輝度の調整（最大値255）
    cv2.imwrite('output.png', ExImg) # 復元画像を保存



if __name__ == '__main__':
    main()