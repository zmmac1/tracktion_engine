/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

class VirtualMidiInputDevice  : public MidiInputDevice
{
public:
    VirtualMidiInputDevice (Engine&, yup::String name, DeviceType, yup::String deviceID, bool isAllMIDIIns);
    ~VirtualMidiInputDevice() override;

    InputDeviceInstance* createInstance (EditPlaybackContext&) override;

    using MidiInputDevice::handleIncomingMidiMessage;
    void handleIncomingMidiMessage (const yup::MidiMessage&, MPESourceID) override;

    void setEnabled (bool) override;
    void loadProps() override;
    void saveProps() override;

    yup::StringArray getMIDIInputSourceDevices() const         { return inputDeviceIDs; }
    void setMIDIInputSourceDevices (const yup::StringArray deviceIDs);
    void toggleMIDIInputSourceDevice (const yup::String& deviceID);

    void handleMessageFromPhysicalDevice (PhysicalMidiInputDevice&, const yup::MidiMessage&);

    DeviceType getDeviceType() const override;
    yup::String getDeviceTypeDescription() const override;

    const bool useAllInputs = false;

private:
    yup::String openDevice() override;
    void closeDevice() override;
    DeviceType deviceType;
    yup::StringArray inputDeviceIDs;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VirtualMidiInputDevice)
};

} // namespace tracktion::inline engine
