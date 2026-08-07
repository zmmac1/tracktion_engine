/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {
namespace legacy {

//==============================================================================
/**
*/
class TracktionArchiveFile
{
public:
    TracktionArchiveFile (Engine&, const yup::File& file);
    ~TracktionArchiveFile();

    bool isValidArchive() const;
    yup::File getFile() const;

    enum class CompressionType
    {
        none                    = 0,
        zip                     = 1,
        lossless                = 2,
        lossyGoodQuality        = 3,
        lossyMediumQuality      = 4,
        lossyLowQuality         = 5
    };

    int getNumFiles() const;
    yup::String getOriginalFileName (int index) const;

    int indexOfFile (const yup::String& name) const;

    /** Create a stream to read one of the archived objects. */
    std::unique_ptr<yup::InputStream> createStoredInputStream (int index) const;

    bool extractFile (int index, const yup::File& destDirectory,
                      yup::File& fileCreated, bool askBeforeOverwriting,
                      std::function<void (bool)> callback = nullptr);
    bool extractAll (const yup::File& destDirectory,
                     yup::Array<yup::File>& filesCreated);
    bool extractAllAsTask (const yup::File& destDirectory,
                           bool warnAboutOverwrite,
                           yup::Array<yup::File>& filesCreated,
                           bool& wasAborted);

    bool addFile (const yup::File&, const yup::File& rootDirectory, CompressionType);
    bool addFile (const yup::File&, const yup::String& filenameToUse, CompressionType);

    void addFileInfo (const yup::String& filename,
                      const yup::String& itemName,
                      const yup::String& itemValue);

    void flush();

public:
    //==============================================================================
    struct IndexEntry
    {
        IndexEntry() {}
        IndexEntry (yup::InputStream&);
        ~IndexEntry();

        void write (yup::OutputStream&);

        // original name - name of the file that went in
        // stored name - name of the compressed file that's stored
        // length = length of the stored object
        // offset - offset from start of file of the stored object
        yup::String originalName, storedName;
        int64_t offset = 0, length = 0;
        // extra names and values - for extra metadata
        yup::StringArray extraNames, extraValues;

        YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IndexEntry)
    };

private:
    Engine& engine;
    yup::File file;
    int64_t indexOffset = 8;
    bool valid = false, needToWriteIndex;

    yup::OwnedArray<IndexEntry> entries;
    void readIndex();

    static int getOggQuality (CompressionType);
    static int getMagicNumber();

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TracktionArchiveFile)
};

} // namespace legacy
} // namespace tracktion::inline engine
