/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

SourceFileReference::SourceFileReference (Edit& e, yup::ValueTree& v, const yup::Identifier& prop)
    : edit (e), source (v, prop, &e.getUndoManager()), state (v)
{
    ignoreUnused (state);
}

SourceFileReference::~SourceFileReference()
{
}

namespace {

yup::File findCommonAncestor (const yup::File& a, const yup::File& b)
{
    for (auto dir = a; dir != dir.getParentDirectory(); dir = dir.getParentDirectory())
        if (b.isAChildOf (dir) || b == dir)
            return dir;

    for (auto dir = b.getParentDirectory(); dir != dir.getParentDirectory(); dir = dir.getParentDirectory())
        if (a.isAChildOf (dir) || a == dir)
            return dir;

    return {};
}

int getPathDepth (const yup::File& f)
{
    int depth = 0;

    if (f != yup::File())
        for (auto dir = f; dir != dir.getParentDirectory(); dir = dir.getParentDirectory())
            ++depth;

    return depth;
}

bool isSystemOrTempFolder (const yup::File& f)
{
    auto path = f.getFullPathName();

   #if YUP_MAC
    if (path.startsWith ("/System") || path.startsWith ("/Library")
        || path.startsWith ("/usr") || path.startsWith ("/var")
        || path.startsWith ("/private/var"))
        return true;
   #elif JUCE_WINDOWS
    auto winDir = yup::File::getSpecialLocation (yup::File::windowsSystemDirectory)
                      .getParentDirectory().getFullPathName();
    if (path.startsWithIgnoreCase (winDir))
        return true;

    auto progFiles = yup::File::getSpecialLocation (yup::File::globalApplicationsDirectory).getFullPathName();
    if (path.startsWithIgnoreCase (progFiles))
        return true;
   #elif JUCE_LINUX
    if (path.startsWith ("/usr") || path.startsWith ("/var")
        || path.startsWith ("/etc") || path.startsWith ("/sys")
        || path.startsWith ("/proc"))
        return true;
   #endif

    auto tempDir = yup::File::getSpecialLocation (yup::File::tempDirectory);
    if (f.isAChildOf (tempDir))
        return true;

    return false;
}

bool shouldUseRelativePath (Edit& edit, const yup::File& audioFile, const yup::File& editFile)
{
    if (editFile == yup::File())
        return false;

   #if JUCE_WINDOWS
    // Different drive letters -> absolute
    if (editFile.getFullPathName()[0] != audioFile.getFullPathName()[0])
        return false;
   #endif

    // "Always relative" mode: skip heuristics
    if (edit.alwaysUseRelativePaths)
        return true;

    // Files inside the project folder -> always relative
    if (auto proj = getProjectForEdit (edit))
    {
        auto projectDir = proj->getDefaultDirectory();

        if (audioFile.isAChildOf (projectDir))
            return true;
    }

    // One or other file is in the system folders -> absolute
    if (isSystemOrTempFolder (audioFile) != isSystemOrTempFolder (editFile))
        return false;

    // Common ancestor depth check
    auto commonAncestor = findCommonAncestor (editFile.getParentDirectory(), audioFile);
    auto ancestorDepth = getPathDepth (commonAncestor);

    // Deep common ancestor (e.g. /Users/foo/project) -> same workspace, use relative
    if (ancestorDepth > 2)
        return true;

    // Shallow common ancestor (e.g. /tmp) but both files are close to it ->
    // the relative path is simple and stable, so allow it.
    // e.g. /tmp/myproject/audio/x.wav and /tmp/myproject/edits/y.edit -> ../audio/x.wav
    if (ancestorDepth > 0)
    {
        auto audioDepth = getPathDepth (audioFile) - ancestorDepth;
        auto editDepth  = getPathDepth (editFile.getParentDirectory()) - ancestorDepth;

        if (audioDepth <= 3 && editDepth <= 3)
            return true;
    }

    return false;

}

} // anonymous namespace

