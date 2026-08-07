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
namespace doublepaul
{
// EXCLUDED:  #include "3rd_party/airwindows/DoublePaul/DoublePaul.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/DoublePaul/DoublePaulProc.cpp"
}
namespace drive
{
// EXCLUDED:  #include "3rd_party/airwindows/Drive/Drive.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Drive/DriveProc.cpp"
}
namespace drumslam
{
// EXCLUDED:  #include "3rd_party/airwindows/DrumSlam/DrumSlam.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/DrumSlam/DrumSlamProc.cpp"
}
namespace dubcenter
{
// EXCLUDED:  #include "3rd_party/airwindows/DubCenter/DubCenter.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/DubCenter/DubCenterProc.cpp"
}
namespace dubsub
{
// EXCLUDED:  #include "3rd_party/airwindows/DubSub/DubSub.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/DubSub/DubSubProc.cpp"
}
namespace dustbunny
{
// EXCLUDED:  #include "3rd_party/airwindows/DustBunny/DustBunny.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/DustBunny/DustBunnyProc.cpp"
}
namespace dyno
{
// EXCLUDED:  #include "3rd_party/airwindows/Dyno/Dyno.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Dyno/DynoProc.cpp"
}
namespace eq
{
// EXCLUDED:  #include "3rd_party/airwindows/EQ/EQ.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/EQ/EQProc.cpp"
}
namespace edisdim
{
// EXCLUDED:  #include "3rd_party/airwindows/EdIsDim/EdIsDim.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/EdIsDim/EdIsDimProc.cpp"
}
namespace electrohat
{
// EXCLUDED:  #include "3rd_party/airwindows/ElectroHat/ElectroHat.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/ElectroHat/ElectroHatProc.cpp"
}
namespace energy
{
// EXCLUDED:  #include "3rd_party/airwindows/Energy/Energy.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Energy/EnergyProc.cpp"
}
namespace ensemble
{
// EXCLUDED:  #include "3rd_party/airwindows/Ensemble/Ensemble.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Ensemble/EnsembleProc.cpp"
}
namespace everytrim
{
// EXCLUDED:  #include "3rd_party/airwindows/EveryTrim/EveryTrim.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/EveryTrim/EveryTrimProc.cpp"
}
namespace facet
{
// EXCLUDED:  #include "3rd_party/airwindows/Facet/Facet.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Facet/FacetProc.cpp"
}
namespace fathomfive
{
// EXCLUDED:  #include "3rd_party/airwindows/FathomFive/FathomFive.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/FathomFive/FathomFiveProc.cpp"
}
namespace awfloor
{
// EXCLUDED:  #include "3rd_party/airwindows/Floor/Floor.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Floor/FloorProc.cpp"
}
namespace focus
{
// EXCLUDED:  #include "3rd_party/airwindows/Focus/Focus.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Focus/FocusProc.cpp"
}
namespace fracture
{
// EXCLUDED:  #include "3rd_party/airwindows/Fracture/Fracture.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Fracture/FractureProc.cpp"
}
namespace fromtape
{
// EXCLUDED:  #include "3rd_party/airwindows/FromTape/FromTape.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/FromTape/FromTapeProc.cpp"
}
namespace gatelope
{
// EXCLUDED:  #include "3rd_party/airwindows/Gatelope/Gatelope.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Gatelope/GatelopeProc.cpp"
}
namespace golem
{
// EXCLUDED:  #include "3rd_party/airwindows/Golem/Golem.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Golem/GolemProc.cpp"
}
namespace gringer
{
// EXCLUDED:  #include "3rd_party/airwindows/Gringer/Gringer.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Gringer/GringerProc.cpp"
}
namespace groovewear
{
// EXCLUDED:  #include "3rd_party/airwindows/GrooveWear/GrooveWear.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/GrooveWear/GrooveWearProc.cpp"
}
namespace guitarconditioner
{
// EXCLUDED:  #include "3rd_party/airwindows/GuitarConditioner/GuitarConditioner.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/GuitarConditioner/GuitarConditionerProc.cpp"
}
namespace hardvacuum
{
// EXCLUDED:  #include "3rd_party/airwindows/HardVacuum/HardVacuum.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/HardVacuum/HardVacuumProc.cpp"
}
namespace hermetrim
{
// EXCLUDED:  #include "3rd_party/airwindows/HermeTrim/HermeTrim.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/HermeTrim/HermeTrimProc.cpp"
}
namespace hermepass
{
// EXCLUDED:  #include "3rd_party/airwindows/Hermepass/Hermepass.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Hermepass/HermepassProc.cpp"
}
namespace highglossdither
{
// EXCLUDED:  #include "3rd_party/airwindows/HighGlossDither/HighGlossDither.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/HighGlossDither/HighGlossDitherProc.cpp"
}
namespace highimpact
{
// EXCLUDED:  #include "3rd_party/airwindows/HighImpact/HighImpact.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/HighImpact/HighImpactProc.cpp"
}
namespace highpass
{
// EXCLUDED:  #include "3rd_party/airwindows/Highpass/Highpass.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Highpass/HighpassProc.cpp"
}
namespace highpass2
{
// EXCLUDED:  #include "3rd_party/airwindows/Highpass2/Highpass2.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Highpass2/Highpass2Proc.cpp"
}
namespace holt
{
// EXCLUDED:  #include "3rd_party/airwindows/Holt/Holt.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Holt/HoltProc.cpp"
}
namespace hombre
{
// EXCLUDED:  #include "3rd_party/airwindows/Hombre/Hombre.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Hombre/HombreProc.cpp"
}
namespace interstage
{
// EXCLUDED:  #include "3rd_party/airwindows/Interstage/Interstage.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Interstage/InterstageProc.cpp"
}
namespace ironoxide5
{
// EXCLUDED:  #include "3rd_party/airwindows/IronOxide5/IronOxide5.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/IronOxide5/IronOxide5Proc.cpp"
}
namespace ironoxideclassic
{
// EXCLUDED:  #include "3rd_party/airwindows/IronOxideClassic/IronOxideClassic.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/IronOxideClassic/IronOxideClassicProc.cpp"
}
namespace leftomono
{
// EXCLUDED:  #include "3rd_party/airwindows/LeftoMono/LeftoMono.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/LeftoMono/LeftoMonoProc.cpp"
}
namespace logical4
{
// EXCLUDED:  #include "3rd_party/airwindows/Logical4/Logical4.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Logical4/Logical4Proc.cpp"
}
namespace loud
{
// EXCLUDED:  #include "3rd_party/airwindows/Loud/Loud.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Loud/LoudProc.cpp"
}
namespace lowpass
{
// EXCLUDED:  #include "3rd_party/airwindows/Lowpass/Lowpass.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Lowpass/LowpassProc.cpp"
}
namespace lowpass2
{
// EXCLUDED:  #include "3rd_party/airwindows/Lowpass2/Lowpass2.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Lowpass2/Lowpass2Proc.cpp"
}
namespace mv
{
// EXCLUDED:  #include "3rd_party/airwindows/MV/MV.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/MV/MVProc.cpp"
}
namespace melt
{
// EXCLUDED:  #include "3rd_party/airwindows/Melt/Melt.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Melt/MeltProc.cpp"
}
namespace midside
{
// EXCLUDED:  #include "3rd_party/airwindows/MidSide/MidSide.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/MidSide/MidSideProc.cpp"
}
namespace monoam
{
// EXCLUDED:  #include "3rd_party/airwindows/MoNoam/MoNoam.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/MoNoam/MoNoamProc.cpp"
}
namespace mojo
{
// EXCLUDED:  #include "3rd_party/airwindows/Mojo/Mojo.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Mojo/MojoProc.cpp"
}
namespace monitoring
{
// EXCLUDED:  #include "3rd_party/airwindows/Monitoring/Monitoring.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Monitoring/MonitoringProc.cpp"
}
namespace ncseventeen
{
// EXCLUDED:  #include "3rd_party/airwindows/NCSeventeen/NCSeventeen.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/NCSeventeen/NCSeventeenProc.cpp"
}
namespace naturalizedither
{
// EXCLUDED:  #include "3rd_party/airwindows/NaturalizeDither/NaturalizeDither.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/NaturalizeDither/NaturalizeDitherProc.cpp"
}
namespace nodedither
{
// EXCLUDED:  #include "3rd_party/airwindows/NodeDither/NodeDither.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/NodeDither/NodeDitherProc.cpp"
}
namespace noise
{
// EXCLUDED:  #include "3rd_party/airwindows/Noise/Noise.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Noise/NoiseProc.cpp"
}
namespace nonlinearspace
{
// EXCLUDED:  #include "3rd_party/airwindows/NonlinearSpace/NonlinearSpace.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/NonlinearSpace/NonlinearSpaceProc.cpp"
}
namespace notjustanothercd
{
// EXCLUDED:  #include "3rd_party/airwindows/NotJustAnotherCD/NotJustAnotherCD.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/NotJustAnotherCD/NotJustAnotherCDProc.cpp"
}
namespace notjustanotherdither
{
// EXCLUDED:  #include "3rd_party/airwindows/NotJustAnotherDither/NotJustAnotherDither.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/NotJustAnotherDither/NotJustAnotherDitherProc.cpp"
}
namespace onecornerclip
{
// EXCLUDED:  #include "3rd_party/airwindows/OneCornerClip/OneCornerClip.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/OneCornerClip/OneCornerClipProc.cpp"
}
namespace pdbuss
{
// EXCLUDED:  #include "3rd_party/airwindows/PDBuss/PDBuss.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/PDBuss/PDBussProc.cpp"
}
namespace pdchannel
{
// EXCLUDED:  #include "3rd_party/airwindows/PDChannel/PDChannel.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/PDChannel/PDChannelProc.cpp"
}
namespace pafnuty
{
// EXCLUDED:  #include "3rd_party/airwindows/Pafnuty/Pafnuty.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Pafnuty/PafnutyProc.cpp"
}
namespace pauldither
{
// EXCLUDED:  #include "3rd_party/airwindows/PaulDither/PaulDither.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/PaulDither/PaulDitherProc.cpp"
}
namespace peaksonly
{
// EXCLUDED:  #include "3rd_party/airwindows/PeaksOnly/PeaksOnly.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/PeaksOnly/PeaksOnlyProc.cpp"
}
namespace phasenudge
{
// EXCLUDED:  #include "3rd_party/airwindows/PhaseNudge/PhaseNudge.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/PhaseNudge/PhaseNudgeProc.cpp"
}
namespace pocketverbs
{
// EXCLUDED:  #include "3rd_party/airwindows/PocketVerbs/PocketVerbs.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/PocketVerbs/PocketVerbsProc.cpp"
}
namespace podcast
{
// EXCLUDED:  #include "3rd_party/airwindows/Podcast/Podcast.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Podcast/PodcastProc.cpp"
}
namespace podcastdeluxe
{
// EXCLUDED:  #include "3rd_party/airwindows/PodcastDeluxe/PodcastDeluxe.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/PodcastDeluxe/PodcastDeluxeProc.cpp"
}
namespace point
{
// EXCLUDED:  #include "3rd_party/airwindows/Point/Point.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Point/PointProc.cpp"
}
namespace pop
{
// EXCLUDED:  #include "3rd_party/airwindows/Pop/Pop.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Pop/PopProc.cpp"
}
namespace powersag
{
// EXCLUDED:  #include "3rd_party/airwindows/PowerSag/PowerSag.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/PowerSag/PowerSagProc.cpp"
}
namespace powersag2
{
// EXCLUDED:  #include "3rd_party/airwindows/PowerSag2/PowerSag2.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/PowerSag2/PowerSag2Proc.cpp"
}
namespace pressure4
{
// EXCLUDED:  #include "3rd_party/airwindows/Pressure4/Pressure4.cpp"
// EXCLUDED:  #include "3rd_party/airwindows/Pressure4/Pressure4Proc.cpp"
}
}
} // namespace tracktion::inline engine


#if JUCE_CLANG
 #pragma clang diagnostic pop
#endif

#if JUCE_WINDOWS
 #pragma warning (pop)
#endif

// EXCLUDED: #include "plugins/airwindows/tracktion_AirWindows2.cpp"

#endif
#endif
