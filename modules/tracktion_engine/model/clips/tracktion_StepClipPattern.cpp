/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

StepClip::Pattern::Pattern (StepClip& c, const yup::ValueTree& v) noexcept
    : clip (c), state (v)
{
}

StepClip::Pattern::Pattern (const Pattern& other) noexcept
    : clip (other.clip), state (other.state)
{
}

yup::String StepClip::Pattern::getName() const                 { return state[IDs::name]; }
void StepClip::Pattern::setName (const yup::String& name)      { state.setProperty (IDs::name, name, clip.getUndoManager()); }

int StepClip::Pattern::getNumNotes() const                      { return state[IDs::numNotes]; }
void StepClip::Pattern::setNumNotes (int n)                     { state.setProperty (IDs::numNotes, n, clip.getUndoManager()); }

BeatDuration StepClip::Pattern::getNoteLength() const           { return BeatDuration::fromBeats (static_cast<double> (state[IDs::noteLength])); }
void StepClip::Pattern::setNoteLength (BeatDuration n)          { state.setProperty (IDs::noteLength, n.inBeats(), clip.getUndoManager()); }

yup::BigInteger StepClip::Pattern::getChannel (int channel) const
{
    yup::BigInteger b;
    b.parseString (state.getChild (channel)[IDs::pattern].toString(), 2);
    return b;
}

void StepClip::Pattern::setChannel (int channel, const yup::BigInteger& c)
{
    while (channel >= state.getNumChildren())
    {
        if (c.isZero())
            return;

        insertChannel (-1);
    }

    state.getChild (channel).setProperty (IDs::pattern, c.toString (2), clip.getUndoManager());
}

yup::Array<int> StepClip::Pattern::getVelocities (int channel) const
{
    yup::Array<int> v;
    auto sa = yup::StringArray::fromTokens (state.getChild (channel)[IDs::velocities].toString(), false);
    v.ensureStorageAllocated (sa.size());

    for (auto& s : sa)
        v.add (s.getIntValue());

    return v;
}

void StepClip::Pattern::setVelocities (int channel, const yup::Array<int>& va)
{
    if (channel >= state.getNumChildren())
    {
        jassertfalse;
        return;
    }

    yup::StringArray sa;
    sa.ensureStorageAllocated (va.size());

    for (int v : va)
        sa.add (yup::String (v));

    state.getChild (channel).setProperty (IDs::velocities, sa.joinIntoString (" "), clip.getUndoManager());
}

static double parseFraction (const yup::String& s)
{
    const int splitIndex = s.indexOfChar ('/');
    const double num    = s.substring (0, splitIndex).getDoubleValue();
    const double denom  = s.substring (splitIndex + 1, s.length()).getDoubleValue();

    if (denom == 0.0)
        return 0.0;

    return num / denom;
}

yup::Array<double> StepClip::Pattern::getGates (int channel) const
{
    yup::Array<double> v;
    auto sa = yup::StringArray::fromTokens (state.getChild (channel)[IDs::gates].toString(), false);
    v.ensureStorageAllocated (sa.size());

    for (auto& s : sa)
    {
        if (s.containsChar ('/'))
            v.add (parseFraction (s));
        else
            v.add (s.getDoubleValue());
    }

    return v;
}

void StepClip::Pattern::setGates (int channel, const yup::Array<double>& ga)
{
    if (channel >= state.getNumChildren())
    {
        jassertfalse;
        return;
    }

    yup::StringArray sa;
    sa.ensureStorageAllocated (ga.size());

    for (double g : ga)
        sa.add (yup::String (g));

    state.getChild (channel).setProperty (IDs::gates, sa.joinIntoString (" "), clip.getUndoManager());
}

yup::Array<float> StepClip::Pattern::getProbabilities (int channel) const
{
    yup::Array<float> v;
    auto pa = yup::StringArray::fromTokens (state.getChild (channel)[IDs::probabilities].toString(), false);
    v.ensureStorageAllocated (pa.size());

    for (auto& p : pa)
        v.add (p.getFloatValue());

    return v;
}

void StepClip::Pattern::setProbabilities (int channel, const yup::Array<float>& pa)
{
    if (channel >= state.getNumChildren())
    {
        jassertfalse;
        return;
    }

    yup::StringArray sa;
    sa.ensureStorageAllocated (pa.size());

    for (double p : pa)
        sa.add (yup::String (p));

    state.getChild (channel).setProperty (IDs::probabilities, sa.joinIntoString (" "), clip.getUndoManager());
}

bool StepClip::Pattern::getNote (int channel, int index) const noexcept
{
    return getChannel (channel) [index];
}

void StepClip::Pattern::setNote (int channel, int index, bool value)
{
    if (getNote (channel, index) != value
          && yup::isPositiveAndBelow (index, getNumNotes())
          && yup::isPositiveAndBelow (channel, (int) maxNumChannels))
    {
        yup::BigInteger b (getChannel (channel));
        b.setBit (index, value);
        setChannel (channel, b);

        if (value)
        {
            if (getVelocity (channel, index) == 0)
                setVelocity (channel, index, defaultNoteValue);

            if (getGate (channel, index) == 0.0)
                setGate (channel, index, 1.0);
        }
    }
}

int StepClip::Pattern::getVelocity (int channel, int index) const
{
    if (! getNote (channel, index))
        return -1;

    auto velocities = getVelocities (channel);

    if (yup::isPositiveAndBelow (index, velocities.size()))
        return velocities[index];

    if (clip.getChannels()[channel] != nullptr)
        return 127;

    return -1;
}

