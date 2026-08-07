/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

/**
    Create a subclass of PropertyStorage to customize how settings are saved
    and recalled
*/
class PropertyStorage
{
public:
    //==============================================================================
    PropertyStorage (yup::String appName_) : appName (appName_) {}
    virtual ~PropertyStorage() {}

    static yup::StringRef settingToString (SettingID);

    //==============================================================================
    virtual yup::File getAppCacheFolder();
    virtual yup::File getAppPrefsFolder();

    virtual void flushSettingsToDisk() {}

    //==============================================================================
    virtual void removeProperty (SettingID);

    virtual yup::var getProperty (SettingID setting, const yup::var& defaultValue = {});
    virtual void setProperty (SettingID setting, const yup::var& value);

    virtual std::unique_ptr<yup::XmlElement> getXmlProperty (SettingID setting);
    virtual void setXmlProperty (SettingID setting, const yup::XmlElement&);

    //==============================================================================
    virtual void removePropertyItem (SettingID setting, yup::StringRef item);

    virtual yup::var getPropertyItem (SettingID setting, yup::StringRef item, const yup::var& defaultValue = {});
    virtual void setPropertyItem (SettingID setting, yup::StringRef item, const yup::var& value);

    virtual std::unique_ptr<yup::XmlElement> getXmlPropertyItem (SettingID setting, yup::StringRef item);
    virtual void setXmlPropertyItem (SettingID setting, yup::StringRef item, const yup::XmlElement&);

    //==============================================================================
    virtual yup::File getDefaultLoadSaveDirectory (yup::StringRef label);
    virtual void setDefaultLoadSaveDirectory (yup::StringRef label, const yup::File& newPath);

    virtual yup::File getDefaultLoadSaveDirectory (ProjectItem::Category);

    //==============================================================================
    virtual yup::String getUserName();
    virtual yup::String getApplicationName()       { return appName; }
    virtual yup::String getApplicationVersion()    { return "Unknown"; }

    //==============================================================================
    // If you are using the default implementation of PropertyStorage, it uses a
    // PropertiesFile to store its state. This accessor method lets you use it in
    // case you need it for things like a yup::PluginListComponent.
    virtual yup::PropertiesFile& getPropertiesFile();

private:
    yup::String appName;

    std::unique_ptr<yup::PropertiesFile> propertiesFile;
};

} // namespace tracktion::inline engine
