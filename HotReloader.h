/*
  ==============================================================================

    HotReloader.h
    Created: 31 Jul 2022 9:26:52am
    Author:  migizo

  ==============================================================================
*/

#pragma once
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

class HotReloader : private juce::Timer, private juce::AsyncUpdater
{
private:
  using CallbackType = std::function<void(const juce::File&)>;

public:
    HotReloader(const juce::File& _targetFile,  //! 監視対象のファイルもしくはディレクトリ
                CallbackType _callback,         //! ファイルの更新あった場合に呼ばれるコールバック
                int _checkIntervalMs = 500,     //! 監視する時間間隔ミリ秒
                bool _isResursive = true,       //! 監視対象がディレクトリであった場合に再起的に子フォルダ,ファイルを確認するか
                bool _enableInit = true)        //! startTimer()で初回のコールバックが呼ばれるまでタイムラグがあるのでコンストラクタでも呼ぶか
        : targetFile(_targetFile),
          reloadCallback(_callback),
          isRecursive(_isResursive)
    {
        jassert (reloadCallback != nullptr);
        jassert (targetFile.exists());

        if (_enableInit) checkAndReloadCallback(targetFile);
        startTimer(_checkIntervalMs);
    }

    ~HotReloader()
    {
        stopTimer();
    }

private:
    
    void checkAndReloadCallback(const juce::File& _file)
    {
        juce::Time tmpLastModTime = _file.getLastModificationTime();
        jassert(tmpLastModTime != juce::Time());
        
        if (_file.isDirectory() && isRecursive)
        {
            juce::Array<juce::File> fileList = _file.findChildFiles(juce::File::TypesOfFileToFind::findFilesAndDirectories, false);
            for (const auto& f: fileList) checkAndReloadCallback(f);
        }
        else if (lastModTime < tmpLastModTime)
        {
            lastModTime = tmpLastModTime;
            triggerAsyncUpdate();
        }
    }
    
    void timerCallback() override
    {
        checkAndReloadCallback(targetFile);
    }

    void handleAsyncUpdate() override
    {
        reloadCallback(targetFile);
    }
    
    juce::File targetFile;
    CallbackType reloadCallback;
    juce::Time lastModTime;
    bool isRecursive;
};
