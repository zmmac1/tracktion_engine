/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

#pragma once

namespace tracktion::inline engine {
    std::string getCmajorVersion();
    std::unique_ptr<yup::AudioPluginFormat> createCmajorPatchPluginFormat (Engine&);

    bool isCmajorPatchPluginFormat (const yup::PluginDescription&);
    bool isCmajorPatchPluginFormat (const yup::String& formatName);
    yup::String getCmajorPatchPluginFormatName();
    yup::String getCmajorPatchCompileError (Plugin&);

} // namespace tracktion::inline engine
