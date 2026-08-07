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
    Smart wrapper for writing to an audio file.

    Internally this opens a File for writng and provides some helper methods to
    append to it and free the file handle when done.
*/
class AudioFileWriter   : public yup::ReferenceCountedObject
{
public:
    AudioFileWriter (const AudioFile& file,
                     yup::AudioFormat* formatToUse,
                     int numChannels,
                     double sampleRate,
                     int bitsPerSample,
                     const yup::StringPairArray& metadata,
                     int quality);

    AudioFileWriter (const AudioFile& file,
                     yup::AudioFormat* formatToUse,
                     int numChannels,
                     double sampleRate,
                     int bitsPerSample,
                     const yup::StringPairArray& metadata,
                     int quality,
                     const yup::AudioChannelSet& channelLayout);

    /** Destructor, calls closeForWriting. */
    ~AudioFileWriter();

    //==============================================================================
    /** Returns true if the file is open and ready to write to. */
    bool isOpen() const noexcept;

    /** Returns the sample rate of the writer, should only be called on an open writer. */
    double getSampleRate() const noexcept;

    /** Returns the num channels of the writer, should only be called on an open writer. */
    int getNumChannels() const noexcept;

    //==============================================================================
    /** Appends an AudioBuffer to the file. */
    bool appendBuffer (yup::AudioBuffer<float>& buffer, int numSamples);

    /** Appends an block of samples to the file. */
    bool appendBuffer (const int** buffer, int numSamples);

    /** Appends a block of samples to the file from an audio format reader. */
    bool writeFromAudioReader (yup::AudioFormatReader&, SampleCount startSample, SampleCount numSamples);

    /** Deletes the writer and releases the file handle. */
    void closeForWriting();

    AudioFile file;

private:
    int samplesUntilFlush;
    std::unique_ptr<yup::AudioFormatWriter> writer;
    yup::CriticalSection writerLock;
};

} // namespace tracktion::inline engine
