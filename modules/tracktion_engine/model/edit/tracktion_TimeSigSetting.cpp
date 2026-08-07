/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

TimeSigSetting::TimeSigSetting (TempoSequence& ts, const yup::ValueTree& v)
    : TrackItem (ts.edit, {}, Type::timeSig),
      state (v), ownerSequence (ts)
{
    auto* um = ts.getUndoManager();

    startBeatNumber.referTo (state, IDs::startBeat, um);
    numerator.referTo (state, IDs::numerator, um, 4);
    denominator.referTo (state, IDs::denominator, um, 4);
    triplets.referTo (state, IDs::triplets, um);

    state.addListener (this);
}

TimeSigSetting::~TimeSigSetting()
{
    state.removeListener (this);

    notifyListenersOfDeletion();
}

yup::String TimeSigSetting::getSelectableDescription()
{
    return TRANS("Time Signature");
}

yup::String TimeSigSetting::getStringTimeSig() const
{
    return yup::String (numerator) + "/" + yup::String (denominator);
}

void TimeSigSetting::setStringTimeSig (const yup::String& s)
{
    if (s.containsChar ('/'))
    {
        numerator = s.upToFirstOccurrenceOf ("/", false, false).trim().getIntValue();
        denominator = s.fromLastOccurrenceOf ("/", false, false).trim().getIntValue();
    }
}

void TimeSigSetting::removeFromEdit()
{
    jassert (Selectable::isSelectableValid (&edit));
    ownerSequence.removeTimeSig (ownerSequence.indexOfTimeSig (this));
}

Track* TimeSigSetting::getTrack() const
{
    return edit.getTempoTrack();
}

ClipPosition TimeSigSetting::getPosition() const
{
    ownerSequence.updateTempoDataIfNeeded();
    auto s = startTime;

    if (auto nextTimeSig = ownerSequence.getTimeSig (ownerSequence.indexOfTimeSig (this) + 1))
        return { { s, nextTimeSig->startTime }, TimeDuration() };

    return { { s, s + TimeDuration::fromSeconds (1.0) }, TimeDuration() };
}

yup::String TimeSigSetting::getName() const
{
    return getStringTimeSig();
}

} // namespace tracktion::inline engine
