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
Project::Project (Engine& e, ProjectManager& pm, const yup::File& fileOrFolder)
   : engine (e), projectManager (pm)
{
    if (fileOrFolder.isDirectory())
        impl = std::make_unique<FolderBasedProject> (*this, fileOrFolder);
    else
        impl = std::make_unique<FileBasedProject> (*this, fileOrFolder);

    for (auto* p : pm.openProjects)
    {
        yup::ignoreUnused (p);
        jassert (p->getProjectFile() != fileOrFolder);
    }

    pm.openProjects.add (this);

    impl->lockFile();
    impl->load();
}

Project::~Project()
{
    cancelPendingUpdate();
    projectManager.openProjects.removeFirstMatchingValue (this);
    save();
    notifyListenersOfDeletion();
}

//==============================================================================
void Project::projectChanged()
{
    triggerAsyncUpdate();
    Selectable::changed();
}

void Project::changed()
{
    impl->changed();
}

void Project::handleAsyncUpdate()
{
    save();
}

void Project::setTemporary (bool t)
{
    impl->temporary = t;
}

//==============================================================================
bool Project::save()
{
    auto result = impl->save();
    cancelPendingUpdate();
    return result;
}

bool Project::isFolderBased() const                                            { return getProjectFile().isDirectory(); }
bool Project::isValid() const                                               { return impl->isValid(); }
bool Project::isReadOnly() const                                            { return impl->isReadOnly(); }
bool Project::isTemporary() const                                           { return impl->isTemporary(); }
ProjectID Project::getProjectID() const                                     { return impl->getProjectID(); }
yup::String Project::getName() const                                       { return impl->getName(); }
yup::String Project::getDescription() const                                   { return getProjectProperty ("description"); }
const yup::File& Project::getProjectFile() const noexcept                  { return impl->getProjectFile(); }
yup::File Project::getDefaultDirectory() const                             { return impl->getDefaultDirectory(); }
yup::File Project::getDirectoryForMedia (ProjectItem::Category c) const    { return impl->getDirectoryForMedia (c); }

yup::String Project::getSourcePathForFile (const yup::File& file) const
{
    auto projectDir = getDefaultDirectory();

    if (file.isAChildOf (projectDir))
        return file.getRelativePathFrom (projectDir).replaceCharacter ('\\', '/');

    return file.getFullPathName();
}

void Project::setName (const yup::String& n)                               { impl->setName (n); }
void Project::setDescription (const yup::String& d)                           { setProjectProperty ("description", yup::String (d).substring (0, 8192)); }
void Project::createNewProjectId()                                          { impl->createNewProjectId(); }

yup::String Project::getProjectProperty (const yup::String& n) const         { return impl->getProjectProperty (n); }
void Project::setProjectProperty (const yup::String& n, const yup::String& v) { impl->setProjectProperty (n, v); }

void Project::refreshProjectPropertiesFromFile()                            { impl->refreshProjectPropertiesFromFile(); }
bool Project::isLibraryProject() const                                      { return impl->isLibraryProject(); }

yup::Array<ProjectItemRef> Project::findOrphanItemRefs()
{
    yup::Array<ProjectItemRef> refs;

    for (auto& item : findOrphanItems())
        refs.add (item->getProjectItemRef());

    return refs;
}

yup::Array<ProjectItem::Ptr> Project::findOrphanItems()
{
    yup::Array<ProjectItem::Ptr> edits, notEdits;

    for (auto& item : getAllProjectItems())
    {
        if (item->isEdit())
            edits.add (item);
        else
            notEdits.add (item);
    }

    for (auto& editPI : edits)
    {
        auto ed = loadEditForExamining (projectManager, editPI->getProjectItemRef());

        for (int i = notEdits.size(); --i >= 0;)
            if (referencesProjectItem (*ed, notEdits.getReference (i)->getProjectItemRef()))
                notEdits.remove (i);
    }

    return notEdits;
}

//==============================================================================
int Project::getNumProjectItems()                                           { return impl->getNumProjectItems(); }
ProjectItemRef Project::getProjectItemRef (int i)                           { return impl->getProjectItemRef (i); }

yup::Array<ProjectItemRef> Project::getAllProjectItemRefs() const             { return impl->getAllProjectItemRefs(); }

