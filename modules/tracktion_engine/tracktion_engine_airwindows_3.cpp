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
namespace purestair
{
// EXCLUDED:  #include "3rd_party/airwindows/PurestAir/PurestAir.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/PurestAir/PurestAirProc.cpp"
}
namespace purestconsolebuss
{
// EXCLUDED:  #include "3rd_party/airwindows/PurestConsoleBuss/PurestConsoleBuss.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/PurestConsoleBuss/PurestConsoleBussProc.cpp"
}
namespace purestconsolechannel
{
// EXCLUDED:  #include "3rd_party/airwindows/PurestConsoleChannel/PurestConsoleChannel.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/PurestConsoleChannel/PurestConsoleChannelProc.cpp"
}
namespace purestdrive
{
// EXCLUDED:  #include "3rd_party/airwindows/PurestDrive/PurestDrive.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/PurestDrive/PurestDriveProc.cpp"
}
namespace purestecho
{
// EXCLUDED:  #include "3rd_party/airwindows/PurestEcho/PurestEcho.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/PurestEcho/PurestEchoProc.cpp"
}
namespace purestgain
{
// EXCLUDED:  #include "3rd_party/airwindows/PurestGain/PurestGain.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/PurestGain/PurestGainProc.cpp"
}
namespace purestsquish
{
// EXCLUDED:  #include "3rd_party/airwindows/PurestSquish/PurestSquish.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/PurestSquish/PurestSquishProc.cpp"
}
namespace purestwarm
{
// EXCLUDED:  #include "3rd_party/airwindows/PurestWarm/PurestWarm.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/PurestWarm/PurestWarmProc.cpp"
}
namespace pyewacket
{
// EXCLUDED:  #include "3rd_party/airwindows/Pyewacket/Pyewacket.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Pyewacket/PyewacketProc.cpp"
}
namespace rawglitters
{
// EXCLUDED:  #include "3rd_party/airwindows/RawGlitters/RawGlitters.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/RawGlitters/RawGlittersProc.cpp"
}
namespace rawtimbers
{
// EXCLUDED:  #include "3rd_party/airwindows/RawTimbers/RawTimbers.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/RawTimbers/RawTimbersProc.cpp"
}
namespace recurve
{
// EXCLUDED:  #include "3rd_party/airwindows/Recurve/Recurve.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Recurve/RecurveProc.cpp"
}
namespace remap
{
// EXCLUDED:  #include "3rd_party/airwindows/Remap/Remap.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Remap/RemapProc.cpp"
}
namespace reseq
{
// EXCLUDED:  #include "3rd_party/airwindows/ResEQ/ResEQ.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/ResEQ/ResEQProc.cpp"
}
namespace righteous4
{
// EXCLUDED:  #include "3rd_party/airwindows/Righteous4/Righteous4.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Righteous4/Righteous4Proc.cpp"
}
namespace rightomono
{
// EXCLUDED:  #include "3rd_party/airwindows/RightoMono/RightoMono.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/RightoMono/RightoMonoProc.cpp"
}
namespace sidedull
{
// EXCLUDED:  #include "3rd_party/airwindows/SideDull/SideDull.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/SideDull/SideDullProc.cpp"
}
namespace sidepass
{
// EXCLUDED:  #include "3rd_party/airwindows/Sidepass/Sidepass.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Sidepass/SidepassProc.cpp"
}
namespace singleendedtriode
{
// EXCLUDED:  #include "3rd_party/airwindows/SingleEndedTriode/SingleEndedTriode.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/SingleEndedTriode/SingleEndedTriodeProc.cpp"
}
namespace slew
{
// EXCLUDED:  #include "3rd_party/airwindows/Slew/Slew.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Slew/SlewProc.cpp"
}
namespace slew2
{
// EXCLUDED:  #include "3rd_party/airwindows/Slew2/Slew2.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Slew2/Slew2Proc.cpp"
}
namespace slewonly
{
// EXCLUDED:  #include "3rd_party/airwindows/SlewOnly/SlewOnly.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/SlewOnly/SlewOnlyProc.cpp"
}
namespace smooth
{
// EXCLUDED:  #include "3rd_party/airwindows/Smooth/Smooth.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Smooth/SmoothProc.cpp"
}
namespace softgate
{
// EXCLUDED:  #include "3rd_party/airwindows/SoftGate/SoftGate.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/SoftGate/SoftGateProc.cpp"
}
namespace spatializedither
{
// EXCLUDED:  #include "3rd_party/airwindows/SpatializeDither/SpatializeDither.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/SpatializeDither/SpatializeDitherProc.cpp"
}
namespace spiral
{
// EXCLUDED:  #include "3rd_party/airwindows/Spiral/Spiral.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Spiral/SpiralProc.cpp"
}
namespace spiral2
{
// EXCLUDED:  #include "3rd_party/airwindows/Spiral2/Spiral2.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Spiral2/Spiral2Proc.cpp"
}
namespace starchild
{
// EXCLUDED:  #include "3rd_party/airwindows/StarChild/StarChild.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/StarChild/StarChildProc.cpp"
}
namespace stereofx
{
// EXCLUDED:  #include "3rd_party/airwindows/StereoFX/StereoFX.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/StereoFX/StereoFXProc.cpp"
}
namespace studiotan
{
// EXCLUDED:  #include "3rd_party/airwindows/StudioTan/StudioTan.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/StudioTan/StudioTanProc.cpp"
}
namespace subsonly
{
// EXCLUDED:  #include "3rd_party/airwindows/SubsOnly/SubsOnly.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/SubsOnly/SubsOnlyProc.cpp"
}
namespace surge
{
// EXCLUDED:  #include "3rd_party/airwindows/Surge/Surge.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Surge/SurgeProc.cpp"
}
namespace surgetide
{
// EXCLUDED:  #include "3rd_party/airwindows/SurgeTide/SurgeTide.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/SurgeTide/SurgeTideProc.cpp"
}
namespace swell
{
// EXCLUDED:  #include "3rd_party/airwindows/Swell/Swell.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Swell/SwellProc.cpp"
}
namespace tpdfdither
{
// EXCLUDED:  #include "3rd_party/airwindows/TPDFDither/TPDFDither.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/TPDFDither/TPDFDitherProc.cpp"
}
namespace tapedelay
{
// EXCLUDED:  #include "3rd_party/airwindows/TapeDelay/TapeDelay.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/TapeDelay/TapeDelayProc.cpp"
}
namespace tapedither
{
// EXCLUDED:  #include "3rd_party/airwindows/TapeDither/TapeDither.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/TapeDither/TapeDitherProc.cpp"
}
namespace tapedust
{
// EXCLUDED:  #include "3rd_party/airwindows/TapeDust/TapeDust.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/TapeDust/TapeDustProc.cpp"
}
namespace tapefat
{
// EXCLUDED:  #include "3rd_party/airwindows/TapeFat/TapeFat.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/TapeFat/TapeFatProc.cpp"
}
namespace thunder
{
// EXCLUDED:  #include "3rd_party/airwindows/Thunder/Thunder.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Thunder/ThunderProc.cpp"
}
namespace totape5
{
// EXCLUDED:  #include "3rd_party/airwindows/ToTape5/ToTape5.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/ToTape5/ToTape5Proc.cpp"
}
namespace tovinyl4
{
// EXCLUDED:  #include "3rd_party/airwindows/ToVinyl4/ToVinyl4.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/ToVinyl4/ToVinyl4Proc.cpp"
}
namespace toneslant
{
// EXCLUDED:  #include "3rd_party/airwindows/ToneSlant/ToneSlant.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/ToneSlant/ToneSlantProc.cpp"
}
namespace transdesk
{
// EXCLUDED:  #include "3rd_party/airwindows/TransDesk/TransDesk.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/TransDesk/TransDeskProc.cpp"
}
namespace tremolo
{
// EXCLUDED:  #include "3rd_party/airwindows/Tremolo/Tremolo.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Tremolo/TremoloProc.cpp"
}
namespace tubedesk
{
// EXCLUDED:  #include "3rd_party/airwindows/TubeDesk/TubeDesk.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/TubeDesk/TubeDeskProc.cpp"
}
namespace unbox
{
// EXCLUDED:  #include "3rd_party/airwindows/UnBox/UnBox.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/UnBox/UnBoxProc.cpp"
}
namespace varimu
{
// EXCLUDED:  #include "3rd_party/airwindows/VariMu/VariMu.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/VariMu/VariMuProc.cpp"
}
namespace vibrato
{
// EXCLUDED:  #include "3rd_party/airwindows/Vibrato/Vibrato.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Vibrato/VibratoProc.cpp"
}
namespace vinyldither
{
// EXCLUDED:  #include "3rd_party/airwindows/VinylDither/VinylDither.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/VinylDither/VinylDitherProc.cpp"
}
namespace voiceofthestarship
{
// EXCLUDED:  #include "3rd_party/airwindows/VoiceOfTheStarship/VoiceOfTheStarship.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/VoiceOfTheStarship/VoiceOfTheStarshipProc.cpp"
}
namespace voicetrick
{
// EXCLUDED:  #include "3rd_party/airwindows/VoiceTrick/VoiceTrick.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/VoiceTrick/VoiceTrickProc.cpp"
}
namespace wider
{
// EXCLUDED:  #include "3rd_party/airwindows/Wider/Wider.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Wider/WiderProc.cpp"
}
namespace curve
{
// EXCLUDED:  #include "3rd_party/airwindows/curve/curve.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/curve/curveProc.cpp"
}
namespace ulawdecode
{
// EXCLUDED:  #include "3rd_party/airwindows/uLawDecode/uLawDecode.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/uLawDecode/uLawDecodeProc.cpp"
}
namespace ulawencode
{
// EXCLUDED:  #include "3rd_party/airwindows/uLawEncode/uLawEncode.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/uLawEncode/uLawEncodeProc.cpp"
}

}
} // namespace tracktion::inline engine


#if JUCE_CLANG
 #pragma clang diagnostic pop
#endif

#if JUCE_WINDOWS
 #pragma warning (pop)
#endif

// EXCLUDED: #include "plugins/airwindows/tracktion_AirWindows3.cpp"

#endif
#endif
