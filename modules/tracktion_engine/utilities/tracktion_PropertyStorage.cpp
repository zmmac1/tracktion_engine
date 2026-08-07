/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

yup::StringRef PropertyStorage::settingToString (SettingID setting)
{
    switch (setting)
    {
        case SettingID::audio_device_setup:                 return "audio_device_setup";
        case SettingID::audiosettings:                      return "audiosettings";
        case SettingID::audioDeviceLayoutPresets:           return "audioDeviceLayoutPresets";
        case SettingID::addAntiDenormalNoise:               return "addAntiDenormalNoiseXXX";    // This setting is obsolete (hopefully)
        case SettingID::autoFreeze:                         return "autoFreeze";
        case SettingID::autoTempoMatch:                     return "AutoTempoMatch";
        case SettingID::autoTempoDetect:                    return "AutoTempoDetect";
        case SettingID::automapVst:                         return "AutomapVst";
        case SettingID::automapNative:                      return "AutomapNative";
        case SettingID::automapGuids1:                      return "AutomapGuids1";
        case SettingID::automapGuids2:                      return "AutomapGuids2";
        case SettingID::cacheSizeSamples:                   return "cacheSizeSamples";
        case SettingID::clickTrackMidiNoteBig:              return "clickTrackMidiNoteBig";
        case SettingID::clickTrackMidiNoteLittle:           return "clickTrackMidiNoteLittle";
        case SettingID::clickTrackSampleSmall:              return "clickTrackSampleSmall";
        case SettingID::clickTrackSampleBig:                return "clickTrackSampleBig";
        case SettingID::crossfadeBlock:                     return "crossfadeBlock";
        case SettingID::compCrossfadeMs:                    return "compCrossfadeMs";
        case SettingID::countInMode:                        return "countInMode";
        case SettingID::customMidiControllers:              return "customMidiControllers";
        case SettingID::deadMansPedal:                      return "deadMansPedal";
        case SettingID::cpu:                                return "cpu";
        case SettingID::defaultMidiOutDevice:               return "defaultMidiDevice";
        case SettingID::defaultWaveOutDevice:               return "defaultWaveDevice";
        case SettingID::defaultMidiInDevice:                return "defaultMidiInDevice";
        case SettingID::defaultWaveInDevice:                return "defaultWaveInDevice";
        case SettingID::externControlIn:                    return "externControlIn";
        case SettingID::externControlOut:                   return "externControlOut";
        case SettingID::externControlNum:                   return "externControlNum";
        case SettingID::externControlMain:                  return "externControlMain";
        case SettingID::externControlShowSelection:         return "externControlShowSelection";
        case SettingID::externControlShowClipSlotSelection: return "externControlShowClipSlotSelection";
        case SettingID::externControlSelectionColor:       return "externControlSelectionColor";
        case SettingID::externControlEnable:                return "externControlEnable";
        case SettingID::externOscInputPort:                 return "externOscInputPort";
        case SettingID::externOscOutputPort:                return "externOscOutputPort";
        case SettingID::externOscOutputAddr:                return "externOscOutputAddr";
        case SettingID::filterControlMappingPresets:        return "FilterControlMappingPresets";
        case SettingID::filterGui:                          return "filterGui";
        case SettingID::findExamples:                       return "findExamples";
        case SettingID::fitClipsToRegion:                   return "fitClipsToRegion";
        case SettingID::freezePoint:                        return "freezePoint";
        case SettingID::hasEnabledMidiDefaultDevs:          return "hasEnabledMidiDefaultDevs";
        case SettingID::knownPluginList:                    return "knownPluginList";
        case SettingID::knownPluginList64:                  return "knownPluginList64";
        case SettingID::lameEncoder:                        return "lameEncoder";
        case SettingID::lastClickTrackLevel:                return "lastClickTrackLevel";
        case SettingID::lastEditRender:                     return "lastEditRender";
        case SettingID::lowLatencyBuffer:                   return "lowLatencyBuffer";
        case SettingID::glideLength:                        return "glideLength";
        case SettingID::grooveTemplates:                    return "GrooveTemplates";
        case SettingID::MCUoneTouchRecord:                  return "MCUoneTouchRecord";
        case SettingID::midiin:                             return "midiin";
        case SettingID::midiout:                            return "midiout";
        case SettingID::midiEditorOctaves:                  return "midiEditorOctaves";
        case SettingID::midiProgramManager:                 return "MidiProgramManager";
        case SettingID::maxLatency:                         return "maxLatency";
        case SettingID::newMarker:                          return "newMarker";
        case SettingID::numThreadsForPluginScanning:        return "numThreadsForPluginScanning";
        case SettingID::projectList:                        return "projectList";
        case SettingID::projects:                           return "projects";
        case SettingID::recentProjects:                     return "recentProjects";
        case SettingID::renameClipRenamesSource:            return "renameClipRenamesSource";
        case SettingID::renameMode:                         return "renameMode";
        case SettingID::renderRecentFilesList:              return "renderRecentFilesList";
        case SettingID::safeRecord:                         return "safeRecord";
        case SettingID::resetCursorOnPlay:                  return "resetCursorOnPlay";
        case SettingID::retrospectiveRecord:                return "retrospectiveRecord";
        case SettingID::reWireEnabled:                      return "ReWireEnabled";
        case SettingID::simplifyAfterRecording:             return "simplifyAfterRecording";
        case SettingID::sendControllerOffMessages:          return "sendControllerOffMessages";
        case SettingID::tempDirectory:                      return "tempDirectory";
        case SettingID::snapCursor:                         return "snapCursor";
        case SettingID::trackExpansionMode:                 return "trackExpansionMode";
        case SettingID::use64Bit:                           return "use64Bit";
        case SettingID::xFade:                              return "xFade";
        case SettingID::xtCount:                            return "xtCount";
        case SettingID::xtIndices:                          return "xtIndices";
        case SettingID::virtualmididevices:                 return "virtualmididevices";
        case SettingID::virtualmidiin:                      return "virtualmidiin";
        case SettingID::midiScanIntervalSeconds:            return "midiScanIntervalSeconds";
        case SettingID::useSeparateProcessForScanning:      return "useSeparateProcessForScanning";
        case SettingID::useRealtime:                        return "useRealtime";
        case SettingID::wavein:                             return "wavein";
        case SettingID::waveout:                            return "waveout";
        case SettingID::windowsDoubleClick:                 return "windowsDoubleClick";

        case SettingID::renderFormat:                       return "renderFormat";
        case SettingID::trackRenderSampRate:                return "trackRenderSampRate";
        case SettingID::trackRenderBits:                    return "trackRenderBits";
        case SettingID::bypassFilters:                      return "bypassFilters";
        case SettingID::markedRegion:                       return "markedRegion";
        case SettingID::editClipRenderSampRate:             return "editClipRenderSampRate";
        case SettingID::editClipRenderBits:                 return "editClipRenderBits";
        case SettingID::editClipRenderDither:               return "editClipRenderDither";
        case SettingID::editClipRealtime:                   return "editClipRealtime";
        case SettingID::editClipRenderStereo:               return "editClipRenderStereo";
        case SettingID::editClipRenderNormalise:            return "editClipRenderNormalise";
        case SettingID::editClipRenderRMS:                  return "editClipRenderRMS";
        case SettingID::editClipRenderRMSLevelDb:           return "editClipRenderRMSLevelDb";
        case SettingID::editClipRenderPeakLevelDb:          return "editClipRenderPeakLevelDb";
        case SettingID::editClipPassThroughFilters:         return "editClipPassThroughFilters";
        case SettingID::exportFormat:                       return "exportFormat";
        case SettingID::renderOnlySelectedClips:            return "renderOnlySelectedClips";
        case SettingID::renderOnlyMarked:                   return "renderOnlyMarked";
        case SettingID::renderNormalise:                    return "renderNormalise";
        case SettingID::renderRMS:                          return "renderRMS";
        case SettingID::renderRMSLevelDb:                   return "renderRMSLevelDb";
        case SettingID::renderPeakLevelDb:                  return "renderPeakLevelDb";
        case SettingID::renderTrimSilence:                  return "renderTrimSilence";
        case SettingID::renderSampRate:                     return "renderSampRate";
        case SettingID::renderStereo:                       return "renderStereo";
        case SettingID::renderChannelConfig:                return "renderChannelConfig";
        case SettingID::renderBits:                         return "renderBits";
        case SettingID::renderDither:                       return "renderDither";
        case SettingID::quality:                            return "quality";
        case SettingID::addId3Info:                         return "addId3Info";
        case SettingID::realtime:                           return "realtime";
        case SettingID::passThroughFilters:                 return "passThroughFilters";
        case SettingID::invalid:                            return "invalid";
        case SettingID::addAcidMetadata:                    return "addAcidMetadata";
    }
    return {};
}

