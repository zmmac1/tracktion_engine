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

//==============================================================================
// ChannelIndex implementation
//==============================================================================

ChannelIndex::ChannelIndex() {}

ChannelIndex::ChannelIndex (int index, yup::AudioChannelSet::ChannelType c)
    : indexInDevice (index), channel (c)
{
}

ChannelIndex ChannelIndex::createMono (int indexInDevice)
{
    return ChannelIndex (indexInDevice, yup::AudioChannelSet::ChannelType (yup::AudioChannelSet::discreteChannel0 + indexInDevice));
}

bool ChannelIndex::operator== (const ChannelIndex& other) const  { return indexInDevice == other.indexInDevice && channel == other.channel; }
bool ChannelIndex::operator!= (const ChannelIndex& other) const  { return ! operator== (other); }

yup::AudioChannelSet::ChannelType channelTypeFromAbbreviatedName (const yup::String& abbreviatedName)
{
    struct NamedChannelTypeCache
    {
        NamedChannelTypeCache()
        {
            for (int i = 0; i < yup::AudioChannelSet::discreteChannel0; ++i)
            {
                const auto channelType = static_cast<yup::AudioChannelSet::ChannelType> (i);
                auto name = yup::AudioChannelSet::getAbbreviatedChannelTypeName (channelType);

                if (name.isNotEmpty())
                    map[name] = channelType;
            }
        }

        std::map<yup::String, yup::AudioChannelSet::ChannelType> map;
    };

    static NamedChannelTypeCache cache;
    const auto result = cache.map.find (abbreviatedName);

    if (result != cache.map.end())
        return result->second;

    // Discrete channels are serialised as their 1-based number (e.g. "1", "2", ...)
    if (auto discreteIndex = abbreviatedName.getIntValue(); discreteIndex > 0)
        return static_cast<yup::AudioChannelSet::ChannelType> (yup::AudioChannelSet::discreteChannel0 + discreteIndex - 1);

    return yup::AudioChannelSet::unknown;
}

yup::AudioChannelSet channelSetFromSpeakerArrangementString (const yup::String& arrangement)
{
    yup::AudioChannelSet cs;

    for (auto& channel : yup::StringArray::fromTokens (arrangement, false))
    {
        const auto ct = channelTypeFromAbbreviatedName (channel);

        if (ct != yup::AudioChannelSet::unknown)
            cs.addChannel (ct);
    }

    return cs;
}


//==============================================================================
// ChannelConfiguration implementation
//==============================================================================

ChannelConfiguration::ChannelConfiguration (std::vector<ChannelIndex> chans)
{
    channels.reserve (chans.size());
    for (auto& c : chans)
        channels.push_back (c);
}

ChannelConfiguration::ChannelConfiguration (const ChannelIndex* chans, size_t numChannels)
{
    channels.reserve (numChannels);
    for (size_t i = 0; i < numChannels; ++i)
        channels.push_back (chans[i]);
}

//==============================================================================
ChannelConfiguration ChannelConfiguration::mono (int deviceChannelIndex)
{
    ChannelConfiguration config;
    config.channels.push_back (ChannelIndex (deviceChannelIndex, yup::AudioChannelSet::centre));
    return config;
}

ChannelConfiguration ChannelConfiguration::left (int deviceChannelIndex)
{
    ChannelConfiguration config;
    config.channels.push_back (ChannelIndex (deviceChannelIndex, yup::AudioChannelSet::left));
    return config;
}

ChannelConfiguration ChannelConfiguration::right (int deviceChannelIndex)
{
    ChannelConfiguration config;
    config.channels.push_back (ChannelIndex (deviceChannelIndex, yup::AudioChannelSet::right));
    return config;
}

ChannelConfiguration ChannelConfiguration::stereo (int firstDeviceChannelIndex)
{
    ChannelConfiguration config;
    config.channels.push_back (ChannelIndex (firstDeviceChannelIndex, yup::AudioChannelSet::left));
    config.channels.push_back (ChannelIndex (firstDeviceChannelIndex + 1, yup::AudioChannelSet::right));
    return config;
}

