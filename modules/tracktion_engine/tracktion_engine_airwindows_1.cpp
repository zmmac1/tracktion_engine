/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2018
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com
*/


#if ! JUCE_PROJUCER_LIVE_BUILD

#include <atomic>
#include <numeric>
#include <set>
#include <string>
#include <math.h>

#include "tracktion_engine.h"

#if TRACKTION_AIR_WINDOWS

// EXCLUDED: #include "plugins/airwindows/tracktion_AirWindowsBase.h"
// EXCLUDED: #include "plugins/airwindows/tracktion_AirWindowsBase.cpp"

#if JUCE_CLANG
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Wnon-virtual-dtor"
 #pragma clang diagnostic ignored "-Wreorder"
 #pragma clang diagnostic ignored "-Wunsequenced"
 #pragma clang diagnostic ignored "-Wint-to-pointer-cast"
 #pragma clang diagnostic ignored "-Wunused-parameter"
 #pragma clang diagnostic ignored "-Wconversion"
 #pragma clang diagnostic ignored "-Woverloaded-virtual"
 #pragma clang diagnostic ignored "-Wshadow"
 #pragma clang diagnostic ignored "-Wdeprecated-register"
 #pragma clang diagnostic ignored "-Wunused-function"
 #pragma clang diagnostic ignored "-Wsign-conversion"
 #pragma clang diagnostic ignored "-Wsign-compare"
 #pragma clang diagnostic ignored "-Wdelete-non-virtual-dtor"
 #pragma clang diagnostic ignored "-Wdeprecated-declarations"
 #pragma clang diagnostic ignored "-Wextra-semi"
 #pragma clang diagnostic ignored "-Wunused-value"
 #pragma clang diagnostic ignored "-Wunused-private-field"
 #pragma clang diagnostic ignored "-Wuninitialized"
 #pragma clang diagnostic ignored "-Widiomatic-parentheses"
 #pragma clang diagnostic ignored "-Wconditional-uninitialized"
 #pragma clang diagnostic ignored "-Wparentheses"
 #pragma clang diagnostic ignored "-Wunused-but-set-variable"
#endif

#if JUCE_WINDOWS
 #pragma warning (push)
 #pragma warning (disable : 4244 4100 4305 4065 4701 4706 4723)
#endif

