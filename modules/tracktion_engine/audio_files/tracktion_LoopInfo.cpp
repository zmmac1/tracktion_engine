/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

LoopInfo::LoopInfo (Engine& e)
    : engine (e), state (IDs::LOOPINFO)
{
    initialiseMissingProps();
}

LoopInfo::LoopInfo (Engine& e, const yup::File& f)
    : engine (e), state (IDs::LOOPINFO)
{
    auto& formatManager = engine.getAudioFileFormatManager();

    if (auto af = formatManager.getFormatFromFileName (f))
    {
        if (auto fin = f.createInputStream())
        {
            const std::unique_ptr<yup::AudioFormatReader> afr (af->createReaderFor (fin.release(), true));
            init (afr.get(), af, f);
        }
    }
}

LoopInfo::LoopInfo (Engine& e, const yup::AudioFormatReader* afr, const yup::AudioFormat* af, const yup::File& f)
    : engine (e), state (IDs::LOOPINFO)
{
    init (afr, af, f);
}

LoopInfo::LoopInfo (Engine& e, const yup::ValueTree& v, juce::UndoManager* u)
    : engine (e), state (v), um (u), maintainParent (v.getParent().isValid())
{
    initialiseMissingProps();
}

LoopInfo::LoopInfo (const LoopInfo& other)
    : engine (other.engine)
{
    *this = other;
}

LoopInfo& LoopInfo::operator= (const LoopInfo& o)
{
    return copyFrom (o.state);
}

//==============================================================================
double LoopInfo::getBpm (const AudioFileInfo& wi) const
{
    return getBeatsPerSecond (wi) * 60.0;
}

void LoopInfo::setBpm (double newBpm, double currentBpm)
{
    if (newBpm != currentBpm)
    {
        jassert (getNumBeats() > 0);
        const double ratio = newBpm / currentBpm;
        setNumBeats (yup::jmax (1.0, getNumBeats() * ratio));
    }
}

void LoopInfo::setBpm (double newBpm, const AudioFileInfo& wi)
{
    // this is a bit of a round about way of calculating the number of beats which
    // will in turn update the BPM but should work until we determine a proper fix

    jassert (wi.sampleRate != 0.0);

    if (wi.sampleRate == 0.0)
        return;

    if (newBpm < 30.0 || newBpm > 1000.0)
        return;

    const double currentBpm = getBpm (wi);

    // Set a dummy number of beats otherwise the ratio will be 0
    if (currentBpm == 0)
    {
        const auto lengthMins = wi.getLengthInSeconds() / 60.0;
        const auto numBeats = newBpm * lengthMins;
        setNumBeats (numBeats);
    }
    else
    {
        setBpm (newBpm, currentBpm);
    }
}

double LoopInfo::getBeatsPerSecond (const AudioFileInfo& wi) const
{
    CRASH_TRACER

    if (wi.sampleRate == 0.0)
        return 2.0;

    auto in  = getInMarker();
    auto out = getOutMarker();

    if (out == -1 || out > wi.lengthInSamples)
        out = wi.lengthInSamples;

    if (out <= 0)
        return 2.0;

    auto length = static_cast<double> (out - in) / wi.sampleRate;

    if (length <= 0)
        return 2.0;

    return getNumBeats() / length;
}

//==============================================================================
int LoopInfo::getDenominator() const              { return getProp<int> (IDs::denominator); }
int LoopInfo::getNumerator() const                { return getProp<int> (IDs::numerator); }
void LoopInfo::setDenominator (int dem)           { setProp (IDs::denominator, dem); }
void LoopInfo::setNumerator (int num)             { setProp (IDs::numerator, num); }
double LoopInfo::getNumBeats() const              { return getProp<double> (IDs::numBeats); }
void LoopInfo::setNumBeats (double b)             { setProp (IDs::numBeats, b); }

//==============================================================================
bool LoopInfo::isLoopable() const                  { const yup::ScopedLock sl (lock); return ! isOneShot() && getNumBeats() > 0.0 && getDenominator() > 0 && getNumerator() > 0; }
bool LoopInfo::isOneShot() const                   { return getProp<bool> (IDs::oneShot); }

//==============================================================================
int LoopInfo::getRootNote() const                  { return getProp<int> (IDs::rootNote); }
void LoopInfo::setRootNote (int note)              { setProp<int> (IDs::rootNote, note); }

