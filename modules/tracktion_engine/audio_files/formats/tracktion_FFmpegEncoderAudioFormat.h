/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

#if TRACKTION_ENABLE_FFMPEG

class FFmpegEncoderAudioFormat : public yup::AudioFormat
{
public:
    /** Creates a FFmpegEncoderAudioFormat that expects to find a working FFmpeg
     executable at the location given.
     */
    FFmpegEncoderAudioFormat (const yup::File& ffmpegExecutableToUse);
    ~FFmpegEncoderAudioFormat() override;
    
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
    yup::File ffmpegExe;
    class Writer;
    
    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FFmpegEncoderAudioFormat)
};

#endif

} // namespace tracktion::inline engine
