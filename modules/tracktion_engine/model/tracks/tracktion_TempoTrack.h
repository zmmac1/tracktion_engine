/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

/**
    A track to represent the "global" items such as tempo, key changes etc.
    This isn't a "real" track, it wraps the TempoSequence and PitchSequence.
*/
class TempoTrack  : public Track
{
public:
    /** Create the TempoTrack for an Edit with a given state. */
    TempoTrack (Edit&, const yup::ValueTree&);

    /** Destructor. */
    ~TempoTrack() override;

    using Ptr = yup::ReferenceCountedObjectPtr<TempoTrack>;

    /** @internal */
    bool isTempoTrack() const override;
    /** @internal */
    yup::String getName() const override;
    /** @internal */
    yup::String getSelectableDescription() override;

    /** @internal */
    int getNumTrackItems() const override;
    /** @internal */
    TrackItem* getTrackItem (int idx) const override;
    /** @internal */
    int indexOfTrackItem (TrackItem*) const override;
    /** @internal */
    int getIndexOfNextTrackItemAt (TimePosition) override;
    /** @internal */
    TrackItem* getNextTrackItemAt (TimePosition) override;
    /** @internal */
    bool canContainPlugin (Plugin*) const override;

private:
    yup::Array<TrackItem*> buildTrackItemList() const;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TempoTrack)
};

} // namespace tracktion::inline engine
