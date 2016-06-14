/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/CCDevTool
 ****************************************************************************/
#include "CCDevClient.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "curl/include/android/curl/curl.h"
#include "curl/include/android/curl/easy.h"
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#include "curl/include/ios/curl/curl.h"
#include "curl/include/ios/curl/easy.h"
#else
#include <curl/curl.h>
#include <curl/easy.h>
#endif


bool isNetworkAbsolutePath(const std::string& filename)
{
    return cocos2d::extension::DevClient::getInstance()->isAbsolutePath(filename);
}
bool isNetworkFileExist(const std::string& filename)
{
    return cocos2d::extension::DevClient::getInstance()->isFileExist(filename);
}
std::string fullPathForNetworkFilename(const std::string& filename)
{
    return cocos2d::extension::DevClient::getInstance()->fullPathForFilename(filename);
}
cocos2d::Data getNetworkData(const std::string& filename, bool forString)
{
    return cocos2d::extension::DevClient::getInstance()->getData(filename, forString);
}


NS_CC_EXT_BEGIN

const char *DevClient::KEY_HOSTNAME = "dev_server_host_name";

DevClient* DevClient::s_sharedDevClient = nullptr;

static size_t writeData(void *ptr, size_t size, size_t nmemb, void *stream)
{
    std::vector<char> *recvBuffer = (std::vector<char>*)stream;
    size_t sizes = size * nmemb;
    recvBuffer->insert(recvBuffer->end(), (char*)ptr, (char*)ptr+sizes);
    return sizes;
}

DevClient* DevClient::getInstance()
{
    if (!s_sharedDevClient)
    {
        s_sharedDevClient = new (std::nothrow) DevClient();
        CCASSERT(s_sharedDevClient, "FATAL: Not enough memory");
        s_sharedDevClient->init();
    }
    
    return s_sharedDevClient;
}

void DevClient::destroyInstance()
{
    CC_SAFE_RELEASE_NULL(s_sharedDevClient);
}

DevClient::DevClient()
: _curl(nullptr)
, _hostChangedCallback(nullptr)
{}

DevClient::~DevClient()
{
    if (_curl)
    {
        curl_easy_cleanup( _curl );
    }
}

bool DevClient::init()
{
    _buffer.reserve(1024*1024*8);
    _hostName = cocos2d::UserDefault::getInstance()->getStringForKey(KEY_HOSTNAME);
    
    _curl = curl_easy_init();
//    curl_easy_setopt(_curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(_curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(_curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(_curl, CURLOPT_LOW_SPEED_LIMIT, 1L);
    curl_easy_setopt(_curl, CURLOPT_LOW_SPEED_TIME, 5L);
    curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, writeData);
    curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &_buffer);
    
    // Is it working?
    curl_easy_setopt(_curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(_curl, CURLOPT_TCP_KEEPIDLE, 120L);
    curl_easy_setopt(_curl, CURLOPT_TCP_KEEPINTVL, 60L);
    
    // FileUtilsを介さずに読み込まれる可能性のあるファイル
    // ex) FileUtilsApple::getValueMapFromFile
    _ignoreExtensionList.emplace_back("plist");
    _ignoreExtensionList.emplace_back("mp3");
    
    return true;
}

void DevClient::setHostName(const std::string& hostname)
{
    _hostName = hostname;
    cocos2d::UserDefault::getInstance()->setStringForKey(KEY_HOSTNAME, _hostName);
    cocos2d::UserDefault::getInstance()->flush();
    
    if (_hostChangedCallback)
    {
        _hostChangedCallback();
    }
}

const std::string& DevClient::getHostName() const
{
    return _hostName;
}

void DevClient::setHostChangedCallback(std::function<void()> callback)
{
    _hostChangedCallback = callback;
}

bool DevClient::perform(const std::string& path, const std::string& api, std::function<void()> successCallback)
{
    char* out = nullptr;
    cocos2d::base64Encode((const unsigned char*)path.c_str(), (uint32_t)path.length(), &out);
    const std::string url = _hostName + api + out;
    free(out);
    curl_easy_setopt(_curl, CURLOPT_URL, url.c_str());
//    CCLOG("DevClient::perform: %s", url.c_str());
    
    long status = -1;
    CURLcode code = curl_easy_perform(_curl);
    if (code == CURLE_OK)
    {
        code = curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &status);
    }
    const bool result( code == CURLE_OK && status == 200 );
    if( result && successCallback )
    {
        successCallback();
    }
    
    _buffer.clear();
    return result;
}

bool DevClient::isIgnoreTarget(const std::string& path) const
{
    if (_hostName.empty())
    {
        return true;
    }
    
    if (const char* ext = strrchr(path.c_str(), '.'))
    {
        const auto it = std::find(_ignoreExtensionList.cbegin(), _ignoreExtensionList.cend(), ext+1);
        return (it != _ignoreExtensionList.end());
    }
    
    return true;
}

bool DevClient::isAbsolutePath(const std::string& path) const
{
    return  !_hostName.empty() &&
            ( path.length() >= _hostName.length() ) &&
            ( memcmp(path.c_str(), _hostName.c_str(), _hostName.length()) == 0 );
}

bool DevClient::isFileExist(const std::string& path)
{
    return !fullPathForFilename(path).empty();
}

std::string DevClient::fullPathForFilename(std::string path)
{
    if (path[0] == '/')
    {
        return "";
    }
    
    if (isAbsolutePath(path))
    {
        path = path.substr(_hostName.length(), std::string::npos);
    }
    
    if (isIgnoreTarget(path))
    {
        return "";
    }
    
    auto cacheIter = _fullPathCache.find(path);
    if( cacheIter != _fullPathCache.end() )
    {
        return cacheIter->second;
    }
    
    std::string ret;
    perform(path, "file/exist/", [&](){
        ret = _hostName + path;
        _fullPathCache.emplace(path, ret);
    });
    return ret;
}

cocos2d::Data DevClient::getData(std::string path, bool forString)
{
    cocos2d::Data ret;
    
    if (!isAbsolutePath(path))
    {
        path = cocos2d::FileUtils::getInstance()->fullPathForFilename(path);
    }
    
    if (isIgnoreTarget(path))
    {
        return ret;
    }
    
    if (isAbsolutePath(path))
    {
        perform(path.substr(_hostName.length(), std::string::npos), "file/data/", [&](){
            if( forString ){
                _buffer.push_back('\0');
            }
            void* data = malloc(_buffer.size());
            memcpy(data, _buffer.data(), _buffer.size());
            ret.fastSet((unsigned char*)data, _buffer.size());
            
            cocos2d::Director::getInstance()->setNextDeltaTimeZero(true);
        });
    }
    return ret;
}

DevClient::FileStat::FileStat(bool d, const char* p)
: isDir(d)
, path(p)
{}

std::vector<DevClient::FileStat> DevClient::getList(std::string path)
{
    std::vector<DevClient::FileStat> ret;
    
    if (_hostName.empty())
    {
        return ret;
    }
    
    if (isAbsolutePath(path))
    {
        path = path.substr(_hostName.length(), std::string::npos);
    }
    
    perform(path, "file/list/", [&](){
        _buffer.push_back('\0');
        const char* p0 = _buffer.data();
        while( char* p1 = strchr(p0, ',') )
        {
            *p1 = '\0';
            ret.emplace_back( (*p0=='1'), p0+1 );
            p0 = p1+1;
        }
    });
    return ret;
}


NS_CC_EXT_END
