/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#include "CCDevClient.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "curl/include/android/curl/curl.h"
#include "curl/include/android/curl/easy.h"
#else
#include "curl/curl.h"
#include "curl/easy.h"
#endif


bool isNetworkAbsolutePath(const std::string& filename)
{
    return cocos2d::extension::DevClient::getInstance()->isAbsolutePath(filename);
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
    curl_easy_setopt(_curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYHOST, 0L);
    //curl_easy_setopt(_curl, CURLOPT_DNS_CACHE_TIMEOUT, 360);
    curl_easy_setopt(_curl, CURLOPT_CONNECTTIMEOUT, 10);
    curl_easy_setopt(_curl, CURLOPT_TIMEOUT, 360);
    curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, writeData);
    curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &_buffer);
    curl_easy_setopt(_curl, CURLOPT_FAILONERROR, true);
    //curl_easy_setopt(_curl, CURLOPT_NOPROGRESS, false);
    
    return true;
}

void DevClient::setHostName(const std::string& hostname)
{
    _hostName = hostname;
    cocos2d::UserDefault::getInstance()->setStringForKey(KEY_HOSTNAME, _hostName);
}

bool DevClient::perform(const std::string& path, const std::string& api, std::function<void()> successCallback)
{
    char* out = nullptr;
    cocos2d::base64Encode((const unsigned char*)path.c_str(), (uint32_t)path.length(), &out);
    const std::string url = _hostName + api + out;
    free(out);
    curl_easy_setopt(_curl, CURLOPT_URL, url.c_str());
    CCLOG("DevClient::perform: %s", url.c_str());
    
    long status = -1;
    CURLcode code = curl_easy_perform(_curl);
    if( code == CURLE_OK ){
        code = curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &status);
    }
    const bool result( code == CURLE_OK && status == 200 );
    if( result && successCallback ){
        successCallback();
    }
    
    _buffer.clear();
    return result;
}

bool DevClient::isAbsolutePath(const std::string& path) const
{
    return memcmp(path.c_str(), _hostName.c_str(), _hostName.length()) == 0;
}

std::string DevClient::fullPathForFilename(const std::string& path)
{
    std::string ret;
    perform(path, "file/exist/", [&](){
        ret = _hostName + path;
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
    if (isAbsolutePath(path))
    {
        perform(path.substr(_hostName.length(), std::string::npos), "file/data/", [&](){
            if( forString ){
                _buffer.push_back('\0');
            }
            void* data = malloc(_buffer.size());
            memcpy(data, _buffer.data(), _buffer.size());
            ret.fastSet((unsigned char*)data, _buffer.size());
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
    if (isAbsolutePath(path))
    {
        path = path.substr(_hostName.length(), std::string::npos);
    }
    
    std::vector<DevClient::FileStat> ret;
    perform(path, "file/list/", [&](){
        const char* data = _buffer.data();
        const char* p0 = data;
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
