/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

static void getPossibleInputDeviceNames (Engine& e,
                                         yup::StringArray& s, yup::StringArray& a,
                                         yup::BigInteger& hasAudio,
                                         yup::BigInteger& hasMidi)
{
    auto& dm = e.getDeviceManager();

    for (int i = 0; i < dm.getNumInputDevices(); ++i)
    {
        if (auto in = dm.getInputDevice (i))
        {
            if (! in->isEnabled())
                continue;

            if (dynamic_cast<MidiInputDevice*> (in) != nullptr)
                hasMidi.setBit (s.size(), true);
            else
                hasAudio.setBit (s.size(), true);

            s.add (in->getName());
            a.add (in->getAliasOrName());
        }
    }
}

static void getPossibleOutputDeviceNames (Engine& e,
                                          yup::StringArray& s, yup::StringArray& a,
                                          yup::BigInteger& hasAudio,
                                          yup::BigInteger& hasMidi)
{
    auto& dm = e.getDeviceManager();

    for (int i = 0; i < dm.getNumOutputDevices(); ++i)
    {
        if (auto out = dm.getOutputDeviceAt (i))
        {
            if (! out->isEnabled())
                continue;

            if (auto m = dynamic_cast<MidiOutputDevice*> (out))
            {
                if (m->isConnectedToExternalController())
                    continue;

                hasMidi.setBit (s.size(), true);
            }
            else
            {
                hasAudio.setBit (s.size(), true);
            }

            s.add (out->getName());
            a.add (out->getAliasOrName());
        }
    }
}


//==============================================================================
InsertPlugin::InsertPlugin (PluginCreationInfo info)  : Plugin (info)
{
    auto um = getUndoManager();
    name.referTo (state, IDs::name, um);
    inputDevice.referTo (state, IDs::inputDevice, um);
    outputDevice.referTo (state, IDs::outputDevice, um);
    manualAdjustMs.referTo (state, IDs::manualAdjustMs, um);

    updateDeviceTypes();
}

InsertPlugin::~InsertPlugin()
{
    notifyListenersOfDeletion();
}

//==============================================================================
const char* InsertPlugin::xmlTypeName ("insert");

yup::ValueTree InsertPlugin::create()
{
    return createValueTree (IDs::PLUGIN,
                            IDs::type, xmlTypeName);
}

yup::String InsertPlugin::getName() const                                   { return name.get().isNotEmpty() ? name : TRANS("Insert Plugin"); }
yup::String InsertPlugin::getPluginType()                                   { return xmlTypeName; }
yup::String InsertPlugin::getShortName (int)                                { return TRANS("Insert"); }
double InsertPlugin::getLatencySeconds()                                     { return latencySeconds; }
void InsertPlugin::getChannelNames (yup::StringArray*, yup::StringArray*)  {}
bool InsertPlugin::takesMidiInput()                                          { return true; }
bool InsertPlugin::canBeAddedToClip()                                        { return false; }
bool InsertPlugin::needsConstantBufferSize()                                 { return true; }

void InsertPlugin::initialise (const PluginInitialisationInfo& info)
{
    updateDeviceTypes();

    initialiseWithoutStopping (info);
}

void InsertPlugin::initialiseWithoutStopping (const PluginInitialisationInfo& info)
{
    TRACKTION_ASSERT_MESSAGE_THREAD
    // This latency number is from trial and error, may need more testing.
    // It should be accurate on well reporting devices though
    int latency = static_cast<int> (timeToSample (manualAdjustMs / 1000.0, info.sampleRate));

    if (auto device = engine.getDeviceManager().deviceManager.getCurrentAudioDevice())
    {
        if (sendDeviceType == audioDevice)
            latency += device->getOutputLatencyInSamples();

        if (returnDeviceType == audioDevice)
            latency += device->getInputLatencyInSamples();
    }

    latency = std::max (0, latency);
    latencyNumSamples = latency;
    latencySeconds = TimeDuration::fromSamples (latency, info.sampleRate).inSeconds();
}

void InsertPlugin::deinitialise()
{
}

void InsertPlugin::applyToBuffer (const PluginRenderContext&)
{
    CRASH_TRACER
    jassertfalse; // This shouldn't be called anymore, it's handled directly by the playback graph
}

yup::String InsertPlugin::getSelectableDescription()
{
    return TRANS("Insert Plugin");
}

void InsertPlugin::restorePluginStateFromValueTree (const yup::ValueTree& v)
{
    if (v.hasProperty (IDs::name))
        name = v.getProperty (IDs::name).toString();

    if (v.hasProperty (IDs::outputDevice))
        outputDevice = v.getProperty (IDs::outputDevice).toString();

    if (v.hasProperty (IDs::inputDevice))
        inputDevice = v.getProperty (IDs::inputDevice).toString();

    for (auto p : getAutomatableParameters())
        p->updateFromAttachedValue();
}

void InsertPlugin::updateDeviceTypes()
{
    CRASH_TRACER
    TRACKTION_ASSERT_MESSAGE_THREAD

    yup::StringArray devices, aliases;
    yup::BigInteger hasAudio, hasMidi;

    auto setDeviceType = [] (DeviceType& deviceType, yup::BigInteger& audio, yup::BigInteger& midi, int index)
    {
        if (audio[index])       deviceType = audioDevice;
        else if (midi[index])   deviceType = midiDevice;
        else                    deviceType = noDevice;
    };

    getPossibleInputDeviceNames (engine, devices, aliases, hasAudio, hasMidi);
    setDeviceType (returnDeviceType, hasAudio, hasMidi, devices.indexOf (inputDevice.get()));

    getPossibleOutputDeviceNames (engine, devices, aliases, hasAudio, hasMidi);
    setDeviceType (sendDeviceType, hasAudio, hasMidi, devices.indexOf (outputDevice.get()));

    propertiesChanged();
    changed();
}

void InsertPlugin::getPossibleDeviceNames (Engine& e,
                                           yup::StringArray& s, yup::StringArray& a,
                                           yup::BigInteger& hasAudio,
                                           yup::BigInteger& hasMidi,
                                           bool forInput)
{
    if (forInput)
        getPossibleInputDeviceNames (e, s, a, hasAudio, hasMidi);
    else
        getPossibleOutputDeviceNames (e, s, a, hasAudio, hasMidi);
}

bool InsertPlugin::hasAudio() const       { return sendDeviceType == audioDevice  || returnDeviceType == audioDevice; }
bool InsertPlugin::hasMidi() const        { return sendDeviceType == midiDevice   || returnDeviceType == midiDevice; }

int InsertPlugin::getLatencyNumSamples() const
{
    return latencyNumSamples.load (std::memory_order_acquire);
}

void InsertPlugin::valueTreePropertyChanged (yup::ValueTree& v, const yup::Identifier& i)
{
    if (v == state)
    {
        auto update = [&i] (juce::CachedValue<yup::String>& deviceName) -> bool
        {
            if (i != deviceName.getPropertyID())
                return false;

            deviceName.forceUpdateOfCachedValue();
            return true;
        };

        if (update (outputDevice) || update (inputDevice))
            updateDeviceTypes();
    }

    Plugin::valueTreePropertyChanged (v, i);
}

} // namespace tracktion::inline engine