//==============================================================================
yup::File PropertyStorage::getAppCacheFolder()
{
    return getAppPrefsFolder();
}

yup::File PropertyStorage::getAppPrefsFolder()
{
    auto f = yup::File::getSpecialLocation (yup::File::userApplicationDataDirectory).getChildFile (getApplicationName());

    if (! f.isDirectory())
        f.createDirectory();

    return f;
}

yup::String PropertyStorage::getUserName()
{
    return yup::SystemStats::getFullUserName();
}

yup::PropertiesFile& PropertyStorage::getPropertiesFile()
{
    if (propertiesFile == nullptr)
    {
        auto file = getAppPrefsFolder().getChildFile ("Settings.xml");
        TRACKTION_LOG ("Settings file: " + file.getFullPathName());

        yup::PropertiesFile::Options options;
        options.millisecondsBeforeSaving = 2000;
        options.storageFormat = yup::PropertiesFile::storeAsXML;

        propertiesFile = std::make_unique<yup::PropertiesFile> (file, options);
    }

    return *propertiesFile;
}

//==============================================================================
void PropertyStorage::removeProperty (SettingID setting)
{
    getPropertiesFile().removeValue (PropertyStorage::settingToString (setting));
}

yup::var PropertyStorage::getProperty (SettingID setting, const yup::var& defaultValue)
{
    return getPropertiesFile().getValue (PropertyStorage::settingToString (setting), defaultValue);
}

