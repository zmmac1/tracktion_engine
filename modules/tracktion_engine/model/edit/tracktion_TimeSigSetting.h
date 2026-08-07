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
class TimeSigSetting  : public TrackItem,
                        private ValueTreeAllEventListener
{
public:
    TimeSigSetting (TempoSequence&, const yup::ValueTree&);
    ~TimeSigSetting() override;

    using Ptr = yup::ReferenceCountedObjectPtr<TimeSigSetting>;
    using Array = yup::ReferenceCountedArray<TimeSigSetting>;

    //==============================================================================
    ClipPosition getPosition() const override;
    BeatPosition getStartBeat() const                     { return startBeatNumber; }

    //==============================================================================
    // time sig in the form "4/4"
    yup::String getStringTimeSig() const;
    void setStringTimeSig (const yup::String&);

    void removeFromEdit();

    //==============================================================================
    Track* getTrack() const override;

    //==============================================================================
    yup::String getName() const override;
    yup::String getSelectableDescription() override;

    //==============================================================================
    yup::ValueTree state;
    TempoSequence& ownerSequence;

    juce::CachedValue<BeatPosition> startBeatNumber;
    juce::CachedValue<int> numerator, denominator;
    juce::CachedValue<bool> triplets;

    TimePosition startTime; // (updated by TempoSequence)
    TimePosition endTime;

private:
    void valueTreeChanged() override { changed(); }

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimeSigSetting)
};

} // namespace tracktion::inline engine
