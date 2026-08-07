/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

static inline HashCode getAudioFileHash (const yup::File& file) noexcept
{
    return file.getFullPathName().hashCode64();
}

//==============================================================================
AudioFile::AudioFile (Engine& e, const yup::File& f) noexcept
    : engine (&e), file (f), hash (getAudioFileHash (f))
{
}

AudioFile::AudioFile (const AudioFile& other) noexcept
    : engine (other.engine), file (other.file), hash (other.hash)
{
}

AudioFile& AudioFile::operator= (const AudioFile& other) noexcept
{
    file = other.file;
    hash = other.hash;
    return *this;
}

AudioFile::~AudioFile() {}

AudioFileInfo AudioFile::getInfo() const
{
    CRASH_TRACER

    if (file == yup::File())
        return AudioFileInfo (*engine);

    return engine->getAudioFileManager().getInfo (*this);
}

yup::String AudioFileInfo::getLongDescription() const
{
    yup::String desc;

    if (sampleRate > 0.0)
    {
        desc << yup::String ((sampleRate / 1000.0) + 0.0001, 1) << " kHz, ";

        if (bitsPerSample > 0) desc << bitsPerSample << " bit ";

        desc << (numChannels == 1 ? TRANS("mono") : TRANS("stereo"))
             << ", " << yup::RelativeTime (getLengthInSeconds()).getDescription();

        yup::StringArray items;

        const int numBeats = yup::roundToInt (loopInfo.getNumBeats());
        if (numBeats == 1)
            items.add (TRANS("1 beat"));
        else if (numBeats > 1)
            items.add (TRANS("123 beats").replace ("123", yup::String (numBeats)));

        if (loopInfo.isLoopable())
        {
            double bpm = loopInfo.getNumBeats() / (getLengthInSeconds() / 60.0);
            items.add (yup::String (bpm, 1) + " bpm");
        }

        if (loopInfo.getNumerator() && loopInfo.getDenominator())
            items.add (yup::String (loopInfo.getNumerator()) + "/" + yup::String (loopInfo.getDenominator()));

        if (loopInfo.getRootNote() != -1)
            items.add (yup::MidiMessage::getMidiNoteName (loopInfo.getRootNote(), true, true,
                                                           engine->getEngineBehaviour().getMiddleCOctave()));

        if (items.size() > 0)
            desc << "\n" << items.joinIntoString (", ");
    }

    return desc;
}

bool AudioFile::isValid() const                         { return hash != 0 && getSampleRate() > 0; }
SampleCount AudioFile::getLengthInSamples() const       { return getInfo().lengthInSamples; }
double AudioFile::getLength() const                     { return getInfo().getLengthInSeconds(); }
int AudioFile::getNumChannels() const                   { return getInfo().numChannels; }
double AudioFile::getSampleRate() const                 { return getInfo().sampleRate; }
int AudioFile::getBitsPerSample() const                 { return getInfo().bitsPerSample; }
bool AudioFile::isFloatingPoint() const                 { return getInfo().isFloatingPoint; }
yup::StringPairArray AudioFile::getMetadata() const    { return getInfo().metadata; }
yup::AudioFormat* AudioFile::getFormat() const         { return getInfo().format; }

bool AudioFile::moveToTrash() const
{
    CRASH_TRACER

    auto& afm = engine->getAudioFileManager();
    afm.checkFileForChangesAsync (*this);
    afm.releaseFile (*this);

    return file.moveToTrash();
}

bool AudioFile::deleteFile() const
{
    CRASH_TRACER

    auto& afm = engine->getAudioFileManager();
    afm.checkFileForChangesAsync (*this);
    afm.releaseFile (*this);

    bool ok = file.deleteFile();
    jassert (ok);
    return ok;
}

bool AudioFile::deleteFiles (Engine& engine, const yup::Array<yup::File>& files)
{
    bool allOK = true;

    for (auto& f : files)
        if (! AudioFile (engine, f).deleteFile())
            allOK = false;

    return allOK;
}

