/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

/** Controls the set of midi-controller-to-parameter mappings. */
class ParameterControlMappings  : public yup::ChangeBroadcaster,
                                  private yup::AsyncUpdater
{
public:
    //==============================================================================
    ParameterControlMappings (Edit&);
    ~ParameterControlMappings() override;

    void loadFromEdit();
    void saveToEdit();

    static ParameterControlMappings* getCurrentlyFocusedMappings (Engine&);

    //==============================================================================
    void checkForDeletedParams();

    //==============================================================================
    /** called by the midi input devices when they get moved. */
    void sendChange (int controllerID, float newValue, int channel);
    bool isParameterMapped (AutomatableParameter& param) const  { return parameters.contains (&param); }
    bool getParameterMapping (AutomatableParameter&, int& channel, int& controllerID) const;
    bool removeParameterMapping (AutomatableParameter&);

    //==============================================================================
    int getNumControllerIDs() const                 { return controllerIDs.size(); }
    void showMappingsListForRow (int);
    int getRowBeingListenedTo() const;
    std::pair<yup::String, yup::String> getTextForRow (int rowNumber) const;

    void listenToRow (int);
    void setLearntParam (bool keepListening);

    struct Mapping
    {
        Mapping (AutomatableParameter* p, int cont, int chan)
            : parameter (p), controllerID (cont), channelID (chan) {}

        AutomatableParameter* parameter = nullptr;
        int controllerID = -1, channelID = -1;
    };

    Mapping getMappingForRow (int row) const;

    void removeMapping (int index);

    /** This will put the surface in listen and assign mode, launching the given dialog window.
        The call will block whilst assignments are made and return when the window is closed.
    */
    void showMappingsEditor (yup::DialogWindow::LaunchOptions&);

private:
    //==============================================================================
    struct ParameterAndIndex
    {
        AutomatableParameter* param;
        int index;
    };

    yup::PopupMenu buildMenu (yup::Array<ParameterAndIndex>& allParams);
    void addPluginToMenu (AutomatableParameterTree::TreeNode*, yup::PopupMenu&, yup::Array<ParameterAndIndex>& allParams, int& index, int addAllItemIndex);
    void addPluginToMenu (Plugin::Ptr, yup::PopupMenu&, yup::Array<ParameterAndIndex>& allParams, int& index, int addAllItemIndex);
    void savePreset (int index);
    void loadPreset (int index);
    void deletePreset (int index);
    yup::StringArray getPresets() const;

    Edit& edit;

    yup::Array<int> controllerIDs, channelIDs;
    yup::ReferenceCountedArray<AutomatableParameter> parameters;
    yup::StringArray parameterFullNames;

    int lastControllerID = 0;
    float lastControllerValue = 0;
    int lastControllerChannel = 0;
    int listeningOnRow = -1;

    yup::CriticalSection lock;

    void tellEditAboutChange();
    int addMapping (int id, int channel, const AutomatableParameter::Ptr&);

    void handleAsyncUpdate() override;
};

} // namespace tracktion::inline engine
