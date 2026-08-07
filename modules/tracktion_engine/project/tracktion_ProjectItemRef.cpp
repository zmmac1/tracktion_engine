/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

ProjectItemRef::ProjectItemRef (const yup::String& pathOrID)
    : value (pathOrID)
{
}

ProjectItemRef::ProjectItemRef (const yup::String& pathOrID, Project& owner)
    : value (pathOrID), ownerProject (&owner)
{
}

ProjectItemRef::ProjectItemRef (ProjectItemID pid)
    : value (pid.toString())
{
}

ProjectItemRef ProjectItemRef::fromPath (const yup::String& relativePath)
{
    return ProjectItemRef (relativePath);
}

ProjectItemRef ProjectItemRef::fromPath (const yup::String& relativePath, Project& owner)
{
    return ProjectItemRef (relativePath, owner);
}

ProjectItemRef ProjectItemRef::fromAbsolutePath (const yup::File& file)
{
    return ProjectItemRef (file.getFullPathName());
}

//==============================================================================
bool ProjectItemRef::isValid() const noexcept
{
    return value.isNotEmpty();
}

bool ProjectItemRef::isProjectItemID() const noexcept
{
    return ProjectItemID (value).isValid();
}

bool ProjectItemRef::isRelativePath() const noexcept
{
    return value.isNotEmpty()
        && ! ProjectItemID (value).isValid()
        && ! yup::File::isAbsolutePath (value);
}

bool ProjectItemRef::isAbsolutePath() const noexcept
{
    return value.isNotEmpty()
        && ! ProjectItemID (value).isValid()
        && yup::File::isAbsolutePath (value);
}

ProjectID ProjectItemRef::getProjectID() const noexcept
{
    ProjectItemID pid (value);

    if (pid.isValid())
        return pid.getProjectID();

    if (ownerProject)
        return ownerProject->getProjectID();

    return {};
}

std::optional<ProjectItemID> ProjectItemRef::getProjectItemID() const noexcept
{
    ProjectItemID pid (value);
    return pid.isValid() ? std::optional<ProjectItemID> (pid) : std::nullopt;
}

yup::String ProjectItemRef::toIDForFilename() const noexcept
{
    ProjectItemID pid (value);
    return pid.isValid() ? pid.toStringSuitableForFilename()
                         : yup::String::toHexString (value.hash());
}

yup::String ProjectItemRef::toString() const noexcept
{
    return value;
}

yup::ReferenceCountedObjectPtr<Project> ProjectItemRef::getProject() const noexcept
{
    return ownerProject.get();
}

yup::File ProjectItemRef::resolve (Engine& engine, const yup::File& projectFolder) const
{
    if (value.isEmpty())
        return {};

    ProjectItemID pid (value);

    if (pid.isValid())
    {
        if (auto item = engine.getProjectManager().getProjectItem (pid))
            return item->getSourceFile();

        return {};
    }

    if (yup::File::isAbsolutePath (value))
        return yup::File(value);

    // Relative path — resolve against provided folder or owner project
    if (projectFolder != yup::File())
        return projectFolder.getChildFile (value);

    if (auto p = ownerProject.get())
        return p->getDefaultDirectory().getChildFile (value);

    jassertfalse;
    return {};
}

//==============================================================================
bool ProjectItemRef::operator== (const ProjectItemRef& other) const    { return value == other.value; }
bool ProjectItemRef::operator!= (const ProjectItemRef& other) const    { return value != other.value; }
bool ProjectItemRef::operator<  (const ProjectItemRef& other) const    { return value <  other.value; }

bool ProjectItemRef::operator== (ProjectItemID pid) const              { return value == pid.toString(); }
bool ProjectItemRef::operator!= (ProjectItemID pid) const              { return value != pid.toString(); }

} // namespace tracktion::inline engine
