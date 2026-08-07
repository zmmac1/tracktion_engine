/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

/** A Node that plays MIDI data from a MidiMessageSequence,
    at a specific MIDI channel.
*/
class MidiNode final    : public tracktion::graph::Node,
                          public TracktionEngineNode
{
public:
    MidiNode (std::vector<yup::MidiMessageSequence> sequences,
              MidiList::TimeBase,
              yup::Range<int> midiChannelNumbers,
              bool useMPE,
              yup::Range<double> editSection,
              LiveClipLevel,
              ProcessState&,
              EditItemID,
              std::function<bool()> shouldBeMutedDelegate = nullptr);

    tracktion::graph::NodeProperties getNodeProperties() override;
    void prepareToPlay (const tracktion::graph::PlaybackInitialisationInfo&) override;
    bool isReadyToProcess() override;
    void process (ProcessContext&) override;

private:
    //==============================================================================
    std::vector<yup::MidiMessageSequence> ms;
    const MidiList::TimeBase timeBase;
    int64_t lastStart = 0;
    size_t currentSequence = 0;
    yup::Range<int> channelNumbers;
    bool useMPEChannelMode;
    yup::Range<double> editRange;
    LiveClipLevel clipLevel;
    EditItemID editItemID;
    std::function<bool()> shouldBeMutedDelegate = nullptr;

    double sampleRate = 44100.0;
    int currentIndex = 0;
    MPESourceID midiSourceID = createUniqueMPESourceID();
    bool wasMute = false, shouldCreateMessagesForTime = false;

    yup::Array<yup::MidiMessage> controllerMessagesScratchBuffer;

    //==============================================================================
    void processSection (Node::ProcessContext&,
                         yup::Range<double> sectionEditTime,
                         double secondsPerTimeBase,
                         yup::MidiMessageSequence&);
};

} // namespace tracktion::inline engine
