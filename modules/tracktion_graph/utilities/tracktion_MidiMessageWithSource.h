/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

#pragma once

namespace tracktion::inline engine {

using MPESourceID = uint32_t;


struct MidiMessageWithSource  : public yup::MidiMessage
{
    MidiMessageWithSource (const yup::MidiMessage& m, MPESourceID source) : yup::MidiMessage (m), mpeSourceID (source) {}
    MidiMessageWithSource (yup::MidiMessage&& m, MPESourceID source) : yup::MidiMessage (std::move (m)), mpeSourceID (source) {}

    MidiMessageWithSource (const MidiMessageWithSource&) = default;
    MidiMessageWithSource (MidiMessageWithSource&&) = default;
    MidiMessageWithSource& operator= (const MidiMessageWithSource&) = default;
    MidiMessageWithSource& operator= (MidiMessageWithSource&&) = default;

    MPESourceID mpeSourceID = {};
};

inline static MPESourceID createUniqueMPESourceID() noexcept
{
    static MPESourceID i = 0;
    return ++i;
}

[[ deprecated ]]
static constexpr MPESourceID notMPE = 0;


} // namespace tracktion::inline graph