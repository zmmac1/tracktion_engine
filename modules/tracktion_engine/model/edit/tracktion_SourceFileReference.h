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
    This class wraps a string that is generally held in a 'source' property, and which
    is a reference to some sort of file, either in the form of a local filename or
    as a project ID that can be resolved.
*/
class SourceFileReference
{
public:
    SourceFileReference (Edit&, yup::ValueTree&, const yup::Identifier& prop);
    ~SourceFileReference();

    //==============================================================================
    Edit& edit;
    juce::CachedValue<yup::String> source;

    yup::File getFile() const;
    bool isUsingProjectReference() const;
    ProjectItemRef getSourceProjectItemRef() const;
    ProjectItem::Ptr getSourceProjectItem() const;

    enum class PathStyle { chooseBest, alwaysRelative, alwaysAbsolute };

    /** Sets this source to reference the given file.
        If allowProjectItems is true and the edit is in a file-based project,
        finds or creates a ProjectItem. Otherwise stores a direct file path
        using the given PathStyle.
    */
    void setToFile (const yup::File&, PathStyle, bool allowProjectItems);

    void setToProjectFileReference (ProjectItemRef);

    static yup::String findPathFromFile (Edit&, const yup::File&, bool useRelativePath);
    static yup::File findFileFromString (Edit&, const yup::String& source);

private:
    yup::ValueTree state;

    YUP_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SourceFileReference)
};


} // namespace tracktion::inline engine