ChannelConfiguration ChannelConfiguration::surround5_1 (int firstDeviceChannelIndex)
{
    ChannelConfiguration config;
    config.channels.push_back (ChannelIndex (firstDeviceChannelIndex + 0, yup::AudioChannelSet::left));
    config.channels.push_back (ChannelIndex (firstDeviceChannelIndex + 1, yup::AudioChannelSet::right));
    config.channels.push_back (ChannelIndex (firstDeviceChannelIndex + 2, yup::AudioChannelSet::centre));
    config.channels.push_back (ChannelIndex (firstDeviceChannelIndex + 3, yup::AudioChannelSet::LFE));
    config.channels.push_back (ChannelIndex (firstDeviceChannelIndex + 4, yup::AudioChannelSet::leftSurround));
    config.channels.push_back (ChannelIndex (firstDeviceChannelIndex + 5, yup::AudioChannelSet::rightSurround));
    return config;
}

ChannelConfiguration ChannelConfiguration::surround7_1 (int firstDeviceChannelIndex)
{
    ChannelConfiguration config;
    config.channels.push_back (ChannelIndex (firstDeviceChannelIndex + 0, yup::AudioChannelSet::left));
    config.channels.push_back (ChannelIndex (firstDeviceChannelIndex + 1, yup::AudioChannelSet::right));
    config.channels.push_back (ChannelIndex (firstDeviceChannelIndex + 2, yup::AudioChannelSet::centre));
    config.channels.push_back (ChannelIndex (firstDeviceChannelIndex + 3, yup::AudioChannelSet::LFE));
    config.channels.push_back (ChannelIndex (firstDeviceChannelIndex + 4, yup::AudioChannelSet::leftSurroundRear));
    config.channels.push_back (ChannelIndex (firstDeviceChannelIndex + 5, yup::AudioChannelSet::rightSurroundRear));
    config.channels.push_back (ChannelIndex (firstDeviceChannelIndex + 6, yup::AudioChannelSet::leftSurroundSide));
    config.channels.push_back (ChannelIndex (firstDeviceChannelIndex + 7, yup::AudioChannelSet::rightSurroundSide));
    return config;
}

ChannelConfiguration ChannelConfiguration::discreteChannels (int numChannels, int firstDeviceChannelIndex)
{
    ChannelConfiguration config;
    config.channels.reserve (static_cast<size_t> (numChannels));

    for (int i = 0; i < numChannels; ++i)
        config.channels.push_back (ChannelIndex::createMono (firstDeviceChannelIndex + i));

    return config;
}

ChannelConfiguration ChannelConfiguration::canonical (int numChannels, int firstDeviceChannelIndex)
{
    if (numChannels == 1)  return mono (firstDeviceChannelIndex);
    if (numChannels == 2)  return stereo (firstDeviceChannelIndex);
    if (numChannels == 6)  return surround5_1 (firstDeviceChannelIndex);
    if (numChannels == 8)  return surround7_1 (firstDeviceChannelIndex);

    return discreteChannels (numChannels, firstDeviceChannelIndex);
}

ChannelConfiguration ChannelConfiguration::fromChannelSet (const yup::AudioChannelSet& channelSet, int firstDeviceChannelIndex)
{
    ChannelConfiguration config;
    const int numChannels = channelSet.size();
    config.channels.reserve (static_cast<size_t> (numChannels));

    for (int i = 0; i < numChannels; ++i)
    {
        auto channelType = channelSet.getTypeOfChannel (i);
        config.channels.push_back (ChannelIndex (firstDeviceChannelIndex + i, channelType));
    }

    return config;
}

//==============================================================================
bool ChannelConfiguration::isStereo() const noexcept
{
    if (channels.size() != 2)
        return false;

    return channels[0].channel == yup::AudioChannelSet::left
        && channels[1].channel == yup::AudioChannelSet::right;
}

bool ChannelConfiguration::contains (const ChannelIndex& channel) const noexcept
{
    for (const auto& c : channels)
        if (c == channel)
            return true;

    return false;
}

bool ChannelConfiguration::containsDeviceChannel (int deviceChannelIndex) const noexcept
{
    for (const auto& c : channels)
        if (c.indexInDevice == deviceChannelIndex)
            return true;

    return false;
}

std::pair<int, int> ChannelConfiguration::getChannelRange() const
{
    int low = 0, high = 0;
    bool hasValidChannel = false;

    for (const auto& c : channels)
    {
        if (c.indexInDevice >= 0)
        {
            if (! hasValidChannel)
            {
                low = c.indexInDevice;
                high = c.indexInDevice;
                hasValidChannel = true;
            }
            else
            {
                low = std::min (low, c.indexInDevice);
                high = std::max (high, c.indexInDevice);
            }
        }
    }

    if (! hasValidChannel)
        return { 0, 0 };

    return { low, high + 1 };
}