bool AudioFile::isWavFile() const               { return file.hasFileExtension ("wav;bwav;bwf"); }
bool AudioFile::isAiffFile() const              { return file.hasFileExtension ("aiff;aif"); }
bool AudioFile::isOggFile() const               { return file.hasFileExtension ("ogg"); }
bool AudioFile::isMp3File() const               { return file.hasFileExtension ("mp3"); }
bool AudioFile::isFlacFile() const              { return file.hasFileExtension ("flac"); }
bool AudioFile::isRexFile() const               { return file.hasFileExtension ("rex;rx2;rcy"); }


//==============================================================================
const int numSamplesPerFlush = 48000 * 6;

AudioFileWriter::AudioFileWriter (const AudioFile& f,
                                  yup::AudioFormat* formatToUse,
                                  int numChannels,
                                  double sampleRate,
                                  int bitsPerSample,
                                  const yup::StringPairArray& metadata,
                                  int quality)
    : file (f), samplesUntilFlush (numSamplesPerFlush)
{
    CRASH_TRACER
    f.engine->getAudioFileManager().releaseFile (file);

    if (file.getFile().getParentDirectory().createDirectory())
    {
        const yup::ScopedLock sl (writerLock);
        writer.reset (AudioFileUtils::createWriterFor (formatToUse, file.getFile(), sampleRate,
                                                       (unsigned int) numChannels, bitsPerSample,
                                                       metadata, quality));
    }
}

AudioFileWriter::AudioFileWriter (const AudioFile& f,
                                  yup::AudioFormat* formatToUse,
                                  int numChannels,
                                  double sampleRate,
                                  int bitsPerSample,
                                  const yup::StringPairArray& metadata,
                                  int quality,
                                  const yup::AudioChannelSet& channelLayout)
    : file (f), samplesUntilFlush (numSamplesPerFlush)
{
    CRASH_TRACER
    f.engine->getAudioFileManager().releaseFile (file);

    if (file.getFile().getParentDirectory().createDirectory())
    {
        const yup::ScopedLock sl (writerLock);
        writer.reset (AudioFileUtils::createWriterFor (formatToUse, file.getFile(), sampleRate,
                                                       (unsigned int) numChannels, bitsPerSample,
                                                       metadata, quality, channelLayout));
    }
}

AudioFileWriter::~AudioFileWriter()
{
    closeForWriting();
}

bool AudioFileWriter::isOpen() const noexcept               { const yup::ScopedLock sl (writerLock); return writer != nullptr; }
double AudioFileWriter::getSampleRate() const noexcept      { jassert (isOpen()); const yup::ScopedLock sl (writerLock); return writer->getSampleRate(); }
int AudioFileWriter::getNumChannels() const noexcept        { jassert (isOpen()); const yup::ScopedLock sl (writerLock); return writer->getNumChannels(); }

void AudioFileWriter::closeForWriting()
{
    {
        const yup::ScopedLock sl (writerLock);
        writer.reset();
    }

    auto& audioFileManager = file.engine->getAudioFileManager();
    audioFileManager.releaseFile (file);
    audioFileManager.checkFileForChanges (file);
}

bool AudioFileWriter::appendBuffer (yup::AudioBuffer<float>& buffer, int num)
{
    num = std::min (num, buffer.getNumSamples());
    const yup::ScopedLock sl (writerLock);

    if (writer != nullptr && writer->writeFromAudioSampleBuffer (buffer, 0, num))
    {
        samplesUntilFlush -= num;

        if (samplesUntilFlush <= 0)
        {
            samplesUntilFlush = numSamplesPerFlush;
            writer->flush();
        }

        return true;
    }

    return false;
}

bool AudioFileWriter::appendBuffer (const int** buffer, int num)
{
    const yup::ScopedLock sl (writerLock);
    return writer != nullptr && writer->write (reinterpret_cast<const float* const*> (buffer), num);
}

bool AudioFileWriter::writeFromAudioReader (yup::AudioFormatReader& reader,
                                            SampleCount startSample,
                                            SampleCount numSamples)
{
    const yup::ScopedLock sl (writerLock);
    return writer != nullptr && writer->writeFromAudioReader (reader, startSample, numSamples);
}

//==============================================================================
AudioProxyGenerator::GeneratorJob::GeneratorJob (const AudioFile& p)
    : ThreadPoolJobWithProgress ("proxy"), proxy (p)
{
}

AudioProxyGenerator::GeneratorJob::~GeneratorJob()
{
    prepareForJobDeletion();
    callBlockingCatching ([this] { proxy.engine->getAudioFileManager().validateFile (proxy, false); });
}