//==============================================================================
SampleCount LoopInfo::getInMarker() const          { return getProp<yup::int64> (IDs::inMarker); }
SampleCount LoopInfo::getOutMarker() const         { return getProp<yup::int64> (IDs::outMarker); }

void LoopInfo::setInMarker (SampleCount in)        { setProp<yup::int64> (IDs::inMarker, in); }
void LoopInfo::setOutMarker (SampleCount out)      { setProp<yup::int64> (IDs::outMarker, out); }

//==============================================================================
int LoopInfo::getNumLoopPoints() const
{
    const yup::ScopedLock sl (lock);
    return getLoopPoints().getNumChildren();
}

LoopInfo::LoopPoint LoopInfo::getLoopPoint (int idx) const
{
    const yup::ScopedLock sl (lock);
    auto lp = getLoopPoints().getChild (idx);

    if (lp.isValid())
        return { static_cast<yup::int64> (lp.getProperty (IDs::value)),
                 (LoopPointType) static_cast<int> (lp.getProperty (IDs::type)) };

    return {};
}

void LoopInfo::addLoopPoint (SampleCount pos, LoopPointType type)
{
    const yup::ScopedLock sl (lock);
    duplicateIfShared();

    auto t = createValueTree (IDs::LOOPPOINT,
                              IDs::value, (yup::int64) pos,
                              IDs::type, (int) type);

    getOrCreateLoopPoints().addChild (t, -1, nullptr);
}

void LoopInfo::changeLoopPoint (int idx, SampleCount pos, LoopPointType type)
{
    const yup::ScopedLock sl (lock);
    duplicateIfShared();

    auto lp = getLoopPoints().getChild (idx);
    lp.setProperty (IDs::value, (yup::int64) pos, um);
    lp.setProperty (IDs::type, (int) type, um);
}

void LoopInfo::deleteLoopPoint (int idx)
{
    const yup::ScopedLock sl (lock);
    duplicateIfShared();
    getLoopPoints().removeChild (idx, um);
    removeChildIfEmpty (IDs::LOOPPOINTS);
}

void LoopInfo::clearLoopPoints()
{
    const yup::ScopedLock sl (lock);
    duplicateIfShared();
    state.removeChild (getLoopPoints(), um);
}

void LoopInfo::clearLoopPoints (LoopPointType type)
{
    const yup::ScopedLock sl (lock);
    auto lps = getLoopPoints();

    for (int i = lps.getNumChildren(); --i >= 0;)
        if (((LoopPointType) int (lps.getChild (i).getProperty (IDs::type))) == type)
            lps.removeChild (i, nullptr);
}

int LoopInfo::getNumTags() const                { const yup::ScopedLock sl (lock); return getTags().getNumChildren(); }
void LoopInfo::clearTags()                      { const yup::ScopedLock sl (lock); duplicateIfShared(); state.removeChild (getTags(), um); }
yup::String LoopInfo::getTag (int idx) const   { const yup::ScopedLock sl (lock); return getTags().getChild (idx).getProperty (IDs::name).toString(); }

void LoopInfo::addTag (const yup::String& tag)
{
    const yup::ScopedLock sl (lock);
    duplicateIfShared();

    yup::ValueTree t (IDs::TAG);
    t.setProperty (IDs::name, tag, um);
    getOrCreateTags().addChild (t, -1, um);
}

void LoopInfo::addTags (const yup::StringArray& tags)
{
    for (const auto& t : tags)
        addTag (t);
}

void LoopInfo::initialiseMissingProps()
{
    const yup::ScopedLock sl (lock);
    setPropertyIfMissing (state, IDs::numBeats, 0.0, um);
    setPropertyIfMissing (state, IDs::denominator, 0, um);
    setPropertyIfMissing (state, IDs::numerator, 0, um);
    setPropertyIfMissing (state, IDs::oneShot, 0, um);
    setPropertyIfMissing (state, IDs::bpm, 0, um);
    setPropertyIfMissing (state, IDs::rootNote, -1, um);
    setPropertyIfMissing (state, IDs::inMarker, 0, um);
    setPropertyIfMissing (state, IDs::outMarker, -1, um);
}

LoopInfo& LoopInfo::copyFrom (const yup::ValueTree& o)
{
    const yup::ScopedLock sl (lock);
    copyValueTree (state, o, um);
    return *this;
}

void LoopInfo::removeChildIfEmpty (const yup::Identifier& i)
{
    auto v = state.getChildWithName (i);

    if (v.getNumChildren() == 0)
        state.removeChild (v, um);
}

