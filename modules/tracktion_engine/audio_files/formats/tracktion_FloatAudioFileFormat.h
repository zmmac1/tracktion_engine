/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

#pragma once

namespace tracktion::inline engine {

/**
    A raw, proprietary, simple floating point format used for freeze files, etc.
*/
class FloatAudioFormat   : public yup::AudioFormat
{
public:
    FloatAudioFormat();
    ~FloatAudioFormat() override;

    //==============================================================================
    yup::Array<int> getPossibleSampleRates() const override;
    yup::Array<int> getPossibleBitDepths() const override;
    bool canDoStereo() const override;
    bool canDoMono() const override;
    bool canHandleFile (const yup::File&);

    //==============================================================================
    using yup::AudioFormat::createReaderFor;
    yup::AudioFormatReader* createReaderFor (yup::InputStream*, bool deleteStreamIfOpeningFails);

    using yup::AudioFormat::createMemoryMappedReader;
    yup::MemoryMappedAudioFormatReader* createMemoryMappedReader (const yup::File&) override;

    using yup::AudioFormat::createWriterFor;
    std::unique_ptr<yup::AudioFormatWriter> createWriterFor (std::unique_ptr<yup::OutputStream>&,
                                                              const yup::WriteOptions&) override;
};

} // namespace tracktion::inline engine
