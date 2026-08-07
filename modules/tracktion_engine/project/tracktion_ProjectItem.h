/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

/**
    Represents a file-based resource that is used in a project.
    This might be an edit, audio file, midi file, etc.
*/
class ProjectItem   : public Selectable,
                      public yup::ReferenceCountedObject,
                      private yup::Timer,
                      private yup::AsyncUpdater
{
public:
    //==============================================================================
    enum class Category
    {
        none,
        edit,
        recorded,
        exports,
        archives,
        imported,
        rendered,
        frozen,
        video
    };

    enum class RenameMode
    {
        always,
        local,
        never
    };

    enum class SetNameMode
    {
        doDefault,
        doDefaultSynchronous,
        forceRenameSynchronous,
        forceNoRename
    };

    using Ptr = yup::ReferenceCountedObjectPtr<ProjectItem>;
    using Array = yup::ReferenceCountedArray<ProjectItem>;

    //==============================================================================
    /** Loads a ProjectItem from a stream that was saved using writeToStream(). */
    ProjectItem (Engine&, ProjectItemRef, yup::InputStream*);

    //==============================================================================
    /** Creates a ProjectItem with some settings. */
    ProjectItem (Engine&,
                 const yup::String& name,
                 const yup::String& type,
                 const yup::String& desc,
                 const yup::String& file,
                 Category,
                 double length,
                 ProjectItemRef);

    /** Creates a folder-backed ProjectItem with an absolute source file path. */
    ProjectItem (Engine&, const yup::File& sourceFile,
                 const yup::String& type, const yup::String& name,
                 Category, Project& ownerProject);

    ~ProjectItem() override;

    //==============================================================================
    yup::String getSelectableDescription() override;
    void selectionStatusChanged (bool isNowSelected) override;

    //==============================================================================
    /** Returns the ProjectItemRef that identifies this item. */
    const ProjectItemRef& getProjectItemRef() const noexcept    { return itemRef; }

    yup::ReferenceCountedObjectPtr<Project> getProject() const;

    // checks whether it's been removed from its project
    bool hasBeenDeleted() const;

    static const char* waveItemType() noexcept      { return "wave"; }
    static const char* editItemType() noexcept      { return "edit"; }
    static const char* midiItemType() noexcept      { return "midi"; }
    static const char* videoItemType() noexcept     { return "video"; }

    const yup::String& getType() const;

    bool isWave() const noexcept                    { return getType() == waveItemType(); }
    bool isMidi() const noexcept                    { return getType() == midiItemType(); }
    bool isEdit() const noexcept                    { return getType() == editItemType(); }
    bool isVideo() const noexcept                   { return getType() == videoItemType(); }

    const yup::String& getName() const;
    void setName (const yup::String& name, SetNameMode);

    Category getCategory() const;
    void setCategory (Category cat);

    // get/set a persistent string property with a name. Setting to empty removes it
    yup::String getNamedProperty (const yup::String& name) const;
    void setNamedProperty (const yup::String& name, const yup::String& value);

    yup::String getDescription() const;

    /** Sets the user-visible description text.
        Pass yup::dontSendNotification to update it without broadcasting a change
        - used when restoring a persisted description during a folder scan, where
        firing a change would propagate to the owning project and retrigger the
        scan. */
    void setDescription (const yup::String& newDesc,
                         yup::NotificationType = yup::sendNotification);

    /** optional set of interesting time markers, for wave files */
    yup::Array<TimePosition> getMarkedPoints() const;
    void setMarkedPoints (const yup::Array<TimePosition>& points);

    /** copies the full description, categories, properties, etc. */
    void copyAllPropertiesFrom (const ProjectItem& other);

    void convertEditFile (std::function<void (bool)> callback);

    double getLength() const;
    void setLength (double length);

    /** name of the project it's inside. */
    yup::String getProjectName() const;

    //==============================================================================
    yup::File getSourceFile();
    yup::String getRawFileName() const       { return file; }
    yup::String getFileName() const;

    enum class FileMode
    {
        absolute,
        relative,
        relativeIfWithinProject
    };

    void setSourceFile (const yup::File&, FileMode mode = FileMode::relativeIfWithinProject);
    bool isForFile (const yup::File&);
    bool isAbsolutePath() const;
    void convertToRelativePath();
    void convertToAbsolutePath();

    /** lets the user rename the file. */
    void renameSourceFile();

    /** Creates a copy of this object and returns the copy. */
    Ptr createCopy();

    void sendChange();

    //==============================================================================
    /** the actual file created may differ, e.g. if an ogg file is chopped up and
        has to become a wav.
    */
    bool copySectionToNewFile (const yup::File& destFile,
                               yup::File& actualFileCreated,
                               double startTime,
                               double length);

    bool deleteSourceFile();

    /** Updates the stored length value in this object.
        Pass yup::dontSendNotification to update it without broadcasting a change
        - a change here propagates to the owning project, which makes project list
        UIs rebuild themselves. */
    void verifyLength (yup::NotificationType = yup::sendNotification);

    /** used when moving to another project. */
    void changeProjectId (ProjectID oldID, ProjectID newID);

    //==============================================================================
    void writeToStream (yup::OutputStream&) const;

    /** Returns a list of search strings for this object, by chopping up the
        name and description into words.
    */
    yup::StringArray getSearchTokens() const;

    Engine& engine; /**< The Engine instance this belongs to. */

private:
    friend class FileBasedProject;
    friend class FolderBasedProject;
    friend class ProjectManager;
    friend class Project;

    ProjectItemRef itemRef;
    yup::String type, objectName, description, file;
    double length = 0;
    yup::File sourceFile, newDstFile;
    SafeSelectable<Project> ownerProject;

    void timerCallback() override;
    void handleAsyncUpdate() override;

    yup::File getRelativeFile (const yup::String&) const;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectItem)
};

} // namespace tracktion::inline engine