yup::ValueTree LoopInfo::getLoopPoints() const       { const yup::ScopedLock sl (lock); return state.getChildWithName (IDs::LOOPPOINTS); }
yup::ValueTree LoopInfo::getOrCreateLoopPoints()     { const yup::ScopedLock sl (lock); return state.getOrCreateChildWithName (IDs::LOOPPOINTS, um); }

yup::ValueTree LoopInfo::getTags() const             { const yup::ScopedLock sl (lock); return state.getChildWithName (IDs::TAGS); }
yup::ValueTree LoopInfo::getOrCreateTags()           { const yup::ScopedLock sl (lock); return state.getOrCreateChildWithName (IDs::TAGS, um); }

void LoopInfo::duplicateIfShared()
{
    const yup::ScopedLock sl (lock);

    if (state.getReferenceCount() <= 1)
        return;

    if (maintainParent)
    {
        auto parent = state.getParent();
        int index = -1;

        auto stateCopy = state.createCopy();

        if (parent.isValid())
        {
            index = parent.indexOf (state);
            parent.removeChild (index, um);
        }

        state = stateCopy;

        if (parent.isValid())
            parent.addChild (state, index, um);
    }
    else
    {
        state = state.createCopy();
    }
}

static bool isSameFormat (const yup::AudioFormat* af1, const yup::AudioFormat* af2)
{
    return af1 == af2 || (af1 != nullptr && af2 != nullptr
                           && af1->getFormatName() == af2->getFormatName());
}

