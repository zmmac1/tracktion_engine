/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

// JUCE Changed the format of PluginDescription::createIdentifierString
// breaking all our saved files. This reverts to the old format
yup::String createIdentifierString (const yup::PluginDescription&);

#if TRACKTION_ENABLE_ARA
class ARAPluginBinding;
#endif


/** Wraps a yup::AudioPluginInstance (VST2, VST3, AU, etc.) as a tracktion Plugin.

    Handles lifecycle (async loading, state persistence), parameter bridging,
    bus layout management, and dry/wet mixing for hosted third-party plugins.
*/
class ExternalPlugin  : public Plugin
{
public:
    ExternalPlugin (PluginCreationInfo);
    ~ExternalPlugin() override;

    yup::String getIdentifierString() override { return createIdentifierString (desc); }

    using Ptr = yup::ReferenceCountedObjectPtr<ExternalPlugin>;

    void selectableAboutToBeDeleted() override;

    /** Creates the ValueTree state for inserting an ExternalPlugin into an Edit. */
    static yup::ValueTree create (Engine&, const yup::PluginDescription&);

    void processingChanged() override;

    //==============================================================================
    // Initialisation & lifecycle

    void initialiseFully() override;
    void forceFullReinitialise();

    /// Returns an error message if the plugin failed to load
    yup::String getLoadError();

    /** Returns true if the plugin has started initialising but not completed yet.
        This happens in the case of some external formats like AUv3. You can use
        this to show an indicator in your UI.
    */
    bool isInitialisingAsync() const;

    static const char* xmlTypeName;

    //==============================================================================
    // State persistence

    void flushPluginStateToValueTree() override;

    /** Writes the current bus layout to the plugin's ValueTree state. */
    void flushAudioBusLayoutToValueTree();

    void restorePluginStateFromValueTree (const yup::ValueTree&) override;

    /** Extracts the raw plugin state blob from the ValueTree. */
    void getPluginStateFromTree (yup::MemoryBlock&);

    //==============================================================================
    // Processing & runtime overrides

    void updateFromMirroredPluginIfNeeded (Plugin&) override;

    void initialise (const PluginInitialisationInfo&) override;
    void deinitialise() override;
    void reset() override;
    void midiPanic() override;
    void setEnabled (bool enabled) override;

    yup::Array<Exportable::ReferencedItem> getReferencedItems() override;
    void reassignReferencedItem (const ReferencedItem&, ProjectItemRef newRef, double newStartTime) override;

    void applyToBuffer (const PluginRenderContext&) override;

    //==============================================================================
    // Channel configuration
    //
    // Main-bus overrides report only the main I/O bus for track routing.
    // getChannelNames() reports ALL channels including sidechain.

    bool producesAudioWhenNoAudioInput() override   { return isAutomationNeeded() || isSynth() || ! noTail(); }
    int getNumOutputChannelsGivenInputs (int numInputs) override;
    BusLayout getBusses() const override;
    void getChannelNames (yup::StringArray* ins, yup::StringArray* outs) override;

    bool isVST() const noexcept             { return desc.name == "VST"; }
    bool isVST3() const noexcept            { return desc.name == "VST3"; }
    bool isAU() const noexcept              { return desc.name == "AudioUnit"; }
    yup::String getName() const override   { return desc.name; }
    yup::String getVendor() override       { return desc.vendor; }
    yup::String getTooltip() override      { return getName() + "$vstfilter"; }
    yup::String getPluginType() override   { return xmlTypeName; }
    bool isSynth() override                 { return desc.isInstrument; }
    bool takesMidiInput() override;
    bool takesAudioInput() override         { return (! isSynth()) || (dryGain->getCurrentValue() > 0.0f); }
    bool isMissing() override;
    bool isDisabled() override;
    double getLatencySeconds() override     { return latencySeconds; }
    bool noTail() override;
    double getTailLength() const override;
    void trackPropertiesChanged() override;

    yup::AudioProcessor* getWrappedAudioProcessor() const override     { return getAudioPluginInstance(); }
    void deleteFromParent() override;

    //==============================================================================
    // selectable stuff
    yup::String getSelectableDescription() override;

    //==============================================================================
    yup::File getFile() const;
    yup::String getPluginUID() const           { return yup::String::toHexString (desc.deprecatedUid); }

    const char* getDebugName() const noexcept   { return debugName.toUTF8(); }

    //==============================================================================
    // Bus layout

    /** Attempts to change the full bus layout of the wrapped plugin.
        Temporarily stops playback to safely reconfigure. */
    bool setAudioBusLayout (yup::AudioProcessor::AudioBusLayout);

    /** Attempts to change a single bus layout. */
    bool setBusLayout (yup::AudioChannelSet, bool isInput, int busIndex);

    //==============================================================================
    int getNumPrograms() const;
    int getCurrentProgram() const;
    yup::String getProgramName (int index);
    yup::String getNumberedProgramName (int i);
    yup::String getCurrentProgramName();
    void setCurrentProgram (int index, bool sendChangeMessage);
    void setCurrentProgramName (const yup::String& name);
    bool hasNameForMidiProgram (int programNum, int bank, yup::String& name) override;
    bool hasNameForMidiNoteNumber (int note, int midiChannel, yup::String& name) override;

    //==============================================================================
    const VSTXML* getVSTXML() const noexcept        { return vstXML.get(); }