namespace tracktion::inline engine {
namespace airwindows
{
namespace adclip7
{
// EXCLUDED:  #include "3rd_party/airwindows/ADClip7/ADClip7.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/ADClip7/ADClip7Proc.cpp"
}
namespace adt
{
// EXCLUDED:  #include "3rd_party/airwindows/ADT/ADT.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/ADT/ADTProc.cpp"
}
namespace aquickvoiceclip
{
// EXCLUDED:  #include "3rd_party/airwindows/AQuickVoiceClip/AQuickVoiceClip.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/AQuickVoiceClip/AQuickVoiceClipProc.cpp"
}
namespace acceleration
{
// EXCLUDED:  #include "3rd_party/airwindows/Acceleration/Acceleration.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Acceleration/AccelerationProc.cpp"
}
namespace air
{
// EXCLUDED:  #include "3rd_party/airwindows/Air/Air.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Air/AirProc.cpp"
}
namespace atmospherebuss
{
// EXCLUDED:  #include "3rd_party/airwindows/AtmosphereBuss/AtmosphereBuss.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/AtmosphereBuss/AtmosphereBussProc.cpp"
}
namespace atmospherechannel
{
// EXCLUDED:  #include "3rd_party/airwindows/AtmosphereChannel/AtmosphereChannel.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/AtmosphereChannel/AtmosphereChannelProc.cpp"
}
namespace aura
{
// EXCLUDED:  #include "3rd_party/airwindows/Aura/Aura.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Aura/AuraProc.cpp"
}
namespace average
{
// EXCLUDED:  #include "3rd_party/airwindows/Average/Average.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Average/AverageProc.cpp"
}
namespace bassdrive
{
// EXCLUDED:  #include "3rd_party/airwindows/BassDrive/BassDrive.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/BassDrive/BassDriveProc.cpp"
}
namespace basskit
{
// EXCLUDED:  #include "3rd_party/airwindows/BassKit/BassKit.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/BassKit/BassKitProc.cpp"
}
namespace biquad
{
// EXCLUDED:  #include "3rd_party/airwindows/Biquad/Biquad.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Biquad/BiquadProc.cpp"
}
namespace biquad2
{
// EXCLUDED:  #include "3rd_party/airwindows/Biquad2/Biquad2.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Biquad2/Biquad2Proc.cpp"
}
namespace bitglitter
{
// EXCLUDED:  #include "3rd_party/airwindows/BitGlitter/BitGlitter.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/BitGlitter/BitGlitterProc.cpp"
}
namespace bitshiftgain
{
// EXCLUDED:  #include "3rd_party/airwindows/BitShiftGain/BitShiftGain.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/BitShiftGain/BitShiftGainProc.cpp"
}
namespace bite
{
// EXCLUDED:  #include "3rd_party/airwindows/Bite/Bite.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Bite/BiteProc.cpp"
}
namespace blockparty
{
// EXCLUDED:  #include "3rd_party/airwindows/BlockParty/BlockParty.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/BlockParty/BlockPartyProc.cpp"
}
namespace brassrider
{
// EXCLUDED:  #include "3rd_party/airwindows/BrassRider/BrassRider.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/BrassRider/BrassRiderProc.cpp"
}
namespace buildatpdf
{
// EXCLUDED:  #include "3rd_party/airwindows/BuildATPDF/BuildATPDF.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/BuildATPDF/BuildATPDFProc.cpp"
}
namespace busscolors4
{
// EXCLUDED:  #include "3rd_party/airwindows/BussColors4/BussColors4.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/BussColors4/BussColors4Proc.cpp"
}
namespace buttercomp
{
// EXCLUDED:  #include "3rd_party/airwindows/ButterComp/ButterComp.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/ButterComp/ButterCompProc.cpp"
}
namespace buttercomp2
{
// EXCLUDED:  #include "3rd_party/airwindows/ButterComp2/ButterComp2.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/ButterComp2/ButterComp2Proc.cpp"
}
namespace c5rawbuss
{
// EXCLUDED:  #include "3rd_party/airwindows/C5RawBuss/C5RawBuss.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/C5RawBuss/C5RawBussProc.cpp"
}
namespace c5rawchannel
{
// EXCLUDED:  #include "3rd_party/airwindows/C5RawChannel/C5RawChannel.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/C5RawChannel/C5RawChannelProc.cpp"
}
namespace cstrip
{
// EXCLUDED:  #include "3rd_party/airwindows/CStrip/CStrip.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/CStrip/CStripProc.cpp"
}
namespace capacitor
{
// EXCLUDED:  #include "3rd_party/airwindows/Capacitor/Capacitor.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Capacitor/CapacitorProc.cpp"
}
namespace channel4
{
// EXCLUDED:  #include "3rd_party/airwindows/Channel4/Channel4.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Channel4/Channel4Proc.cpp"
}
namespace channel5
{
// EXCLUDED:  #include "3rd_party/airwindows/Channel5/Channel5.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Channel5/Channel5Proc.cpp"
}
namespace channel6
{
// EXCLUDED:  #include "3rd_party/airwindows/Channel6/Channel6.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Channel6/Channel6Proc.cpp"
}
namespace channel7
{
// EXCLUDED:  #include "3rd_party/airwindows/Channel7/Channel7.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Channel7/Channel7Proc.cpp"
}
namespace chorus
{
// EXCLUDED:  #include "3rd_party/airwindows/Chorus/Chorus.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Chorus/ChorusProc.cpp"
}
namespace chorusensemble
{
// EXCLUDED:  #include "3rd_party/airwindows/ChorusEnsemble/ChorusEnsemble.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/ChorusEnsemble/ChorusEnsembleProc.cpp"
}
namespace cliponly
{
// EXCLUDED:  #include "3rd_party/airwindows/ClipOnly/ClipOnly.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/ClipOnly/ClipOnlyProc.cpp"
}
namespace coils
{
// EXCLUDED:  #include "3rd_party/airwindows/Coils/Coils.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Coils/CoilsProc.cpp"
}
namespace cojones
{
// EXCLUDED:  #include "3rd_party/airwindows/Cojones/Cojones.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Cojones/CojonesProc.cpp"
}
namespace compresaturator
{
// EXCLUDED:  #include "3rd_party/airwindows/Compresaturator/Compresaturator.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Compresaturator/CompresaturatorProc.cpp"
}
namespace console4buss
{
// EXCLUDED:  #include "3rd_party/airwindows/Console4Buss/Console4Buss.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Console4Buss/Console4BussProc.cpp"
}
namespace console4channel
{
// EXCLUDED:  #include "3rd_party/airwindows/Console4Channel/Console4Channel.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Console4Channel/Console4ChannelProc.cpp"
}
namespace console5buss
{
// EXCLUDED:  #include "3rd_party/airwindows/Console5Buss/Console5Buss.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Console5Buss/Console5BussProc.cpp"
}
namespace console5channel
{
// EXCLUDED:  #include "3rd_party/airwindows/Console5Channel/Console5Channel.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Console5Channel/Console5ChannelProc.cpp"
}
namespace console5darkch
{
// EXCLUDED:  #include "3rd_party/airwindows/Console5DarkCh/Console5DarkCh.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Console5DarkCh/Console5DarkChProc.cpp"
}
namespace console6buss
{
// EXCLUDED:  #include "3rd_party/airwindows/Console6Buss/Console6Buss.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Console6Buss/Console6BussProc.cpp"
}
namespace console6channel
{
// EXCLUDED:  #include "3rd_party/airwindows/Console6Channel/Console6Channel.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Console6Channel/Console6ChannelProc.cpp"
}
namespace crunchygroovewear
{
// EXCLUDED:  #include "3rd_party/airwindows/CrunchyGrooveWear/CrunchyGrooveWear.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/CrunchyGrooveWear/CrunchyGrooveWearProc.cpp"
}
namespace crystal
{
// EXCLUDED:  #include "3rd_party/airwindows/Crystal/Crystal.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Crystal/CrystalProc.cpp"
}
namespace dcvoltage
{
// EXCLUDED:  #include "3rd_party/airwindows/DCVoltage/DCVoltage.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/DCVoltage/DCVoltageProc.cpp"
}
namespace debess
{
// EXCLUDED:  #include "3rd_party/airwindows/DeBess/DeBess.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/DeBess/DeBessProc.cpp"
}
namespace deess
{
// EXCLUDED:  #include "3rd_party/airwindows/DeEss/DeEss.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/DeEss/DeEssProc.cpp"
}
namespace dehiss
{
// EXCLUDED:  #include "3rd_party/airwindows/DeHiss/DeHiss.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/DeHiss/DeHissProc.cpp"
}
namespace derez
{
// EXCLUDED:  #include "3rd_party/airwindows/DeRez/DeRez.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/DeRez/DeRezProc.cpp"
}
namespace derez2
{
// EXCLUDED:  #include "3rd_party/airwindows/DeRez2/DeRez2.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/DeRez2/DeRez2Proc.cpp"
}
namespace deckwrecka
{
// EXCLUDED:  #include "3rd_party/airwindows/Deckwrecka/Deckwrecka.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Deckwrecka/DeckwreckaProc.cpp"
}
namespace density
{
// EXCLUDED:  #include "3rd_party/airwindows/Density/Density.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Density/DensityProc.cpp"
}
namespace desk
{
// EXCLUDED:  #include "3rd_party/airwindows/Desk/Desk.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Desk/DeskProc.cpp"
}
namespace desk4
{
// EXCLUDED:  #include "3rd_party/airwindows/Desk4/Desk4.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Desk4/Desk4Proc.cpp"
}
namespace distance
{
// EXCLUDED:  #include "3rd_party/airwindows/Distance/Distance.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Distance/DistanceProc.cpp"
}
namespace distance2
{
// EXCLUDED:  #include "3rd_party/airwindows/Distance2/Distance2.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Distance2/Distance2Proc.cpp"
}
namespace ditherfloat
{
// EXCLUDED:  #include "3rd_party/airwindows/DitherFloat/DitherFloat.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/DitherFloat/DitherFloatProc.cpp"
}
namespace dithermediskers
{
// EXCLUDED:  #include "3rd_party/airwindows/DitherMeDiskers/DitherMeDiskers.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/DitherMeDiskers/DitherMeDiskersProc.cpp"
}
namespace dithermetimbers
{
// EXCLUDED:  #include "3rd_party/airwindows/DitherMeTimbers/DitherMeTimbers.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/DitherMeTimbers/DitherMeTimbersProc.cpp"
}
namespace ditherbox
{
// EXCLUDED:  #include "3rd_party/airwindows/Ditherbox/Ditherbox.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Ditherbox/DitherboxProc.cpp"
}
}
} // namespace tracktion::inline engine


#if JUCE_CLANG
 #pragma clang diagnostic pop
#endif

#if JUCE_WINDOWS
 #pragma warning (pop)
#endif

// EXCLUDED: #include "plugins/airwindows/tracktion_AirWindows1.cpp"

#endif
#endif
