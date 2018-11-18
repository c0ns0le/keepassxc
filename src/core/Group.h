/*
 *  Copyright (C) 2010 Felix Geyer <debfx@fobos.de>
 *  Copyright (C) 2017 KeePassXC Team <team@keepassxc.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 or (at your option)
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KEEPASSX_GROUP_H
#define KEEPASSX_GROUP_H

#include <QImage>
#include <QPixmap>
#include <QPixmapCache>
#include <QPointer>

#include "core/CustomData.h"
#include "core/Database.h"
#include "core/Entry.h"
#include "core/TimeInfo.h"

class Group : public QObject
{
    Q_OBJECT

public:
    enum TriState
    {
        Inherit,
        Enable,
        Disable
    };
    enum MergeMode
    {
        Default, // Determine merge strategy from parent or fallback (Synchronize)
        Duplicate, // lossy strategy regarding deletions, duplicate older changes in a new entry
        KeepLocal, // merge history forcing local as top regardless of age
        KeepRemote, // merge history forcing remote as top regardless of age
        KeepNewer, // merge history
        Synchronize, // merge history keeping most recent as top entry and appling deletions
    };

    enum CloneFlag
    {
        CloneNoFlags = 0,
        CloneNewUuid = 1, // generate a random uuid for the clone
        CloneResetTimeInfo = 2, // set all TimeInfo attributes to the current time
        CloneIncludeEntries = 4, // clone the group entries
    };
    Q_DECLARE_FLAGS(CloneFlags, CloneFlag)

    struct GroupData
    {
        QString name;
        QString notes;
        int iconNumber;
        QUuid customIcon;
        TimeInfo timeInfo;
        bool isExpanded;
        QString defaultAutoTypeSequence;
        Group::TriState autoTypeEnabled;
        Group::TriState searchingEnabled;
        Group::MergeMode mergeMode;

        bool operator==(const GroupData& other) const;
        bool operator!=(const GroupData& other) const;
        bool equals(const GroupData& other, CompareItemOptions options) const;
    };

    Group();
    ~Group();

    static Group* createRecycleBin();

    const QUuid& uuid() const;
    const QString uuidToHex() const;
    QString name() const;
    QString notes() const;
    QImage icon() const;
    QPixmap iconPixmap() const;
    QPixmap iconScaledPixmap() const;
    int iconNumber() const;
    const QUuid& iconUuid() const;
    const TimeInfo& timeInfo() const;
    bool isExpanded() const;
    QString defaultAutoTypeSequence() const;
    QString effectiveAutoTypeSequence() const;
    Group::TriState autoTypeEnabled() const;
    Group::TriState searchingEnabled() const;
    Group::MergeMode mergeMode() const;
    bool resolveSearchingEnabled() const;
    bool resolveAutoTypeEnabled() const;
    Entry* lastTopVisibleEntry() const;
    bool isExpired() const;
    CustomData* customData();
    const CustomData* customData() const;

    bool equals(const Group* other, CompareItemOptions options) const;

    static const int DefaultIconNumber;
    static const int RecycleBinIconNumber;
    static CloneFlags DefaultCloneFlags;
    static Entry::CloneFlags DefaultEntryCloneFlags;
    static const QString RootAutoTypeSequence;

    Group* findChildByName(const QString& name);
    Entry* findEntryByUuid(const QUuid& uuid) const;
    Entry* findEntryByPath(const QString& entryPath);
    Group* findGroupByUuid(const QUuid& uuid);
    Group* findGroupByPath(const QString& groupPath);
    QStringList locate(const QString& locateTerm, const QString& currentPath = {"/"}) const;
    Entry* addEntryWithPath(const QString& entryPath);
    void setUuid(const QUuid& uuid);
    void setName(const QString& name);
    void setNotes(const QString& notes);
    void setIcon(int iconNumber);
    void setIcon(const QUuid& uuid);
    void setTimeInfo(const TimeInfo& timeInfo);
    void setExpanded(bool expanded);
    void setDefaultAutoTypeSequence(const QString& sequence);
    void setAutoTypeEnabled(TriState enable);
    void setSearchingEnabled(TriState enable);
    void setLastTopVisibleEntry(Entry* entry);
    void setExpires(bool value);
    void setExpiryTime(const QDateTime& dateTime);
    void setMergeMode(MergeMode newMode);

    bool canUpdateTimeinfo() const;
    void setUpdateTimeinfo(bool value);

    Group* parentGroup();
    const Group* parentGroup() const;
    void setParent(Group* parent, int index = -1);
    QStringList hierarchy() const;

    Database* database();
    const Database* database() const;
    QList<Group*> children();
    const QList<Group*>& children() const;
    QList<Entry*> entries();
    const QList<Entry*>& entries() const;
    QList<Entry*> entriesRecursive(bool includeHistoryItems = false) const;
    QList<const Group*> groupsRecursive(bool includeSelf) const;
    QList<Group*> groupsRecursive(bool includeSelf);
    QSet<QUuid> customIconsRecursive() const;

    Group* clone(Entry::CloneFlags entryFlags = DefaultEntryCloneFlags,
                 CloneFlags groupFlags = DefaultCloneFlags) const;

    void copyDataFrom(const Group* other);
    QString print(bool recursive = false, int depth = 0);

    void addEntry(Entry* entry);
    void removeEntry(Entry* entry);

signals:
    void dataChanged(Group* group);
    void aboutToAdd(Group* group, int index);
    void added();
    void aboutToRemove(Group* group);
    void removed();
    void aboutToMove(Group* group, Group* toGroup, int index);
    void moved();
    void modified();
    void entryAboutToAdd(Entry* entry);
    void entryAdded(Entry* entry);
    void entryAboutToRemove(Entry* entry);
    void entryRemoved(Entry* entry);
    void entryDataChanged(Entry* entry);

private slots:
    void updateTimeinfo();

private:
    template <class P, class V> bool set(P& property, const V& value);

    void setParent(Database* db);

    void connectDatabaseSignalsRecursive(Database* db);
    void cleanupParent();
    void recCreateDelObjects();

    Entry* findEntryByPathRecursive(const QString& entryPath, const QString& basePath);
    Group* findGroupByPathRecursive(const QString& groupPath, const QString& basePath);

    QPointer<Database> m_db;
    QUuid m_uuid;
    GroupData m_data;
    QPointer<Entry> m_lastTopVisibleEntry;
    QList<Group*> m_children;
    QList<Entry*> m_entries;

    QPointer<CustomData> m_customData;

    QPointer<Group> m_parent;

    bool m_updateTimeinfo;

    friend void Database::setRootGroup(Group* group);
    friend Entry::~Entry();
    friend void Entry::setGroup(Group* group);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Group::CloneFlags)

#endif // KEEPASSX_GROUP_H
