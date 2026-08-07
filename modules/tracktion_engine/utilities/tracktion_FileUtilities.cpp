/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

yup::File getNonExistentSiblingWithIncrementedNumberSuffix (const yup::File& file, bool addHashSymbol)
{
    if (! file.existsAsFile())
        return file;

    auto name = yup::File::createLegalFileName (file.getFileNameWithoutExtension());

    int drop = 0;
    bool foundDigit = false;
    bool foundHash  = ! addHashSymbol;

    for (int i = name.length(); --i >= 0;)
    {
        if (name[i] == '#' && foundDigit)
            foundHash = true;
        else if (yup::String ("0123456789").containsChar (name[i]))
            foundDigit = true;
        else
            break;

        drop++;
    }

    bool inForm = foundDigit && foundHash;
    int number = inForm ? name.getTrailingIntValue() : 0;
    name = inForm ? name.dropLastCharacters(drop).trimEnd() : name;

    yup::File newFileName;

    do
    {
        number++;

        auto nameWithNumber = name + (addHashSymbol ? " #" : " ")
                                + yup::String (number) + file.getFileExtension();

        newFileName = file.getParentDirectory()
                          .getChildFile (nameWithNumber);

    } while (newFileName.exists());

    return newFileName;
}

//==============================================================================
bool isMidiFile (const yup::File& f)             { return f.hasFileExtension ("mid;rmi;rmid;midi"); }
bool isTracktionEditFile (const yup::File& f)    { return f.hasFileExtension (editFileSuffix) || f.hasFileExtension (legacyEditFileSuffix); }
bool isTracktionArchiveFile (const yup::File& f) { return f.hasFileExtension (archiveFileSuffix_v2) || f.hasFileExtension (legacyArchiveFileSuffix); }
bool isTracktionProjectFile (const yup::File& f) { return f.hasFileExtension (projectFileSuffix); }
bool isTracktionPresetFile (const yup::File& f)  { return f.hasFileExtension (presetFileSuffix); }

bool isTracktionProjectFolder (const yup::File& f)
{
    if (! f.isDirectory())
        return false;

    if (f.getChildFile ("project_info.json").existsAsFile())
        return true;

    for (auto& child : f.findChildFiles (yup::File::findFiles, false))
        if (isTracktionEditFile (child))
            return true;

    return false;
}

bool isTracktionProjectFileOrFolder (const yup::File& f)
{
    return isTracktionProjectFile (f) || isTracktionProjectFolder (f);
}

//==============================================================================
FileDragList::Ptr FileDragList::getFromDrag (const yup::DragAndDropTarget::SourceDetails& s)
{
    return dynamic_cast<FileDragList*> (s.description.getObject());
}

yup::var FileDragList::create (const yup::Array<yup::File>& files, PreferredLayout preferredLayout)
{
    FileDragList* l = new FileDragList();
    l->preferredLayout = preferredLayout;
    l->files = files;
    return l;
}

yup::var FileDragList::create (const yup::File& file, PreferredLayout peferredLayout)
{
    yup::Array<yup::File> files;
    files.add (file);
    return create (files, peferredLayout);
}

} // namespace tracktion::inline engine
