/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

static int getFloatFileHeaderIntV1()  { return (int) yup::ByteOrder::littleEndianInt ("TRKF"); }
static int getFloatFileHeaderIntV2()  { return (int) yup::ByteOrder::littleEndianInt ("TF64"); }


//==============================================================================
class FloatAudioFormatReader  : public yup::AudioFormatReader
{
public:
    FloatAudioFormatReader (yup::InputStream* in)
        : AudioFormatReader (in, TRANS("Tracktion audio file"))
    {
        usesFloatingPointData = true;

        auto header = in->readInt();
        if (header == getFloatFileHeaderIntV1())
        {
            dataStartOffset = in->readInt();
            sampleRate      = in->readInt();
            lengthInSamples = in->readInt();
            numChannels     = (unsigned int) in->readShort();
            bigEndian       = in->readShort() != 0;
            bitsPerSample   = 32;

            if (sampleRate < 32000 || sampleRate > 192000 || numChannels < 1 || numChannels > 16)
                sampleRate = 0;
        }
        else if (header == getFloatFileHeaderIntV2())
        {
            dataStartOffset = in->readInt();
            sampleRate      = in->readInt();
            lengthInSamples = in->readInt64();
            numChannels     = (unsigned int) in->readShort();
            bigEndian       = in->readShort() != 0;
            bitsPerSample   = 32;

            if (sampleRate < 32000 || sampleRate > 192000 || numChannels < 1 || numChannels > 16)
                sampleRate = 0;
        }
    }

    bool readSamples (int* const* destSamples, int numDestChannels, int startOffsetInDestBuffer,
                      yup::int64 startSampleInFile, int numSamples) override
    {
        clearSamplesBeyondAvailableLength (destSamples, numDestChannels, startOffsetInDestBuffer,
                                           startSampleInFile, numSamples, lengthInSamples);

        if (numSamples <= 0)
            return true;

        input->setPosition (4 * startSampleInFile * numChannels + dataStartOffset);
        const int bytesPerFrame = 4 * (int) numChannels;

        while (numSamples > 0)
        {
            const int tempBufSize = 480 * 3 * 4; // (keep this a multiple of 3)
            char tempBuffer [tempBufSize];

            const int numThisTime = std::min (tempBufSize / bytesPerFrame, numSamples);
            const int bytesRead = input->read (tempBuffer, numThisTime * bytesPerFrame);

            if (bytesRead < numThisTime * bytesPerFrame)
            {
                jassert (bytesRead >= 0);
                std::memset (tempBuffer + bytesRead, 0, (size_t) (numThisTime * bytesPerFrame - bytesRead));
            }

            if (bigEndian)
                ReadHelper<yup::AudioData::Float32, yup::AudioData::Float32, yup::AudioData::BigEndian>
                    ::read (destSamples, startOffsetInDestBuffer, numDestChannels, tempBuffer, (int) numChannels, numThisTime);
            else
                ReadHelper<yup::AudioData::Float32, yup::AudioData::Float32, yup::AudioData::LittleEndian>
                    ::read (destSamples, startOffsetInDestBuffer, numDestChannels, tempBuffer, (int) numChannels, numThisTime);

            startOffsetInDestBuffer += numThisTime;
            numSamples -= numThisTime;
        }

        return true;
    }

    int dataStartOffset;
    bool bigEndian = false;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FloatAudioFormatReader)
};

//==============================================================================
class FloatAudioFormatWriter  : public yup::AudioFormatWriter
{
public:
    FloatAudioFormatWriter (yup::OutputStream* out, double sampleRate_, unsigned int numChannels_)
        : AudioFormatWriter (out,
                             TRANS("Tracktion audio file"),
                             sampleRate_,
                             numChannels_,
                             32),
          lengthInSamples (0)
    {
        usesFloatingPointData = true;
        writeHeader();
    }

    ~FloatAudioFormatWriter() override
    {
        output->setPosition (0);
        writeHeader();
    }

