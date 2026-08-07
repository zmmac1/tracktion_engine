/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

ProjectItemID::ProjectItemID (const yup::String& asString) noexcept
{
    int mid = 0;
    int pid = 0;
    int* number = &pid;

    for (auto p = asString.getCharPointer();; ++p)
    {
        auto c = *p;
        auto hex = yup::CharacterFunctions::getHexDigitValue (c);

        if (c == 0)
        {
            break; // End of string
        }
        else if (hex >= 0)
        {
            *number = ((*number) << 4) | hex;
        }
        else if ((c == '/' || c == '_') && number == &pid)
        {
            number = &mid;
        }
        else
        {
            // Invalid character encountered
            mid = 0;
            pid = 0;
            break;
        }
    }

    *this = ProjectItemID (mid, ProjectID (pid));
}

ProjectItemID::ProjectItemID (int itemID, ProjectID projectID) noexcept
   : combinedID ((((int64_t) projectID.toInt()) << 32) | itemID)
{
}

ProjectID ProjectItemID::getProjectID() const   { return ProjectID ((int) (combinedID >> 32)); }
int ProjectItemID::getItemID() const            { return (int) combinedID; }

yup::String ProjectItemID::toString() const
{
    return yup::String::toHexString (getProjectID().toInt()) + '/' + yup::String::toHexString (getItemID());
}

yup::String ProjectItemID::toStringSuitableForFilename() const
{
    return yup::String::toHexString (getProjectID().toInt()) + '_' + yup::String::toHexString (getItemID());
}

ProjectItemID ProjectItemID::createNewID (ProjectID projectID) noexcept
{
    return ProjectItemID (yup::Random::getSystemRandom().nextInt (0x3ffffff), projectID);
}

ProjectItemID ProjectItemID::fromProperty (const yup::ValueTree& v, const yup::Identifier& prop)
{
    return ProjectItemID (v.getProperty (prop).toString());
}

ProjectItemID ProjectItemID::withNewProjectID (ProjectID newProjectID) const
{
    return { getItemID(), newProjectID };
}

} // namespace tracktion::inline engine
