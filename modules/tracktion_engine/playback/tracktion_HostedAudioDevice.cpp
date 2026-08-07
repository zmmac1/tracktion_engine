/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2019
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

//==============================================================================
class HostedAudioDevice : public yup::AudioIODevice
{
public:
    HostedAudioDevice (HostedAudioDeviceInterface& aif, std::function<void (HostedAudioDevice*)> onDestroy_)
        : AudioIODevice ("Hosted Device", "Hosted Device"), audioIf (aif), onDestroy (onDestroy_)
    {}

    ~HostedAudioDevice() override
    {
        if (onDestroy)
            onDestroy (this);
    }

    yup::StringArray getOutputChannelNames() override      { return audioIf.getOutputChannelNames();               }
    yup::StringArray getInputChannelNames() override       { return audioIf.getInputChannelNames();                }
    yup::Array<double> getAvailableSampleRates() override  { return { audioIf.parameters.sampleRate };             }
    yup::Array<int> getAvailableBufferSizes() override     { return { audioIf.parameters.blockSize };              }
    int getDefaultBufferSize() override                     { return audioIf.parameters.blockSize;                  }

    yup::String open (const yup::BigInteger& inputChannels,
                       const yup::BigInteger& outputChannels,
                       double sampleRate, int bufferSizeSamples) override
    {
        ignoreUnused (inputChannels, outputChannels, sampleRate, bufferSizeSamples);
        return {};
    }

    void close() override                                   {}
    void start (yup::AudioIODeviceCallback* callback_) override
    {
        callback = callback_;
        callback->audioDeviceAboutToStart (this);
    }

    void stop() override
    {
        callback->audioDeviceStopped();
        callback = nullptr;
    }

    bool isOpen() override                                  { return true;  }
    bool isPlaying() override                               { return true;  }
    yup::String getLastError() override                    { return {};    }
    int getCurrentBitDepth() override                       { return 16;    }
    int getOutputLatencyInSamples() override                { return audioIf.parameters.outputLatencyNumSamples; }
    int getInputLatencyInSamples() override                 { return audioIf.parameters.inputLatencyNumSamples; }
    bool hasControlPanel() const override                   { return false; }
    bool showControlPanel() override                        { return false; }
    bool setAudioPreprocessingEnabled (bool) override       { return false; }
    int getCurrentBufferSizeSamples() override              { return audioIf.parameters.blockSize;      }
    double getCurrentSampleRate() override                  { return audioIf.parameters.sampleRate;     }

    yup::BigInteger getActiveOutputChannels() const override
    {
        yup::BigInteger res;
        res.setRange (0, audioIf.parameters.outputChannels, true);
        return res;
    }

    yup::BigInteger getActiveInputChannels() const override
    {
        yup::BigInteger res;
        res.setRange (0, audioIf.parameters.inputChannels, true);
        return res;
    }

    void processBlock (yup::AudioBuffer<float>& buffer)
    {
        if (callback != nullptr)
            callback->audioDeviceIOCallbackWithContext (buffer.getArrayOfReadPointers(),
                                                        std::min (buffer.getNumChannels(), audioIf.parameters.inputChannels),
                                                        buffer.getArrayOfWritePointers(),
                                                        std::min (buffer.getNumChannels(), audioIf.parameters.outputChannels),
                                                        buffer.getNumSamples(),
                                                        {});
    }

    void settingsChanged()
    {
        if (callback != nullptr)
            callback->audioDeviceAboutToStart (this);
    }

private:
    HostedAudioDeviceInterface& audioIf;
    std::function<void (HostedAudioDevice*)> onDestroy;
    yup::AudioIODeviceCallback* callback = nullptr;
};

//==============================================================================
class HostedAudioDeviceType  : public yup::AudioIODeviceType
{
public:
    HostedAudioDeviceType (HostedAudioDeviceInterface& aif)
        : AudioIODeviceType ("Hosted Device"), audioIf (aif)
    {}

    ~HostedAudioDeviceType() override
    {
        if (audioIf.deviceType == this)
            audioIf.deviceType = nullptr;
    }

