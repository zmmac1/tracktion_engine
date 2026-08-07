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
    Contains the implementation logic for a file-based (.tracktion) project.
    Project delegates to this class via the pimpl pattern.
*/
class FileBasedProject  : public ProjectBase
{
public:
    FileBasedProject (Project& owner, const yup::File& projectFile);
    ~FileBasedProject() override = default;

    //==============================================================================
    bool save() override;
    bool isValid() const override;
    bool isReadOnly() const override;
    ProjectID getProjectID() const override;
    yup::String getName() const override;
    const yup::File& getProjectFile() const noexcept override          { return file; }
    yup::File getDefaultDirectory() const override;
    yup::File getDirectoryForMedia (ProjectItem::Category) const override;

    void setName (const yup::String&) override;
    void createNewProjectId() override;
    void setNewProjectId (ProjectID) override;

    yup::String getProjectProperty (const yup::String&) const override;
    void setProjectProperty (const yup::String&, const yup::String&) override;

    void refreshProjectPropertiesFromFile() override;
    bool isLibraryProject() const override;

    //==============================================================================
    int getNumProjectItems() override;
    ProjectItemRef getProjectItemRef (int index) override;
    yup::Array<ProjectItemRef> getAllProjectItemRefs() const override;
    ProjectItem::Ptr getProjectItemAt (int index) override;
    yup::Array<ProjectItem::Ptr> getAllProjectItems() override;
    int getIndexOf (const ProjectItemRef&) const override;

    ProjectItem::Ptr getProjectItemFor (const ProjectItemRef&) override;
    ProjectItem::Ptr getProjectItemForFile (const yup::File&) override;

    //==============================================================================
    ProjectItem::Ptr createNewItem (const yup::File&, const yup::String& type,
                                    const yup::String& name, const yup::String& description,
                                    ProjectItem::Category, bool atTopOfList) override;
    bool removeProjectItem (const ProjectItemRef&, bool deleteSourceMaterial) override;
    void moveProjectItem (int from, int to) override;
    ProjectItem::Ptr createNewEdit() override;
    void redirectIDsFromProject (ProjectID oldProjId, ProjectID newProjId) override;

    //==============================================================================
    void mergeArchiveContents (const yup::File&) override;
    void mergeOtherProjectIntoThis (const yup::File&) override;
    void refreshFolderStructure() override;
    void createDefaultFolders() override;

    //==============================================================================
    yup::String getSelectableDescription() const override;

    void load() override;
    void changed() override;
    void lockFile() override;
    void unlockFile() override;
    void sourceFileMoved (const yup::File&, const yup::File&) override {}

    //==============================================================================
    static ProjectID generateNewProjectId (ProjectManager&);

private:
    friend class Project;

    yup::File file;
    ProjectID projectId;

    yup::NamedValueSet properties;
    yup::CriticalSection objectLock, propertyLock;

    std::unique_ptr<yup::BufferedInputStream> stream;
    std::unique_ptr<yup::FileInputStream> fileLockingStream;

    struct ObjectInfo
    {
        int itemID = 0, fileOffset = 0;
        ProjectItem::Ptr item;
    };

    yup::Array<ObjectInfo> objects;
    int objectOffset = 0, indexOffset = 0;
    std::atomic<bool> readOnly { false }, hasChanged { false };

    void saveTo (yup::FileOutputStream&);
    bool readProjectHeader (yup::InputStream&, bool clearObjectInfo = true);
    yup::BufferedInputStream* getInputStream();
    void loadAllProjectItems();
    bool loadProjectItem (ObjectInfo&);
    void ensureFolderCreated (ProjectItem::Category);

    ProjectItem::Ptr quickAddProjectItem (const yup::String& relPathName,
                                          const yup::String& type,
                                          const yup::String& name,
                                          const yup::String& description,
                                          ProjectItem::Category,
                                          ProjectItemID);

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileBasedProject)
};

} // namespace tracktion::inline engine
