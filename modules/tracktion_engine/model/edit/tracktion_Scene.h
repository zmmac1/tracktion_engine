/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

class SceneList;

//==============================================================================
/**
    A Scene is a collection of ClipSlots across tracks.
    This class provides an interface to interact with them all rather than having
    to iterate through each Track's SlotList finding the relevant ClipSlot index.
*/
class Scene : public Selectable,
              private yup::ValueTree::Listener
{
public:
    /** Creates a Scene for a given state. */
    Scene (const yup::ValueTree&, SceneList&);

    /** Destructor. */
    ~Scene() override;

    int getIndex();

    yup::ValueTree state;
    Edit& edit;

    juce::CachedValue<yup::String> name;
    juce::CachedValue<yup::Color> colour;

    SceneList& sceneList;

    /** @internal */
    yup::String getSelectableDescription() override;

private:
    void valueTreePropertyChanged (yup::ValueTree&, const yup::Identifier&) override;
};

//==============================================================================
//==============================================================================
/**
    Notifies UI components of changes to the scenes and clips
*/
class SceneWatcher : private yup::ValueTree::Listener,
                     private yup::Timer
{
public:
    /** Creates a SceneWatcher.
        You don't need to create one of these manually, use the instance in SceneList
        @see SceneList::sceneWatcher
    */
    SceneWatcher (const yup::ValueTree& scenesState, Edit&);

    /** A listener interface. */
    struct Listener
    {
        /** Destructor. */
        virtual ~Listener() = default;

        /** Destructor. */
        virtual void slotUpdated (int audioTrackIndex, int slotIndex) = 0;
    };

    /** Adds a Listener. */
    void addListener (Listener*);

    /** Removes a previously added Listener. */
    void removeListener (Listener*);

private:
    yup::ValueTree scenesState;
    Edit& edit;
    uint32_t callback = 0;

    enum class RecordingState
    {
        none,
        pending,
        recording,
    };

    struct Value
    {
        uint32_t lastSeen = 0;
        LaunchHandle::PlayState playState;
        std::optional<LaunchHandle::QueueState> queueState;
        RecordingState recordingState = RecordingState::none;
    };

    std::map<std::pair<int, int>, Value> lastStates;

    yup::ListenerList<Listener> listeners;

    void checkForScenes();
    RecordingState getRecordingState (ClipSlot& slot);

    void timerCallback() override;

    void valueTreeChildAdded (yup::ValueTree&, yup::ValueTree&) override;
    void valueTreeChildRemoved (yup::ValueTree&, yup::ValueTree&, int) override;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SceneWatcher)
};

//==============================================================================
//==============================================================================
/**
    Represents the Scenes in an Edit.
*/
class SceneList : private ValueTreeObjectList<Scene>
{
public:
    /** Creates a SceneList for an Edit with a given state. */
    SceneList (const yup::ValueTree&, Edit&);

    /** Destructor. */
    ~SceneList() override;

    /** Returns the number of Scenes in the Edit. */
    int getNumScenes() { return size(); }

    /** Returns the Scenes in the Edit. */
    yup::Array<Scene*> getScenes();

    //==============================================================================
    /** Adds Scenes to ensure numScenes are preset in the list.
        N.B. This also adds ClipSlots to all tracks to ensure they have the same
        number of slots.
    */
    void ensureNumberOfScenes (int numScenes);

    /** Inserts a new scene with the given index. If the index is < 0 or greater
        than the current number of scenes, the new one will be added at the end of the list
     */
    Scene* insertScene (int index);

    /** Deletes a specific Scene. */
    void deleteScene (Scene&);

    yup::ValueTree state;  /**< The state of this SceneList. */
    Edit& edit;             /**< The Edit this SceneList belongs to. */

    /** A SceneWatcher you can listen to in order to be notified of changes to Slots. */
    SceneWatcher sceneWatcher { state, edit };

private:
    //==============================================================================
    /** @internal */
    bool isSuitableType (const yup::ValueTree&) const override;
    /** @internal */
    Scene* createNewObject (const yup::ValueTree&) override;
    /** @internal */
    void deleteObject (Scene*) override;

    /** @internal */
    void newObjectAdded (Scene*) override;
    /** @internal */
    void objectRemoved (Scene*) override;
    /** @internal */
    void objectOrderChanged() override;
};

} // namespace tracktion::inline engine
