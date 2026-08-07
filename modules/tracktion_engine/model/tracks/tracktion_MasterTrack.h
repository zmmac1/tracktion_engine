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
    A track to represent the master plugins.
    This isn't a "real" track, it wraps the master plugin list.
*/
class MasterTrack   : public Track
{
public:
    /** Create the MasterTrack for an Edit with a given state. */
    MasterTrack (Edit&, const yup::ValueTree&);

    /** Destructor. */
    ~MasterTrack() override;

    using Ptr = yup::ReferenceCountedObjectPtr<MasterTrack>;

    /** @internal */
    void initialise() override;
    /** @internal */
    bool isMasterTrack() const override;
    /** @internal */
    yup::String getName() const override;
    /** @internal */
    yup::String getSelectableDescription() override;

    /** @internal */
    bool canContainPlugin (Plugin*) const override;

    /** Returns the channel configuration of the default output device. */
    ChannelConfiguration getChannelConfiguration() const;

private:
    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MasterTrack)
};

} // namespace tracktion::inline engine
