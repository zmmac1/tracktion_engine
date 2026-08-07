/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

/** */
class AutomationTrack  : public Track
{
public:
    AutomationTrack (Edit&, const yup::ValueTree&);
    ~AutomationTrack() override;

    //==============================================================================
    bool isAutomationTrack() const override            { return true; }
    yup::String getSelectableDescription() override;
    bool canContainPlugin (Plugin*) const override     { return false; }
    yup::String getName() const override;

    //==============================================================================
    using Ptr = yup::ReferenceCountedObjectPtr<AutomationTrack>;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AutomationTrack)
};

} // namespace tracktion::inline engine
