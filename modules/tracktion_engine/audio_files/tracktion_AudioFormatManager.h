/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

/** Keeps a list of available wave formats and can create a format object
    for a file.
*/
class AudioFileFormatManager
{
public:
    AudioFileFormatManager();
    ~AudioFileFormatManager();

    //==============================================================================
    const yup::Array<yup::AudioFormat*>& getWriteFormats() const     { return writeFormats; }

    void addLameFormat (std::unique_ptr<yup::AudioFormat> lameForArray,
                        std::unique_ptr<yup::AudioFormat> lameForAccess);

    void addFormat (std::function<yup::AudioFormat*()> formatCreator, bool isWritable, bool isMemoryMappable);

    //==============================================================================
    yup::AudioFormat* getFormatFromFileName (const yup::File&) const;
    yup::AudioFormat* getNamedFormat (const yup::String& formatName) const;

    bool canOpen (const yup::File&) const;
    yup::String getValidFileExtensions() const;

    yup::AudioFormat* getDefaultFormat() const     { return wavFormat.get(); }
    yup::AudioFormat* getWavFormat() const         { return wavFormat.get(); }
    yup::AudioFormat* getAiffFormat() const        { return aiffFormat.get(); }
    yup::AudioFormat* getFrozenFileFormat() const  { return floatFormat.get(); }
    yup::AudioFormat* getOggFormat() const         { return oggFormat.get(); }
    yup::AudioFormat* getFlacFormat() const        { return flacFormat.get(); }
    yup::AudioFormat* getNativeAudioFormat() const { return nativeAudioFormat.get(); }
    yup::AudioFormat* getLameFormat() const        { return lameFormat.get(); }

   #if TRACKTION_ENABLE_REX
    yup::AudioFormat* getRexFormat() const         { return rexFormat.get(); }
   #endif

    yup::AudioFormatManager readFormatManager, writeFormatManager, memoryMappedFormatManager;

private:
    yup::Array<yup::AudioFormat*> allFormats, readFormats, writeFormats;
    yup::OwnedArray<yup::AudioFormat> additionalFormats;

    std::unique_ptr<yup::AudioFormat> wavFormat, aiffFormat, floatFormat, nativeAudioFormat,
                                       mp3ReadFormat, oggFormat, flacFormat, lameFormat;

   #if TRACKTION_ENABLE_REX
    std::unique_ptr<yup::AudioFormat> rexFormat;
   #endif
};

} // namespace tracktion::inline engine
