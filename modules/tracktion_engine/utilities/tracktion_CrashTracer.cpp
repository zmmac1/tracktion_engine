/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

struct CrashStackTracer::CrashTraceThreads
{
    CrashTraceThreads()
    {
        entries.ensureStorageAllocated (64);
    }

    inline void push (CrashStackTracer* c)
    {
        entries.add (c);
    }

    inline void pop (CrashStackTracer* c)
    {
        entries.removeFirstMatchingValue (c);
    }

    void dump() const
    {
      #if TRACKTION_LOG_ENABLED
        yup::Array<yup::Thread::ThreadID> threads;

        for (int i = entries.size(); --i >= 0;)
            threads.addIfNotAlreadyThere (entries.getUnchecked (i)->threadID);

        for (int j = 0; j < threads.size(); ++j)
        {
            TRACKTION_LOG ("Thread " + yup::String (j) + ":");

            auto thread = threads.getUnchecked(j);
            int n = 0;

            for (int i = entries.size(); --i >= 0;)
            {
                auto& s = *entries.getUnchecked (i);

                if (s.threadID == thread)
                {
                    if (s.pluginName != nullptr)
                        TRACKTION_LOG ("  ** Plugin crashed: " + yup::String (s.pluginName));

                    TRACKTION_LOG ("  " + yup::String (n++) + ": "
                                    + yup::File::createFileWithoutCheckingPath (s.file).getFileName()
                                    + ":" + yup::String (s.function) + ":" + yup::String (s.line));

                }
            }
        }
      #endif
    }

    void dump (yup::OutputStream& os, yup::Thread::ThreadID threadIDToDump) const
    {
        yup::Array<yup::Thread::ThreadID> threads;

        for (int i = entries.size(); --i >= 0;)
        {
            auto entryThreadID = entries.getUnchecked (i)->threadID;

            if (entryThreadID == threadIDToDump || entryThreadID == yup::Thread::ThreadID())
                threads.addIfNotAlreadyThere (entryThreadID);
        }

        for (int j = 0; j < threads.size(); ++j)
        {
            os.writeText ("Thread " + yup::String (j) + ":\n", false, false, nullptr);

            auto thread = threads.getUnchecked (j);
            int n = 0;

            for (int i = entries.size(); --i >= 0;)
            {
                auto& s = *entries.getUnchecked (i);

                if (s.threadID == thread)
                {
                    if (s.pluginName != nullptr)
                        os.writeText ("  ** Plugin crashed: " + yup::String (s.pluginName) + "\n", false, false, nullptr);

                    os.writeText ("  " + yup::String (n++) + ": "
                                   + yup::File::createFileWithoutCheckingPath (s.file).getFileName()
                                   + ":" + yup::String (s.function) + ":" + yup::String (s.line) + "\n", false, false, nullptr);

                }
            }
        }
    }

    yup::StringArray getCrashedPlugins() const
    {
        yup::StringArray plugins;

        for (auto s : entries)
            if (s->pluginName != nullptr)
                plugins.add (s->pluginName);

        return plugins;
    }

    yup::String getCrashedPlugin (yup::Thread::ThreadID thread)
    {
        for (int i = entries.size(); --i >= 0;)
        {
            auto& s = *entries.getUnchecked (i);

            if (s.threadID == thread)
                if (s.pluginName != nullptr)
                    return s.pluginName;
        }

        return {};
    }

    yup::String getCrashLocation (yup::Thread::ThreadID thread)
    {
        for (int i = entries.size(); --i >= 0;)
        {
            auto& s = *entries.getUnchecked (i);

            if (s.threadID == thread)
                return yup::File::createFileWithoutCheckingPath (s.file).getFileName()
                     + ":" + yup::String (s.function) + ":" + yup::String (s.line);
        }

        return "UnknownLocation";
    }

    yup::Array<CrashStackTracer*, yup::SpinLock, 100> entries;
};

static CrashStackTracer::CrashTraceThreads crashStack;

CrashStackTracer::CrashStackTracer (const char* f, const char* fn, int l, const char* plugin)
    : file (f), function (fn), pluginName (plugin), line (l), threadID (yup::Thread::getCurrentThreadId())
{
    crashStack.push (this);

   #if ! TRACKTION_LOG_ENABLED
    yup::ignoreUnused (file, function, pluginName, line);
   #endif
}

CrashStackTracer::~CrashStackTracer()
{
    crashStack.pop (this);
}

yup::StringArray CrashStackTracer::getCrashedPlugins()
{
    return crashStack.getCrashedPlugins();
}

void CrashStackTracer::dump()
{
    TRACKTION_LOG ("Crashed");
    TRACKTION_LOG (yup::newLine);
    crashStack.dump();
}

void CrashStackTracer::dump (yup::OutputStream& os)
{
    dump (os, yup::Thread::ThreadID());
}

void CrashStackTracer::dump (yup::OutputStream& os, yup::Thread::ThreadID threadID)
{
    os.writeText ("Crashed", false, false, nullptr);
    os.writeText (yup::newLine, false, false, nullptr);
    crashStack.dump (os, threadID);
}

yup::String CrashStackTracer::getCrashedPlugin (yup::Thread::ThreadID threadID)
{
    return crashStack.getCrashedPlugin (threadID);
}

yup::String CrashStackTracer::getCrashLocation (yup::Thread::ThreadID threadID)
{
    return crashStack.getCrashLocation (threadID);
}

//==============================================================================
DeadMansPedalMessage::DeadMansPedalMessage (PropertyStorage& ps, const yup::String& message)
    : file (getDeadMansPedalFile (ps))
{
    file.replaceWithText (message);
}

DeadMansPedalMessage::~DeadMansPedalMessage()
{
    auto success = file.deleteFile();
    yup::ignoreUnused (success);
    jassert (success);
}

yup::String DeadMansPedalMessage::getAndClearLastMessage (PropertyStorage& propertyStorage)
{
    auto file = getDeadMansPedalFile (propertyStorage);
    auto s = file.loadFileAsString();

    if (s.isNotEmpty())
    {
        file.deleteFile();
        TRACKTION_LOG (s);
    }

    return s;
}

yup::File DeadMansPedalMessage::getDeadMansPedalFile (PropertyStorage& propertyStorage)
{
    auto folder = propertyStorage.getAppPrefsFolder();
    jassert (folder.exists() && folder.hasWriteAccess());
    return folder.getChildFile ("deadMansPedal");
}

} // namespace tracktion::inline engine
