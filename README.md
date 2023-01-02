# Rapid Text To Speech – OpenJTalkとOpenALで、割と高速で、割と長文も怖くないテキスト読み上げシステムを作る
以前、[Open JTalkで生成した音声をALSAで鳴らす](https://gist.github.com/ll0s0ll/923793b7f01695763740804c95fc0a3c)で、読み上げの高速化を試しましたが、もう少し改善できないか試してみました。今回は以下をポイントとして作成しました。

- はじめの音声が出るまでの時間を短くする
- 長文を読み上げる時でも、はじめの音声が出るまでの時間を短くする
- 読み上げを一時停止、再生できる

動作を収録した動画を撮りました。

https://user-images.githubusercontent.com/3046839/210189279-1ea458c6-2f93-4da6-a43c-91d4fdeb278a.mp4

## ざっくりとした仕組み
OpenJTalkでテキストを音声に変換して、OpenALを使って音声を再生しています。OpenALにはストリーミング形式のような機能があって、音声ファイルを継続的に読み込ませることで、音声を途切れなく再生し続けることができます。その機能を利用して、OpenJTalkでテキストを細切れに音声へ変換し、OpenALに読み込ませ続けることで、長文時の発話までの待ち時間を短縮しています。また、OpenALには音声の一時停止機能もありますので、それを利用して発話をコントロールできるようにしています。

- **OpenAL**  
    OpenAL (Open Audio Library)はクロスプラットフォームのオーディオAPIであるフリーソフト。マルチチャンネル3次元定位オーディオを効率よく表現するように設計された。APIのスタイルと慣習は意図的にOpenGLと似せてある。[Wikipedia](http://ja.wikipedia.org/wiki/OpenAL)

- **OpenJTalk**  
    [Open JTalk](http://open-jtalk.sourceforge.net/)は、名古屋工業大学の徳田・李研究室で開発された日本語テキスト音声合成システムです。入力したテキストを解析して合成音声を生成してくれます。形態素解析エンジンの[MeCab](http://mecab.googlecode.com/svn/trunk/mecab/doc/index.html)（和布蕪、めかぶ）、奈良先端大学を中心にして開発された形態素解析用辞書の[naist-jdic](http://naist-jdic.sourceforge.jp/)、隠れマルコフモデル(HMM)に基づく音声合成エンジン[hts_engine](http://hts-engine.sourceforge.net/)が使われています。

## 使い方
※Open JTalkは1.05、hts_engine APIは1.06のバージョンを対象にしています。

### Raspberry Pi
1. OpenJTalkを[ダウンロード](http://sourceforge.net/projects/open-jtalk/files/Open%20JTalk/)します。現時点では最新は1.08ですが、バージョン1.05をダウンロードして、任意の場所に展開します。

1. 辞書ファイルを[ダウンロード](http://sourceforge.net/projects/open-jtalk/files/Dictionary/open_jtalk_dic-1.05/)します。現時点では最新は1.08ですが、バージョン1.05をダウンロードして、任意の場所に展開します。

1. ボイスファイルをダウンロードします。OpenJTalk標準は[こちら](http://sourceforge.net/projects/open-jtalk/files/HTS%20voice/hts_voice_nitech_jp_atr503_m001-1.04/)。バージョン1.04をダウンロードして、任意の場所に展開します。RapidTextToSpeechのボイスパラメータは、[MMDAgent](http://www.mmdagent.jp/)に含まれている’Mei’のボイスに合わせて設定されています。もちろんOpenJTalk標準でも動作します。

1. OpenJTalkをmakeします。

1. RapidTextToSpeechのソースコードを入手する。zipは[こちら](https://github.com/ll0s0ll/Rapid_Text_To_Speech/archive/master.zip)。
    ```sh
    $ git clone https://github.com/ll0s0ll/Rapid_Text_To_Speech
    ```

1. 任意の場所に展開し、makeします。makeのオプションに、make済みOpenJTalkディレクトリへのパスをOJTDIRで、make済みHTSEngineディレクトリへのパスをHTSEDIRで指定してください。例.
    ```sh
    $ make OJTDIR=/path/to/openjtalkdir HTSEDIR=/path/to/htsenginedir
    ```

1. makeが完了したら実行します。オプションに、辞書ファイルディレクトリ、ボイスファイルディレクトリへのパスを指定します。例.
    ```sh
    $ ./RapidTextToSpeech -d /path/to/dicdir -v /path/to/voicedir
    ```

### Mac OS X (Lion 10.7)
上記のRaspberry Piの場合と同様ですが、makeのオプションに、`OS=MACOSX`を指定して、makeしてください。例.
```sh
$ make OJTDIR=/path/to/openjtalkdir HTSEDIR=/path/to/htsenginedir OS=MACOSX
```

## プログラムについて
プログラムが起動すると、「>」を表示して、発話するテキストを待ち受けする状態になります。テキストを入力すると発話します。発話が開始した時にビープが1回、発話が終了したときにビープが2回なるようになっています。発話が開始すると、再び待ち受け状態に戻ります。発話中でもテキストを入力すると、現在の発話を終了して、新しいテキストを発話します。待ち受け中に下記のコマンドを入力すると、それぞれの動作をします。

### コマンド
- exit  
    RapidTextToSpeech を終了します。

- pause  
    読み上げを一時停止します。resumeで読み上げを再開します。

- resume  
    pauseで一時停止した読み上げを再開します。

- stop  
    読み上げを停止します。再開はできません。
