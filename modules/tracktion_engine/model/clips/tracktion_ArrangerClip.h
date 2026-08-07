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
class ArrangerClip  : public Clip
{
public:
    ArrangerClip (const yup::ValueTree&, EditItemID, ClipOwner& targetParent);
    ~ArrangerClip() override;

    using Ptr = yup::ReferenceCountedObjectPtr<ArrangerClip>;

    yup::String getSelectableDescription() override;
    bool canBeAddedTo (ClipOwner&) override;
    yup::Color getDefaultColor() const override;
    void initialise() override;

    bool isMidi() const override                                    { return false; }
    bool isMuted() const override                                   { return false; }

protected:
    void valueTreePropertyChanged (yup::ValueTree&, const yup::Identifier&) override;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ArrangerClip)
};

} // namespace tracktion::inline engine