    //==============================================================================
    bool write (const int** data, int numSamps)
    {
        lengthInSamples += numSamps;

        for (int j = 0; j < numSamps; ++j)
        {
            for (unsigned int i = 0; i < numChannels; ++i)
            {
                if (const float* chan = (float*) (data[i]))
                {
                    float val = chan[j];
                    JUCE_UNDENORMALISE (val);
                    output->writeFloat (val);
                }
                else
                {
                    break;
                }
            }
        }

        return true;
    }

    bool flush() override
    {
        auto pos = output->getPosition();
        output->setPosition (0);
        writeHeader();
        output->setPosition (pos);

        return true;
    }

private:
    yup::int64 lengthInSamples;

    void writeHeader()
    {
        const int headerSize = 512;
        output->writeInt (getFloatFileHeaderIntV2());
        output->writeInt (headerSize);
        output->writeInt (yup::roundToInt (sampleRate));
        output->writeInt64 (lengthInSamples);
        output->writeShort ((short)numChannels);
        output->writeShort (0); // big-endian

        while (output->getPosition() < headerSize)
            output->writeByte (0);
    }

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FloatAudioFormatWriter)
};

//==============================================================================
class MemoryMappedFloatReader   : public yup::MemoryMappedAudioFormatReader
{
public:
    MemoryMappedFloatReader (const yup::File& f, const FloatAudioFormatReader& reader)
        : MemoryMappedAudioFormatReader (f, reader,
                                         reader.dataStartOffset,
                                         reader.lengthInSamples * 4 * reader.numChannels,
                                         4 * (int) reader.numChannels),
          bigEndian (reader.bigEndian)
    {
    }

    bool readSamples (int* const* destSamples, int numDestChannels, int startOffsetInDestBuffer,
                      yup::int64 startSampleInFile, int numSamples) override
    {
        clearSamplesBeyondAvailableLength (destSamples, numDestChannels, startOffsetInDestBuffer,
                                           startSampleInFile, numSamples, lengthInSamples);

        if (map == nullptr || ! mappedSection.contains ({ startSampleInFile, startSampleInFile + numSamples }))
        {
            jassertfalse; // you must make sure that the window contains all the samples you're going to attempt to read.
            return false;
        }

        if (bigEndian)
            ReadHelper<yup::AudioData::Float32, yup::AudioData::Float32, yup::AudioData::BigEndian>
                ::read (destSamples, startOffsetInDestBuffer, numDestChannels, sampleToPointer (startSampleInFile), (int) numChannels, numSamples);
        else
            ReadHelper<yup::AudioData::Float32, yup::AudioData::Float32, yup::AudioData::LittleEndian>
                ::read (destSamples, startOffsetInDestBuffer, numDestChannels, sampleToPointer (startSampleInFile), (int) numChannels, numSamples);

        return true;
    }

    void getSample (yup::int64 sample, float* result) const noexcept
    {
        if (map == nullptr || ! mappedSection.contains (sample))
        {
            jassertfalse; // you must make sure that the window contains all the samples you're going to attempt to read.

            std::memset (result, 0, sizeof (float) * numChannels);
            return;
        }

        const void* sourceData = sampleToPointer (sample);

        if (bigEndian)
            ReadHelper<yup::AudioData::Float32, yup::AudioData::Float32, yup::AudioData::BigEndian>
                ::read (&result, 0, 1, sourceData, 1, (int) numChannels);
        else
            ReadHelper<yup::AudioData::Float32, yup::AudioData::Float32, yup::AudioData::LittleEndian>
                ::read (&result, 0, 1, sourceData, 1, (int) numChannels);
    }

    using yup::MemoryMappedAudioFormatReader::readMaxLevels;

