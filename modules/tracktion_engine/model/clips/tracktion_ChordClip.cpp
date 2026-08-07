/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

ChordClip::ChordClip (const yup::ValueTree& v, EditItemID id, ClipOwner& targetParent)
    : Clip (v, targetParent, id, Type::chord)
{
    if (clipName.get().isEmpty())
        clipName = TRANS("Chord");

    auto pgen = state.getChildWithName (IDs::PATTERNGENERATOR);

    if (pgen.isValid())
    {
        patternGenerator = std::make_unique<PatternGenerator> (*this, pgen);

        if (patternGenerator->getChordProgression().isEmpty())
            patternGenerator->setChordProgressionFromChordNames ({ "i" });
    }

    state.addListener (this);
}

ChordClip::~ChordClip()
{
    notifyListenersOfDeletion();

    state.removeListener (this);
}

void ChordClip::initialise()
{
    Clip::initialise();

    speedRatio = 1.0; // not used
}

yup::String ChordClip::getSelectableDescription()
{
    return TRANS("Chord Clip");
}

yup::Color ChordClip::getDefaultColor() const
{
    return yup::Colors::red.withHue (1.0f / 9.0f);
}

//==============================================================================
bool ChordClip::canBeAddedTo (ClipOwner& co)
{
    return isChordTrack (co);
}

void ChordClip::valueTreeChildAdded (yup::ValueTree&, yup::ValueTree& c)
{
    if (c.hasType (IDs::PATTERNGENERATOR))
    {
        patternGenerator = std::make_unique<PatternGenerator> (*this, c);

        if (patternGenerator->getChordProgression().isEmpty())
            patternGenerator->setChordProgressionFromChordNames ({ "i" });
    }

    changed();
    triggerAsyncUpdate();
}

void ChordClip::valueTreeChildRemoved (yup::ValueTree& p, yup::ValueTree& c, int)
{
    if (p == state && c.hasType (IDs::PATTERNGENERATOR))
    {
        patternGenerator = nullptr;
    }
    changed();
    triggerAsyncUpdate();
}

void ChordClip::valueTreePropertyChanged (yup::ValueTree& v, const yup::Identifier& i)
{
    changed();
    triggerAsyncUpdate();

    Clip::valueTreePropertyChanged (v, i);
}

void ChordClip::valueTreeParentChanged (yup::ValueTree& v)
{
    if (! v.getParent().isValid())
        edit.sendTempoOrPitchSequenceChangedUpdates();

    Clip::valueTreeParentChanged (v);
}

PatternGenerator* ChordClip::getPatternGenerator()
{
    if (! state.getChildWithName (IDs::PATTERNGENERATOR).isValid())
        state.addChild (yup::ValueTree (IDs::PATTERNGENERATOR), -1, &edit.getUndoManager());

    jassert (patternGenerator != nullptr);
    return patternGenerator.get();
}

void ChordClip::pitchTempoTrackChanged()
{
    if (patternGenerator != nullptr)
        patternGenerator->refreshPatternIfNeeded();

    changed();
}

yup::Color ChordClip::getColor() const
{
    return Clip::getColor();
}

void ChordClip::handleAsyncUpdate()
{
    edit.sendTempoOrPitchSequenceChangedUpdates();
}

} // namespace tracktion::inline engine
