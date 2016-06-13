
# 概要
- 任意のPCへファイルサーバーを構築
- cocos2d-xアプリからそれへアクセスする

cocos2d-x 環境から、PC上のファイルへアクセスする為の<br>
サーバーツールです。

NodeWebkitで構築したCCDevServerを起動させて、
同期させるパスを定義します。

Client用ソースコードを適用し、
cocos2d-xのプロジェクトからアクセスします。

![image](https://github.com/yassy0413/CCDevTool/blob/develop/doc/image.jpg)

# 必要なもの

- npm<br>[Node.js](https://nodejs.org/en/)をインストールすると一緒に入ります
- git<br>setup.commandの実行に必要。

# 動作確認環境
- OX X EI Capitan
- Windows8, 10
- cocos2d-x 3.8

# 簡単セットアップ
※現在はMAC用のみ

**setup.command**を実行する<br>
※ターミナルで npm と git が使える必要があります

実行後、以下のファイルが作成されていれば成功です。

- kickstart
- node_modules
- nwjs-osx-x64
- nwjs-win-x64

# 手動セットアップ

1.**install npm**を実行して、node_modules をダウンロード

2.[KickStart](http://www.99lime.com/elements/)のダウンロード<br>
- HTML-KickStart-masterを、**kickstart**として<br>
ダウンロード、展開する

3.[NodeWebkit](http://nwjs.io/)のダウンロード
- MAC用は**nwjs-osx-x64**として<br>
- WIN用は**nwjs-win-x64**として<br>
ダウンロード、展開する

# サーバーの起動
- MAC<br>セットアップ完了後、do.commandを実行
- Windows<br>セットアップ完了後、do.batを実行

# クライアントの実装
1.以下をビルド・プロジェクトへ追加して下さい。
- CCDevClient.h
- CCDevClient.cpp

2.CCFileUtils.cppの修正

現状どうしても加工が必要になるので、コード量を最小限に抑える為<br>
CCDevClient.cppで定義している関数をexternして使います。

```cpp
#include "CCFileUtils.h"

// for DevClient
extern bool isNetworkAbsolutePath(const std::string& filename);
extern bool isNetworkFileExist(const std::string& filename);
extern std::string fullPathForNetworkFilename(const std::string& filename);
extern cocos2d::Data getNetworkData(const std::string& filename, bool forString);
```

```cpp
bool FileUtils::isFileExist(const std::string& filename) const
{
    if (isAbsolutePath(filename))
    {
        // for DevClient
        if (isNetworkAbsolutePath(filename) )
        {
            return isNetworkFileExist(filename);
        }
        return isFileExistInternal(filename);
    }
    ...
}
```

```cpp
bool FileUtils::isAbsolutePath(const std::string& path) const
{
    // for DevClient
    if (isNetworkAbsolutePath(path) )
    {
        return true;
    }
    return (path[0] == '/');
}
```

```cpp
static Data getData(const std::string& filename, bool forString)
{
    if (filename.empty())
    {
        return Data::Null;
    }
    
    // for DevClient
    Data ret = getNetworkData(filename, forString);
    if (!ret.isNull())
    {
        return ret;
    }
    
    unsigned char* buffer = nullptr;
    
    ...
```

```cpp
std::string FileUtils::getPathForFilename(const std::string& filename, const std::string& resolutionDirectory, const std::string& searchPath) const
{
    ...
    
    std::string path = searchPath;
    path += file_path;
    path += resolutionDirectory;
    
    // for DevClient
    const std::string networkPath = fullPathForNetworkFilename(path+file);
    if (!networkPath.empty()){
        return networkPath;
    }
    
    path = getFullPathForDirectoryAndFilename(path, file);
    
    ...
    
```


# Qiita
http://qiita.com/yassy