void PropertyStorage::setProperty (SettingID setting, const yup::var& value)
{
    getPropertiesFile().setValue (PropertyStorage::settingToString (setting), value);
}

std::unique_ptr<yup::XmlElement> PropertyStorage::getXmlProperty (SettingID setting)
{
    return std::unique_ptr<yup::XmlElement> (getPropertiesFile().getXmlValue (PropertyStorage::settingToString (setting)));
}

void PropertyStorage::setXmlProperty (SettingID setting, const yup::XmlElement& xml)
{
    getPropertiesFile().setValue (PropertyStorage::settingToString (setting), &xml);
}

//==============================================================================
void PropertyStorage::removePropertyItem (SettingID setting, yup::StringRef item)
{
    getPropertiesFile().removeValue (PropertyStorage::settingToString (setting) + "_" + item);
}

yup::var PropertyStorage::getPropertyItem (SettingID setting, yup::StringRef item, const yup::var& defaultValue)
{
    return getPropertiesFile().getValue (PropertyStorage::settingToString (setting) + "_" + item, defaultValue);
}

void PropertyStorage::setPropertyItem (SettingID setting, yup::StringRef item, const yup::var& value)
{
    getPropertiesFile().setValue (PropertyStorage::settingToString (setting) + "_" + item, value);
}

std::unique_ptr<yup::XmlElement> PropertyStorage::getXmlPropertyItem (SettingID setting, yup::StringRef item)
{
    return std::unique_ptr<yup::XmlElement> (getPropertiesFile().getXmlValue (PropertyStorage::settingToString (setting) + "_" + item));
}

void PropertyStorage::setXmlPropertyItem (SettingID setting, yup::StringRef item, const yup::XmlElement& xml)
{
    getPropertiesFile().setValue (PropertyStorage::settingToString (setting) + "_" + item, &xml);
}

//==============================================================================
yup::File PropertyStorage::getDefaultLoadSaveDirectory (yup::StringRef)
{
    return yup::File::getSpecialLocation (yup::File::userDocumentsDirectory);
}

void PropertyStorage::setDefaultLoadSaveDirectory (yup::StringRef, const yup::File&)
{
}

yup::File PropertyStorage::getDefaultLoadSaveDirectory (ProjectItem::Category)
{
    return yup::File::getSpecialLocation (yup::File::userDocumentsDirectory);
}

} // namespace tracktion::inline engine