    void readMaxLevels (yup::int64 startSampleInFile, yup::int64 numSamples,
                        yup::Range<float>* results, int numChannelsToRead) override
    {
        if (numSamples <= 0)
        {
            for (int i = 0; i < numChannelsToRead; ++i)
                results[i] = {};

            return;
        }

        if (map == nullptr || ! mappedSection.contains ({ startSampleInFile, startSampleInFile + numSamples }))
        {
            jassertfalse; // you must make sure that the window contains all the samples you're going to attempt to read.

            for (int i = 0; i < numChannelsToRead; ++i)
                results[i] = {};

            return;
        }

        switch (bitsPerSample)
        {
            case 8:     scanMinAndMax<yup::AudioData::UInt8> (startSampleInFile, numSamples, results, numChannelsToRead); break;
            case 16:    scanMinAndMax<yup::AudioData::Int16> (startSampleInFile, numSamples, results, numChannelsToRead); break;
            case 24:    scanMinAndMax<yup::AudioData::Int24> (startSampleInFile, numSamples, results, numChannelsToRead); break;
            case 32:
            {
                if (usesFloatingPointData) scanMinAndMax<yup::AudioData::Float32> (startSampleInFile, numSamples, results, numChannelsToRead);
                else                       scanMinAndMax<yup::AudioData::Int32>   (startSampleInFile, numSamples, results, numChannelsToRead);
                break;
            }
            default:    jassertfalse; break;
        }
    }

private:
    const bool bigEndian;

    template <typename SampleType>
    void scanMinAndMax (int64_t startSampleInFile, int64_t numSamples, yup::Range<float>* results, int numChannelsToRead) const
    {
        typedef yup::AudioData::Pointer<SampleType, yup::AudioData::LittleEndian, yup::AudioData::Interleaved, yup::AudioData::Const> SourceType;

        for (int i = 0; i < numChannelsToRead; ++i)
            results[i] = SourceType (sampleToPointer (startSampleInFile), (int) numChannels).findMinAndMax ((size_t) numSamples);
    }

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MemoryMappedFloatReader)
};

//==============================================================================
FloatAudioFormat::FloatAudioFormat()  : AudioFormat ("Tracktion audio file", ".trkaudio") {}
FloatAudioFormat::~FloatAudioFormat() {}

yup::Array<int> FloatAudioFormat::getPossibleSampleRates() const     { return { 22050, 32000, 44100, 48000, 88200, 96000, 176400, 192000 }; }
yup::Array<int> FloatAudioFormat::getPossibleBitDepths() const       { return { 32 }; }

bool FloatAudioFormat::canDoStereo() const    { return true; }
bool FloatAudioFormat::canDoMono() const      { return true; }

bool FloatAudioFormat::canHandleFile (const yup::File& f)
{
    return f.hasFileExtension (".trkaudio")
       #if JUCE_WINDOWS
        || f.hasFileExtension ({})
       #endif
        || f.hasFileExtension (".freeze");
}

yup::AudioFormatReader* FloatAudioFormat::createReaderFor (yup::InputStream* in, bool deleteStreamIfOpeningFails)
{
    std::unique_ptr<FloatAudioFormatReader> r (new FloatAudioFormatReader (in));

    if (r->sampleRate > 0)
        return r.release();

    if (! deleteStreamIfOpeningFails)
        r->input = nullptr;

    return {};
}

yup::MemoryMappedAudioFormatReader* FloatAudioFormat::createMemoryMappedReader (const yup::File& file)
{
    if (auto fin = file.createInputStream())
    {
        FloatAudioFormatReader reader (fin.release());

        if (reader.lengthInSamples > 0)
            return new MemoryMappedFloatReader (file, reader);
    }

    return {};
}

std::unique_ptr<yup::AudioFormatWriter> FloatAudioFormat::createWriterFor (std::unique_ptr<yup::OutputStream>& out,
                                                                            const yup::WriteOptions& opts)
{
    return std::make_unique<FloatAudioFormatWriter> (out.release(), opts.getSampleRate(), opts.getNumChannels());
}

} // namespace tracktion::inline engine
