/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

//==============================================================================
/**
*/
class QuantisationType  : private yup::ValueTree::Listener
{
public:
    //==============================================================================
    QuantisationType();
    QuantisationType (const yup::ValueTree&, juce::UndoManager*);
    ~QuantisationType() override;

    QuantisationType (const QuantisationType&);
    QuantisationType& operator= (const QuantisationType&);
    bool operator== (const QuantisationType&) const;

    void applyQuantisationToSequence (yup::MidiMessageSequence&, Edit&, TimePosition sequenceStartOffset);

    //==============================================================================
    /** this type may represent "no quantising" */
    bool isEnabled() const;

    static yup::StringArray getAvailableQuantiseTypes (bool translated);
    /// Returns the list of quantise types, annotated with their musical note equivalents to
    /// avoid confusion between beat fractions and note names
    static yup::StringArray getAvailableQuantiseTypes (bool translated, int timeSigDenominator);
    static yup::StringArray getAvailableQuantiseTypes (bool translated, const Edit&);
    static yup::String beatFractionToNoteName (double beatFraction, bool isTriplet, int timeSigDenominator);
    static yup::String getDefaultType (bool translated);

    void setType (const yup::String& newTypeName);
    yup::String getType (bool translated) const;
    yup::String getType (bool translated, int timeSigDenominator) const;
    yup::String getType (bool translated, const Edit&) const;

    /** Returns the TimecodeSnapType level for the current quantisation type.
        Because the TimecodeSnapType level means something different if the time sig is
        triplets, this also needs to return the triplet type.
    */
    int getTimecodeSnapTypeLevel (bool& isTriplet) const noexcept;

    /** the proportion 0..1 of the amount to quantise stuff by. */
    float getProportion() const                             { return proportion; }
    void setProportion (float prop);

    /** If false, quantising a note on will keep the note its original length.
        If true (the default), note offs will be quantised to the same grid.
    */
    bool isQuantisingNoteOffs() const                       { return quantiseNoteOffs; }
    void setIsQuantisingNoteOffs (bool isQuantising)        { quantiseNoteOffs = isQuantising; }

    //==============================================================================
    BeatPosition roundBeatToNearest (BeatPosition beatNumber) const;
    BeatPosition roundBeatUp (BeatPosition beatNumber) const;
    BeatPosition roundBeatToNearestNonZero (BeatPosition beatNumber) const;

    TimePosition roundToNearest (TimePosition, const Edit&) const;
    TimePosition roundUp (TimePosition, const Edit&) const;

    yup::ValueTree state;
    juce::CachedValue<yup::String> typeName;
    juce::CachedValue<float> proportion;
    juce::CachedValue<bool> quantiseNoteOffs;

private:
    int typeIndex = 0;
    double fractionOfBeat = 0;

    /// Strips the parenthesised note annotation from a quantise type name,
    /// e.g. "1/2 beat (1/8 note)" -> "1/2 beat"
    static yup::String stripNoteAnnotation (const yup::String&);

    void initialiseCachedValues (juce::UndoManager*);

    void updateType();
    void updateFraction();
    TimePosition roundTo (TimePosition, double adjustment, const Edit&) const;
    BeatPosition roundToBeat (BeatPosition beatNumber, double adjustment) const;

    void valueTreePropertyChanged (yup::ValueTree&, const yup::Identifier&) override;
    void valueTreeChildAdded (yup::ValueTree&, yup::ValueTree&) override {}
    void valueTreeChildRemoved (yup::ValueTree&, yup::ValueTree&, int) override {}
    void valueTreeChildOrderChanged (yup::ValueTree&, int, int) override {}
    void valueTreeParentChanged (yup::ValueTree&) override {}
};

} // namespace tracktion::inline engine
