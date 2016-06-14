/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/CCDevTool
 ****************************************************************************/
#ifndef __CCDEVCLIENT_H__
#define __CCDEVCLIENT_H__

#include "cocos2d.h"
#include "cocos-ext.h"
#include <unordered_map>


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
     * Set the host name.
     */
    void setHostName(const std::string& hostname);
    
    /**
     * Get the host name.
     */
    const std::string& getHostName() const;
    
    /**
     * Set the callback with changed host.
     */
    void setHostChangedCallback(std::function<void()> callback);
    
    /**
     *  Checks whether the path is an absolute path.
     */
    bool isAbsolutePath(const std::string& path) const;
    
    /**
     *  Checks whether a file exists.
     */
    bool isFileExist(const std::string& path);
    
    /**
     *  Returns the fullpath for a given filename.
     */
    std::string fullPathForFilename(std::string filename);
    
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
    
    /**
     *
     */
    bool isIgnoreTarget(const std::string& path) const;
    
private:
    static DevClient* s_sharedDevClient;
    
    void* _curl;
    std::vector<char> _buffer;
    std::string _hostName;
    std::vector<std::string> _ignoreExtensionList;
    std::unordered_map<std::string, std::string> _fullPathCache;
    std::function<void()> _hostChangedCallback;
};

NS_CC_EXT_END

#endif