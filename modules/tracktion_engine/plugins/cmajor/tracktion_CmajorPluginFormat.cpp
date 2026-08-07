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
yup::String getCmajorPatchPluginFormatName()
{
    return "Cmajor";
}

bool isCmajorPatchPluginFormat (const yup::String& formatName)
{
    return formatName == getCmajorPatchPluginFormatName();
}

bool isCmajorPatchPluginFormat (const yup::PluginDescription& pd)
{
    return isCmajorPatchPluginFormat (pd.name);
}

//==============================================================================
#if TRACKTION_ENABLE_CMAJOR

} // namespace tracktion::inline engine

// N.B. cmajor's cmaj_JUCEPlugin.h guards on JUCE_MAJOR_VERSION == 8, so on JUCE 9 it
// falls back to the deprecated Font constructors. The vendored repo can't be patched,
// so the deprecation warning is suppressed for this header only.
JUCE_BEGIN_IGNORE_DEPRECATION_WARNINGS
#include "cmajor/helpers/cmaj_JUCEPluginFormat.h"
JUCE_END_IGNORE_DEPRECATION_WARNINGS
#include "cmajor/helpers/cmaj_FileBasedCacheDatabase.h"

namespace tracktion::inline engine {

std::string getCmajorVersion() { return cmaj::Library::getVersion(); }

cmaj::CacheDatabaseInterface::Ptr createCompilerCache (tracktion::Engine& engine)
{
    auto cacheFolder = engine.getTemporaryFileManager().getTempDirectory().getChildFile ("cmajor_patch_cache");

    if (cacheFolder.createDirectory())
        return choc::com::create<cmaj::FileBasedCacheDatabase> (cacheFolder.getFullPathName().toStdString(), 100u);

    return {};
}

std::unique_ptr<yup::AudioPluginFormat> createCmajorPatchPluginFormat (tracktion::Engine& engine)
{
    TRACKTION_LOG ("Cmajor plugin format enabled");

    jassert (getCmajorPatchPluginFormatName() == cmaj::plugin::JUCEPluginFormat::pluginFormatName);

    auto onPatchChange = [&engine] (cmaj::plugin::SinglePatchJITPlugin& plugin)
    {
        for (auto& edit : engine.getActiveEdits().getEdits())
        {
            if (auto p = edit->getPluginCache().getPluginFor (plugin))
            {
                if (! p->isInitialising())
                {
                    if (auto ex = dynamic_cast<ExternalPlugin*> (p.get()))
                    {
                        plugin.suspendProcessing (true);
                        plugin.fillInPluginDescription (ex->desc);
                        ex->forceFullReinitialise();
                        plugin.suspendProcessing (false);
                    }
                    else
                    {
                        jassertfalse;
                    }
                }

                return;
            }
        }
    };

    return std::make_unique<cmaj::plugin::JUCEPluginFormat> (createCompilerCache (engine),
                                                             std::move (onPatchChange),
                                                             engine.getPropertyStorage().getApplicationVersion().toStdString());
}

yup::String getCmajorPatchCompileError (Plugin& p)
{
    if (auto ex = dynamic_cast<ExternalPlugin*> (&p))
        if (auto plugin = dynamic_cast<cmaj::plugin::SinglePatchJITPlugin*> (ex->getAudioPluginInstance()))
            if (plugin->isStatusMessageError)
                return plugin->statusMessage;

    return {};
}

#else

//==============================================================================
std::string getCmajorVersion() { return {}; }
yup::String getCmajorPatchCompileError (Plugin&) { return {}; }
std::unique_ptr<yup::AudioPluginFormat> createCmajorPatchPluginFormat (tracktion::Engine&)  { return {}; }

#endif

} // namespace tracktion::inline engine
