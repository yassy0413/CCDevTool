/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#ifndef __CCDEVCLIENT_H__
#define __CCDEVCLIENT_H__

#include "cocos2d.h"
#include "cocos-ext.h"

NS_CC_EXT_BEGIN

/**
 * ネットワーク開発環境
 */
class DevClient : public Ref
{
public:
    static const char* KEY_HOSTNAME;
    
    /**
     *  Gets the instance of DevClient.
     */
    static DevClient* getInstance();
    
    /**
     *  Destroys the instance of DevClient.
     */
    static void destroyInstance();
    
    /**
     * Set host name.
     */
    void setHostName(const std::string& hostname);
    
    /**
     *  Checks whether the path is an absolute path.
     */
    bool isAbsolutePath(const std::string& path) const;
    
    /**
     *  Returns the fullpath for a given filename.
     */
    std::string fullPathForFilename(const std::string &filename);
    
    /**
     *  Creates file data from a file.
     *  @return A data object.
     */
    cocos2d::Data getData(std::string path, bool forString);
    
    /**
     *  Get file list.
     */
    struct FileStat {
        bool isDir;
        std::string path;
        FileStat(bool d, const char* p);
    };
    std::vector<FileStat> getList(std::string path);
    
    
CC_CONSTRUCTOR_ACCESS:
    /**
     * The default constructor.
     */
    DevClient();
    
    /**
     * The destructor of DevClient.
     */
    virtual ~DevClient();
    
    virtual bool init();
    
    /**
     *
     */
    bool perform(const std::string& path, const std::string& api, std::function<void()> successCallback);
    
private:
    static DevClient* s_sharedDevClient;
    
    void* _curl;
    std::vector<char> _buffer;
    std::string _hostName;
};

NS_CC_EXT_END

#endif