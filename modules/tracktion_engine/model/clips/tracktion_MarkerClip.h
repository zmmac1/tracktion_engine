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
class MarkerClip   : public Clip
{
public:
    MarkerClip (const yup::ValueTree&, EditItemID, ClipOwner& targetParent);
    ~MarkerClip() override;

    int getMarkerID()                                               { return markerID; }
    void setMarkerID (int newID);

    bool isSyncAbsolute() const noexcept                            { return syncType == syncAbsolute; }
    bool isSyncBarsBeats() const noexcept                           { return syncType == syncBarsBeats; }

    yup::String getSelectableDescription() override;
    bool canBeAddedTo (ClipOwner&) override;
    yup::Color getDefaultColor() const override;
    void initialise() override;

    bool isMidi() const override                                    { return false; }
    bool isMuted() const override                                   { return false; }

protected:
    void valueTreePropertyChanged (yup::ValueTree&, const yup::Identifier&) override;

private:
    juce::CachedValue<int> markerID;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MarkerClip)
};

} // namespace tracktion::inline engine