    //==============================================================================
    // Plugin instance access

    /** Returns the underlying yup::AudioPluginInstance, or nullptr if not yet loaded. */
    yup::AudioPluginInstance* getAudioPluginInstance() const;

    /** If enabled, the wrapped yup::AudioPluginInstance is destroyed synchronously when
        this plugin is deleted rather than via the shared asynchronous deleter. ARA-bound
        instances need this: the ARA spec requires a bound plugin instance to be destroyed
        before the document controller it is bound to. */
    void setDeletesPluginInstanceSynchronously (bool shouldDeleteSynchronously) noexcept    { deleteInstanceSynchronously = shouldDeleteSynchronously; }

   #if TRACKTION_ENABLE_ARA
    /** Takes ownership of this plugin's ARA document binding
        (see ARADocumentHolder::bindPluginToDocument). The binding is destroyed -
        unregistering from the ARA document - just before the wrapped instance is
        deleted, so it can never dangle or outlive the plugin. */
    void setARADocumentBinding (std::unique_ptr<ARAPluginBinding>);

    /** Returns true if this plugin owns an ARA document binding, i.e. it was bound via
        ARADocumentHolder::bindPluginToDocument (the browser/panel case). The bind lasts
        for the lifetime of the wrapped instance, so a plugin returned from the
        PluginCache may already be bound.
        NB: clip-player instances are ARA-bound too, but they're owned by their
        ARAClipPlayer and never pass through the PluginCache, so they don't show here. */
    bool isBoundToARADocument() const noexcept          { return araBinding != nullptr; }
   #endif

    /** The plugin's description (format, name, manufacturer, file path, etc.). */
    yup::PluginDescription desc;

    /** Dry/wet mix values and their automatable parameters. */
    juce::CachedValue<float> dryValue, wetValue;
    AutomatableParameter::Ptr dryGain, wetGain;

    ActiveNoteList getActiveNotes() const           { return activeNotes; }

    //==============================================================================
    std::unique_ptr<EditorComponent> createEditor() override;

    //==============================================================================
    // Deprecated

    /** @deprecated Use getAudioPluginInstance()->getTotalNumInputChannels() instead. */
    [[deprecated ("Use getAudioPluginInstance()->getTotalNumInputChannels() instead")]]
    int getTotalNumInputChannels() const;

private:
    //==============================================================================
    yup::CriticalSection processMutex;
    yup::String debugName, identiferString, loadError;

    class ProcessorChangedManager;
    class LoadedInstance;
    std::unique_ptr<LoadedInstance> loadedInstance;
    std::atomic<bool> hasLoadedInstance { false }, isInstancePrepared { false }, isAsyncInitialising { false };
    bool deleteInstanceSynchronously = false;

   #if TRACKTION_ENABLE_ARA
    std::unique_ptr<ARAPluginBinding> araBinding;
   #endif

    std::unique_ptr<VSTXML> vstXML;
    int latencySamples = 0;
    double latencySeconds = 0;

    double lastSampleRate = 0.0;
    int lastBlockSizeSamples = 0;

    yup::MidiBuffer midiBuffer;
    MPESourceID midiSourceID = createUniqueMPESourceID();
    std::atomic<bool> midiPanicNeeded { false };

    ActiveNoteList activeNotes;

    class PluginPlayHead;
    std::unique_ptr<PluginPlayHead> playhead;

    bool fullyInitialised = false, supportsMPE = false, isFlushingLayoutToState = false;

    struct MPEChannelRemapper;
    std::unique_ptr<MPEChannelRemapper> mpeRemapper;

    void prepareIncomingMidiMessages (MidiMessageArray& incoming, int numSamples, bool isPlaying);

    yup::Array<ExternalAutomatableParameter*> autoParamForParamNumbers;

    //==============================================================================
    void startPluginInstanceCreation (const yup::PluginDescription&);
    void completePluginInstanceCreation (std::unique_ptr<yup::AudioPluginInstance>);
    void deletePluginInstance();

    //==============================================================================
    void buildParameterTree() const override;
    void buildParameterTree (const VSTXML::Group*, AutomatableParameterTree::TreeNode*, yup::SortedSet<int>&) const;

    //==============================================================================
    void doFullInitialisation();
    void buildParameterList();
    void refreshParameterValues();
    void updateDebugName();
    void processPluginBlock (yup::AudioPluginInstance&, const PluginRenderContext&, bool processedBypass);

    std::unique_ptr<yup::PluginDescription> findMatchingPlugin() const;

    void valueTreePropertyChanged (yup::ValueTree&, const yup::Identifier&) override;

    //==============================================================================
    static bool requiresAsyncInstantiation (Engine&, const yup::PluginDescription&);

    //==============================================================================
    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExternalPlugin)
};

//==============================================================================
/** Specialised AutomatableParameter for wet/dry.
    Having a subclass just lets it label itself more nicely.
 */
struct PluginWetDryAutomatableParam  : public AutomatableParameter
{
    PluginWetDryAutomatableParam (const yup::String& xmlTag, const yup::String& name, Plugin&);
    ~PluginWetDryAutomatableParam() override;

    yup::String valueToString (float value) override;
    float stringToValue (const yup::String& s) override;

    PluginWetDryAutomatableParam() = delete;
    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginWetDryAutomatableParam)
};

} // namespace tracktion::inline engine
