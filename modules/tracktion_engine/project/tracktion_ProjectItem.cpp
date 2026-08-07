/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2024
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion::inline engine {

static const char* slashEscapeSeq = "[[slash]]";

struct StringMap
{
    StringMap() {}

    StringMap (const StringMap& other)
        : keys (other.keys),
          values (other.values)
    {
    }

    StringMap& operator= (const StringMap& other)
    {
        keys = other.keys;
        values = other.values;
        return *this;
    }

    //==============================================================================
    void set (const yup::String& key, const yup::String& value)
    {
        if (value.isEmpty())
        {
            remove (key);
        }
        else
        {
            const int index = keys.indexOf (key);

            if (index >= 0)
            {
                values.set (index, value);
            }
            else
            {
                keys.add (key);
                values.add (value);
            }
        }
    }

    void remove (const yup::String& key)
    {
        const int index = keys.indexOf (key);

        if (index >= 0)
        {
            keys.remove (index);
            values.remove (index);
        }
    }

    void clear()
    {
        keys.clear();
        values.clear();
    }

    yup::String get (const yup::String& key) const
    {
        return values [keys.indexOf (key)];
    }

    int size() const
    {
        return keys.size();
    }

    yup::String getKeyAt (int index) const
    {
        return keys[index];
    }

    yup::String getValueAt (int index) const
    {
        return values[index];
    }

    void remove (int index)
    {
        keys.remove (index);
        values.remove (index);
    }

    bool containsKey (const yup::String& key) const
    {
        return keys.contains (key);
    }

    bool containsValue (const yup::String& value) const
    {
        return values.contains (value);
    }

    yup::String getKeyForValue (const yup::String& value) const
    {
        return keys [values.indexOf (value)];
    }

    yup::StringArray getKeysForValue (const yup::String& value) const
    {
        yup::StringArray s;

        for (int i = values.size(); --i >= 0;)
            if (values[i] == value)
                s.add (keys[i]);

        return s;
    }

    bool operator== (const StringMap& other) const
    {
        return keys == other.keys && values == other.values;
    }

    //==============================================================================
    yup::String toString() const
    {
        yup::String s;

        for (int i = 0; i < keys.size(); ++i)
        {
            s << keys[i].replace ("|", slashEscapeSeq)
              << '|'
              << values[i].replace ("|", slashEscapeSeq);

            if (i < keys.size() - 1)
                s << '|';
        }

        return s;
    }

    static StringMap recreateFromString (const yup::String& stringVersion)
    {
        StringMap sm;
        auto n = stringVersion.getCharPointer();

        for (;;)
        {
            int len = 0;
            yup::juce_wchar c = 0;

            for (;;)
            {
                c = n[len];

                if (c == '|' || c == 0)
                    break;

                ++len;
            }

            if (c == 0)
                break;

            yup::String key (n, (size_t) len);
            n += len + 1;
            len = 0;

            for (;;)
            {
                c = n[len];
                if (c == '|' || c == 0)
                    break;

                ++len;
            }

            if (key.contains (slashEscapeSeq))
                key = key.replace (slashEscapeSeq, "|");

            yup::String val (n, (size_t) len);

            if (val.contains (slashEscapeSeq))
                val = val.replace (slashEscapeSeq, "|");

            sm.set (key, val);

            if (c == 0)
                break;

            n += len + 1;
        }

        return sm;
    }

    yup::StringArray keys, values;

    YUP_LEAK_DETECTOR (StringMap)
};


//==============================================================================
ProjectItem::ProjectItem (Engine& e,
                          const yup::String& name_, const yup::String& type_,
                          const yup::String& desc_, const yup::String& file_,
                          ProjectItem::Category category_,
                          double length_, ProjectItemRef ref)
   : Selectable(),
     ReferenceCountedObject(),
     engine (e),
     itemRef (ref),
     type (type_),
     objectName (name_),
     description (desc_),
     file (file_),
     length (length_)
{
    setCategory (category_);
}

static yup::String readStringAutoDetectingUTF (yup::InputStream& in)
{
    yup::MemoryOutputStream mo;

    while (char byte = in.readByte())
        mo << byte;

    return mo.toString();
}

ProjectItem::ProjectItem (Engine& e, const yup::File& src,
                          const yup::String& type_, const yup::String& name_,
                          Category category_, Project& owner)
   : engine (e),
     itemRef (ProjectItemRef (src.getFullPathName(), owner)),
     type (type_),
     objectName (name_),
     description ("|MediaObjectCategory|" + yup::String ((int) category_)),
     file (src.getFullPathName()),
     sourceFile (src),
     ownerProject (&owner)
{
    // Category is encoded directly into 'description' above rather than going
    // via setCategory()/sendChange(). Freshly-scanned items have no listeners,
    // and firing a change here would propagate to the owning Project and
    // retrigger any UI that reloads on project-changed notifications, creating
    // an infinite scan/reload cycle for folder-based projects.
}

ProjectItem::ProjectItem (Engine& e, ProjectItemRef ref, yup::InputStream* in)
   : engine (e), itemRef (ref)
{
    objectName  = readStringAutoDetectingUTF (*in);
    type        = readStringAutoDetectingUTF (*in);
    description = readStringAutoDetectingUTF (*in);
    file        = readStringAutoDetectingUTF (*in);
    length      = in->readDouble();
}

ProjectItem::~ProjectItem()
{
    notifyListenersOfDeletion();
}

void ProjectItem::writeToStream (yup::OutputStream& out) const
{
    out.writeString (objectName);
    out.writeString (type);
    out.writeString (description);
    out.writeString (file);
    out.writeDouble (length);
}

void ProjectItem::sendChange()
{
    changed();

    if (auto pp = getProject())
        pp->changed();
}

yup::String ProjectItem::getSelectableDescription()
{
    if (isEdit())               return TRANS ("Edit");
    if (isWave() || isMidi())   return TRANS ("Audio File");
    if (isVideo())              return TRANS ("Video File");

    return TRANS("Project item of type 'XXX'").replace ("XXX", type);
}

void ProjectItem::selectionStatusChanged (bool isNowSelected)
{
    // Update the length quietly: broadcasting a change here dirties the whole
    // project, which makes the project tab rebuild its lists and lose the
    // selection that's being made
    if (isNowSelected && getLength() == 0)
        verifyLength (yup::dontSendNotification);
}

//==============================================================================
yup::File ProjectItem::getRelativeFile (const yup::String& name) const
{
    if (auto pp = getProject())
    {
        auto localName = name;

       #if JUCE_WINDOWS
        if (localName.containsChar ('/'))
        {
            if (localName.startsWithChar ('/'))
                localName = "C:" + localName.replaceCharacter ('/', '\\');
            else
                localName = localName.replaceCharacter ('/', '\\');
        }
       #else
        if (localName.containsChar ('\\'))
            localName = localName.replaceCharacter ('\\', '/');
       #endif

        auto projectDir = pp->getDefaultDirectory();
        return projectDir.getChildFile (localName);
    }

    jassertfalse;
    return {};
}

yup::File ProjectItem::getSourceFile()
{
    if (sourceFile == yup::File())
    {
        auto f = getRelativeFile (file);

        if (f.existsAsFile())
        {
            sourceFile = f;
        }
        else if (engine.getAudioFileFormatManager().canOpen (f))
        {
            // if not found, look for a compressed version to use..
            auto substitute = f.withFileExtension ("flac");

            if (! substitute.existsAsFile())
                substitute = f.withFileExtension ("ogg");

            if (substitute.existsAsFile())
                sourceFile = substitute;
        }
    }

    return sourceFile;
}

bool ProjectItem::isForFile (const yup::File& f)
{
    if (sourceFile != yup::File())
        return f == sourceFile;

    return file.endsWithIgnoreCase (f.getFileName()) && getSourceFile() == f;
}

void ProjectItem::setSourceFile (const yup::File& f, FileMode mode)
{
    if (auto pp = getProject())
    {
        auto oldSourceFile = getSourceFile();

        auto projectDir = pp->getDefaultDirectory();
        bool relative = false;

        if (mode == FileMode::relativeIfWithinProject)
            relative = f.isAChildOf (projectDir);
        else if (mode == FileMode::relative)
            relative = true;

        if (relative)
            file = f.getRelativePathFrom (projectDir);
        else
            file = f.getFullPathName();

        if (! itemRef.isProjectItemID())
        {
            if (ownerProject)
                itemRef = ProjectItemRef (f.getFullPathName(), *ownerProject);
            else
                jassertfalse;
        }

        sourceFile = yup::File();

        changed();
        pp->changed();

        triggerAsyncUpdate();

        if (oldSourceFile != yup::File() && oldSourceFile != f)
            pp->sourceFileMoved (oldSourceFile, f);
    }
}

bool ProjectItem::isAbsolutePath() const
{
    return file.isNotEmpty() && yup::File::isAbsolutePath (getRawFileName());
}

void ProjectItem::convertToRelativePath()
{
    auto source = getSourceFile();

    if (source.existsAsFile() && isAbsolutePath())
        setSourceFile (source, FileMode::relative);
}

void ProjectItem::convertToAbsolutePath()
{
    auto source = getSourceFile();

    if (source.existsAsFile() && ! isAbsolutePath())
        setSourceFile (source, FileMode::absolute);
}

void ProjectItem::handleAsyncUpdate()
{
    if (isEdit())
        EditFileOperations::updateEditFiles();

    for (auto edit : engine.getActiveEdits().getEdits())
        if (edit != nullptr)
            edit->sendSourceFileUpdate();
}

yup::String ProjectItem::getFileName() const
{
    if (file.isEmpty())
        return {};

    auto standardised = file.replaceCharacter ('\\', '/');

    if (standardised.containsChar ('/'))
        return standardised.fromLastOccurrenceOf ("/", false, false);

    return standardised;
}

//==============================================================================
Project::Ptr ProjectItem::getProject() const
{
    if (auto p = ownerProject.get())
        return p;

    return engine.getProjectManager().getProject (itemRef.getProjectID());
}

bool ProjectItem::hasBeenDeleted() const
{
    if (auto p = getProject())
    {
        if (itemRef.getProjectItemID().has_value())
            return p->getProjectItemFor (itemRef) == nullptr;

        if (itemRef.isValid())
            return ! itemRef.resolve (engine, p->getProjectFile()).existsAsFile();

        return false;
    }

    return true;
}

const yup::String& ProjectItem::getName() const
{
    return objectName;
}

void ProjectItem::setName (const yup::String& n, SetNameMode mode)
{
    if (n.isEmpty())
        return;

    if (objectName != n)
    {
        if (auto pp = getProject())
        {
            objectName = n.substring (0, 256);

            // also rename the file if it is in the project folder
            auto src = getSourceFile();

            bool shouldRename = false;
            const bool isSync = (mode == SetNameMode::doDefaultSynchronous
                              || mode == SetNameMode::forceRenameSynchronous);

            auto mrm = (ProjectItem::RenameMode) static_cast<int> (engine.getPropertyStorage().getProperty (SettingID::renameMode, (int) RenameMode::local));

            if (mode == SetNameMode::forceNoRename)                 shouldRename = false;
            else if (mode == SetNameMode::forceRenameSynchronous)   shouldRename = true;
            else if (mrm == RenameMode::always)                     shouldRename = true;
            else if (mrm == RenameMode::never)                      shouldRename = false;
            else if (mrm == RenameMode::local)                      shouldRename = src.isAChildOf (pp->getDefaultDirectory());

            if (shouldRename)
            {
                // Don't use "withFileExtension" here as it will strip any name text after a period "."
                const auto legalFileName = yup::File::createLegalFileName (n);
                newDstFile = src.getParentDirectory().getChildFile (legalFileName + src.getFileExtension());
                jassert (newDstFile.hasFileExtension (src.getFileExtension()));
                jassert (newDstFile.getFileNameWithoutExtension() == legalFileName);

                if (isSync)
                    timerCallback();
                else
                    startTimer (1);
            }
            else
            {
                engine.getUIBehaviour().editNamesMayHaveChanged();
            }

            sendChange();
        }
    }
}

void ProjectItem::timerCallback()
{
    stopTimer();

    Ptr selfRef (this);  // prevent destruction during setSourceFile -> reload

    auto src = getSourceFile();
    auto dst = getNonExistentSiblingWithIncrementedNumberSuffix (newDstFile, false);

    TransportControl::stopAllTransports (engine, false, true);

    auto& afm = engine.getAudioFileManager();
    afm.releaseFile (AudioFile (engine, src));

    if (! dst.existsAsFile() && src.moveFileTo (dst))
    {
        afm.checkFileForChanges (AudioFile (engine, dst));
        afm.checkFileForChanges (AudioFile (engine, src));

        setSourceFile (dst);

        SelectionManager::refreshAllPropertyPanels();
        engine.getUIBehaviour().editNamesMayHaveChanged();
        sendChange();
    }
}

ProjectItem::Category ProjectItem::getCategory() const
{
    auto cat = (Category) getNamedProperty ("MediaObjectCategory").getIntValue();

    if (cat == Category::none && isEdit())
        return Category::edit;

    return cat;
}

void ProjectItem::setCategory (ProjectItem::Category cat)
{
    setNamedProperty ("MediaObjectCategory", yup::String ((int) cat));
    sendChange();
}

const yup::String& ProjectItem::getType() const
{
    return type;
}

yup::String ProjectItem::getDescription() const
{
    return description.upToFirstOccurrenceOf ("|", false, false);
}

void ProjectItem::setDescription (const yup::String& newDesc, yup::NotificationType notify)
{
    if (newDesc != getDescription())
    {
        if (description.containsChar ('|'))
            description = newDesc.removeCharacters ("|")
                            + description.fromFirstOccurrenceOf ("|", true, false);
        else
            description = newDesc.removeCharacters ("|");

        if (notify != yup::dontSendNotification)
            sendChange();
    }
}

void ProjectItem::copyAllPropertiesFrom (const ProjectItem& other)
{
    objectName = other.objectName;
    description = other.description;
}

yup::String ProjectItem::getNamedProperty (const yup::String& name) const
{
    if (description.containsChar ('|'))
    {
        const StringMap map (StringMap::recreateFromString (description.fromFirstOccurrenceOf ("|", false, false)));

        return map.get (name);
    }

    return {};
}

void ProjectItem::setNamedProperty (const yup::String& name, const yup::String& value)
{
    if (auto pp = getProject())
    {
        if (! pp->isReadOnly())
        {
            StringMap map;

            if (description.containsChar ('|'))
                map = StringMap::recreateFromString (description.fromFirstOccurrenceOf ("|", false, false));

            if (map.get (name) != value)
            {
                map.set (name, value);
                description = getDescription() + '|' + map.toString();
                sendChange();
            }
        }
    }
}

yup::Array<TimePosition> ProjectItem::getMarkedPoints() const
{
    yup::Array<TimePosition> marks;
    auto m = getNamedProperty ("marks");

    if (m.isNotEmpty())
    {
        yup::StringArray toks;
        toks.addTokens (m, true);

        for (auto& t : toks)
            marks.add (TimePosition::fromSeconds (t.getDoubleValue()));
    }

    return marks;
}

void ProjectItem::setMarkedPoints (const yup::Array<TimePosition>& points)
{
    if (auto pp = getProject())
    {
        if (! pp->isReadOnly())
        {
            yup::String m;

            for (auto& p : points)
                m << p.inSeconds() << " ";

            setNamedProperty ("marks", m.trim());

            // stop the ui getting chuggy when moving points
            pp->cancelAnyPendingUpdates();
        }
    }
}

void ProjectItem::convertEditFile (std::function<void (bool)> callback)
{
    auto f = getSourceFile();

    if (f.hasFileExtension (editFileSuffix))
    {
        if (callback)
            callback (true);

        return;
    }

    auto newFile = f.withFileExtension (editFileSuffix);

    if (newFile.existsAsFile())
    {
        yup::String m (TRANS("There appears to already be a converted Edit in the project folder."));
        m << "\n" << TRANS("Do you want to use this, or create a new conversion?");

        auto safeThis = makeSafeRef (*this);

        engine.getUIBehaviour().showOkCancelAlertBoxAsync (TRANS("Converted Edit Already Exists"), m,
                                                           TRANS("Use Existing"),
                                                           TRANS("Create New"),
                                                           [safeThis, f, newFile, callback] (bool useExisting) mutable
                                                           {
                                                               if (auto self = safeThis.get())
                                                               {
                                                                   if (useExisting)
                                                                   {
                                                                       self->setSourceFile (newFile);

                                                                       if (callback)
                                                                           callback (true);

                                                                       return;
                                                                   }

                                                                   newFile.copyFileTo (newFile.getNonexistentSibling());

                                                                   if (f.existsAsFile() && f != newFile)
                                                                   {
                                                                       if (! f.copyFileTo (newFile))
                                                                       {
                                                                           self->engine.getUIBehaviour().showWarningAlert (TRANS("Unable to Open Edit"),
                                                                                                                           TRANS("The selected Edit file could not be converted to the current project format.")
                                                                                                                             + "\n\n"
                                                                                                                             + TRANS("Please ensure you can write to the Edit directory and try again."));
                                                                           if (callback)
                                                                               callback (false);

                                                                           return;
                                                                       }

                                                                       self->setSourceFile (newFile);
                                                                   }

                                                                   if (callback)
                                                                       callback (true);
                                                               }
                                                           });
        return;
    }

    if (f.existsAsFile() && f != newFile)
    {
        if (! f.copyFileTo (newFile))
        {
            engine.getUIBehaviour().showWarningAlert (TRANS("Unable to Open Edit"),
                                                      TRANS("The selected Edit file could not be converted to the current project format.")
                                                        + "\n\n"
                                                        + TRANS("Please ensure you can write to the Edit directory and try again."));
            if (callback)
                callback (false);

            return;
        }

        setSourceFile (newFile);
    }

    if (callback)
        callback (true);
}

double ProjectItem::getLength() const
{
    return length;
}

void ProjectItem::setLength (double l)
{
    if (std::abs (length - l) > 0.001)
    {
        length = l;
        sendChange();
    }
}

yup::String ProjectItem::getProjectName() const
{
    if (auto p = getProject())
        return p->getName();

    return {};
}

//==============================================================================
void ProjectItem::verifyLength (yup::NotificationType notification)
{
    double len = 0.0;

    if (isWave())
    {
        len = AudioFile (engine, getSourceFile()).getLength();
    }
    else if (isMidi())
    {
        yup::FileInputStream in (getSourceFile());

        if (in.openedOk())
        {
            yup::MidiFile mf;
            mf.readFrom (in);
            mf.convertTimestampTicksToSeconds();

            len = mf.getLastTimestamp();
        }
    }
    else if (isEdit())
    {
        // don't do this - too slow..
        //Edit tempEdit (this);
        //len = tempEdit.getLength();
    }

    if (len > 0.001 && len != getLength())
    {
        if (notification == yup::dontSendNotification)
            length = len;
        else
            setLength (len);
    }
}

//==============================================================================
bool ProjectItem::copySectionToNewFile (const yup::File& destFile,
                                        yup::File& actualFileCreated,
                                        double startTime, double lengthToCopy)
{
    actualFileCreated = destFile;

    if (isWave())
    {
        actualFileCreated = destFile;
        return AudioFileUtils::copySectionToNewFile (engine,
                                                     getSourceFile(), actualFileCreated,
                                                     { TimePosition::fromSeconds (startTime), TimeDuration::fromSeconds (lengthToCopy) }) > 0;
    }

    if (isEdit())
        return getSourceFile().copyFileTo (destFile);

    return false;
}

bool ProjectItem::deleteSourceFile()
{
    bool ok = true;
    auto f = getSourceFile();

    if (f.existsAsFile())
    {
        // extra steps in case it's in use by a strip or something..
        TransportControl::stopAllTransports (engine, false, true);
        auto& afm = engine.getAudioFileManager();

        AudioFile af (engine, f);

        for (int attempts = 3; --attempts >= 0;)
        {
            afm.releaseFile (af);
            ok = f.moveToTrash();

            if (ok)
                break;

            yup::Thread::sleep (800);
        }

        afm.checkFileForChangesAsync (af);

        if (! ok)
        {
            auto info = f.getFullPathName() + "  "
                         + yup::File::descriptionOfSizeInBytes (f.getSize());

            if (! f.hasWriteAccess())
                info << "  (read only)";

            if (f.isDirectory())
                info << "  (directory)";

            info << "  modified: " << f.getLastModificationTime().toString (true, true);

            TRACKTION_LOG_ERROR (info);
        }
    }

    return ok;
}

//==============================================================================
void ProjectItem::changeProjectId (ProjectID oldID, ProjectID newID)
{
    if (auto pid = itemRef.getProjectItemID(); pid && pid->getProjectID() == oldID)
        itemRef = ProjectItemID (pid->getItemID(), newID);

    if (isEdit())
    {
        auto ed = loadEditForExamining (engine.getProjectManager(), itemRef);

        for (auto exp : Exportable::addAllExportables (*ed))
            for (auto& item : exp->getReferencedItems())
                if (auto pid2 = item.itemRef.getProjectItemID(); pid2 && pid2->getProjectID() == oldID)
                    exp->reassignReferencedItem (item, pid2->withNewProjectID (newID), 0.0);

        EditFileOperations (*ed).save (false, true, false);
    }
}


//==============================================================================
ProjectItem::Ptr ProjectItem::createCopy()
{
    if (auto pp = getProject())
    {
        // create a new unique name
        auto newName = getName().trim();

        if (newName.fromLastOccurrenceOf ("(", false, false).containsOnly ("0123456789()")
             && newName.retainCharacters ("0123456789()").length() > 0)
        {
            while (newName.length() > 1
                    && yup::String ("0123456789()").containsChar (newName.getLastCharacter()))
                newName = newName.dropLastCharacters (1);
        }

        if (! newName.endsWithIgnoreCase (TRANS("Copy")))
            newName = newName + " - " + TRANS("Copy");

        auto nameStem = newName;
        int index = 2;

        for (;;)
        {
            bool alreadyThere = false;

            for (int i = pp->getNumProjectItems(); --i >= 0;)
            {
                if (pp->getProjectItemAt (i)->getName().equalsIgnoreCase (newName))
                {
                    alreadyThere = true;
                    break;
                }
            }

            if (alreadyThere)
                newName = nameStem + "(" + yup::String (index++) + ")";
            else
                break;
        }

        auto source = getSourceFile();
        auto fileExtension = source.getFileExtension();

        if (fileExtension.isEmpty())
            if (auto af = engine.getAudioFileFormatManager().getFormatFromFileName (source))
                fileExtension = af->getFileExtensions()[0];

        auto newFile = source.getParentDirectory()
                             .getNonexistentChildFile (source.getFileNameWithoutExtension(),
                                                       fileExtension, true);

        if (pp->isReadOnly())
        {
            engine.getUIBehaviour().showWarningMessage (TRANS("Can't create a new item because this project is read-only"));
        }
        else if (source.copyFileTo (newFile))
        {
            return pp->createNewItem (newFile, getType(), newName, description, getCategory(), true);
        }
    }

    return {};
}

//==============================================================================
static void tokenise (const yup::String& s, yup::StringArray& toks)
{
    auto t = s.getCharPointer();
    auto start = t;

    while (! t.isEmpty())
    {
        if (! (t.isLetterOrDigit()))
        {
            if (t > start + 1)
                toks.add (yup::String (start, t));

            start = t;
        }

        ++t;
    }

    if (t.getAddress() > start.getAddress() + 1)
        toks.add (yup::String (start, t));
}

yup::StringArray ProjectItem::getSearchTokens() const
{
    yup::StringArray toks;
    tokenise (objectName, toks);
    tokenise (getDescription(), toks);
    return toks;
}

} // namespace tracktion::inline engine
