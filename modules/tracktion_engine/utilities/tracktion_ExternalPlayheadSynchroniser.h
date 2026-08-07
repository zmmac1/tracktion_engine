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
/** Converts an Edit's internal transport information to a yup::AudioPlayHead::CurrentPositionInfo. */
yup::AudioPlayHead::CurrentPositionInfo getCurrentPositionInfo (Edit&);

/** Syncs an Edit's transport and tempo sequence to a juce AudioPlayHead. */
void synchroniseEditPosition (Edit&, const yup::AudioPlayHead::CurrentPositionInfo&);


//==============================================================================
//==============================================================================
/** An ExternalPlayheadSynchroniser is used to synchronise the internal Edit's playhead with an
    AudioProcessor, for use in plugins.

    Just create one when you create your Edit and repeatedly call the synchronise method
    in your process block like this:
    @code
    void processBlock (yup::AudioBuffer<float>& buffer, yup::MidiBuffer& midi) override
    {
        // Update position info first
        externalPlayheadSynchroniser.synchronise (*this);
        ...
    @endcode
*/
class ExternalPlayheadSynchroniser
{
public:
    /** Constructs an ExternalPlayheadSynchroniser. */
    ExternalPlayheadSynchroniser (Edit&);

    /** Synchronises the Edit's playback position with an AudioPlayHead if possible.
        Generally you'd call this at the start of your plugin's processBlock method.
        @returns true if it was able to synchronise, false otherwise.
    */
    bool synchronise (yup::AudioPlayHead&);

    /** Synchronises the Edit's playback position with the AudioProcessor's playhead if possible.
        Generally you'd call this at the start of your plugin's processBlock method.
        @returns true if it was able to synchronise, false otherwise.
    */
    bool synchronise (yup::AudioProcessor&);

    /** Returns the current position info, useful for graphical displays etc. */
    yup::AudioPlayHead::CurrentPositionInfo getPositionInfo() const;

private:
    Edit& edit;
    yup::AudioPlayHead::CurrentPositionInfo positionInfo;
    mutable yup::SpinLock positionInfoLock;
};

} // namespace tracktion::inline engine
