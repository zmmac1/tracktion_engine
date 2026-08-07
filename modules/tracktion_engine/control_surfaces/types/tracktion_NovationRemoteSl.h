/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

class NovationRemoteSl  : public ControlSurface,
                          private yup::AsyncUpdater
{
public:
    NovationRemoteSl (ExternalControllerManager&);
    ~NovationRemoteSl();

    void initialiseDevice (bool connect) override;
    void shutDownDevice() override;
    void updateMiscFeatures() override;
    void acceptMidiMessage (int, const yup::MidiMessage&) override;
    void moveFader (int channelNum, float newSliderPos) override;
    void movePanPot (int channelNum, float newPan) override;
    void updateSoloAndMute (int channelNum, Track::MuteAndSoloLightState, bool isBright) override;
    void soloCountChanged (bool) override;
    void playStateChanged (bool isPlaying) override;
    void recordStateChanged (bool isRecording) override;
    void automationReadModeChanged (bool isReading) override;
    void automationWriteModeChanged (bool isWriting) override;
    void faderBankChanged (int newStartChannelNumber, const yup::StringArray& trackNames) override;
    void parameterChanged (int parameterNumber, const ParameterSetting&) override;
    void clearParameter (int parameterNumber) override;
    bool wantsMessage (int, const yup::MidiMessage&) override;
    bool eatsAllMessages() override;
    bool canChangeSelectedPlugin() override;
    void currentSelectionChanged (yup::String) override;
    bool showingPluginParams() override;
    bool showingTracks() override;

private:
    enum RightMode
    {
        rmVol,
        rmPan,
        rmMute,
        rmSolo,
    };

    enum LeftMode
    {
        lmTracks,
        lmPlugins,
        lmParam1,
        lmParam2,
    };

    RightMode rightMode = rmVol;
    LeftMode leftMode = lmTracks;
    int activeTrack = 0;
    int trackOffset = 0;
    int pluginOffset = 0;
    int tempo = 0;
    bool isLocked = false;
    bool wasLocked = false;

    bool leftTopDirty = true;
    bool leftBottomDirty = true;
    bool rightTopDirty = true;
    bool rightBottomDirty = true;

    bool mute[8];
    bool solo[8];
    yup::String trackNames[8];
    float pan[8];
    float level[8];
    ParameterSetting param[16];
    bool online = true;

    yup::String screenContents[4];

    void setRightMode (RightMode);
    void setLeftMode (LeftMode);
    void setLock (bool l, bool w);
    void clearLeft();
    void clearRight();
    void clearAllLights();
    void refreshLeft (bool includeTitle);
    void refreshRight (bool includeTitle);
    void drawString (const yup::String&, int panel);
    yup::String padAndLimit (const yup::String&, int max);

    void handleAsyncUpdate() override;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NovationRemoteSl)
};

} // namespace tracktion::inline engine
