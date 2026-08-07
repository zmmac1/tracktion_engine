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
struct AudioFileUtils
{
    static yup::AudioFormatReader* createReaderFor (Engine&, const yup::File&);
    static yup::AudioFormatReader* createReaderFindingFormat (Engine&, const yup::File&, yup::AudioFormat*&);
    static yup::MemoryMappedAudioFormatReader* createMemoryMappedReader (Engine&, const yup::File&, yup::AudioFormat*&);

    struct MappedFileAndReader
    {
        std::unique_ptr<yup::MemoryMappedFile> mappedFile;
        std::unique_ptr<yup::AudioFormatReader> reader;
    };

    static std::unique_ptr<MappedFileAndReader> createMappedFileAndReaderFor (Engine&, const yup::File&);

    static yup::AudioFormatWriter* createWriterFor (Engine&, const yup::File&,
                                                     double sampleRate, unsigned int numChannels, int bitsPerSample,
                                                     const yup::StringPairArray& metadata, int quality);

    static yup::AudioFormatWriter* createWriterFor (yup::AudioFormat*, const yup::File&,
                                                     double sampleRate, unsigned int numChannels, int bitsPerSample,
                                                     const yup::StringPairArray& metadata, int quality);

    static yup::AudioFormatWriter* createWriterFor (yup::AudioFormat*, const yup::File&,
                                                     double sampleRate, unsigned int numChannels, int bitsPerSample,
                                                     const yup::StringPairArray& metadata, int quality,
                                                     const yup::AudioChannelSet& channelLayout);

    static SampleRange scanForNonZeroSamples (Engine&, const yup::File&, float maxZeroLevelDb);

    static SampleRange copyNonSilentSectionToNewFile (Engine&,
                                                      const yup::File& sourceFile,
                                                      const yup::File& destFile,
                                                      float maxZeroLevelDb);

    static SampleRange trimSilence (Engine&, const yup::File&, float maxZeroLevelDb);

    /** Reverses a file updating a progress value and checking the exit status of a given job. */
    static bool reverse (Engine&, const yup::File& source, const yup::File& destination,
                         std::atomic<float>& progress, yup::ThreadPoolJob* job = nullptr,
                         bool canCreateWavIntermediate = true);

    // returns length of file created, or -1
    static SampleCount copySectionToNewFile (Engine& e,
                                             const yup::File& sourceFile,
                                             const yup::File& destFile,
                                             SampleRange range);

    static SampleCount copySectionToNewFile (Engine& e,
                                             const yup::File& sourceFile,
                                             const yup::File& destFile,
                                             TimeRange range);

    static void addBWAVStartToMetadata (yup::StringPairArray& metadata, SampleCount start);

    static void applyBWAVStartTime (const yup::File&, SampleCount start);

    static SampleCount getFileLengthSamples (Engine& e, const yup::File&);

    //==============================================================================
    template<class TargetFormat>
    static bool convertToFormat (Engine& e, const yup::File& sourceFile, yup::OutputStream& destStream,
                                 int quality, const yup::StringPairArray& metadata,
                                 yup::int64 blockSize = -1,
                                 std::function<bool()> continuePredicate = {},
                                 std::function<void (float)> updateProgress = {})
    {
        std::unique_ptr<yup::AudioFormatReader> reader (createReaderFor (e, sourceFile));
        return convertToFormat<TargetFormat> (reader.get(), destStream, quality, metadata, blockSize,
                                              continuePredicate, updateProgress);
    }

