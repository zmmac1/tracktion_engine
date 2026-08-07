/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

class PatchBayPlugin   : public Plugin
{
public:
    PatchBayPlugin (PluginCreationInfo);
    ~PatchBayPlugin() override;

    //==============================================================================
    struct Wire
    {
        Wire (const yup::ValueTree&, juce::UndoManager*);

        yup::ValueTree state;
        juce::CachedValue<int> sourceChannelIndex, destChannelIndex;
        juce::CachedValue<float> gainDb;
    };

    //==============================================================================
    int getNumWires() const;
    Wire* getWire (int index) const;

    void makeConnection (int inputChannel, int outputChannel, float gainDb, juce::UndoManager*);
    void breakConnection (int inputChannel, int outputChannel);

    //==============================================================================
    static const char* getPluginName()                  { return NEEDS_TRANS("Patch Bay"); }
    static const char* xmlTypeName;

    yup::String getName() const override               { return TRANS("Patch Bay Plugin"); }
    yup::String getPluginType() override               { return xmlTypeName; }
    yup::String getShortName (int) override            { return TRANS("Patch"); }
    yup::String getSelectableDescription() override    { return TRANS("Patch Bay Plugin"); }
    bool canBeAddedToClip() override                    { return false; }
    bool canBeAddedToRack() override                    { return false; }
    bool canBeDisabled() override                       { return false; }

    int getNumOutputChannelsGivenInputs (int numInputChannels) override;
    BusLayout getBusses() const override   { return BusLayout::singlePassThrough(); }

    void initialise (const PluginInitialisationInfo&) override;
    void deinitialise() override;
    void valueTreeParentChanged (yup::ValueTree&) override;

    void getChannelNames (yup::StringArray*, yup::StringArray*) override;
    void applyToBuffer (const PluginRenderContext&) override;

private:
    int getNumInputChannelsFromChain() const;
    int getNumOutputChannelsFromChain() const;
    struct WireList;
    std::unique_ptr<WireList> list;
    std::unique_ptr<LambdaValueTreeAllEventListener> parentListener;
    SafeSelectable<Plugin> inputPlugin, outputPlugin;

    void cacheInputAndOutputPlugins();
    void listenToParentList();

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PatchBayPlugin)
};

} // namespace tracktion::inline engine
