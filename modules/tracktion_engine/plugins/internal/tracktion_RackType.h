/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

struct RackConnection
{
    RackConnection (const yup::ValueTree&, juce::UndoManager*);

    yup::ValueTree state;

    juce::CachedValue<EditItemID> sourceID, destID;
    juce::CachedValue<int> sourcePin, destPin;
};


//==============================================================================
class RackType  : public EditItem,
                  public Selectable,
                  public yup::ReferenceCountedObject,
                  public MacroParameterElement,
                  private yup::ValueTree::Listener
{
public:
    RackType (Edit&, const yup::ValueTree&);
    ~RackType() override;

    using Ptr = yup::ReferenceCountedObjectPtr<RackType>;

    yup::ValueTree createStateCopy (bool includeAutomation);
    yup::Result restoreStateFromValueTree (const yup::ValueTree&);

    //==============================================================================
    static Ptr createTypeToWrapPlugins (const Plugin::Array&, Edit&);

    //==============================================================================
    // tells the rack type about each instance that will be using it..
    // These are called by the initialise methods of the instances.
    void registerInstance (RackInstance*, const PluginInitialisationInfo&);
    void initialisePluginsIfNeeded (const PluginInitialisationInfo&) const;
    void deregisterInstance (RackInstance*);

    void updateAutomatableParamPositions (TimePosition);

    double getLatencySeconds (double sampleRate, int blockSize);

    //==============================================================================
    yup::Array<Plugin*> getPlugins() const;

    // check whether this plugin's allowed to go in a rack, so e.g. racks
    // themselves aren't allowed.
    static bool isPluginAllowed (const Plugin::Ptr&);

    /** Adds a plugin to the Rack optionally connecting it to the input and outputs.
        @returns    true if the plugin was added, false otherwise
    */
    bool addPlugin (const Plugin::Ptr&, yup::Point<float> pos, bool canAutoConnect);

    Plugin* getPluginForID (EditItemID);

    yup::Point<float> getPluginPosition (const Plugin::Ptr&) const;
    yup::Point<float> getPluginPosition (int index) const;
    void setPluginPosition (int index, yup::Point<float> pos);

    void flushStateToValueTree();

    //==============================================================================
    yup::Array<const RackConnection*> getConnections() const noexcept;

    bool addConnection (EditItemID source, int sourcePin,
                        EditItemID dest, int destPin);

    bool removeConnection (EditItemID source, int sourcePin,
                           EditItemID dest, int destPin);

    // check that this doesn't create loops, etc
    bool isConnectionLegal (EditItemID source, int sourcePin,
                            EditItemID dest, int destPin) const;

    yup::Array<EditItemID> getPluginsWhichTakeInputFrom (EditItemID) const;

    void checkConnections();
    static void removeBrokenConnections (yup::ValueTree&, juce::UndoManager*);

    void createInstanceForSideChain (Track&, const yup::BigInteger& channelMask,
                                     EditItemID pluginID, int pinIndex);

    //==============================================================================
    yup::String getName() const override               { return rackName; }

    yup::StringArray getInputNames() const;
    yup::StringArray getOutputNames() const;

    int addInput (int index, const yup::String& name);
    int addOutput (int index, const yup::String& name);
    void removeInput (int index);
    void removeOutput (int index);

    void renameInput (int index, const yup::String& name);
    void renameOutput (int index, const yup::String& name);

    bool pasteClipboard();

    //==============================================================================
    ModifierList& getModifierList() const noexcept;

    //==============================================================================
    juce::UndoManager* getUndoManager() const;

    //==============================================================================
    yup::String getSelectableDescription() override;

    //==============================================================================
    yup::StringArray getAudioInputNamesWithChannels() const;
    yup::StringArray getAudioOutputNamesWithChannels() const;

    static RackType::Ptr findRackTypeContaining (const Plugin&);

    static const char* getRackPresetPrefix() noexcept   { return "rackpreset::"; }

    //==============================================================================
    struct WindowState  : public PluginWindowState
    {
        WindowState (RackType&, yup::ValueTree windowStateTree);

        RackType& rack;
        yup::ValueTree state;
        YUP_DECLARE_NON_COPYABLE (WindowState)
    };

    yup::Array<WindowState*> getWindowStates() const;

    void loadWindowPosition();
    void saveWindowPosition();
    void hideWindowForShutdown();

    yup::ValueTree state;
    juce::CachedValue<yup::String> rackName;

private:
    struct PluginInfo
    {
        Plugin::Ptr plugin;
        yup::ValueTree state;
    };

    struct RackPluginList;
    std::unique_ptr<RackPluginList> pluginList;

    struct ConnectionList;
    std::unique_ptr<ConnectionList> connectionList;

    struct WindowStateList;
    std::unique_ptr<WindowStateList> windowStateList;

    mutable std::unique_ptr<ModifierList> modifierList;

    yup::Array<RackInstance*> activeRackInstances;

    //==============================================================================
    bool arePluginsConnectedIndirectly (EditItemID src, EditItemID dest, int depth = 0) const;

    void removeAllInputsAndOutputs();
    void addDefaultInputs();
    void addDefaultOutputs();

    void triggerUpdate();
    void updateRenderContext();

    //==============================================================================
    void valueTreeChildAdded (yup::ValueTree&, yup::ValueTree&) override;
    void valueTreeChildRemoved (yup::ValueTree&, yup::ValueTree&, int) override;
    void valueTreeChildOrderChanged (yup::ValueTree&, int, int) override;
    void valueTreePropertyChanged (yup::ValueTree&, const yup::Identifier&) override;
    void valueTreeParentChanged (yup::ValueTree&) override;
    void valueTreeRedirected (yup::ValueTree&) override;
};


//==============================================================================
class RackTypeList
{
public:
    RackTypeList (Edit&);
    ~RackTypeList();

    void initialise (const yup::ValueTree&);
    bool isInitialised() const;

    int size() const;
    RackType::Ptr getRackType (int index) const;
    RackType::Ptr getRackTypeForID (EditItemID) const;
    RackType::Ptr findRackContaining (Plugin&) const;
    RackType::Ptr addRackTypeFrom (const yup::ValueTree&);
    RackType::Ptr addNewRack();
    RackType::Ptr duplicateRack (EditItemID);
    RackType::Ptr duplicateRack (RackType&);
    void removeRackType (const RackType::Ptr&);
    void importRackFiles (const yup::Array<yup::File>&);

    const yup::Array<RackType*>& getTypes() const noexcept;

private:
    Edit& edit;
    yup::ValueTree state;

    struct ValueTreeList;
    std::unique_ptr<ValueTreeList> list;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RackTypeList)
};

} // namespace tracktion::inline engine