ChannelConfiguration ChannelConfiguration::intersection (const ChannelConfiguration& other) const
{
    ChannelConfiguration result;

    for (const auto& c : channels)
        if (other.containsDeviceChannel (c.indexInDevice))
            result.addChannel (c);

    return result;
}

yup::AudioChannelSet ChannelConfiguration::toChannelSet() const
{
    yup::AudioChannelSet channelSet;

    for (const auto& ci : channels)
        channelSet.addChannel (ci.channel);

    return channelSet;
}

//==============================================================================
void ChannelConfiguration::setNumChannels (int numChannels, int firstDeviceChannelIndex)
{
    channels.clear();

    if (numChannels == 1)
    {
        channels.push_back (ChannelIndex (firstDeviceChannelIndex, yup::AudioChannelSet::centre));
    }
    else if (numChannels == 2)
    {
        channels.push_back (ChannelIndex (firstDeviceChannelIndex, yup::AudioChannelSet::left));
        channels.push_back (ChannelIndex (firstDeviceChannelIndex + 1, yup::AudioChannelSet::right));
    }
    else
    {
        for (int i = 0; i < numChannels; ++i)
            channels.push_back (ChannelIndex::createMono (firstDeviceChannelIndex + i));
    }
}

//==============================================================================
bool ChannelConfiguration::operator== (const ChannelConfiguration& other) const
{
    if (channels.size() != other.channels.size())
        return false;

    for (size_t i = 0; i < channels.size(); ++i)
        if (channels[i] != other.channels[i])
            return false;

    return true;
}

//==============================================================================
choc::value::Value ChannelConfiguration::toJSON() const
{
    return choc::value::createArray (static_cast<uint32_t> (channels.size()),
        [this] (uint32_t index)
        {
            const auto& channel = channels[index];
            return choc::json::create (
                "index", channel.indexInDevice,
                "type", yup::AudioChannelSet::getAbbreviatedChannelTypeName (channel.channel).toStdString());
        });
}

ChannelConfiguration ChannelConfiguration::fromJSON (const choc::value::ValueView& json)
{
    ChannelConfiguration config;

    if (json.isArray())
    {
        for (const auto& channel : json)
        {
            auto index = channel["index"].getWithDefault<int> (-1);
            auto type = channel["type"].toString();
            config.channels.push_back (ChannelIndex (index, channelTypeFromAbbreviatedName (yup::String(type))));
        }
    }

    return config;
}

std::string ChannelConfiguration::toString() const
{
    return choc::json::toString (toJSON());
}

ChannelConfiguration ChannelConfiguration::fromString (std::string_view s)
{
    try
    {
        return fromJSON (choc::json::parse (s));
    }
    catch (const choc::json::ParseError&)
    {
        return {};
    }
}

ChannelConfiguration ChannelConfiguration::reversed() const
{
    const auto numChans = channels.size();

    if (numChans <= 1)
        return *this;

    // Collect the device indices and reverse them
    std::vector<int> indices;
    indices.reserve (numChans);

    for (const auto& c : channels)
        indices.push_back (c.indexInDevice);

    std::reverse (indices.begin(), indices.end());

    // Build new config: same channel types, reversed device indices
    ChannelConfiguration result;
    result.channels.reserve (numChans);

    for (size_t i = 0; i < numChans; ++i)
        result.channels.push_back (ChannelIndex (indices[i], channels[i].channel));

    return result;
}

yup::String ChannelConfiguration::getDescription() const
{
    yup::MemoryOutputStream desc;
    bool isFirst = true;

    for (const auto& ci : channels)
    {
        if (! isFirst)
            desc << ", ";
        else
            isFirst = false;

        desc << ci.indexInDevice << " (" << yup::AudioChannelSet::getAbbreviatedChannelTypeName (ci.channel) << ")";
    }

    return desc.toString();
}

yup::String ChannelConfiguration::getMatchingPresetName() const
{
    for (const auto& preset : getChannelConfigurationPresets())
        if (preset.config == *this)
            return preset.name;

    return TRANS("From Edit");
}

std::vector<ChannelConfigurationPreset> getChannelConfigurationPresets()
{
    return
    {
        { TRANS("Mono"),           ChannelConfiguration::mono() },
        { TRANS("Stereo"),         ChannelConfiguration::stereo() },
        { TRANS("5.1 Surround"),   ChannelConfiguration::surround5_1() },
        { TRANS("7.1 Surround"),   ChannelConfiguration::surround7_1() },
        { TRANS("From Edit"),      {} }
    };
}

}} // namespace tracktion { inline namespace engine
