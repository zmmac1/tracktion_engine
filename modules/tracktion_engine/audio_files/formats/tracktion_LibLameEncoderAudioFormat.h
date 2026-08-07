/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

#if TRACKTION_ENABLE_LIBLAME

//==============================================================================
/**
    An AudioFormat that encodes MP3 data using the LAME shared library.

    LAMEEncoderAudioFormat and FFmpegEncoderAudioFormat both write a temporary
    wave file, run a command line tool over it, and then copy the result into the
    destination. This class instead loads libmp3lame at runtime and encodes
    incrementally, writing MP3 data straight to the yup::OutputStream as it is
    produced, so it can just as easily fill a block of memory as a file.

    The library is opened with yup::DynamicLibrary and every symbol is resolved
    by name, so no LAME headers are needed to build this and nothing is linked at
    build time. Check isLibraryLoaded() before use - if it returns false then
    createWriterFor() will always return nullptr.

    LAME is LGPLv2 and is loaded dynamically, which keeps the distribution
    obligations to shipping the licence text and letting users substitute their
    own build of the library. Don't switch this to static linking without
    checking the licensing implications.

    @see LAMEManager, FFmpegEncoderAudioFormat
*/
class LibLameEncoderAudioFormat  : public yup::AudioFormat
{
public:
    /** Creates a LibLameEncoderAudioFormat, loading the LAME shared library from
        the given location.

        This can be a full path, or a bare library name in which case the
        platform's usual library search path is used.

        @see getDefaultLibraryName, isLibraryLoaded
    */
    explicit LibLameEncoderAudioFormat (const yup::String& lameSharedLibraryToUse);

    ~LibLameEncoderAudioFormat() override;

    //==============================================================================
    /** Returns the usual name of the LAME shared library on the current platform. */
    static yup::String getDefaultLibraryName();

    /** Returns true if the library was opened and every symbol needed for encoding
        was found in it.
    */
    bool isLibraryLoaded() const;

    /** Returns the version string reported by the library, or an empty string if it
        isn't loaded.
    */
    yup::String getLibraryVersion() const;

    //==============================================================================
    bool canHandleFile (const yup::File&) override;
    yup::Array<int> getPossibleSampleRates() override;
    yup::Array<int> getPossibleBitDepths() override;
    bool canDoStereo() override;
    bool canDoMono() override;
    bool isCompressed() override;
    yup::StringArray getQualityOptions() override;

    yup::AudioFormatReader* createReaderFor (yup::InputStream*, bool deleteStreamIfOpeningFails) override;

    std::unique_ptr<yup::AudioFormatWriter> createWriterFor (std::unique_ptr<yup::OutputStream>&,
                                                              const yup::WriteOptions&) override;

private:
    struct LameLibrary;
    class Writer;

    // Shared with the writers, so the library can't be unloaded while one is still encoding
    std::shared_ptr<LameLibrary> lame;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LibLameEncoderAudioFormat)
};

#endif

} // namespace tracktion::inline engine