ProjectItem::Ptr Project::getProjectItemAt (int i)                          { return impl->getProjectItemAt (i); }
yup::Array<ProjectItem::Ptr> Project::getAllProjectItems()                    { return impl->getAllProjectItems(); }
int Project::getIndexOf (const ProjectItemRef& ref) const                   { return impl->getIndexOf (ref); }

ProjectItem::Ptr Project::getProjectItemFor (const ProjectItemRef& ref)     { return impl->getProjectItemFor (ref); }
ProjectItem::Ptr Project::getProjectItemForFile (const yup::File& f)       { return impl->getProjectItemForFile (f); }

//==============================================================================
ProjectItem::Ptr Project::createNewItem (const yup::File& f, const yup::String& type,
                                         const yup::String& name, const yup::String& desc,
                                         const ProjectItem::Category cat, bool atTop)
{
    return impl->createNewItem (f, type, name, desc, cat, atTop);
}

bool Project::removeProjectItem (const ProjectItemRef& ref, bool del)       { return impl->removeProjectItem (ref, del); }
void Project::moveProjectItem (int from, int to)                            { impl->moveProjectItem (from, to); }
ProjectItem::Ptr Project::createNewEdit()                                   { return impl->createNewEdit(); }
void Project::redirectIDsFromProject (ProjectID oldId, ProjectID newId)     { impl->redirectIDsFromProject (oldId, newId); }

//==============================================================================
void Project::mergeArchiveContents (const yup::File& f)                    { impl->mergeArchiveContents (f); }
void Project::mergeOtherProjectIntoThis (const yup::File& f)               { impl->mergeOtherProjectIntoThis (f); }
void Project::refreshFolderStructure()                                      { impl->refreshFolderStructure(); }
void Project::createDefaultFolders()                                        { impl->createDefaultFolders(); }

//==============================================================================
yup::String Project::getSelectableDescription()                            { return impl->getSelectableDescription(); }

void Project::lockFile()                                                    { impl->lockFile(); }
void Project::unlockFile()                                                  { impl->unlockFile(); }
void Project::sourceFileMoved (const yup::File& o, const yup::File& n)    { impl->sourceFileMoved (o, n); }
void Project::reload (ReloadMode mode)                                      { impl->reload (mode); }
void Project::setNewProjectId (ProjectID newID)                             { impl->setNewProjectId (newID); }

//==============================================================================
Project::Ptr convertToFolderBasedProject (Project& project)
{
    // Only convert valid file-based projects
    if (! project.getProjectID().isValid() || ! project.getProjectFile().existsAsFile())
        return nullptr;

    auto projectDir = project.getDefaultDirectory();
    projectDir.createDirectory();

    // Write project metadata to project_info.json
    {
        auto info = std::make_unique<yup::DynamicObject>();
        info->setProperty ("name", project.getName());
        info->setProperty ("description", project.getDescription());
        info->setProperty ("projectId", project.getProjectID().toInt());

        auto jsonString = yup::JSON::toString (yup::var (info.release()));
        projectDir.getChildFile ("project_info.json").replaceWithText (jsonString);
    }

    // Convert edit source references from ProjectItemIDs to direct file paths
    for (int i = 0; i < project.getNumProjectItems(); ++i)
    {
        if (auto item = project.getProjectItemAt (i))
        {
            if (item->isEdit())
            {
                auto editFile = item->getSourceFile();
                auto edit = loadEditFromFile (project.engine, editFile, Edit::forExamining);

                if (edit != nullptr)
                {
                    for (auto exportable : Exportable::addAllExportables (*edit))
                    {
                        for (auto& ref : exportable->getReferencedItems())
                        {
                            if (ref.itemRef.isValid())
                            {
                                if (auto projItem = project.getProjectItemFor (ref.itemRef))
                                    exportable->reassignReferencedItem (ref, ProjectItemRef::fromAbsolutePath (projItem->getSourceFile()), 0.0);
                            }
                        }
                    }

                    EditFileOperations (*edit).save (false, true, false);
                }
            }
        }
    }

    // Delete the project file — save() sets hasChanged=false so any
    // pending async update will be a no-op after this point.
    project.save();
    project.unlockFile();
    project.getProjectFile().deleteFile();

    // Return a new folder-based project
    return project.projectManager.createNewProject (projectDir);
}

} // namespace tracktion::inline engine