    void scanForDevices() override                                          {}
    yup::StringArray getDeviceNames (bool = false) const override          { return { "Hosted Device" }; }
    int getDefaultDeviceIndex (bool) const override                         { return 0; }
    int getIndexOfDevice (yup::AudioIODevice*, bool) const override        { return 0; }
    bool hasSeparateInputsAndOutputs() const override                       { return false; }

    yup::AudioIODevice* createDevice (const yup::String&, const yup::String&) override
    {
        auto device = new HostedAudioDevice (audioIf, [ptr = makeWeakRef (*this)] (HostedAudioDevice* d)
                                                      {
                                                          if (ptr != nullptr)
                                                              ptr->devices.removeFirstMatchingValue (d);
                                                      });
        devices.add (device);
        return device;
    }

    void processBlock (yup::AudioBuffer<float>& buffer)
    {
        for (auto device : devices)
            device->processBlock (buffer);
    }

    void settingsChanged()
    {
        for (auto device : devices)
            device->settingsChanged();
    }

private:
    HostedAudioDeviceInterface& audioIf;
    yup::Array<HostedAudioDevice*> devices;

    YUP_DECLARE_WEAK_REFERENCEABLE (HostedAudioDeviceType)
};

//==============================================================================
class HostedMidiInputDevice : public MidiInputDevice
{
public:
    HostedMidiInputDevice (HostedAudioDeviceInterface& aif)
        : MidiInputDevice (aif.engine, TRANS("MIDI Input"), "MIDI Input")
    {
    }

    DeviceType getDeviceType() const override
    {
        return virtualMidiDevice;
    }

    yup::String getDeviceTypeDescription() const override
    {
        return TRANS("MIDI Input");
    }

    InputDeviceInstance* createInstance (EditPlaybackContext& epc) override
    {
        return new HostedMidiInputDeviceInstance (*this, epc);
    }

    void loadProps() override
    {
        auto n = engine.getPropertyStorage().getXmlPropertyItem (SettingID::midiin, getName());
        MidiInputDevice::loadMidiProps (n.get());
    }

    void saveProps() override
    {
        yup::XmlElement n ("SETTINGS");

        MidiInputDevice::saveMidiProps (n);

        engine.getPropertyStorage().setXmlPropertyItem (SettingID::midiin, getName(), n);
    }

    void processBlock (yup::MidiBuffer& midi)
    {
        // Process the messages via the base class to update the keyboard state
        for (auto mm : midi)
            MidiInputDevice::handleIncomingMidiMessage (nullptr, mm.getMessage());

        // Pending messages will then be filled with the messages to process
        const yup::ScopedLock sl (pendingMidiMessagesMutex);

        for (auto instance : instances)
            if (auto hostedInstance = dynamic_cast<HostedMidiInputDeviceInstance*> (instance))
                hostedInstance->processBlock (pendingMidiMessages);

        pendingMidiMessages.clear();
    }

    using MidiInputDevice::handleIncomingMidiMessage;

    void handleIncomingMidiMessage (const yup::MidiMessage& m, MPESourceID) override
    {
        const yup::ScopedLock sl (pendingMidiMessagesMutex);
        pendingMidiMessages.addEvent (m, 0);
    }

    yup::String openDevice() override { return {}; }
    void closeDevice() override {}

private:
    //==============================================================================
    class HostedMidiInputDeviceInstance : public MidiInputDeviceInstanceBase
    {
    public:
        HostedMidiInputDeviceInstance (HostedMidiInputDevice& dev, EditPlaybackContext& epc)
            : MidiInputDeviceInstanceBase (dev, epc), mpeSourceID (dev.getMPESourceID())
        {
        }

        void processBlock (yup::MidiBuffer& midi)
        {
            const auto globalStreamTime = edit.engine.getDeviceManager().getCurrentStreamTime();

            // N.B. This assumes that the number of samples processed per block is constant.
            // I.e. that there is no speed compensation set (which shouldn't be the case when
            // running as a plugin)
            for (auto mmm : midi)
            {
                const auto blockStreamTime = tracktion::graph::sampleToTime (mmm.samplePosition, sampleRate);

                auto msg = mmm.getMessage();
                msg.setTimeStamp (globalStreamTime + blockStreamTime);
                handleIncomingMidiMessage (std::move (msg), mpeSourceID);
            }
        }