void StepClip::Pattern::setVelocity (int channel, int index, int value)
{
    if (! yup::isPositiveAndBelow (channel, (int) maxNumChannels))
        return;

    setNote (channel, index, true);

    auto velocities = getVelocities (channel);
    const int size = velocities.size();

    if (! yup::isPositiveAndNotGreaterThan (index, size))
        velocities.insertMultiple (size, 127, index - size);

    velocities.set (index, value);
    setVelocities (channel, velocities);
}

double StepClip::Pattern::getGate (int channel, int index) const
{
    if (! getNote (channel, index))
        return 0.0;

    auto gates = getGates (channel);

    if (yup::isPositiveAndBelow (index, gates.size()))
        return gates[index];

    return 1.0;
}

float StepClip::Pattern::getProbability (int channel, int index) const
{
    if (! getNote (channel, index))
        return 1.0;

    auto p = getProbabilities (channel);

    if (yup::isPositiveAndBelow (index, p.size()))
        return p[index];

    return 1.0;
}

void StepClip::Pattern::setGate (int channel, int index, double value)
{
    if (! yup::isPositiveAndBelow (channel, (int) maxNumChannels))
        return;

    setNote (channel, index, value != 0.0);

    auto gates = getGates (channel);
    const int size = gates.size();

    if (! yup::isPositiveAndNotGreaterThan (index, size))
        gates.insertMultiple (size, 1.0, index - size);

    gates.set (index, value);
    setGates (channel, gates);
}

void StepClip::Pattern::setProbability (int channel, int index, float value)
{
    if (! yup::isPositiveAndBelow (channel, (int) maxNumChannels))
        return;

    setNote (channel, index, value != 0.0);

    auto p = getProbabilities (channel);
    const int size = p.size();

    if (! yup::isPositiveAndNotGreaterThan (index, size))
        p.insertMultiple (size, 1.0f, index - size);

    p.set (index, value);
    setProbabilities (channel, p);
}

void StepClip::Pattern::clear()
{
    state.removeAllChildren (clip.getUndoManager());
}

void StepClip::Pattern::clearChannel (int channel)
{
    setChannel (channel, yup::BigInteger());
}

void StepClip::Pattern::insertChannel (int channel)
{
    state.addChild (yup::ValueTree (IDs::CHANNEL), channel, clip.getUndoManager());
}

void StepClip::Pattern::removeChannel (int channel)
{
    state.removeChild (channel, clip.getUndoManager());
}

void StepClip::Pattern::randomiseChannel (int channel)
{
    clearChannel (channel);

    yup::Random r;
    for (int i = 0; i < getNumNotes(); ++i)
        setNote (channel, i, r.nextBool());
}

void StepClip::Pattern::randomiseSteps()
{
    yup::Random r;
    const int numChannels = clip.getChannels().size();
    const int numSteps = getNumNotes();

    yup::Array<yup::BigInteger> chans;
    chans.insertMultiple (0, yup::BigInteger(), numChannels);

    for (int i = 0; i < numSteps; ++i)
        chans.getReference (r.nextInt (numChannels)).setBit (i);

    for (int i = 0; i < numChannels; ++i)
        setChannel (i, chans.getReference (i));
}

void StepClip::Pattern::shiftChannel (int channel, bool toTheRight)
{
    yup::BigInteger c (getChannel (channel));

    // NB: Notes are added in reverse order
    if (toTheRight)
        c <<= 1;
    else
        c >>= 1;

    setChannel (channel, c);
}

void StepClip::Pattern::toggleAtInterval (int channel, int interval)
{
    yup::BigInteger c = getChannel (channel);

    for (int i = 0; i < getNumNotes(); ++i)
        c.setBit (i, (i % interval) == 0);

    setChannel (channel, c);
}

StepClip::Pattern::CachedPattern::CachedPattern (const Pattern& p, int c)
    : notes (p.getChannel (c)),
      velocities (p.getVelocities (c)),
      gates (p.getGates (c)),
      probabilities (p.getProbabilities (c))
{
}

bool StepClip::Pattern::CachedPattern::getNote (int index) const noexcept
{
    return notes[index];
}

int StepClip::Pattern::CachedPattern::getVelocity (int index) const noexcept
{
    if (! getNote (index))
        return -1;

    if (yup::isPositiveAndBelow (index, velocities.size()))
        return velocities[index];

    return 127;
}

double StepClip::Pattern::CachedPattern::getGate (int index) const noexcept
{
    if (! getNote (index))
        return 0.0;

    if (yup::isPositiveAndBelow (index, gates.size()))
        return gates[index];

    return 1.0;
}

float StepClip::Pattern::CachedPattern::getProbability (int index) const noexcept
{
    if (! getNote (index))
        return 0.0;

    if (yup::isPositiveAndBelow (index, probabilities.size()))
        return probabilities[index];

    return 1.0;
}

//==============================================================================
StepClip::Pattern StepClip::PatternInstance::getPattern() const
{
    return clip.getPattern (patternIndex);
}

int StepClip::PatternInstance::getSequenceIndex() const
{
    return clip.getPatternSequence().indexOf (this);
}

yup::String StepClip::PatternInstance::getSelectableDescription()
{
    return clip.getName() + "  -  "
         + TRANS("Section 123").replace ("123", yup::String (getSequenceIndex() + 1)) + " ("
         + TRANS("Variation 123").replace ("123", yup::String (patternIndex + 1)) + ")";
}

} // namespace tracktion::inline engine
