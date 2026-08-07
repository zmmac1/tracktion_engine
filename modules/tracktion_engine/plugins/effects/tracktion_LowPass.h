/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

class LowPassPlugin  : public Plugin
{
public:
    LowPassPlugin (PluginCreationInfo);
    ~LowPassPlugin() override;

    //==============================================================================
    static const char* getPluginName()                  { return NEEDS_TRANS("Low/High-Pass Filter"); }
    static const char* xmlTypeName;

    yup::String getName() const override               { return "LPF/HPF"; }
    yup::String getPluginType() override               { return xmlTypeName; }
    yup::String getShortName (int) override            { return "HP/LP"; }
    yup::String getSelectableDescription() override    { return TRANS("Low/High-Pass Filter"); }

    void initialise (const PluginInitialisationInfo&) override;
    void deinitialise() override;

    int getNumOutputChannelsGivenInputs (int numInputChannels) override  { return yup::jmin (numInputChannels, 2); }
    BusLayout getBusses() const override   { return BusLayout::singleStereoInOut(); }
    void applyToBuffer (const PluginRenderContext&) override;

    bool isLowPass() const noexcept                     { return mode.get() != "highpass"; }

    juce::CachedValue<float> frequencyValue;
    juce::CachedValue<yup::String> mode;
    AutomatableParameter::Ptr frequency;

private:
    yup::IIRFilter filter[2];
    float currentFilterFreq = 0;
    bool isCurrentlyLowPass = false;

    void updateFilters();

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LowPassPlugin)
};

} // namespace tracktion::inline engine
