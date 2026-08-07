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
    A project backend that represents a folder on disk.
    Files in the folder are discovered as ProjectItems.
*/
class FolderBasedProject  : public ProjectBase
{
public:
    FolderBasedProject (Project& owner, const yup::File& folder);
    ~FolderBasedProject() override = default;

    //==============================================================================
    bool save() override;
    bool isValid() const override;
    bool isReadOnly() const override;
    ProjectID getProjectID() const override;
    yup::String getName() const override;
    const yup::File& getProjectFile() const noexcept override;
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

    void changed() override;
    void sourceFileMoved (const yup::File& oldFile, const yup::File& newFile) override;

    /** Clears the cached items so the folder is rescanned on next access.
        Pass ReloadMode::immediate to rescan straight away. */
    void reload (ReloadMode = ReloadMode::lazy) override;

private:
    yup::File folder;
    mutable yup::Array<ProjectItem::Ptr> cachedItems;
    mutable yup::CriticalSection itemLock;
    mutable bool itemsScanned = false;

    yup::NamedValueSet properties;
    yup::CriticalSection propertyLock;

    yup::File getInfoFile() const;
    void loadPropertiesFromFile();
    bool savePropertiesToFile();

    void ensureScanned() const;
    void scanFolder() const;
    static yup::String inferType (const yup::File&);
    static ProjectItem::Category inferCategory (const yup::File&, const yup::File& root);

    // Per-item descriptions are persisted in project_info.json, keyed by the
    // item's path, because folder-based ProjectItems are recreated on every scan
    // and so have nowhere else to store their description.
    yup::String getItemDescriptionKey (const yup::File&) const;
    void syncItemDescriptionsToProperties();

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FolderBasedProject)
};

} // namespace tracktion::inline engine