yup::ThreadPoolJob::JobStatus AudioProxyGenerator::GeneratorJob::runJob()
{
    CRASH_TRACER

    auto& afm = proxy.engine->getAudioFileManager();
    yup::FloatVectorOperations::disableDenormalisedNumberSupport();
    proxy.deleteFile();

    if (render())
        afm.checkFileForChangesAsync (proxy);
    else
        proxy.deleteFile();

    progress = 1.0f;

    afm.proxyGenerator.removeFinishedJob (this);
    return jobHasFinished;
}

//==============================================================================
AudioProxyGenerator::AudioProxyGenerator()
{
}

AudioProxyGenerator::~AudioProxyGenerator()
{
    CRASH_TRACER
}

AudioProxyGenerator::GeneratorJob* AudioProxyGenerator::findJob (const AudioFile& proxy) const noexcept
{
    for (auto j : activeJobs)
        if (j->proxy == proxy)
            return j;

    return {};
}

static bool checkProxyStatus (const AudioFile& f)
{
    if (f.getFile().existsAsFile())
    {
        if (f.isValid())
            return true;

        f.deleteFile();
    }

    return false;
}

void AudioProxyGenerator::beginJob (GeneratorJob* j)
{
    CRASH_TRACER
    std::unique_ptr<GeneratorJob> job (j);

    if (! checkProxyStatus (job->proxy))
    {
        const yup::ScopedLock sl (jobListLock);

        if (findJob (job->proxy) == nullptr)
        {
            job->proxy.engine->getBackgroundJobs().addJob (j, true);
            activeJobs.add (job.release());
        }
    }
}

bool AudioProxyGenerator::isProxyBeingGenerated (const AudioFile& proxyFile) const noexcept
{
    const yup::ScopedLock sl (jobListLock);
    return findJob (proxyFile) != nullptr;
}

float AudioProxyGenerator::getProportionComplete (const AudioFile& proxyFile) const noexcept
{
    const yup::ScopedLock sl (jobListLock);

    if (auto j = findJob (proxyFile))
        return j->progress;

    return 1.0f;
}

void AudioProxyGenerator::removeFinishedJob (GeneratorJob* j)
{
    const yup::ScopedLock sl (jobListLock);
    activeJobs.removeAllInstancesOf (j);
}

void AudioProxyGenerator::deleteProxy (const AudioFile& proxyFile)
{
    CRASH_TRACER
    GeneratorJob* j = nullptr;

    {
        const yup::ScopedLock sl (jobListLock);
        j = findJob (proxyFile);
    }

    if (j != nullptr)
        proxyFile.engine->getBackgroundJobs().removeJob (j, true, 10000);

    proxyFile.deleteFile();
}


//==============================================================================
AudioFileInfo::AudioFileInfo (Engine& e)
    : engine (&e), loopInfo (e)
{
}

AudioFileInfo::AudioFileInfo (const AudioFile& file, yup::AudioFormatReader* reader, yup::AudioFormat* f)
    : engine (file.engine), hashCode (file.getHash()), format (f),
      fileModificationTime (file.getFile().getLastModificationTime()),
      loopInfo (*file.engine, reader, f, file.getFile())
{
    if (reader != nullptr)
    {
        wasParsedOk     = true;
        sampleRate      = reader->sampleRate;
        lengthInSamples = reader->lengthInSamples;
        numChannels     = (int) reader->numChannels;
        bitsPerSample   = (int) reader->bitsPerSample;
        isFloatingPoint = reader->usesFloatingPointData;
        needsCachedProxy = dynamic_cast<yup::WavAudioFormat*> (format) == nullptr
                              && dynamic_cast<yup::AiffAudioFormat*> (format) == nullptr
                              && dynamic_cast<FloatAudioFormat*> (format) == nullptr;
        channelLayout   = reader->getChannelLayout();
        metadata = reader->metadataValues;
    }
    else
    {
        wasParsedOk = false;
        format = nullptr;
        sampleRate = 0;
        lengthInSamples = 0;
        numChannels = 0;
        bitsPerSample = 0;
        isFloatingPoint = false;
        needsCachedProxy = false;
    }
}

