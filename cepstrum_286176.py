import soundfile as sf
import numpy as np
from scipy.fft import rfft, irfft
import scipy.signal as sig
from matplotlib import pyplot as plt
 
 
# wavの読み込み
data, samplerate = sf.read('vowel_e.wav')             # wavファイルを開き、波形とサンプリングレートを取得       
N = 1024                                              # データのサンプル数
freq = np.arange(N//2+1) * (samplerate / N)           # 周波数軸
 
# ケプストラム分析
eps = np.finfo(np.float64).eps                        # マシンイプシロン(誤差を無くすため，微調整)
spec = rfft(data[:N])                                 # 時間波形を離散フーリエ変換
spec_abs = np.abs(spec)                               # 絶対値
spec_log = np.log(spec_abs+eps)                       # スペクトルを対数変換する
ceps = irfft(spec_log)                                # 対数スペクトルを逆フーリエ変換してケプストラム波形を作る


# リフタリング
index = 40                                            # ローパスリフターのカットオフ指標
ceps[index:len(ceps) - index] = 0                     # ケプストラム波形の高次を0にする（ローパスリフター）
ceps_env = np.real(rfft(ceps))                        # ケプストラム波形を再度フーリエ変換して実部だけを取り出す



# ここからグラフ描画
fig = plt.figure()
ax = fig.add_subplot(111)

# データプロット
ax.plot(freq, 10*spec_log, c="red")
ax.plot(freq, 10*ceps_env, c='blue')

# 軸のラベルを設定
ax.set_xlabel('Frequency [Hz]')
ax.set_ylabel('Amplitude')
 
# スケールの設定
ax.set_xlim(0, 4000)
ax.set_ylim(-50, 50)
 
# グラフの表示
plt.show()
plt.close()