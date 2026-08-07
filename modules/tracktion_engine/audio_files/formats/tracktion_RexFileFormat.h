/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

#if TRACKTION_ENABLE_REX

namespace tracktion::inline engine {

class RexAudioFormat    : public yup::AudioFormat
{
public:
    RexAudioFormat();
    ~RexAudioFormat();

    yup::Array<int> getPossibleSampleRates() override    { return {}; }
    yup::Array<int> getPossibleBitDepths() override      { return {}; }

    bool canDoStereo() override     { return true; }
    bool canDoMono() override       { return true; }
    bool isCompressed() override    { return true; }

    yup::AudioFormatReader* createReaderFor (yup::InputStream*, bool deleteStreamIfOpeningFails) override;
    std::unique_ptr<yup::AudioFormatWriter> createWriterFor (std::unique_ptr<yup::OutputStream>&,
                                                              const yup::WriteOptions&) override;

    static const char* const rexTempo;
    static const char* const rexDenominator;
    static const char* const rexNumerator;
    static const char* const rexBeatPoints;

    static yup::String getErrorLoadingDLL();

private:
    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RexAudioFormat)
};

} // namespace tracktion::inline engine

#endif