AudioFileInfo AudioFileInfo::parse (const AudioFile& file)
{
    if (! file.isNull())
    {
        yup::AudioFormat* format = nullptr;

        if (auto reader = std::unique_ptr<yup::AudioFormatReader> (AudioFileUtils::createReaderFindingFormat (*file.engine, file.getFile(), format)))
            return AudioFileInfo (file, reader.get(), format);
    }

    return AudioFileInfo (file, nullptr, nullptr);
}

//==============================================================================
class TracktionThumbnailCache  : public yup::AudioThumbnailCache
{
public:
    TracktionThumbnailCache (Engine& e)
       #if YUP_DEBUG
        : yup::AudioThumbnailCache (3)
       #else
        : yup::AudioThumbnailCache (50)
       #endif
        , engine (e)
    {
    }

    void saveNewlyFinishedThumbnail (const yup::AudioThumbnailBase& thumb, yup::int64 hash)
    {
        CRASH_TRACER
        auto st = getActiveSmartThumbnail (thumb);
        auto thumbFile = getThumbFile (st, hash);

        if (thumbFile.deleteFile())
        {
            thumbFile.getParentDirectory().createDirectory();

            yup::FileOutputStream fo (thumbFile);

            if (! fo.openedOk())
                return;

            thumb.saveTo (fo);
        }
    }

    bool loadNewThumb (yup::AudioThumbnailBase& thumb, yup::int64 hash)
    {
        CRASH_TRACER
        auto st = getActiveSmartThumbnail (thumb);
        auto thumbFile = getThumbFile (st, hash);

        if (st != nullptr
              && st->file.getFile().getLastModificationTime() > thumbFile.getLastModificationTime()
                                                                  + yup::RelativeTime::seconds (0.1))
        {
            thumbFile.deleteFile();
            return false;
        }

        yup::FileInputStream fin (thumbFile);
        return fin.openedOk() && thumb.loadFrom (fin);
    }

private:
    Engine& engine;

    yup::File getThumbFolder (Edit* edit) const
    {
        if (edit != nullptr)
            return edit->getTempDirectory (false);

        return engine.getTemporaryFileManager().getThumbnailsFolder();
    }

    SmartThumbnail* getActiveSmartThumbnail (const yup::AudioThumbnailBase& thumb)
    {
        auto& map = engine.getAudioFileManager().thumbnailMap;

        if (auto found = map.find (&thumb); found != map.end())
            return found->second;

        return nullptr;
    }

    yup::File getThumbFile (const SmartThumbnail* st, HashCode hash) const
    {
        auto thumbFolder = getThumbFolder (st != nullptr ? st->edit : nullptr);

        return thumbFolder.getChildFile ("thumbnail_" + yup::String::toHexString (hash) + ".thumb");
    }
};

//==============================================================================
struct MemoryBufferReader  : public AudioFormatReaderWithTimeout
{
    MemoryBufferReader (choc::buffer::InterleavedView<const float> buf, double sr)
        : sourceBuffer (buf)
    {
        sampleRate            = sr;
        numChannels           = static_cast<unsigned int> (buf.getNumChannels());
        lengthInSamples       = static_cast<yup::int64> (buf.getNumFrames());
        usesFloatingPointData = true;
        bitsPerSample         = 32;
    }

    void setReadTimeout (int) override {}

    bool readSamples (int* const* destSamples, int numDestChannels,
                      int startOffsetInDestBuffer,
                      yup::int64 startSampleInFile,
                      int numSamples) override
    {
        auto destOffset = static_cast<choc::buffer::FrameCount> (startOffsetInDestBuffer);
        auto destEnd = static_cast<choc::buffer::FrameCount> (startOffsetInDestBuffer + numSamples);
        auto srcStart = static_cast<choc::buffer::FrameCount> (startSampleInFile);
        auto srcAvailable = sourceBuffer.getNumFrames() - std::min (srcStart, sourceBuffer.getNumFrames());

        // usesFloatingPointData = true, so int** is really float**
        auto dest = choc::buffer::createChannelArrayView (reinterpret_cast<float* const*> (destSamples),
                                                          static_cast<choc::buffer::ChannelCount> (numDestChannels),
                                                          destEnd)
                        .getFrameRange ({ destOffset, destEnd });

        auto src = sourceBuffer.getFrameRange ({ srcStart, srcStart + std::min (srcAvailable,
                                                                                static_cast<choc::buffer::FrameCount> (numSamples)) });

        choc::buffer::copyIntersectionAndClearOutside (dest, src);
        return true;
    }

