/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

class ToneGeneratorPlugin   : public Plugin
{
public:
    ToneGeneratorPlugin (PluginCreationInfo);
    ~ToneGeneratorPlugin() override;

    //==============================================================================
    static const char* getPluginName()                  { return NEEDS_TRANS("Tone Generator"); }
    static const char* xmlTypeName;

    yup::String getName() const override               { return TRANS(getPluginName()); }
    yup::String getPluginType() override               { return xmlTypeName; }
    yup::String getShortName (int) override            { return getName(); }
    bool producesAudioWhenNoAudioInput() override       { return true; }
    bool isSynth() override                             { return true; }

    int getNumOutputChannelsGivenInputs (int numInputChannels) override { return yup::jmax (1, numInputChannels); }
    BusLayout getBusses() const override   { return BusLayout::singleStereoInOut(); }
    void initialise (const PluginInitialisationInfo&) override;
    void deinitialise() override;
    void applyToBuffer (const PluginRenderContext&) override;
    yup::String getSelectableDescription() override    { return getName(); }

    void restorePluginStateFromValueTree (const yup::ValueTree&) override;

    enum class OscType
    {
        sin,
        triangle,
        sawUp,
        sawDown,
        square,
        noise
    };

    static yup::StringArray getOscTypeNames();

    juce::CachedValue<float> oscType, bandLimit, frequency, level;
    AutomatableParameter::Ptr oscTypeParam, bandLimitParam, frequencyParam, levelParam;

private:
    //==============================================================================
    yup::AudioBuffer<float> scratch;
    std::atomic<bool> bandLimitOsc { false };

    yup::dsp::Oscillator<float> sine;
    yup::dsp::Oscillator<float> triangle;
    yup::dsp::Oscillator<float> sawUp;
    yup::dsp::Oscillator<float> sawDown;
    yup::dsp::Oscillator<float> square;
    yup::dsp::Oscillator<float> noise;

    void initialiseOscilators();

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToneGeneratorPlugin)
};

} // namespace tracktion::inline engine
