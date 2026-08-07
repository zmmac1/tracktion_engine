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
ImpulseResponsePlugin::ImpulseResponsePlugin (PluginCreationInfo info)
    : Plugin (info)
{
    auto um = getUndoManager();

    name.referTo (state, IDs::name, um);

    const yup::NormalisableRange frequencyRange { frequencyToMidiNote (10.0f), frequencyToMidiNote (20'000.0f) };

    highPassCutoffValue.referTo (state, IDs::highPassFrequency, um, frequencyRange.start);
    lowPassCutoffValue.referTo (state, IDs::lowPassFrequency, um, frequencyRange.end);
    gainValue.referTo (state, IDs::gain, um, 0.0f);
    mixValue.referTo (state, IDs::mix, um, 1.0f);
    qValue.referTo (state, IDs::filterSlope, um, 1.0f / yup::MathConstants<float>::sqrt2);

    normalise.referTo (state, IDs::normalise, um, true);
    trimSilence.referTo (state, IDs::trimSilence, um, false);

    yup::NormalisableRange volumeRange { -12.0f, 6.0f };
    volumeRange.setSkewForCentre (0.0f);

    // Initialises parameter and attaches to value
    gainParam = addParam (IDs::gain.toString(), TRANS ("Gain"), volumeRange,
                          [] (float value)             { return yup::Decibels::toString (value); },
                          [] (const yup::String& s)   { return s.getFloatValue(); });
    gainParam->attachToCurrentValue (gainValue);

    highPassCutoffParam = addParam (IDs::highPassMidiNoteNumber.toString(), TRANS ("Low Cut"), frequencyRange,
                                    [] (float value)             { return yup::String (midiNoteToFrequency (value), 1) + " Hz"; },
                                    [] (const yup::String& s)   { return frequencyToMidiNote (s.getFloatValue()); });
    highPassCutoffParam->attachToCurrentValue (highPassCutoffValue);

    lowPassCutoffParam =  addParam (IDs::lowPassMidiNoteNumber.toString(), TRANS ("High Cut"), frequencyRange,
                                    [] (float value)             { return yup::String (midiNoteToFrequency (value), 1) + " Hz"; },
                                    [] (const yup::String& s)   { return frequencyToMidiNote (s.getFloatValue()); });
    lowPassCutoffParam->attachToCurrentValue (lowPassCutoffValue);

    mixParam = addParam (IDs::mix.toString(), TRANS("Mix"), { 0.0f, 1.0f, 0.0f },
                         [] (float value)             { return yup::String (yup::roundToInt (value * 100.0f)) + "%"; },
                         [] (const yup::String& s)   { return s.getFloatValue() / 100.0f; });
    mixParam->attachToCurrentValue (mixValue);

    filterQParam = addParam (IDs::filterQ.toString(), TRANS("Filter Q"), { 0.1f, 14.0f, 0.0f },
                             [] (float value)       { return yup::String (value); },
                             [] (const yup::String& s)   { return s.getFloatValue(); });
    filterQParam->attachToCurrentValue (qValue);

    loadImpulseResponseFromState();
}

ImpulseResponsePlugin::~ImpulseResponsePlugin()
{
    notifyListenersOfDeletion();
    gainParam->detachFromCurrentValue();
    highPassCutoffParam->detachFromCurrentValue();
    lowPassCutoffParam->detachFromCurrentValue();
    mixParam->detachFromCurrentValue();
    filterQParam->detachFromCurrentValue();
}

const char* ImpulseResponsePlugin::getPluginName() { return NEEDS_TRANS ("Impulse Response"); }

//==============================================================================
bool ImpulseResponsePlugin::loadImpulseResponse (const void* sourceData, size_t sourceDataSize)
{
    auto is = std::make_unique<yup::MemoryInputStream> (sourceData, sourceDataSize, false);
    auto& formatManager = engine.getAudioFileFormatManager().readFormatManager;

    if (auto reader = std::unique_ptr<yup::AudioFormatReader> (formatManager.createReaderFor (is.release())))
    {
        yup::AudioBuffer<float> buffer ((int) reader->numChannels, (int) reader->lengthInSamples);
        reader->read (&buffer, 0, buffer.getNumSamples(), 0, true, true);

        return loadImpulseResponse (std::move (buffer), reader->sampleRate, (int) reader->bitsPerSample);
    }

    return false;
}

bool ImpulseResponsePlugin::loadImpulseResponse (const yup::File& fileImpulseResponse)
{
    yup::MemoryBlock fileDataMemoryBlock;

    if (fileImpulseResponse.loadFileAsData (fileDataMemoryBlock))
        return loadImpulseResponse (fileDataMemoryBlock.getData(), fileDataMemoryBlock.getSize());

    return false;
}

bool ImpulseResponsePlugin::loadImpulseResponse (yup::AudioBuffer<float>&& bufferImpulseResponse,
                                                 double sampleRateToStore,
                                                 int bitDepthToStore)
{
    yup::MemoryBlock fileDataMemoryBlock;
    auto os = std::unique_ptr<yup::OutputStream> (std::make_unique<yup::MemoryOutputStream> (fileDataMemoryBlock, false));

    if (auto writer = std::unique_ptr<yup::AudioFormatWriter> (yup::FlacAudioFormat()
                                                                    .createWriterFor (os,
                                                                                      yup::WriteOptions()
                                                                                        .withSampleRate (sampleRateToStore)
                                                                                        .withNumChannels (bufferImpulseResponse.getNumChannels())
                                                                                        .withBitsPerSample (std::min (24, bitDepthToStore)))))
    {
        if (writer->writeFromAudioSampleBuffer (bufferImpulseResponse, 0, bufferImpulseResponse.getNumSamples()))
        {
            writer.reset(); // Delete the writer to flush the block before moving the buffer on
            state.setProperty (IDs::irFileData, yup::var (std::move (fileDataMemoryBlock)), getUndoManager());
            return true;
        }
    }

    return false;
}


//==============================================================================
yup::String ImpulseResponsePlugin::getName() const             { return getPluginName(); }
yup::String ImpulseResponsePlugin::getShortName (int)          { return "IR"; }
yup::String ImpulseResponsePlugin::getPluginType()             { return xmlTypeName; }
yup::String ImpulseResponsePlugin::getSelectableDescription()  { return getName(); }

double ImpulseResponsePlugin::getLatencySeconds()
{
    return processorChain.get<convolutionIndex>().getLatency() / sampleRate;
}

void ImpulseResponsePlugin::initialise (const PluginInitialisationInfo& info)
{
    yup::dsp::ProcessSpec processSpec;
    processSpec.sampleRate = info.sampleRate;
    processSpec.maximumBlockSize = (uint32_t) info.blockSizeSamples;
    processSpec.numChannels = 2;
    processorChain.prepare (processSpec);

    // Update smoothers
    lowFreqSmoother.setTargetValue (midiNoteToFrequency (lowPassCutoffParam->getCurrentValue()));
    highFreqSmoother.setTargetValue (midiNoteToFrequency (highPassCutoffParam->getCurrentValue()));
    gainSmoother.setTargetValue (gainParam->getCurrentValue());
    qSmoother.setTargetValue (filterQParam->getCurrentValue());

    const auto wetDry = getWetDryLevels (mixParam->getCurrentValue());
    wetGainSmoother.setTargetValue (wetDry.wet);
    dryGainSmoother.setTargetValue (wetDry.dry);

    const double smoothTime = 0.01;
    lowFreqSmoother.reset (info.sampleRate, smoothTime);
    highFreqSmoother.reset (info.sampleRate, smoothTime);
    gainSmoother.reset (info.sampleRate, smoothTime);
    qSmoother.reset (info.sampleRate, smoothTime);
    wetGainSmoother.reset (info.sampleRate, smoothTime);
    dryGainSmoother.reset (info.sampleRate, smoothTime);
}

void ImpulseResponsePlugin::deinitialise()
{}

void ImpulseResponsePlugin::reset()
{
    processorChain.reset();
}

void ImpulseResponsePlugin::applyToBuffer (const PluginRenderContext& fc)
{
    // Update smoothers
    lowFreqSmoother.setTargetValue (midiNoteToFrequency (lowPassCutoffParam->getCurrentValue()));
    highFreqSmoother.setTargetValue (midiNoteToFrequency (highPassCutoffParam->getCurrentValue()));
    gainSmoother.setTargetValue (gainParam->getCurrentValue());
    qSmoother.setTargetValue (filterQParam->getCurrentValue());

    const auto wetDryGain = getWetDryLevels (mixParam->getCurrentValue());
    wetGainSmoother.setTargetValue (wetDryGain.wet);
    dryGainSmoother.setTargetValue (wetDryGain.dry);

    // Update gains and filter params
    auto hpf = processorChain.get<HPFIndex>().state;
    auto& lpf = processorChain.get<LPFIndex>().state;
    auto& gain = processorChain.get<gainIndex>();

    AudioScratchBuffer dryBuffer (*fc.destBuffer);

    if (gainSmoother.isSmoothing() || lowFreqSmoother.isSmoothing() || highFreqSmoother.isSmoothing() || qSmoother.isSmoothing())
    {
        const int blockSize = 32;
        int numSamplesLeft = fc.bufferNumSamples;
        int numSamplesDone = 0;

        for (;;)
        {
            const int numThisTime = std::min (blockSize, numSamplesLeft);

            // Process sub-block
            auto inOutBlock = yup::dsp::AudioBlock<float> (*fc.destBuffer).getSubBlock (size_t (numSamplesDone + fc.bufferStartSample),
                                                                                         size_t (numThisTime));
            yup::dsp::ProcessContextReplacing <float> context (inOutBlock);
            processorChain.process (context);

            // Update params
            const auto qFactor = qSmoother.skip (numThisTime);
            *hpf = yup::dsp::IIR::ArrayCoefficients<float>::makeHighPass (sampleRate, highFreqSmoother.skip (numThisTime), qFactor);
            *lpf = yup::dsp::IIR::ArrayCoefficients<float>::makeLowPass (sampleRate, lowFreqSmoother.skip (numThisTime), qFactor);
            gain.setGainLinear (yup::Decibels::decibelsToGain (gainSmoother.skip (numThisTime)));

            numSamplesDone += numThisTime;
            numSamplesLeft -= blockSize;

            if (numSamplesDone == fc.bufferNumSamples)
                break;
        }
    }
    else
    {
        // Update params
        const auto qFactor = qSmoother.getCurrentValue();
        *hpf = yup::dsp::IIR::ArrayCoefficients<float>::makeHighPass (sampleRate, highFreqSmoother.getCurrentValue(), qFactor);
        *lpf = yup::dsp::IIR::ArrayCoefficients<float>::makeLowPass (sampleRate, lowFreqSmoother.getCurrentValue(), qFactor);
        gain.setGainLinear (yup::Decibels::decibelsToGain (gainSmoother.getCurrentValue()));

        yup::dsp::AudioBlock<float> inOutBlock (*fc.destBuffer);
        yup::dsp::ProcessContextReplacing <float> context (inOutBlock);
        processorChain.process (context);
    }

    const bool isMixed = wetGainSmoother.getCurrentValue() < 1.0f;
    jassert (fc.bufferStartSample == 0); // This assumes bufferStartSample is always 0 which should be the case
    wetGainSmoother.applyGain (*fc.destBuffer, fc.bufferNumSamples);
    dryGainSmoother.applyGain (dryBuffer.buffer, dryBuffer.buffer.getNumSamples());
    jassert (fc.bufferNumSamples == dryBuffer.buffer.getNumSamples());
    jassert (fc.destBuffer->getNumChannels() == dryBuffer.buffer.getNumChannels());

    if (isMixed)
    {
        for (int c = 0; c < fc.destBuffer->getNumChannels(); ++c)
            fc.destBuffer->addFrom (c, fc.bufferStartSample, dryBuffer.buffer,
                                    c, fc.bufferStartSample, fc.bufferNumSamples);
    }
}

void ImpulseResponsePlugin::restorePluginStateFromValueTree (const yup::ValueTree& v)
{
    copyPropertiesToCachedValues (v, gainValue, highPassCutoffValue, lowPassCutoffValue, mixValue, qValue);

    state.setProperty (IDs::name, v[IDs::name], getUndoManager());

    if (auto irFileData = v.getProperty (IDs::irFileData).getBinaryData())
        state.setProperty (IDs::irFileData, yup::var (yup::MemoryBlock (*irFileData)), getUndoManager());

    for (auto p : getAutomatableParameters())
        p->updateFromAttachedValue();
}

//==============================================================================
void ImpulseResponsePlugin::loadImpulseResponseFromState()
{
    if (auto irFileData = state.getProperty (IDs::irFileData).getBinaryData())
    {
        auto is = std::make_unique<yup::MemoryInputStream> (*irFileData, false);

        if (auto reader = std::unique_ptr<yup::AudioFormatReader> (yup::FlacAudioFormat()
                                                                        .createReaderFor (is.release(), true)))
        {
            yup::AudioBuffer<float> loadIRBuffer ((int) reader->numChannels, (int) reader->lengthInSamples);
            reader->read (&loadIRBuffer, 0, (int) reader->lengthInSamples, 0, true, true);

            jassert (reader->numChannels > 0);
            processorChain.get<convolutionIndex>().loadImpulseResponse (std::move (loadIRBuffer),
                                                                        reader->sampleRate,
                                                                        reader->numChannels > 1 ? yup::dsp::Convolution::Stereo::yes
                                                                                                : yup::dsp::Convolution::Stereo::no,
                                                                        trimSilence.get() ? yup::dsp::Convolution::Trim::yes
                                                                                          : yup::dsp::Convolution::Trim::no,
                                                                        normalise.get() ? yup::dsp::Convolution::Normalise::yes
                                                                                        : yup::dsp::Convolution::Normalise::no);
        }
    }
}

void ImpulseResponsePlugin::valueTreePropertyChanged (yup::ValueTree& v, const yup::Identifier& id)
{
    if (v == state)
    {
        if (id == IDs::irFileData || id == IDs::normalise || id == IDs::trimSilence)
            loadImpulseResponseFromState();
    }
    else
    {
        Plugin::valueTreePropertyChanged (v, id);
    }
}

} // namespace tracktion::inline engine