    choc::buffer::InterleavedView<const float> sourceBuffer;
};

//==============================================================================
struct AudioFileManager::KnownFile
{
    KnownFile (const AudioFile& f)
        : file (f), info (AudioFileInfo::parse (file))
    {
    }

    KnownFile (const AudioFile& f, AudioFileInfo i,
               choc::buffer::InterleavedView<const float> buf)
        : file (f), info (std::move (i)), memoryBuffer (buf)
    {
    }

    AudioFile file;
    AudioFileInfo info;
    std::optional<choc::buffer::InterleavedView<const float>> memoryBuffer;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KnownFile)
};

//==============================================================================
enum { initialTimerDelay = 10 };

bool SmartThumbnail::enabled = true;

SmartThumbnail::SmartThumbnail (Engine& e, const AudioFile& f, yup::Component& componentToRepaint, Edit* ed)
    : SmartThumbnail (e, f, componentToRepaint, ed,
                      e.getUIBehaviour().createAudioThumbnail (256,
                                                               e.getAudioFileFormatManager().readFormatManager,
                                                               e.getAudioFileManager().getAudioThumbnailCache()))
{
}

SmartThumbnail::SmartThumbnail (Engine& e, const AudioFile& f, yup::Component& componentToRepaint, Edit* ed,
                                std::unique_ptr<yup::AudioThumbnailBase> thumbnailToUse)
    : file (f), engine (e), edit (ed),
      thumbnail (std::move (thumbnailToUse)),
      component (componentToRepaint)
{
    TRACKTION_ASSERT_MESSAGE_THREAD
    assert (thumbnail && "thumbnail must be valid!");
    startTimer (initialTimerDelay);
    engine.getAudioFileManager().activeThumbnails.add (this);
    engine.getAudioFileManager().thumbnailMap[thumbnail.get()] = this;

    // Ensure the AudioFileManager knows about this type of thumbnail
    auto& thumbRef = *thumbnail; // Work around a clang warning
    const auto thumbTypeHashCode = std::hash<std::string>{} (typeid (thumbRef).name());
    engine.getAudioFileManager().thumbnailTypeHashes.insert (thumbTypeHashCode);
}

SmartThumbnail::~SmartThumbnail()
{
    TRACKTION_ASSERT_MESSAGE_THREAD

    engine.getAudioFileManager().thumbnailMap.erase (thumbnail.get());
    engine.getAudioFileManager().activeThumbnails.removeAllInstancesOf (this);
    thumbnail->clear();
}

bool SmartThumbnail::areThumbnailsFullyLoaded (Engine& engine)
{
    TRACKTION_ASSERT_MESSAGE_THREAD

    for (auto thumb : engine.getAudioFileManager().activeThumbnails)
        if (! thumb->isFullyLoaded())
            return false;

    return true;
}

void SmartThumbnail::setNewFile (const AudioFile& newFile)
{
    if (file != newFile)
    {
        file = newFile;

        audioFileChanged();
        component.repaint();
    }
}

//==============================================================================
void SmartThumbnail::drawChannel (yup::Graphics& g, yup::Rectangle<int> r,
                                  TimeRange time, int channelNum, float verticalZoomFactor)
{
    thumbnail->drawChannel (g, r,
                            time.getStart().inSeconds(), time.getEnd().inSeconds(),
                            channelNum, verticalZoomFactor);
}

void SmartThumbnail::drawChannels (yup::Graphics& g, yup::Rectangle<int> r,
                                   TimeRange time, float verticalZoomFactor)
{
    thumbnail->drawChannels (g, r,
                             time.getStart().inSeconds(), time.getEnd().inSeconds(),
                             verticalZoomFactor);
}

double SmartThumbnail::getProportionComplete() const noexcept
{
    if (auto sampleRate = file.getSampleRate(); sampleRate > 0)
    {
        const auto totalSamples = toSamples (TimePosition::fromSeconds (getTotalLength()), sampleRate);
        return yup::jlimit (0.0, 1.0, double (getNumSamplesFinished()) / double (std::max ((SampleCount) 1, totalSamples)));
    }

    return 0.0;
}