    private:
        const double sampleRate = context.getSampleRate();
        MPESourceID mpeSourceID;

        HostedMidiInputDevice& getHostedMidiInputDevice() const   { return static_cast<HostedMidiInputDevice&> (owner); }
    };

    //==============================================================================
    yup::MidiBuffer pendingMidiMessages;
    yup::CriticalSection pendingMidiMessagesMutex;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HostedMidiInputDevice)
};

//==============================================================================
class HostedMidiOutputDevice : public MidiOutputDevice
{
public:
    HostedMidiOutputDevice (HostedAudioDeviceInterface& aif)
        : MidiOutputDevice (aif.engine, { TRANS("MIDI Output"), yup::String() }),
          audioIf (aif)
    {
    }

    MidiOutputDeviceInstance* createInstance (EditPlaybackContext& epc) override
    {
        return new HostedMidiOutputDeviceInstance (*this, epc);
    }

    void processBlock (yup::MidiBuffer& midi)
    {
        for (auto m : toSend)
        {
            auto t = m.getTimeStamp() * audioIf.parameters.sampleRate;
            midi.addEvent (m, int (t));
        }

        toSend.clear();
    }

    void sendMessageNow (const yup::MidiMessage& message) override
    {
        toSend.addMidiMessage (message, 0, {});
        toSend.sortByTimestamp();
    }

private:
    struct HostedMidiOutputDeviceInstance : public MidiOutputDeviceInstance
    {
        HostedMidiOutputDeviceInstance (HostedMidiOutputDevice& o, EditPlaybackContext& epc)
            : MidiOutputDeviceInstance (o, epc), outputDevice (o)
        {
        }

        bool sendMessages (MidiMessageArray& mma, TimePosition editTime) override
        {
            // Adjust these messages to be relative to time 0.0 which will be the next call to processBlock
            // The device delay is also subtracted as this will have been added when rendering
            const auto deltaTime = -editTime - outputDevice.getDeviceDelay();
            outputDevice.toSend.mergeFromAndClearWithOffset (mma, deltaTime.inSeconds());
            return true;
        }

        HostedMidiOutputDevice& outputDevice;
    };

    HostedAudioDeviceInterface& audioIf;
    MidiMessageArray toSend;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HostedMidiOutputDevice)
};

//==============================================================================
HostedAudioDeviceInterface::HostedAudioDeviceInterface (Engine& e)
    : engine (e)
{
}

void HostedAudioDeviceInterface::initialise (const Parameters& p)
{
    parameters = p;

    auto& dm = engine.getDeviceManager();

    // First check for an existing hosted device
    if (deviceType == nullptr)
        for (auto device : dm.deviceManager.getAvailableDeviceTypes())
            if (auto hostedAudioDeviceType = dynamic_cast<HostedAudioDeviceType*> (device))
                deviceType = hostedAudioDeviceType;

    // Otherwise add a new hosted device type
    if (deviceType == nullptr)
    {
        deviceType = new HostedAudioDeviceType (*this);
        dm.deviceManager.addAudioDeviceType (std::unique_ptr<HostedAudioDeviceType> (deviceType));
    }

    dm.deviceManager.setCurrentAudioDeviceType ("Hosted Device", true);
    dm.initialise (parameters.inputChannels, parameters.outputChannels);
    jassert (dm.deviceManager.getCurrentAudioDeviceType() == "Hosted Device");
    jassert (dm.deviceManager.getCurrentDeviceTypeObject() == deviceType);

    // Outputs
    {
        if (parameters.outputLatencyNumSamples > 0)
        {
            outputLatencyProcessor = std::make_unique<LatencyProcessor>();
            outputLatencyProcessor->setLatencyNumSamples (parameters.outputLatencyNumSamples);
        }

        for (int i = 0; i < dm.getNumWaveOutDevices(); i++)
            if (auto wo = dm.getWaveOutDevice (i))
                wo->setEnabled (true);
    }

    // Inputs
    {
        if (parameters.inputLatencyNumSamples > 0)
        {
            inputLatencyProcessor = std::make_unique<LatencyProcessor>();
            inputLatencyProcessor->setLatencyNumSamples (parameters.inputLatencyNumSamples);
        }

        // Set devices to mono first and then dispatch synchronously before
        // changing the properties of the devices as they may have changed
        for (auto wi : dm.getWaveInputDevices())
            dm.setDeviceNumChannels (*wi, 1);

        dm.dispatchPendingUpdates();

        for (auto wi : dm.getWaveInputDevices())
        {
            wi->setMonitorMode (InputDevice::MonitorMode::on);
            wi->setEnabled (true);
        }
    }

    if (deviceType != nullptr)
        deviceType->settingsChanged();
}

