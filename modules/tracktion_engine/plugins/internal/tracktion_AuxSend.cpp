/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

AuxSendPlugin::AuxSendPlugin (PluginCreationInfo info) : Plugin (info)
{
    auto um = getUndoManager();
    busNumber.referTo (state, IDs::busNum, um);
    gainLevel.referTo (state, IDs::auxSendSliderPos, um, decibelsToVolumeFaderPosition (0.0f));
    invertPhase.referTo (state, IDs::phase, um, false);
    lastVolumeBeforeMute.referTo (state, IDs::lastVolumeBeforeMuteDb, um, 0.0f);

    gain = addParam ("send level", TRANS("Send level"), { 0.0f, 1.0f },
                     [] (float value)             { return yup::Decibels::toString (volumeFaderPositionToDB (value)); },
                     [] (const yup::String& s)   { return decibelsToVolumeFaderPosition (dbStringToDb (s)); });

    gain->attachToCurrentValue (gainLevel);

    quickParamName = "send level";
}

AuxSendPlugin::~AuxSendPlugin()
{
    notifyListenersOfDeletion();
    gain->detachFromCurrentValue();
}

bool AuxSendPlugin::shouldProcess()
{
    const yup::ScopedLock sl (ownerTrackLock);

    if (ownerTrack != nullptr)
    {
        // If this track gets disabled when muted,
        // we don't need to worry about this since
        // our plugin will be disabled anyway
        if (! ownerTrack->processAudioNodesWhileMuted())
            return true;

        return ! ownerTrack->isMuted (true);
    }

    return true;
}

const char* AuxSendPlugin::xmlTypeName = "auxsend";

yup::ValueTree AuxSendPlugin::create()
{
    return createValueTree (IDs::PLUGIN,
                            IDs::type, xmlTypeName);
}

yup::String AuxSendPlugin::getName() const
{
    yup::String nm (edit.getAuxBusName (busNumber));

    if (nm.isNotEmpty())
        return "S:" + nm;

    return TRANS("Aux Send") + " #" + yup::String (busNumber + 1);
}

yup::String AuxSendPlugin::getShortName (int)
{
    auto nm = edit.getAuxBusName (busNumber);

    if (nm.isNotEmpty())
        return "S:" + nm;

    return TRANS("Send") + ":" + yup::String (busNumber + 1);
}

void AuxSendPlugin::initialise (const PluginInitialisationInfo& info)
{
    lastGain = volumeFaderPositionToGain (gain->getCurrentValue());

    initialiseWithoutStopping (info);
}

void AuxSendPlugin::initialiseWithoutStopping (const PluginInitialisationInfo&)
{
    TRACKTION_ASSERT_MESSAGE_THREAD
    auto newOwnerTrack = getOwnerTrack();

    const yup::ScopedLock sl (ownerTrackLock);
    ownerTrack = newOwnerTrack;
}

void AuxSendPlugin::deinitialise()
{
}

void AuxSendPlugin::applyToBuffer (const PluginRenderContext&)
{
}

yup::String AuxSendPlugin::getBusName()
{
    auto busName = edit.getAuxBusName (busNumber);

    if (busName.isNotEmpty())
        return busName;

    return getDefaultBusName (busNumber);
}

void AuxSendPlugin::setGainDb (float newDb)
{
    float newPos = decibelsToVolumeFaderPosition (newDb);

    if (gain->getCurrentValue() != newPos)
    {
        gain->setParameter (newPos, yup::sendNotification);
        changed();
    }
}

void AuxSendPlugin::setMute (bool b)
{
    if (b)
    {
        lastVolumeBeforeMute = getGainDb();
        setGainDb (lastVolumeBeforeMute - 0.01f); // needed so that automation is recorded correctly
        setGainDb (-100.0f);
    }
    else
    {
        if (lastVolumeBeforeMute < -100.0f)
            lastVolumeBeforeMute = 0.0f;

        setGainDb (getGainDb() + 0.01f); // needed so that automation is recorded correctly
        setGainDb (lastVolumeBeforeMute);
    }
}

bool AuxSendPlugin::isMute()
{
    return getGainDb() <= -90.0f;
}

yup::String AuxSendPlugin::getDefaultBusName (int index)
{
    return "Bus #" + yup::String (index + 1);
}

yup::StringArray AuxSendPlugin::getBusNames (Edit& ed, int maxNumBusses)
{
    yup::StringArray buses;

    for (int i = 0; i < maxNumBusses; ++i)
    {
        auto nm = getDefaultBusName (i);

        if (ed.getAuxBusName (i).isNotEmpty())
            nm << " (" << ed.getAuxBusName (i) << ")";

        buses.add (nm);
    }

    return buses;
}

void AuxSendPlugin::restorePluginStateFromValueTree (const yup::ValueTree& v)
{
    copyPropertiesToCachedValues (v, gainLevel, busNumber, invertPhase);

    for (auto p : getAutomatableParameters())
        p->updateFromAttachedValue();
}

bool AuxSendPlugin::isOwnedBy (Track& t)
{
    const yup::ScopedLock sl (ownerTrackLock);
    return &t == ownerTrack;
}

} // namespace tracktion::inline engine