//==============================================================================
void SmartThumbnail::releaseFile()
{
    clear();
    thumbnailIsInvalid = true;
    yup::MessageManager::callAsync ([ref = makeWeakRef (*this), this]() mutable
                                     {
                                         if (ref != nullptr)
                                             startTimer (400);
                                     });
}

void SmartThumbnail::createThumbnailReader()
{
    if (enabled)
    {
        // This hashing takes in to account the type of the thumb to avoid clashes if
        // you're using different types for different displays
        // N.B. if this changes, AudioFileManager::callListeners will also need to be updated
        auto& thumbRef = *thumbnail; // Work around a clang warning
        const auto thumbTypeHashCode = std::hash<std::string>{} (typeid (thumbRef).name());
        const auto hashCode = static_cast<yup::int64> (hash ((size_t) file.getHash(), thumbTypeHashCode));

        // this breaks thumbnails in 64-bit mode
        //setReader (new CacheAudioFormatReader (file), hashCode);

        setReader (AudioFileUtils::createReaderFor (engine, file.getFile()), hashCode);
        thumbnailIsInvalid = false;
    }
    else
    {
        thumbnailIsInvalid = true;
    }
}

void SmartThumbnail::audioFileChanged()
{
    CRASH_TRACER
    auto& proxyGen = engine.getAudioFileManager().proxyGenerator;

    wasGeneratingProxy = proxyGen.isProxyBeingGenerated (file);

    clear();

    if (file.getFile().exists())
        createThumbnailReader();
    else
        thumbnailIsInvalid = true;

    lastProgress = 0.0f;
    component.repaint();
    startTimer (200);
}

//==============================================================================
void SmartThumbnail::clear()
{
    thumbnail->clear();
}

bool SmartThumbnail::setSource (yup::InputSource* source)
{
    return thumbnail->setSource (source);
}

void SmartThumbnail::setReader (yup::AudioFormatReader* reader, yup::int64 hashCode)
{
    thumbnail->setReader (reader, hashCode);
}

bool SmartThumbnail::loadFrom (yup::InputStream& stream)
{
    return thumbnail->loadFrom (stream);
}

void SmartThumbnail::saveTo (yup::OutputStream& stream) const
{
    thumbnail->saveTo (stream);
}

int SmartThumbnail::getNumChannels() const noexcept
{
    return thumbnail->getNumChannels();
}

double SmartThumbnail::getTotalLength() const noexcept
{
    return thumbnail->getTotalLength();
}

void SmartThumbnail::drawChannel (yup::Graphics& g,
                                  const yup::Rectangle<int>& r,
                                  double startTimeSeconds,
                                  double endTimeSeconds,
                                  int channelNum,
                                  float verticalZoomFactor)
{
    thumbnail->drawChannel (g, r,
                            startTimeSeconds,
                            endTimeSeconds,
                            channelNum,
                            verticalZoomFactor);
}

void SmartThumbnail::drawChannels (yup::Graphics& g,
                                   const yup::Rectangle<int>& r,
                                   double startTimeSeconds,
                                   double endTimeSeconds,
                                   float verticalZoomFactor)
{
    thumbnail->drawChannels (g, r,
                             startTimeSeconds,
                             endTimeSeconds,
                             verticalZoomFactor);
}

bool SmartThumbnail::isFullyLoaded() const noexcept
{
    return thumbnail->isFullyLoaded();
}

yup::int64 SmartThumbnail::getNumSamplesFinished() const noexcept
{
    return thumbnail->getNumSamplesFinished();
}

float SmartThumbnail::getApproximatePeak() const
{
    return thumbnail->getApproximatePeak();
}

void SmartThumbnail::getApproximateMinMax (double startTime, double endTime, int channelIndex,
                                           float& minValue, float& maxValue) const noexcept
{
    thumbnail->getApproximateMinMax (startTime, endTime, channelIndex,
                                     minValue, maxValue);
}

yup::int64 SmartThumbnail::getHashCode() const
{
    return thumbnail->getHashCode();
}

void SmartThumbnail::reset (int numChannels, double sampleRate, yup::int64 totalSamplesInSource)
{
    thumbnail->reset (numChannels, sampleRate, totalSamplesInSource);
}

