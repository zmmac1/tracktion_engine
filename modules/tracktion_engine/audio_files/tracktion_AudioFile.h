/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

//==============================================================================
/**
*/
struct AudioFileInfo
{
    AudioFileInfo (Engine&);
    AudioFileInfo (const AudioFile&, yup::AudioFormatReader*, yup::AudioFormat*);

    static AudioFileInfo parse (const AudioFile&);

    Engine* engine = nullptr;
    bool wasParsedOk = false;
    HashCode hashCode = 0;
    yup::AudioFormat* format = nullptr;
    double sampleRate = 0;
    SampleCount lengthInSamples = 0;
    int numChannels = 0;
    int bitsPerSample = 0;
    bool isFloatingPoint = false;
    bool needsCachedProxy = false;
    yup::AudioChannelSet channelLayout;
    yup::StringPairArray metadata;
    yup::Time fileModificationTime;
    LoopInfo loopInfo;

    double getLengthInSeconds() const noexcept
    {
        if (sampleRate > 0)
            return static_cast<double> (lengthInSamples) / sampleRate;

        return 0.0;
    }

    yup::String getLongDescription() const;
};

//==============================================================================
/**
*/
class AudioFile
{
public:
    AudioFile() = delete;
    inline AudioFile (Engine& e) noexcept : engine (&e) {}
    explicit AudioFile (Engine&, const yup::File&) noexcept;
    AudioFile (const AudioFile&) noexcept;
    AudioFile& operator= (const AudioFile&) noexcept;
    ~AudioFile();

    const yup::File& getFile() const noexcept  { return file; }
    HashCode getHash() const noexcept           { return hash; }
    yup::String getHashString() const          { return yup::String::toHexString (hash); }

    inline bool operator== (const AudioFile& other) const noexcept      { return hash == other.hash; }
    inline bool operator!= (const AudioFile& other) const noexcept      { return hash != other.hash; }

    bool deleteFile() const;
    static bool deleteFiles (Engine&, const yup::Array<yup::File>& files);

    bool moveToTrash() const;

    //==============================================================================
    bool isNull() const noexcept                { return hash == 0; }
    bool isValid() const;

    AudioFileInfo getInfo() const;

    int64_t getLengthInSamples() const;
    double getLength() const;
    int getNumChannels() const;
    double getSampleRate() const;
    int getBitsPerSample() const;
    bool isFloatingPoint() const;
    yup::StringPairArray getMetadata() const;

    yup::AudioFormat* getFormat() const;

    bool isWavFile() const;
    bool isAiffFile() const;
    bool isOggFile() const;
    bool isMp3File() const;
    bool isFlacFile() const;
    bool isRexFile() const;

    Engine* engine = nullptr;

private:
    yup::File file;
    HashCode hash = 0;
};

} // namespace tracktion::inline engine