void LoopInfo::init (const yup::AudioFormatReader* afr, const yup::AudioFormat* af, const yup::File& file)
{
    if (afr == nullptr || af == nullptr)
        return;

    auto& formatManager = engine.getAudioFileFormatManager();
    const yup::ScopedLock sl (lock);

   #if TRACKTION_ENABLE_REX
    if (isSameFormat (af, formatManager.getRexFormat()))
    {
        setDenominator (afr->metadataValues[RexAudioFormat::rexDenominator].getIntValue());
        setNumerator (afr->metadataValues[RexAudioFormat::rexDenominator].getIntValue());
        const double bpm = afr->metadataValues[RexAudioFormat::rexTempo].getDoubleValue();
        setProp (IDs::bpm, bpm);
        setProp (IDs::numBeats, bpm * ((static_cast<double> (afr->lengthInSamples) / afr->sampleRate) / 60.0));

        yup::StringArray beatPoints;
        beatPoints.addTokens (afr->metadataValues[RexAudioFormat::rexBeatPoints], ";", {});
        beatPoints.removeEmptyStrings();

        for (int i = 0; i < beatPoints.size(); ++i)
            addLoopPoint (beatPoints[i].getIntValue(), LoopInfo::LoopPointType::manual);
    }
    else
   #endif
    if (isSameFormat (af, formatManager.getAiffFormat()))
    {
        const int rootNote = afr->metadataValues[yup::AiffAudioFormat::appleRootSet] == "1"
                                ? afr->metadataValues[yup::AiffAudioFormat::appleRootNote].getIntValue() : -1;

        setRootNote (rootNote);
        setProp (IDs::oneShot, afr->metadataValues[yup::AiffAudioFormat::appleOneShot] == "1");
        setDenominator (afr->metadataValues[yup::AiffAudioFormat::appleDenominator].getIntValue());
        setNumerator (afr->metadataValues[yup::AiffAudioFormat::appleNumerator].getIntValue());
        const double numBeats = afr->metadataValues[yup::AiffAudioFormat::appleBeats].getDoubleValue();
        setProp (IDs::numBeats, numBeats);
        setProp (IDs::bpm, (numBeats * 60.0) / (static_cast<double> (afr->lengthInSamples) / afr->sampleRate));

        yup::StringArray t;
        t.addTokens (afr->metadataValues[yup::AiffAudioFormat::appleTag], ";", {});
        t.add (afr->metadataValues[yup::AiffAudioFormat::appleKey]);
        t.removeEmptyStrings();

        for (int i = 0; i < t.size(); ++i)
            addTag (t[i]);
    }
    else if (isSameFormat (af, formatManager.getWavFormat()))
    {
        auto s = afr->metadataValues[yup::WavAudioFormat::tracktionLoopInfo];

        if (s.isNotEmpty())
        {
            if (auto n = yup::parseXML (s))
                copyFrom (yup::ValueTree::fromXml (*n));
        }
        else
        {
            const int rootNote = afr->metadataValues[yup::WavAudioFormat::acidRootSet] == "1"
                                    ? afr->metadataValues[yup::WavAudioFormat::acidRootNote].getIntValue() : -1;
            const double numBeats = afr->metadataValues[yup::WavAudioFormat::acidBeats].getDoubleValue();

            if (rootNote == -1)
                if (auto smplNote = afr->metadataValues["MidiUnityNote"].getIntValue())
                    setRootNote (smplNote);

            setRootNote (rootNote);
            setNumBeats (numBeats);
            setProp (IDs::oneShot, afr->metadataValues[yup::WavAudioFormat::acidOneShot] == "1");
            setDenominator (afr->metadataValues[yup::WavAudioFormat::acidDenominator].getIntValue());
            setNumerator (afr->metadataValues[yup::WavAudioFormat::acidNumerator].getIntValue());
            setNumBeats (afr->metadataValues[yup::WavAudioFormat::acidBeats].getDoubleValue());
            setProp (IDs::bpm, (numBeats * 60.0) / (static_cast<double> (afr->lengthInSamples) / afr->sampleRate));
        }
    }
    else if (isSameFormat (af, formatManager.getNativeAudioFormat()))
    {
        yup::String s = afr->metadataValues[yup::WavAudioFormat::tracktionLoopInfo];

        if (s.isNotEmpty())
        {
            if (auto n = yup::parseXML (s))
                copyFrom (yup::ValueTree::fromXml (*n));
        }
        else
        {
            const yup::String tempoString = afr->metadataValues["tempo"];
            auto bpm = tempoString.getDoubleValue();

            setProp (IDs::bpm, bpm);

            if (tempoString.isNotEmpty())
            {
                const double fileDuration = static_cast<double> (afr->lengthInSamples) / afr->sampleRate;
                setNumBeats ((fileDuration / 60.0) * bpm);
            }

            const yup::String beatCount = afr->metadataValues["beat count"];

            if (beatCount.isNotEmpty() && bpm == 0)
            {
                const double fileDuration = static_cast<double> (afr->lengthInSamples) / afr->sampleRate;
                const int beats = beatCount.getIntValue();

                setProp (IDs::bpm, beats / fileDuration / 60.0);
                setNumBeats (beats);
            }

            const yup::String timeSig (afr->metadataValues["time signature"]);

            if (timeSig.isNotEmpty())
            {
                setDenominator (timeSig.upToFirstOccurrenceOf ("/", false, false).getIntValue());
                setNumerator (timeSig.fromFirstOccurrenceOf ("/", false, false).getIntValue());
            }

            const yup::String keySig (afr->metadataValues["key signature"]);

            if (keySig.isNotEmpty())
            {
                bool sharpOrFlat = keySig[1] == '#' || keySig[1] == 'b';
                setRootNote (Pitch::getPitchFromString (engine, keySig.substring (0, sharpOrFlat ? 2 : 1)));
                addTag (keySig.getLastCharacter() == 'm' ? "minor" : "major");
            }
        }
    }
    else if (afr->metadataValues.containsKey (yup::WavAudioFormat::acidBeats)
              || afr->metadataValues.containsKey (yup::WavAudioFormat::acidNumerator))
    {
        const int rootNote = afr->metadataValues[yup::WavAudioFormat::acidRootSet] == "1"
                                ? afr->metadataValues[yup::WavAudioFormat::acidRootNote].getIntValue() : -1;
        const double numBeats = afr->metadataValues[yup::WavAudioFormat::acidBeats].getDoubleValue();

        setRootNote (rootNote);
        setNumBeats (numBeats);
        setProp (IDs::oneShot, afr->metadataValues[yup::WavAudioFormat::acidOneShot] == "1");
        setDenominator (afr->metadataValues[yup::WavAudioFormat::acidDenominator].getIntValue());
        setNumerator (afr->metadataValues[yup::WavAudioFormat::acidNumerator].getIntValue());
        setNumBeats (afr->metadataValues[yup::WavAudioFormat::acidBeats].getDoubleValue());
        setProp (IDs::bpm, (numBeats * 60.0) / (static_cast<double> (afr->lengthInSamples) / afr->sampleRate));
    }
    else if (afr->metadataValues.containsKey ("MidiUnityNote"))
    {
        auto note = afr->metadataValues["MidiUnityNote"].getIntValue();
        if (note > 0)
            setRootNote (note);
    }

    if (file != yup::File() && float (state.getProperty (IDs::bpm)) < 0.001f)
        deduceTempo (file, *afr);

    initialiseMissingProps();
}

