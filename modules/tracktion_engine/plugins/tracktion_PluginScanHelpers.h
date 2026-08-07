/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

#pragma once

namespace tracktion::inline engine {

struct PluginScanHelpers
{

static constexpr const char* commandLineUID = "PluginScan";

struct CustomScanner;

//==============================================================================
struct PluginScanMasterProcess  : private yup::ChildProcessCoordinator
{
    PluginScanMasterProcess (Engine& e) : engine (e) {}

    bool ensureChildProcessLaunched()
    {
        if (launched)
            return true;

        crashed = false;
        // don't get stdout or strerr from the child process. We don't do anything with it and it fills up the pipe and hangs
        launched = launchWorkerProcess (yup::File::getSpecialLocation (yup::File::currentExecutableFile),
                                        commandLineUID, 0, 0);

        if (launched)
        {
            TRACKTION_LOG ("----- Launched Plugin Scan Process");
        }
        else
        {
            TRACKTION_LOG_ERROR ("Failed to launch child process");
            showVirusCheckerWarning();
        }

        return launched;
    }

    bool sendScanRequest (yup::AudioPluginFormat& format,
                          const yup::String& fileOrIdentifier, int requestID)
    {
        yup::XmlElement m ("SCAN");
        m.setAttribute ("id", requestID);
        m.setAttribute ("type", format.getName());
        m.setAttribute ("file", fileOrIdentifier);

        return sendMessageToWorker (createScanMessage (m));
    }

    bool waitForReply (int requestID, const yup::String& fileOrIdentifier,
                       yup::OwnedArray<yup::PluginDescription>& result,
                       CustomScanner& scanner)
    {
      #if ! TRACKTION_LOG_ENABLED
        yup::ignoreUnused (fileOrIdentifier);
      #endif

        auto start = yup::Time::getCurrentTime();

        for (;;)
        {
            auto reply = findReply (requestID);

            auto elapsed = yup::Time::getCurrentTime() - start;

            if (reply == nullptr || ! reply->hasTagName ("FOUND"))
            {
                if (crashed)
                {
                    TRACKTION_LOG_ERROR ("Plugin crashed:  " + fileOrIdentifier);
                    return false;
                }

                if (scanner.shouldAbortScan() || ! launched)
                {
                    TRACKTION_LOG ("Plugin scan cancelled");
                    return false;
                }

                yup::Thread::sleep (10);
                continue;
            }

            if (reply->getNumChildElements() == 0)
                TRACKTION_LOG ("No plugins found in: " + fileOrIdentifier);

            for (auto e : reply->getChildIterator())
            {
                yup::PluginDescription desc;

                if (desc.loadFromXml (*e))
                {
                    auto newDesc = new yup::PluginDescription (desc);
                    newDesc->lastInfoUpdateTime = yup::Time::getCurrentTime().toMilliseconds();
                    result.add (newDesc);

                    TRACKTION_LOG ("Added " + desc.name + ": " + desc.name + "  [" + elapsed.getDescription() + "]");
                }
                else
                {
                    jassertfalse;
                }
            }

            return true;
        }
    }

    void handleMessage (const yup::XmlElement& xml)
    {
        if (xml.hasTagName ("FOUND"))
        {
            const yup::ScopedLock sl (replyLock);
            replies.add (new yup::XmlElement (xml));
        }
    }

    void handleConnectionLost() override
    {
        crashed = true;
    }

    volatile bool launched = false, crashed = false;

private:
    Engine& engine;
    yup::OwnedArray<yup::XmlElement> replies;
    yup::CriticalSection replyLock;
    bool hasShownVirusCheckerWarning = false;

    void showVirusCheckerWarning()
    {
        if (! hasShownVirusCheckerWarning)
        {
            hasShownVirusCheckerWarning = true;

            engine
                .getUIBehaviour().showWarningAlert ("Plugin Scanning...",
                                                    TRANS("There are some problems in launching a child-process to scan for plugins.")
                                                       + "\n\n"
                                                       + TRANS("If you have a virus-checker or firewall running, you may need to temporarily disable it for the scan to work correctly."));
        }
    }

    std::unique_ptr<yup::XmlElement> findReply (int requestID)
    {
        for (int i = replies.size(); --i >= 0;)
            if (replies.getUnchecked(i)->getIntAttribute ("id") == requestID)
                return std::unique_ptr<yup::XmlElement> (replies.removeAndReturn (i));

        return {};
    }

