# WavePlayer

## 音声ファイル（wav, aif, mp3）を再生するVST3プラグインです。

### 使い方

「Load Wave File」ボタンを押すと、再生するファイルを指定出来ます。あとは見れば分かるはず。

### その他
その他、SyncRoom読み上げちゃん (https://github.com/XiAce-Lite/SyncRoomChatToolV2) と連動して、SyncRoomのチャットログが出来る度にチャットを読み上げることも出来ます（と言うか、こっちが本来の目的。ループバックがないオーディオインターフェイスでも、読み上げさせたかった）

読み上げちゃん側で合成する音声などは指定します。

### 言い訳
JUCEとVisual Studio 2022にて作成。Claude Sonnet 4 で大元のソースを出力後、Github Copilotにて機能追加しています。作者は、C++は全然知らないです。
