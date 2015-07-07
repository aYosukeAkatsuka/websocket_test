# このリポジトリは

libwebsocketsの動作検証用リポジトリです。

# How to build

MacとLinux(Ubuntu 14.04 64bit)で動作確認しました。  
ビルド方法は以下。

## サブモジュール取得

* `$ git submodule update --init`

## libwebsocketsのビルド 

* `$ cd [root of source tree]`
* `$ cd libwebsockets`
* `$ cmake . `
* `$ make`

## テスト用サーバのビルド

* `$ cd [root of source tree]`
* `$ cd server`
* `$ make`

## テスト用クライアントのビルド

* `$ cd [root of source tree]`
* `$ cd client`
* `$ make`