std::optional<float> LoopInfo::getCueTempo (const yup::StringPairArray& metadata)
{
    if (auto tempoStr = metadata["CueLabel0Text"]; tempoStr.isNotEmpty())
        if (tempoStr.contains ("Tempo:"))
            if (auto val = tempoStr.fromFirstOccurrenceOf ("Tempo: ", false, false).getFloatValue(); val >= 50 && val <= 250)
                return val;

    return {};
}

static yup::StringArray reverseTokens (yup::StringRef stringToTokenise, yup::StringRef breakCharacters, yup::StringRef quoteCharacters)
{
    auto tokens = yup::StringArray::fromTokens (stringToTokenise, breakCharacters, quoteCharacters);
    std::reverse (tokens.strings.begin(), tokens.strings.end());
    return tokens;
}

std::optional<float> LoopInfo::getFileNameTempo (const yup::String& rawName)
{
    auto name = rawName.replace (" ", "_").replace ("-", "_");

    for (auto token : reverseTokens (name, "_", ""))
    {
        if (token.containsIgnoreCase ("bpm"))
        {
            token = token.replace ("bpm", "", true).trim();

            while (token.startsWith ("0"))
                token = token.substring (1);

            auto val = token.getIntValue();

            if (val > 50 && val < 250 && yup::String (val) == token)
                return float (val);
        }
    }

    for (auto token : reverseTokens (name, "_", ""))
    {
        auto val = token.getIntValue();

        while (token.startsWith ("0"))
            token = token.substring (1);

        if (val > 50 && val < 250 && yup::String (val) == token)
            return float (val);
    }

    return {};
}

std::optional<int> LoopInfo::getFileNameRootNote (const yup::String& rawName)
{
    auto name = rawName.replace (" ", "_").toLowerCase();

    for (auto token : reverseTokens (name, "_", ""))
    {
        if (token.endsWith ("min")) token = token.dropLastCharacters (3);
        else if (token.endsWith ("maj")) token = token.dropLastCharacters (3);
        else if (token.endsWith ("m")) token = token.dropLastCharacters (1);

        if (token == "a")   return 57;
        if (token == "a#")  return 58;
        if (token == "bb")  return 58;
        if (token == "b")   return 59;
        if (token == "c")   return 60;
        if (token == "c#")  return 61;
        if (token == "db")  return 61;
        if (token == "d")   return 62;
        if (token == "d#")  return 63;
        if (token == "eb")  return 63;
        if (token == "e")   return 64;
        if (token == "f")   return 65;
        if (token == "f#")  return 66;
        if (token == "gb")  return 66;
        if (token == "g")   return 67;
        if (token == "g#")  return 68;
        if (token == "ab")  return 68;
    }

    return {};
}

bool LoopInfo::deduceTempo (const yup::File& file, const yup::AudioFormatReader& afr)
{
    auto len = static_cast<double> (afr.lengthInSamples) / afr.sampleRate;
    if (len <= 1.0 || len > 60.0)
        return false;

    auto fn = file.getFileNameWithoutExtension();

    auto tempo = getCueTempo (afr.metadataValues);
    if (! tempo.has_value())
        tempo = getFileNameTempo (fn);

    if (! tempo.has_value())
        return false;

    auto beats = *tempo / 60 * len;
    auto rem = std::fmod (beats, 4.0f);
    if (rem < 0.0f || (rem > 0.1f && rem < 3.9f) || rem > 4.0f)
        return false;

    setNumBeats (beats);
    setProp (IDs::oneShot, false);
    setDenominator (4);
    setNumerator (4);
    setProp (IDs::bpm, (beats * 60.0) / (static_cast<double> (afr.lengthInSamples) / afr.sampleRate));

    if (auto root = getFileNameRootNote (fn))
    {
        setRootNote (*root);
       #if LOG_DEDUCED_TEMPO
        DBG(fn + yup::String::formatted (": %.1f bpm %.1f beats ", *tempo, beats) + yup::MidiMessage::getMidiNoteName (*root, true, false, 4));
       #endif
    }
    else
    {
       #if LOG_DEDUCED_TEMPO
        DBG(fn + yup::String::formatted (": %.1f bpm %.1f beats", *tempo, beats));
       #endif
    }

    return true;
}

} // namespace tracktion::inline engine