    void handleMessageFromWorker (const yup::MemoryBlock& mb) override
    {
        if (auto xml = yup::parseXML (mb.toString()))
            handleMessage (*xml);
    }

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginScanMasterProcess)
};

//==============================================================================
struct PluginScanChildProcess  : public yup::ChildProcessWorker,
                                 private yup::AsyncUpdater
{
    PluginScanChildProcess()
    {
        yup::addDefaultFormatsToManager (pluginFormatManager);
    }

    void handleConnectionMade() override {}

    void handleConnectionLost() override
    {
        std::exit (0);
    }

    void handleScanMessage (int requestID, const yup::String& formatName, const yup::String& fileOrIdentifier)
    {
        yup::XmlElement result ("FOUND");
        result.setAttribute ("id", requestID);

        for (int i = 0; i < pluginFormatManager.getNumFormats(); ++i)
        {
            auto format = pluginFormatManager.getFormat (i);

            if (format->getName() == formatName)
            {
                yup::OwnedArray<yup::PluginDescription> found;
                format->findAllTypesForFile (found, fileOrIdentifier);

                for (auto pd : found)
                    result.addChildElement (pd->createXml().release());

                break;
            }
        }

        sendMessageToCoordinator (createScanMessage (result));
    }

    void handleMessage (const yup::XmlElement& xml)
    {
        if (xml.hasTagName ("SCAN"))
            handleScanMessage (xml.getIntAttribute ("id"),
                               xml.getStringAttribute ("type"),
                               xml.getStringAttribute ("file"));
    }

private:
    yup::AudioPluginFormatManager pluginFormatManager;
    yup::OwnedArray<yup::XmlElement, yup::CriticalSection> pendingMessages;

    void handleMessageFromCoordinator (const yup::MemoryBlock& mb) override
    {
        if (auto xml = yup::parseXML (mb.toString()))
        {
            pendingMessages.add (xml.release());
            triggerAsyncUpdate();
        }
    }

    void handleMessageSafely (const yup::XmlElement& m)
    {
       #if JUCE_WINDOWS
        __try
        {
       #endif

            handleMessage (m);

       #if JUCE_WINDOWS
        }
        __except (1)
        {
            yup::Process::terminate();
        }
       #endif
    }

    void handleAsyncUpdate() override
    {
        while (pendingMessages.size() > 0)
            if (auto xml = pendingMessages.removeAndReturn (0))
                handleMessageSafely (*xml);
    }

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginScanChildProcess)
};

//==============================================================================
struct CustomScanner  : public yup::KnownPluginList::CustomScanner
{
    CustomScanner (Engine& e) : engine (e) {}

    bool findPluginTypesFor (yup::AudioPluginFormat& format,
                             yup::OwnedArray<yup::PluginDescription>& result,
                             const yup::String& fileOrIdentifier) override
    {
        CRASH_TRACER

        if (engine.getPluginManager().usesSeparateProcessForScanning()
            && shouldUseSeparateProcessToScan (format, fileOrIdentifier))
        {
            if (masterProcess != nullptr && masterProcess->crashed)
                masterProcess = nullptr;

            if (masterProcess == nullptr)
                masterProcess = std::make_unique<PluginScanMasterProcess> (engine);

            if (masterProcess->ensureChildProcessLaunched())
            {
                auto requestID = yup::Random().nextInt();

                if (! shouldAbortScan()
                     && masterProcess->sendScanRequest (format, fileOrIdentifier, requestID)
                     && ! shouldAbortScan())
                {
                    if (masterProcess->waitForReply (requestID, fileOrIdentifier, result, *this))
                        return true;

                    // if there's a crash, give it a second chance with a fresh child process,
                    // in case the real culprit was whatever plugin preceded this one.
                    if (masterProcess != nullptr && masterProcess->crashed && ! shouldAbortScan())
                    {
                        masterProcess.reset();
                        masterProcess = std::make_unique<PluginScanMasterProcess> (engine);

                        return masterProcess->ensureChildProcessLaunched()
                                 && ! shouldAbortScan()
                                 && masterProcess->sendScanRequest (format, fileOrIdentifier, requestID)
                                 && ! shouldAbortScan()
                                 && masterProcess->waitForReply (requestID, fileOrIdentifier, result, *this);
                    }
                }

                return false;
            }

            // panic! Can't run the child for some reason, so just do it here..
            TRACKTION_LOG_ERROR ("Falling back to scanning in main process..");
            masterProcess.reset();
        }

        format.findAllTypesForFile (result, fileOrIdentifier);
        return true;
    }

    static bool shouldUseSeparateProcessToScan (yup::AudioPluginFormat& format, const yup::String fileOrIdentifier)
    {
        auto name = format.getName();

        // AUv3s should be scanned in the same process but on a background thread as the child process scans in the main thread
        if (name.containsIgnoreCase ("AudioUnit"))
            return ! requiresUnblockedMessageThread (format, fileOrIdentifier);

        return name.containsIgnoreCase ("VST")
                || name.containsIgnoreCase ("LADSPA");
    }

    static bool requiresUnblockedMessageThread (yup::AudioPluginFormat& format, const yup::String fileOrIdentifier)
    {
        yup::PluginDescription desc;
        desc.fileOrIdentifier = fileOrIdentifier;
        desc.deprecatedUid = 0;
        desc.uniqueId = {};

        return format.requiresUnblockedMessageThreadDuringCreation (desc);
    }

    void scanFinished() override
    {
        TRACKTION_LOG ("----- Ended Plugin Scan");
        abortScan = false;
        masterProcess.reset();

        if (auto callback = engine.getPluginManager().scanCompletedCallback)
            callback();
    }

    void cancelScan()
    {
        abortScan = true;
    }

    bool shouldAbortScan() const
    {
        return abortScan || shouldExit();
    }

    Engine& engine;
    std::unique_ptr<PluginScanMasterProcess> masterProcess;
    std::atomic<bool> abortScan { false };
};


static yup::MemoryBlock createScanMessage (const yup::XmlElement& xml)
{
    yup::MemoryOutputStream mo;
    xml.writeTo (mo, yup::XmlElement::TextFormat().withoutHeader().singleLine());
    return mo.getMemoryBlock();
}

};

} // namespace tracktion::inline engine
