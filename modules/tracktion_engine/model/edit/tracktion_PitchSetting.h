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
class PitchSetting   : public TrackItem,
                       private ValueTreeAllEventListener
{
public:
    PitchSetting (Edit&, const yup::ValueTree&);
    ~PitchSetting() override;

    using Ptr = yup::ReferenceCountedObjectPtr<PitchSetting>;
    using Array = yup::ReferenceCountedArray<PitchSetting>;

    yup::String getSelectableDescription() override;

    BeatPosition getStartBeatNumber() const       { return startBeat; }
    void setStartBeat (BeatPosition);

    int getPitch() const                    { return pitch; }
    void setPitch (int);

    Scale::ScaleType getScale() const       { return scale; }
    void setScaleID (Scale::ScaleType);

    void removeFromEdit();

    yup::String getName() const override;
    Track* getTrack() const override;
    ClipPosition getPosition() const override;

    yup::ValueTree state;
    juce::CachedValue<BeatPosition> startBeat;
    juce::CachedValue<int> pitch;
    juce::CachedValue<bool> accidentalsSharp;
    juce::CachedValue<Scale::ScaleType> scale;

private:
    void valueTreeChanged() override { changed(); }

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PitchSetting)
};

inline bool operator< (const PitchSetting& p1, const PitchSetting& p2) noexcept
{
    return p1.startBeat.get() < p2.startBeat.get();
}

} // namespace tracktion::inline engine
