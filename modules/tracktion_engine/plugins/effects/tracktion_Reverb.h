/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

class ReverbPlugin : public Plugin
{
public:
    ReverbPlugin (PluginCreationInfo);
    ~ReverbPlugin() override;

    //==============================================================================
    static const char* getPluginName()                  { return NEEDS_TRANS("Reverb"); }
    static const char* xmlTypeName;

    yup::String getName() const override               { return TRANS("Reverb"); }
    yup::String getPluginType() override               { return xmlTypeName; }

    void initialise (const PluginInitialisationInfo&) override;
    void deinitialise() override;
    void reset() override;
    int getNumOutputChannelsGivenInputs (int numInputChannels) override { return yup::jmin (numInputChannels, 2); }
    BusLayout getBusses() const override   { return BusLayout::singleStereoInOut(); }
    void applyToBuffer (const PluginRenderContext&) override;
    yup::String getSelectableDescription() override    { return TRANS("Reverb Plugin"); }
    void restorePluginStateFromValueTree (const yup::ValueTree&) override;

    //==============================================================================
    void setRoomSize (float value);
    float getRoomSize();

    void setDamp (float value);
    float getDamp();

    void setWet (float value);
    float getWet();

    void setDry (float value);
    float getDry();

    void setWidth (float value);
    float getWidth();

    void setMode (float value);
    float getMode();

    juce::CachedValue<float> roomSizeValue, dampValue, wetValue,
                             dryValue, widthValue, modeValue;

    AutomatableParameter::Ptr roomSizeParam, dampParam, wetParam,
                              dryParam, widthParam, modeParam;

private:
    bool outputSilent = true;
    yup::Reverb reverb;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverbPlugin)
};

} // namespace tracktion::inline engine
