/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

class EditInputDevices  : private yup::ChangeListener,
                          private yup::AsyncUpdater,
                          private yup::ValueTree::Listener
{
public:
    EditInputDevices (Edit&, const yup::ValueTree& state);
    ~EditInputDevices() override;

    int getMaxNumInputs() const;

    bool isInputDeviceAssigned (const InputDevice&);

    void clearAllInputs (AudioTrack&, juce::UndoManager*);
    void clearInputsOfDevice (AudioTrack&, const InputDevice&, juce::UndoManager*);
    InputDeviceInstance* getInputInstance (const AudioTrack&, int index) const;
    yup::Array<InputDeviceInstance*> getDevicesForTargetTrack (const AudioTrack&) const;

    /** @internal */
    yup::ValueTree getInstanceStateForInputDevice (const InputDevice&);

private:
    Edit& edit;
    yup::ValueTree state, editState { edit.state };

    void removeNonExistantInputDeviceStates();
    void addTrackDeviceInstanceToContext (const yup::ValueTree&) const;
    void removeTrackDeviceInstanceToContext (const yup::ValueTree&) const;
    InputDevice* getTrackDeviceForState (const yup::ValueTree&) const;

    void changeListenerCallback (yup::ChangeBroadcaster*) override;
    void handleAsyncUpdate() override;

    void valueTreeChildAdded (yup::ValueTree&, yup::ValueTree&) override;
    void valueTreeChildRemoved (yup::ValueTree&, yup::ValueTree&, int) override;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditInputDevices)
};

} // namespace tracktion::inline engine
