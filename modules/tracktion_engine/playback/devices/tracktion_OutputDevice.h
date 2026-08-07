/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

/** Base class for audio or midi output devices, to which a track's output can be sent.
    The DeviceManager keeps a list of these objects.
*/
class OutputDevice   : public IODevice
{
public:
    OutputDevice (Engine&, yup::String name, yup::String deviceID);
    ~OutputDevice() override;

    //==============================================================================
    bool isMidi() const override                     { return false; }
    bool isTrackDevice() const override              { return false; }

protected:
    //==============================================================================
    virtual yup::String openDevice() = 0;
    virtual void closeDevice() = 0;
};


//==============================================================================
class OutputDeviceInstance
{
public:
    OutputDeviceInstance (OutputDevice&, EditPlaybackContext&);
    virtual ~OutputDeviceInstance();

    OutputDevice& owner;
    EditPlaybackContext& context;
    Edit& edit;

protected:
    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OutputDeviceInstance)
};

} // namespace tracktion::inline engine
