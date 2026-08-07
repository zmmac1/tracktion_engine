/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

/**
    Acts as a holder for a ControlSurface object.
*/
class ExternalController  : private yup::AsyncUpdater,
                            private SelectableListener,
                            private AutomatableParameter::Listener,
                            private yup::ChangeListener,
                            private yup::Timer
{
public:
    //==============================================================================
    ExternalController (Engine&, ControlSurface*);
    ~ExternalController() override;

    //==============================================================================
    yup::String getName() const;

    bool wantsDevice (const MidiID& m);

    bool needsMidiChannel() const               { return needsChannel; }
    bool needsMidiBackChannel() const           { return needsBackChannel; }

    yup::String getDesiredMidiChannel() const;
    yup::String getDesiredMidiBackChannel() const;

    bool needsOSCSocket() const                 { return needsOSC; }

    int getNumDevices() const;
    void setNumDevices (int);

    int getMainDevice() const;
    void setMainDevice (int);

    yup::String getMidiInputDevice (int idx) const;
    void setMidiInputDevice (int idx, const yup::String& nameOfMidiInput);

    yup::String getBackChannelDevice (int idx) const;
    void setBackChannelDevice (int idx, const yup::String& nameOfMidiOutput);
    bool isUsingMidiOutputDevice (const MidiOutputDevice* d) const noexcept;

    int getOSCInputPort()                       { return oscInputPort; }
    void setOSCInputPort (int port);
    int getOSCOutputPort()                      { return oscOutputPort; }
    void setOSCOutputPort (int port);
    yup::String getOSCOutputAddress()          { return oscOutputAddr; }
    void setOSCOutputAddress (const yup::String addr);

    bool isEnabled() const;
    void setEnabled (bool);

    bool isDeletable() const                    { return deletable; }
    void deleteController();

    bool wantsMidiClock() const                 { return wantsClock; }

    void currentEditChanged (Edit*);
    void currentSelectionManagerChanged (SelectionManager*);

    // these will be called by tracktion when stuff happens, and
    // will pass the message on to the controller.

    // channels are virtual - i.e. not restricted to physical chans
    void moveFader (int channelNum, float newSliderPos);
    void moveMasterFader (float newPos);
    void movePanPot (int channelNum, float newPan);
    void moveMasterPanPot (float newPos);
    void updateSoloAndMute (int channelNum, Track::MuteAndSoloLightState, bool isBright);
    void soloCountChanged (bool);
    void playStateChanged (bool isPlaying);
    void recordStateChanged (bool isRecording);
    void automationModeChanged (bool isReading, bool isWriting);
    void snapChanged (bool isOn);
    void loopChanged (bool isOn);
    void clickChanged (bool isOn);
    void channelLevelChanged (int channel, std::span<const float> levels);
    void masterLevelsChanged (std::span<const float> levels);
    void timecodeChanged (int barsOrHours, int beatsOrMinutes, int ticksOrSeconds, int millisecs, bool isBarsBeats, bool isFrames);
    void trackSelected (int channel, bool isSelected);
    void selectOtherObject (SelectableClass::Relationship, bool moveFromCurrentPlugin);
    void muteOrUnmutePlugin();
    void muteOrUnmutePluginsInTrack();
    void changePluginPreset (int delta);
    void soloPluginTrack();
    void auxSendLevelsChanged();

    void updateDeviceState();
    void updateParameters();
    void updateMarkers();
    void selectedPluginChanged();
    void selectableObjectChanged (Selectable*) override;
    void selectableObjectAboutToBeDeleted (Selectable*) override;
    void curveHasChanged (AutomatableParameter&) override;
    void currentValueChanged (AutomatableParameter&) override;
    void updateTrackSelectLights();
    void updateTrackRecordLights();
    void updatePunchLights();
    void updateScrollLights();
    void updateUndoLights();
    void updatePadColors();
    void clearPadColors();

    int getNumFaderChannels() const noexcept;
    int getFaderIndexInActiveRegion (int num) const noexcept;
    yup::Range<int> getActiveChannels() const noexcept;

    int getNumParameterControls() const noexcept;
    yup::Range<int> getActiveParams() const noexcept;

    void midiInOutDevicesChanged();
    void oscSettingsChanged();

    //==============================================================================
    void handleAsyncUpdate() override;
    void acceptMidiMessage (MidiInputDevice&, const yup::MidiMessage&);
    bool wantsMessage (MidiInputDevice&, const yup::MidiMessage&);
    bool eatsAllMessages() const;
    bool canSetEatsAllMessages();
    void setEatsAllMessages (bool eatAll);

    //==============================================================================
    yup::Color getSelectionColor() const             { return selectionColor; }
    bool getShowTrackSelectionColor() const            { return showTrackSelection; }
    bool getShowClipSlotSelectionColor() const         { return showClipSlotSelection; }
    void setSelectionColor (yup::Color);
    void setShowTrackSelectionColor (bool);
    void setShowClipSlotSelectionColor (bool);

    bool shouldTrackBeColored (int channelNum);
    void getTrackColor (int channelNum, yup::Color&);
    std::optional<ColorArea> getColoredArea (const Edit&);

    bool shouldPluginBeColored (Plugin*);
    void getPluginColor (Plugin*, yup::Color&);
    void repaintParamSource();
    void redrawTracks();

    ControlSurface& getControlSurface() const noexcept  { return *controlSurface; }
    template <typename Type>
    Type* getControlSurfaceIfType() const noexcept      { return dynamic_cast<Type*> (controlSurface.get()); }

    ExternalControllerManager& getExternalControllerManager() const noexcept    { return controlSurface->externalControllerManager; }

    Edit* getEdit() const                               { return getControlSurface().getEdit(); }
    TransportControl* getTransport() const noexcept     { return getControlSurface().getTransport(); }

    static yup::String shortenName (yup::String, int maxLen);

    yup::String getInputDeviceName (int idx) const     { return inputDeviceNames[idx]; }
    yup::String getOutputDeviceName (int idx) const    { return outputDeviceNames[idx]; }

    yup::StringArray getMidiInputPorts() const;
    yup::StringArray getMidiOutputPorts() const;

    static yup::String getNoDeviceSelectedMessage();

    Engine& engine;

private:
    void timerCallback() override;
    void changeListenerCallback (yup::ChangeBroadcaster*) override;

    static constexpr int maxDevices = 4;
    friend class ExternalControllerManager;
    friend class ControlSurface;
    friend class MackieC4;
    friend class MackieMCU;

    int numDevices = 1;
    int mainDevice = 0;
    yup::StringArray inputDeviceNames, outputDeviceNames;

    std::unique_ptr<ControlSurface> controlSurface;
    int oscInputPort, oscOutputPort;
    yup::String oscOutputAddr;

    bool enabled;
    mutable std::optional<bool> hasMidiInput;
    int maxTrackNameChars;
    int channelStart = 0;
    int padStart = 0;
    int startParamNumber = 0;
    bool needsBackChannel = false;
    bool needsChannel = false;
    bool needsOSC = false;
    bool usesSettings = false;
    bool deletable = false;
    bool allowBankingOffEnd = false;
    AutomatableParameter::Array currentParams;
    SafeSelectable<Selectable> currentParamSource, lastRegisteredSelectable;
    SafeScopedListener lastRegisteredListener;
    bool showTrackSelection = false;
    bool showClipSlotSelection = true;
    yup::Color selectionColor;
    int startMarkerNumber = 0;
    int auxBank = 0;
    bool followsTrackSelection;
    bool processMidi = false, updateParams = false;

    std::vector<MidiInputDevice*> inputDevices;
    std::vector<MidiOutputDevice*> outputDevices;

    yup::Array<std::pair<int, yup::MidiMessage>> pendingMidiMessages;
    yup::CriticalSection incomingMidiLock;

    int getMarkerBankOffset() const   { return startMarkerNumber; }
    int getFaderBankOffset() const    { return channelStart;      }
    int getAuxBankOffset() const      { return auxBank;           }
    int getParamBankOffset() const    { return startParamNumber;  }
    int getClipSlotOffset() const     { return padStart;          }

    void changeFaderBank (int delta, bool moveSelection);
    void changePadBank (int delta);
    void changeParamBank (int delta);
    void updateParamList();
    void changeMarkerBank (int delta);
    void changeAuxBank (int delta);
    void setAuxBank (int num);

    void userMovedParameterControl (int paramNumber, float newValue, bool delta);
    void userPressedParameterControl (int paramNumber);
    void userMovedParameterControl (int parameter, bool touch);
    void userPressedGoToMarker (int marker);

    Plugin* getCurrentPlugin() const;

    bool wantsClock;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExternalController)
};

} // namespace tracktion::inline engine