void SmartThumbnail::addBlock (yup::int64 sampleNumberInSource, const yup::AudioBuffer<float>& buffer,
                               int startOffsetInBuffer, int numSamples)
{
    thumbnail->addBlock (sampleNumberInSource, buffer,
                         startOffsetInBuffer, numSamples);
}

//==============================================================================
void SmartThumbnail::timerCallback()
{
    CRASH_TRACER

    auto& afm = engine.getAudioFileManager();
    auto& proxyGen = afm.proxyGenerator;

    if (getTimerInterval() == initialTimerDelay)
        audioFileChanged();

    const bool isGeneratingNow = proxyGen.isProxyBeingGenerated (file);

    if (wasGeneratingProxy != isGeneratingNow || (thumbnailIsInvalid && file.getFile().exists()))
    {
        wasGeneratingProxy = isGeneratingNow;

        if (! isGeneratingNow)
        {
            afm.checkFileForChanges (file);
            createThumbnailReader();
        }
        else
        {
            thumbnailIsInvalid = true;
        }

        component.repaint();
    }

    if (isGeneratingNow || ! isFullyLoaded())
    {
        float progress = isGeneratingNow ? proxyGen.getProportionComplete (file)
                                         : (float) getProportionComplete();

        if (lastProgress != progress)
        {
            lastProgress = progress;
            component.repaint();
        }
    }
    else if (! thumbnailIsInvalid || ! file.getFile().exists())
    {
        component.repaint();
        stopTimer();
    }
}

//==============================================================================
AudioFileManager::AudioFileManager (Engine& e)
    : engine (e), cache (e), thumbnailCache (std::make_unique<TracktionThumbnailCache> (e))
{
}

AudioFileManager::~AudioFileManager()
{
    clearFiles();
}

AudioFileManager::KnownFile& AudioFileManager::findOrCreateKnown (const AudioFile& f)
{
    auto hash = f.getHash();
    auto kf = knownFiles.find (hash);

    if (kf != knownFiles.end())
        return *kf->second.get();

    knownFiles[hash] = std::make_unique<KnownFile> (f);
    return *knownFiles[hash].get();
}

void AudioFileManager::clearFiles()
{
    CRASH_TRACER
    const yup::ScopedLock sl (knownFilesLock);
    knownFiles.clear();
}

void AudioFileManager::removeFile (HashCode hash)
{
    const yup::ScopedLock sl (knownFilesLock);

    auto f = knownFiles.find (hash);

    if (f != knownFiles.end())
        knownFiles.erase (f);
}

AudioFile AudioFileManager::getAudioFile (ProjectItemRef sourceID)
{
    return AudioFile (engine, engine.getProjectManager().findSourceFile (sourceID));
}

AudioFileInfo AudioFileManager::getInfo (const AudioFile& file)
{
    const yup::ScopedLock sl (knownFilesLock);
    return findOrCreateKnown (file).info;
}

void AudioFileManager::registerMemoryBuffer (const std::string& filename,
                                             choc::buffer::InterleavedView<const float> buffer,
                                             double sampleRate)
{
    AudioFile af (engine, yup::File (yup::String (filename)));

    AudioFileInfo info (engine);
    info.wasParsedOk      = true;
    info.hashCode         = af.getHash();
    info.sampleRate       = sampleRate;
    info.lengthInSamples  = static_cast<SampleCount> (buffer.getNumFrames());
    info.numChannels      = static_cast<int> (buffer.getNumChannels());
    info.bitsPerSample    = 32;
    info.isFloatingPoint  = true;
    info.needsCachedProxy = false;

    const yup::ScopedLock sl (knownFilesLock);
    knownFiles[af.getHash()] = std::make_unique<KnownFile> (af, std::move (info), buffer);
}

void AudioFileManager::unregisterMemoryBuffer (const std::string& filename)
{
    AudioFile af (engine, yup::File (yup::String (filename)));
    removeFile (af.getHash());
}

std::unique_ptr<AudioFormatReaderWithTimeout> AudioFileManager::createMemoryReader (const AudioFile& file) const
{
    const yup::ScopedLock sl (knownFilesLock);

    auto it = knownFiles.find (file.getHash());

    if (it != knownFiles.end() && it->second->memoryBuffer.has_value())
        return std::make_unique<MemoryBufferReader> (*(it->second->memoryBuffer),
                                                     it->second->info.sampleRate);

    return {};
}

