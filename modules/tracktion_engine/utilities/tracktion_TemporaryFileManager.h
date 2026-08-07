/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

/**
    Holds info about where temp files should go, and tidies up old ones when needed.

    You shouldn't have to ever create your own instance of this class - the
    Engine has a TemporaryFileManager object that is shared.

    @see Engine::getTemporaryFileManager()
*/
class TemporaryFileManager
{
public:
    //==============================================================================
    /** You shouldn't have to ever create your own instance of this class - the
        Engine itself has a TemporaryFileManager that is shared.
    */
    TemporaryFileManager (Engine&);

    ~TemporaryFileManager();

    //==============================================================================
    /** */
    bool wasTempFolderSuccessfullyCreated() const;

    /** */
    bool isDiskSpaceDangerouslyLow() const;

    /** */
    int64_t getMaxSpaceAllowedForTempFiles() const;

    /** */
    int getMaxNumTempFiles() const;

    /** */
    void cleanUp();

    /** */
    const yup::File& getTempDirectory() const;

    /** */
    bool setTempDirectory (const yup::File& newFile);

    /** */
    void ressetToDefaultLocation();

    /** */
    yup::File getThumbnailsFolder() const;

    /** */
    yup::File getTempFile (const yup::String& filename) const;

    /** */
    yup::File getUniqueTempFile (const yup::String& prefix, const yup::String& ext) const;

    //==============================================================================
    /** */
    static AudioFile getFileForCachedClipRender (const AudioClipBase&, HashCode);

    /** */
    static AudioFile getFileForCachedCompRender (const AudioClipBase&, HashCode);

    /** */
    static AudioFile getFileForCachedFileRender (Edit&, HashCode hash);

    /** */
    static yup::File getFreezeFileForDevice (Edit&, OutputDevice&);

    /** */
    static yup::String getDeviceIDFromFreezeFile (Edit&, const yup::File& deviceFreezeFile);

    /** */
    static yup::File getFreezeFileForTrack (const AudioTrack&);

    /** */
    static yup::Array<yup::File> getFrozenTrackFiles (Edit&);

    /** */
    static void purgeOrphanFreezeAndProxyFiles (Edit&);

    /** */
    void purgeOrphanEditTempFolders (ProjectManager&);

    //==============================================================================
private:
    Engine& engine;
    yup::File tempDir;

    void updateDir();

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TemporaryFileManager)
};

} // namespace tracktion::inline engine