void HostedAudioDeviceInterface::prepareToPlay (double sampleRate, int blockSize)
{
    auto newMaxChannels = std::max (parameters.inputChannels,
                                    parameters.outputChannels);

    if (parameters.sampleRate != sampleRate
        || parameters.blockSize != blockSize
        || maxChannels != newMaxChannels)
    {
        maxChannels = newMaxChannels;
        parameters.sampleRate = sampleRate;
        parameters.blockSize  = blockSize;

        if (inputLatencyProcessor)
            inputLatencyProcessor->prepareToPlay (sampleRate, blockSize, parameters.inputChannels);

        if (outputLatencyProcessor)
            outputLatencyProcessor->prepareToPlay (sampleRate, blockSize, parameters.outputChannels);

        if (deviceType != nullptr)
            deviceType->settingsChanged();
    }
}

void HostedAudioDeviceInterface::processBlock (yup::AudioBuffer<float>& buffer, yup::MidiBuffer& midi)
{
    if (inputLatencyProcessor)
    {
        inputLatencyProcessor->writeAudio (toBufferView (buffer));
        inputLatencyProcessor->readAudioOverwriting (toBufferView (buffer));
    }

    for (auto& input : midiInputs)
        if (auto hostedInput = dynamic_cast<HostedMidiInputDevice*> (input.get()))
            hostedInput->processBlock (midi);

    midi.clear();

    if (deviceType != nullptr)
        deviceType->processBlock (buffer);

    if (outputLatencyProcessor)
    {
        outputLatencyProcessor->writeAudio (toBufferView (buffer));
        outputLatencyProcessor->readAudioOverwriting (toBufferView (buffer));
    }

    for (auto& output : midiOutputs)
        if (auto hostedOutput = dynamic_cast<HostedMidiOutputDevice*> (output.get()))
            hostedOutput->processBlock (midi);
}

bool HostedAudioDeviceInterface::isHostedMidiInputDevice (const MidiInputDevice& d)
{
    return dynamic_cast<const HostedMidiInputDevice*> (&d) != nullptr;
}

yup::StringArray HostedAudioDeviceInterface::getInputChannelNames()
{
    yup::StringArray res;

    for (int i = 0; i < parameters.inputChannels; i++)
    {
        if (i < parameters.inputNames.size())
            res.add (parameters.inputNames[i]);
        else
            res.add (yup::String (i + 1));
    }

    return res;
}

yup::StringArray HostedAudioDeviceInterface::getOutputChannelNames()
{
    yup::StringArray res;

    for (int i = 0; i < parameters.outputChannels; i++)
    {
        if (i < parameters.outputNames.size())
            res.add (parameters.outputNames[i]);
        else
            res.add (yup::String (i + 1));
    }

    return res;
}

std::shared_ptr<MidiOutputDevice> HostedAudioDeviceInterface::createMidiOutput()
{
    midiOutputs.push_back (std::make_shared<HostedMidiOutputDevice> (*this));
    return midiOutputs.back();
}

std::shared_ptr<MidiInputDevice> HostedAudioDeviceInterface::createMidiInput()
{
    midiInputs.push_back (std::make_shared<HostedMidiInputDevice> (*this));
    return midiInputs.back();
}

} // namespace tracktion::inline engine
