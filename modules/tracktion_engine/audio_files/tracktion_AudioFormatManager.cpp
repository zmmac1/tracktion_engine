/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

AudioFileFormatManager::AudioFileFormatManager()
{
    CRASH_TRACER

    // NB default must be first!

    wavFormat = std::make_unique<yup::WavAudioFormat>();
    readFormats.add (wavFormat.get());
    writeFormats.add (wavFormat.get());

    aiffFormat = std::make_unique<yup::AiffAudioFormat>();
    readFormats.add (aiffFormat.get());
    writeFormats.add (aiffFormat.get());

    floatFormat = std::make_unique<FloatAudioFormat>();
    readFormats.add (floatFormat.get());

    oggFormat = std::make_unique<yup::OggVorbisAudioFormat>();
    readFormats.add (oggFormat.get());
    writeFormats.add (oggFormat.get());

    flacFormat = std::make_unique<yup::FlacAudioFormat>();
    readFormats.add (flacFormat.get());
    writeFormats.add (flacFormat.get());

   #if TRACKTION_ENABLE_REX
    rexFormat = std::make_unique<RexAudioFormat>();
    readFormats.add (rexFormat.get());
   #endif

   #if YUP_MAC || JUCE_IOS
    nativeAudioFormat = std::make_unique<yup::CoreAudioFormat>();
    readFormats.add (nativeAudioFormat.get());
   #elif JUCE_WINDOWS
    nativeAudioFormat = std::make_unique<yup::WindowsMediaAudioFormat>();
    readFormats.add (nativeAudioFormat.get());
   #endif

   #if JUCE_USE_MP3AUDIOFORMAT
    mp3ReadFormat = std::make_unique<yup::MP3AudioFormat>();
    readFormats.add (mp3ReadFormat.get());
   #endif

    // NB when adding new formats -
    //  this bit assumes all writable formats are also readable
    allFormats = readFormats;


    readFormatManager.registerFormat (new yup::WavAudioFormat(), true);
    readFormatManager.registerFormat (new yup::AiffAudioFormat(), false);
    readFormatManager.registerFormat (new FloatAudioFormat(), false);
    readFormatManager.registerFormat (new yup::OggVorbisAudioFormat(), false);
    readFormatManager.registerFormat (new yup::FlacAudioFormat(), false);

   #if TRACKTION_ENABLE_REX
    readFormatManager.registerFormat (new RexAudioFormat(), false);
   #endif

   #if YUP_MAC || JUCE_IOS
    readFormatManager.registerFormat (new yup::CoreAudioFormat(), false);
   #elif JUCE_WINDOWS
    readFormatManager.registerFormat (new yup::WindowsMediaAudioFormat(), false);
   #endif

   #if JUCE_USE_MP3AUDIOFORMAT
    readFormatManager.registerFormat (new yup::MP3AudioFormat(), false);
   #endif

    writeFormatManager.registerFormat (new yup::WavAudioFormat(), true);
    writeFormatManager.registerFormat (new yup::AiffAudioFormat(), false);
    writeFormatManager.registerFormat (new FloatAudioFormat(), false);
    writeFormatManager.registerFormat (new yup::OggVorbisAudioFormat(), false);
    writeFormatManager.registerFormat (new yup::FlacAudioFormat(), false);

    memoryMappedFormatManager.registerFormat (new yup::WavAudioFormat(), true);
    memoryMappedFormatManager.registerFormat (new yup::AiffAudioFormat(), false);
    memoryMappedFormatManager.registerFormat (new FloatAudioFormat(), false);
}

AudioFileFormatManager::~AudioFileFormatManager()
{
    CRASH_TRACER
}

void AudioFileFormatManager::addLameFormat (std::unique_ptr<yup::AudioFormat> lameForArray,
                                            std::unique_ptr<yup::AudioFormat> lameForAccess)
{
    if (lameFormat != nullptr || lameForArray == nullptr || lameForAccess == nullptr)
        return;

    lameFormat = std::move (lameForAccess);
    writeFormats.add (lameForArray.get());
    writeFormatManager.registerFormat (lameForArray.release(), false);
}

void AudioFileFormatManager::addFormat (std::function<yup::AudioFormat*()> formatCreator, bool isWritable, bool isMemoryMappable)
{
    jassert (formatCreator);

    auto* af = additionalFormats.add (formatCreator());
    readFormats.add (af);
    allFormats.add (af);

    readFormatManager.registerFormat (formatCreator(), false);

    if (isWritable)
        writeFormatManager.registerFormat (formatCreator(), false);

    if (isMemoryMappable)
        memoryMappedFormatManager.registerFormat (formatCreator(), false);
}

yup::AudioFormat* AudioFileFormatManager::getFormatFromFileName (const yup::File& f) const
{
    for (auto format : allFormats)
        if (format->canHandleFile (f))
            return format;

    return {};
}

bool AudioFileFormatManager::canOpen (const yup::File& f) const
{
    return getFormatFromFileName (f) != nullptr;
}

yup::String AudioFileFormatManager::getValidFileExtensions() const
{
    return "wav;aiff;aif;ogg;mp3;mid;midi;flac;au;voc;caf;w64;rx2;rcy;rex;m4a;wfaf";
}

yup::AudioFormat* AudioFileFormatManager::getNamedFormat (const yup::String& formatName) const
{
    for (auto format : allFormats)
        if (format->getFormatName() == formatName)
            return format;

    return getDefaultFormat();
}

} // namespace tracktion::inline engine
