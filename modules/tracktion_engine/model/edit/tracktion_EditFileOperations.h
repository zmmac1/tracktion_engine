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
    Contains methods for saving an Edit to a file.
*/
class EditFileOperations
{
public:
    EditFileOperations (Edit&);
    ~EditFileOperations();

    yup::File getEditFile() const;

    void save (bool warnOfFailure, bool forceSaveEvenIfNotModified, bool offerToDiscardChanges,
               std::function<void (bool)> callback = nullptr);

    void saveAs (const yup::File&, bool forceOverwriteExisting, std::function<void (bool)> callback);
    void saveAs (const yup::File& f)   { saveAs (f, false, nullptr); }
    void saveAs();

    bool writeToFile (const yup::File&, bool writeQuickBinaryVersion);

    bool saveTempVersion (bool forceSaveEvenIfUnchanged);
    void deleteTempVersion();
    yup::File getTempVersionFile() const;
    static yup::File getTempVersionOfEditFile (const yup::File&);
    static void updateEditFiles();

    yup::Time getTimeOfLastSave() const    { return timeOfLastSave; }

private:
    Edit& edit;
    yup::ValueTree state;

    struct SharedDataPimpl;
    std::unique_ptr<SharedDataPimpl> sharedDataPimpl;

    yup::Time& timeOfLastSave;
    EditSnapshot::Ptr& editSnapshot;
};

//==============================================================================
/** Loads an edit from a file, ready for playback / editing */
std::unique_ptr<Edit> loadEditFromFile (Engine&, const yup::File&,
                                        Edit::EditRole role = Edit::EditRole::forEditing,
                                        Edit::LoadContext* = nullptr);

/** Loads an edit from a ValueTree, ready for playback / editing */
std::unique_ptr<Edit> loadEditFromState (Engine&, const yup::ValueTree&,
                                         Edit::EditRole role = Edit::EditRole::forEditing,
                                         Edit::LoadContext* = nullptr);

/** Creates a new edit for a file, ready for playback / editing */
std::unique_ptr<Edit> createEmptyEdit (Engine&, const yup::File&);

/** Uses the ProjectManager to find an Edit file and load it as a ValueTree. */
yup::ValueTree loadEditFromProjectManager (ProjectManager&, ProjectItemRef);

/** Uses the ProjectManager to find an Edit file and open it.
    If the ref is ID-based, looks up via ProjectManager.
    If the ref is path-based with an owner project, resolves the file directly.
*/
std::unique_ptr<Edit> loadEditForExamining (ProjectManager&, ProjectItemRef,
                                            Edit::EditRole role = Edit::EditRole::forExamining,
                                            Edit::LoadContext* = nullptr);

/** Legacy, will be deprecated soon. Use version that returns an edit.
    Loads a ValueTree from a file to load an Edit.
    If the file is empty, a new Edit state will be created with the given ProjectItemID.
*/
yup::ValueTree loadEditFromFile (Engine&, const yup::File&, ProjectItemRef);

/** Legacy, will be deprecated soon. Use version that returns an edit.
    Creates an empty Edit with no project. */
yup::ValueTree createEmptyEdit (Engine&);

/** Converts old edit formats to the latest structure */
yup::ValueTree updateLegacyEdit (const yup::ValueTree&);

/** Converts old edit formats to the latest structure */
void updateLegacyEdit (yup::XmlElement& editXML);


} // namespace tracktion::inline engine
