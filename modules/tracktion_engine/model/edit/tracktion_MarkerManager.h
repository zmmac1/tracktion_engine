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
class MarkerManager   : public yup::ChangeBroadcaster,
                        private yup::ValueTree::Listener
{
public:
    MarkerManager (Edit&, const yup::ValueTree&);
    ~MarkerManager() override;

    int getNextUniqueID (int start = 1);
    void checkForDuplicates (MarkerClip&, bool changeOthers);

    /** Creates a MarkerClip with using getNewMarkerMode(). */
    MarkerClip::Ptr createMarker (int number, TimePosition, TimeDuration, SelectionManager*);
    MarkerClip::Ptr createMarker (int number, TimePosition, TimeDuration, Clip::SyncType, SelectionManager*);

    Clip::SyncType getNewMarkerMode() const;

    MarkerClip* getMarkerByID (int);
    MarkerClip* getNextMarker (TimePosition);
    MarkerClip* getPrevMarker (TimePosition);

    yup::ReferenceCountedArray<MarkerClip> getMarkers() const;

private:
    Edit& edit;
    yup::ValueTree state;

    void valueTreeChanged (const yup::ValueTree&);

    void valueTreePropertyChanged (yup::ValueTree&, const yup::Identifier&) override;
    void valueTreeChildAdded (yup::ValueTree&, yup::ValueTree&) override;
    void valueTreeChildRemoved (yup::ValueTree&, yup::ValueTree&, int) override;
    void valueTreeChildOrderChanged (yup::ValueTree&, int, int) override {}
    void valueTreeParentChanged (yup::ValueTree&) override;
    void valueTreeRedirected (yup::ValueTree&) override {}
};

} // namespace tracktion::inline engine