yup::String SourceFileReference::findPathFromFile (Edit& edit, const yup::File& newFile, bool useRelativePath)
{
    if (! useRelativePath)
        return newFile.getFullPathName();

    auto editFile = edit.editFileRetriever ? edit.editFileRetriever()
                                           : getEditFileFromProjectManager (edit);

    if (shouldUseRelativePath (edit, newFile, editFile))
        return newFile.getRelativePathFrom (editFile);

    return newFile.getFullPathName();
}

yup::File SourceFileReference::findFileFromString (Edit& edit, const yup::String& sourceDescription)
{
    if (sourceDescription.isEmpty())
        return {};

    ProjectItemRef ref (sourceDescription);

    if (ref.isProjectItemID())
    {
        if (auto projectItem = edit.engine.getProjectManager().getProjectItem (ref))
            return projectItem->getSourceFile();

        return {};
    }

    if (edit.filePathResolver)
        return edit.filePathResolver (sourceDescription);

    return getEditFileFromProjectManager (edit).getChildFile (sourceDescription);
}

yup::File SourceFileReference::getFile() const
{
    jassert (source.get() == state[source.getPropertyID()].toString());
    return findFileFromString (edit, source.get());
}

bool SourceFileReference::isUsingProjectReference() const
{
    return getSourceProjectItemRef().isProjectItemID();
}

ProjectItemRef SourceFileReference::getSourceProjectItemRef() const
{
    jassert (source.get() == state[source.getPropertyID()].toString());

    if (auto pid = ProjectItemID (source.get()); pid.isValid())
        return pid;

    if (auto project = getProjectForEdit (edit))
        return ProjectItemRef (source.get(), *project);

    return {};
}

ProjectItem::Ptr SourceFileReference::getSourceProjectItem() const
{
    jassert (source.get() == state[source.getPropertyID()].toString());
    auto ref = getSourceProjectItemRef();

    if (ref.isValid())
        return edit.engine.getProjectManager().getProjectItem (ref);

    return {};
}

void SourceFileReference::setToFile (const yup::File& file, PathStyle pathStyle, bool allowProjectItems)
{
    auto project = getProjectForEdit (edit);

    // Store as a direct file path if project items aren't allowed,
    // or if there's no file-based project
    if (! allowProjectItems || project == nullptr || project->isFolderBased())
    {
        if (pathStyle == PathStyle::alwaysAbsolute)
            source = file.getFullPathName();
        else
            source = findPathFromFile (edit, file, true);

        return;
    }

    // File-based project: find or create a ProjectItem
    auto oldFile = getFile();

    if (auto projectItem = getSourceProjectItem())
    {
        if (! projectItem->getSourceFile().existsAsFile())
        {
            projectItem->setSourceFile (file);
        }
        else
        {
            if (auto existingItem = project->getProjectItemForFile (file))
            {
                setToProjectFileReference (existingItem->getProjectItemRef());
            }
            else
            {
                projectItem = project->createNewItem (file, ProjectItem::waveItemType(),
                                                      file.getFileNameWithoutExtension(),
                                                      {}, ProjectItem::Category::imported, false);

                if (projectItem != nullptr)
                    setToProjectFileReference (projectItem->getProjectItemRef());
            }
        }
    }
    else
    {
        auto newItem = project->createNewItem (file, ProjectItem::waveItemType(),
                                               file.getFileNameWithoutExtension(),
                                               {}, ProjectItem::Category::imported, false);

        if (newItem != nullptr)
            setToProjectFileReference (newItem->getProjectItemRef());
    }

    if (getFile() != oldFile)
        edit.restartPlayback();
}

void SourceFileReference::setToProjectFileReference (ProjectItemRef newID)
{
    auto oldFile = getFile();
    source = newID.toString();

    if (getFile() != oldFile)
        edit.restartPlayback();
}

} // namespace tracktion::inline engine