    template<class TargetFormat>
    static bool convertToFormat (yup::AudioFormatReader* reader,
                                 yup::OutputStream& destStream,
                                 int quality, const yup::StringPairArray& metadata,
                                 yup::int64 blockSize = -1,
                                 std::function<bool()> continuePredicate = {},
                                 std::function<void (float)> updateProgress = {})
    {
        if (reader != nullptr)
        {
            // NB: must write to a temp file because the archiver relies on the destStream's position
            // being left pointing to the end of the data that was written, whereas some formats
            // may leave the position set elsewhere.
            yup::TemporaryFile tempFile;

            if (auto out = std::unique_ptr<yup::OutputStream> (tempFile.getFile().createOutputStream()))
            {
                TargetFormat format;

                std::unordered_map<yup::String, yup::String> metadataMap;

                for (const auto& key : metadata.getAllKeys())
                    metadataMap[key] = metadata[key];

                if (auto writer = std::unique_ptr<yup::AudioFormatWriter> (format.createWriterFor (out,
                                                                                                    yup::WriteOptions().withSampleRate (reader->sampleRate)
                                                                                                                                    .withNumChannels (static_cast<int> (reader->numChannels))
                                                                                                                                    .withBitsPerSample (static_cast<int> (reader->bitsPerSample))
                                                                                                                                    .withMetadataValues (metadataMap)
                                                                                                                                    .withQualityOptionIndex (quality))))
                {
                    out.release();

                    if (blockSize < 0)
                    {
                        if (writer->writeFromAudioReader (*reader, 0, -1))
                        {
                            writer = nullptr;
                            destStream << tempFile.getFile();
                            return true;
                        }
                    }
                    else
                    {
                        yup::int64 numLeft = reader->lengthInSamples;

                        if (numLeft <= 0)
                            return false;

                        for (yup::int64 startSample = 0;;)
                        {
                            if (continuePredicate && ! continuePredicate())
                                return false;

                            const yup::int64 numThisTime = std::min (blockSize, numLeft);

                            if (! writer->writeFromAudioReader (*reader, startSample, numThisTime))
                                return false;

                            startSample += numThisTime;
                            numLeft -= numThisTime;

                            if (updateProgress)
                                updateProgress (static_cast<float> (startSample) / static_cast<float> (reader->lengthInSamples));

                            if (numLeft <= 0)
                            {
                                writer = nullptr;
                                destStream << tempFile.getFile();
                                return true;
                            }
                        }
                    }
                }
            }
            else
            {
                jassertfalse;
            }
        }

        return false;
    }

    //==============================================================================
    template <typename SourceFormat>
    inline static bool readFromFormat (Engine& engine, yup::InputStream& source, const yup::File& dest)
    {
        struct ForwardingInputStream  : public yup::InputStream
        {
            ForwardingInputStream (yup::InputStream& s)  : stream (s) {}

            yup::int64 getTotalLength() override        { return stream.getTotalLength(); }
            bool isExhausted() override                  { return stream.isExhausted(); }
            int read (void* d, int sz) override          { return stream.read (d, sz); }
            yup::int64 getPosition() override           { return stream.getPosition(); }
            bool setPosition (yup::int64 pos) override  { return stream.setPosition (pos); }

            InputStream& stream;
        };

        SourceFormat sourceFormat;

        if (auto reader = std::unique_ptr<yup::AudioFormatReader> (sourceFormat.createReaderFor (new ForwardingInputStream (source), true)))
        {
            auto& manager = engine.getAudioFileFormatManager().writeFormatManager;

            auto* destFormat = manager.findFormatForFileExtension (dest.getFileExtension());

            if (destFormat == nullptr)
                destFormat = manager.findFormatForFileExtension ("wav");

            AudioFileWriter writer (AudioFile (engine, dest), destFormat,
                                    (int) reader->numChannels,
                                    reader->sampleRate,
                                    (int) reader->bitsPerSample,
                                    reader->metadataValues, 0);

            if (writer.isOpen() && writer.writeFromAudioReader (*reader, 0, -1))
                return true;
        }

        jassertfalse;
        return false;
    }

    //==============================================================================
    class EnvelopeFollower
    {
    public:
        EnvelopeFollower() {}

        void processEnvelope (const float* inputBuffer, float* outputBuffer, int numSamples) noexcept
        {
            for (int i = 0; i < numSamples; ++i)
            {
                float envIn = std::abs (inputBuffer[i]);

                if (envelope < envIn)
                    envelope += envAttack * (envIn - envelope);
                else if (envelope > envIn)
                    envelope -= envRelease * (envelope - envIn);

                outputBuffer[i] = envelope;
            }
        }

        /** Sets the times for the vaious stages of the envelope.
            1 is an instant attack/release, 0 will never change the value.
        */
        void setCoefficients (float attack, float release) noexcept
        {
            envAttack = attack;
            envRelease = release;
        }

    private:
        float envelope = 0.0f;
        float envAttack = 1.0f, envRelease = 1.0f;

        YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnvelopeFollower)
    };
};

} // namespace tracktion::inline engine
