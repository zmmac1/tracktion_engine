/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion { inline namespace engine
{

/**
    A base class for input/output devices.

    See InputDevice and OutputDevice for more details.
*/
class IODevice   : public Selectable
{
public:
    //==============================================================================
    IODevice (Engine&, yup::String name, yup::String deviceID);
    ~IODevice() override;

    //==============================================================================
    const yup::String& getName() const                       { return name; }
    yup::String getDeviceID() const                          { return deviceID; }

    bool isEnabled() const;
    virtual void setEnabled (bool) = 0;

    virtual bool isMidi() const = 0;
    virtual bool isTrackDevice() const = 0;
    virtual yup::String getDeviceTypeDescription() const = 0;

    /// the alias is the name shown in the draggable input device components
    void setAlias (const yup::String& newAlias);
    /// Returns the alias if set, otherwise the name.
    yup::String getAliasOrName() const;
    /// Returns the alias, or an empty string if not set.
    yup::String getAliasIfSet() const;

    yup::String getSelectableDescription() override;

    //==============================================================================
    Engine& engine;

protected:
    std::atomic<bool> enabled { false };

    const yup::String deviceID, name;

private:
    yup::String alias;
    yup::String getAliasPropName() const;

    const yup::String& getType() const; // Deprecated: use getDeviceTypeDescription() instead
};


}} // namespace tracktion { inline namespace engine