bool AudioFileManager::checkFileTime (KnownFile& f)
{
    if (f.memoryBuffer.has_value())
        return false;

    if (! f.info.wasParsedOk
        || f.info.fileModificationTime != f.file.getFile().getLastModificationTime())
    {
        f.info = AudioFileInfo::parse (f.file);
        return true;
    }

    return false;
}

void AudioFileManager::checkFileForChanges (const AudioFile& file)
{
    CRASH_TRACER

    bool changed = false;

    {
        const yup::ScopedLock sl (knownFilesLock);

        auto f = knownFiles.find (file.getHash());

        if (f != knownFiles.end())
            changed = checkFileTime (*f->second);
    }

    if (changed)
    {
        releaseFile (file);
        callListenersOnMessageThread (file);
    }
}

void AudioFileManager::checkFilesForChanges()
{
    TRACKTION_ASSERT_MESSAGE_THREAD

    yup::Array<AudioFile> changedFiles;

    {
        const yup::ScopedLock sl (knownFilesLock);

        for (auto& f : knownFiles)
            if (checkFileTime (*f.second))
                changedFiles.add (f.second->file);
    }

    for (auto& f : changedFiles)
    {
        releaseFile (f);
        callListeners (f);
    }
}

void AudioFileManager::releaseAllFiles()
{
    cache.releaseAllFiles();

    const yup::ScopedLock sl (activeThumbnailLock);

    for (auto t : activeThumbnails)
        t->releaseFile();
}

void AudioFileManager::releaseFile (const AudioFile& file)
{
    cache.releaseFile (file);

    const yup::ScopedLock sl (activeThumbnailLock);

    for (auto t : activeThumbnails)
        if (t->file == file)
            t->releaseFile();
}

void AudioFileManager::callListeners (const AudioFile& file)
{
    CRASH_TRACER
    TRACKTION_ASSERT_MESSAGE_THREAD

    for (auto h : thumbnailTypeHashes)
    {
        const auto hashCode = hash ((size_t) file.getHash(), h);
        thumbnailCache->removeThumb (static_cast<yup::int64> (hashCode));
    }

    const yup::ScopedLock sl (activeThumbnailLock);

    for (auto t : activeThumbnails)
        if (t->file == file)
            t->audioFileChanged();
}

void AudioFileManager::callListenersOnMessageThread (const AudioFile& file)
{
    if (yup::MessageManager::existsAndIsCurrentThread())
        callListeners (file);
    else
        yup::MessageManager::callAsync ([file, eng = makeWeakRef (engine)]
                                         {
                                             if (eng != nullptr)
                                                 eng->getAudioFileManager().callListeners (file);
                                         });
}

void AudioFileManager::forceFileUpdate (const AudioFile& file)
{
    CRASH_TRACER
    TRACKTION_ASSERT_MESSAGE_THREAD

    // this doesn't check for file time and is used when files are changed rapidly such as when recording
    const yup::ScopedLock sl (knownFilesLock);

    auto f = knownFiles.find (file.getHash());

    if (f != knownFiles.end())
    {
        f->second->info = AudioFileInfo::parse (f->second->file);
        releaseFile (file);
        callListeners (file);
    }
}

void AudioFileManager::validateFile (const AudioFile& file, bool updateInfo)
{
    if (updateInfo)
    {
        getInfo (file);
        forceFileUpdate (file);
    }

    cache.validateFile (file);
}

void AudioFileManager::checkFileForChangesAsync (const AudioFile& file)
{
    const yup::ScopedLock sl (knownFilesLock);
    filesToCheck.addIfNotAlreadyThere (file);
    triggerAsyncUpdate();
}

void AudioFileManager::handleAsyncUpdate()
{
    CRASH_TRACER
    AudioFile fileToCheck (engine);

    {
        const yup::ScopedLock sl (knownFilesLock);

        if (filesToCheck.isEmpty())
            return;

        fileToCheck = filesToCheck.getUnchecked (filesToCheck.size() - 1);
        filesToCheck.removeLast();

        if (filesToCheck.size() > 0)
            triggerAsyncUpdate();
    }

    if (! fileToCheck.isNull())
        checkFileForChanges (fileToCheck);
}

} // namespace tracktion::inline engine
