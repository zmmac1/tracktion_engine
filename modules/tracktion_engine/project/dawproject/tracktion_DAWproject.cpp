/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine { namespace dawproject {

//==============================================================================
std::unique_ptr<Edit> parseDAWproject (Engine& engine,
                                       const yup::File& file,
                                       ParseOptions options)
{
    if (! file.existsAsFile())
    {
        DBG ("DAWproject: File does not exist: " << file.getFullPathName());
        return nullptr;
    }

    DAWprojectImporter importer (engine, options);
    return importer.importFromFile (file);
}

//==============================================================================
tl::expected<std::unique_ptr<yup::XmlElement>, yup::String> createDAWproject (Edit& edit,
                                                                                WriteOptions options)
{
    DAWprojectExporter exporter (edit, options);
    return exporter.createProjectXml();
}

//==============================================================================
yup::Result writeDAWprojectFile (const yup::File& file,
                                  Edit& edit,
                                  WriteOptions options)
{
    DAWprojectExporter exporter (edit, options);
    return exporter.writeToFile (file);
}

}} // namespace tracktion::inline engine::dawproject
