/*
 * This file is part of xPlay.
 *
 * xPlay is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * xPlay is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "xPlayerMusicLibraryWidget.h"
#include "xPlayerUI.h"
#include "xMusicFile.h"

#include <QApplication>
#include <QHeaderView>
#include <QMenu>
#include <QDebug>

xPlayerMusicLibraryWidgetItem::xPlayerMusicLibraryWidgetItem(const xMusicDirectory& entry, xPlayerMusicLibraryWidgetItem* parent):
        QTreeWidgetItem(QStringList(entry.name())),
        itemMusicDirectory(entry),
        itemMusicFile(nullptr),
        itemTotalSize(0),
        itemParent(parent) {
    setTextAlignment(1, Qt::AlignRight);
    itemSaveBackground = background(0);
}

xPlayerMusicLibraryWidgetItem::xPlayerMusicLibraryWidgetItem(xMusicFile* file, xPlayerMusicLibraryWidgetItem* parent):
        QTreeWidgetItem(QStringList(file->getTrackName())),
        itemMusicDirectory(),
        itemMusicFile(file),
        itemTotalSize(0),
        itemParent(parent) {
    setTextAlignment(1, Qt::AlignRight);
    itemSaveBackground = background(0);
}

std::uintmax_t xPlayerMusicLibraryWidgetItem::getTotalSize() const {
    return itemTotalSize;
}

void xPlayerMusicLibraryWidgetItem::updateTotalSize(std::uintmax_t size) {
    double totalSize = static_cast<double>(size) / 1024.0;
    // Switch over to the next level if we need more than 4 digits.
    if (totalSize < 10000) {
        setText(1, QString("%1 KB").arg(totalSize, -1, 'f', 2));
    } else {
        totalSize /= 1024.0;
        if (totalSize < 10000) {
            setText(1, QString("%1 MB").arg(totalSize, -1, 'f', 2));
        } else {
            totalSize /= 1024.0;
            setText(1, QString("%1 GB").arg(totalSize, -1, 'f', 2));
        }
    }
    // Cache size.
    itemTotalSize = size;
}

void xPlayerMusicLibraryWidgetItem::mark(const Qt::BrushStyle& brushStyle, bool children) {
    QBrush currentBackground;
    // Mark item.
    currentBackground = background(0);
    currentBackground.setStyle(brushStyle);
    setBackground(0, currentBackground);
    if (children) {
        for (int i = 0; i < childCount(); ++i) {
            // Mark children.
            auto entryChild = child(i);
            currentBackground = entryChild->background(0);
            currentBackground.setStyle(brushStyle);
            entryChild->setBackground(0, currentBackground);
            for (int j = 0; j < entryChild->childCount(); ++j) {
                // Mark grand children.
                auto entryGrandChild = entryChild->child(j);
                currentBackground = entryGrandChild->background(0);
                currentBackground.setStyle(brushStyle);
                entryGrandChild->setBackground(0, currentBackground);
            }
        }
    }
}

void xPlayerMusicLibraryWidgetItem::mark(const QBrush& brush, bool children) {
    // Mark item.
    setBackground(0, brush);
    itemSaveBackground = brush;
    if (children) {
        for (int i = 0; i < childCount(); ++i) {
            // Mark children.
            auto entryChild = reinterpret_cast<xPlayerMusicLibraryWidgetItem*>(child(i));
            entryChild->setBackground(0, brush);
            entryChild->itemSaveBackground = brush;
            for (int j = 0; j < entryChild->childCount(); ++j) {
                // Mark grand children.
                auto entryGrandChild = reinterpret_cast<xPlayerMusicLibraryWidgetItem*>(entryChild->child(j));
                entryGrandChild->setBackground(0, brush);
                entryGrandChild->itemSaveBackground = brush;
            }
        }
    }
}

void xPlayerMusicLibraryWidgetItem::restoreMark(bool children) {
    // Restore marking for item.
    setBackground(0, itemSaveBackground);
    if (children) {
        for (int i = 0; i < childCount(); ++i) {
            // Restore marking for children.
            auto entryChild = reinterpret_cast<xPlayerMusicLibraryWidgetItem*>(child(i));
            entryChild->setBackground(0, entryChild->itemSaveBackground);
            for (int j = 0; j < entryChild->childCount(); ++j) {
                // Restore marking for grand children.
                auto entryGrandChild = reinterpret_cast<xPlayerMusicLibraryWidgetItem*>(entryChild->child(j));
                entryGrandChild->setBackground(0, entryGrandChild->itemSaveBackground);
            }
        }
    }
}

void xPlayerMusicLibraryWidgetItem::clearTotalSize() {
    setText(1, QString());
}

bool xPlayerMusicLibraryWidgetItem::isParentOf(xPlayerMusicLibraryWidgetItem* item) const {
    if (item == nullptr) {
        return false;
    }
    for (int i = 0; i < this->childCount(); ++i) {
        auto child = this->child(i);
        if (reinterpret_cast<xPlayerMusicLibraryWidgetItem*>(child) == item) {
            return true;
        }
        for (int j = 0; j < child->childCount(); ++j) {
            auto grandChild = child->child(j);
            if (reinterpret_cast<xPlayerMusicLibraryWidgetItem*>(grandChild) == item) {
                return true;
            }
        }
    }
    return false;
}

bool xPlayerMusicLibraryWidgetItem::isChildOf(xPlayerMusicLibraryWidgetItem* item) const {
    if (item == nullptr) {
        return false;
    }
    if (itemParent) {
        if (itemParent == item) {
            return true;
        } else {
            return (itemParent->itemParent == item);
        }
    } else {
        return false;
    }
}

xPlayerMusicLibraryWidgetItem* xPlayerMusicLibraryWidgetItem::artist() const {
    if (itemMusicFile) {
        if (itemParent) {
            return itemParent->itemParent;
        }
        return nullptr;
    } else {
        return itemParent;
    }
}

xPlayerMusicLibraryWidgetItem* xPlayerMusicLibraryWidgetItem::album() const {
    if (itemMusicFile) {
        return itemParent;
    }
    return nullptr;
}

xMusicFile* xPlayerMusicLibraryWidgetItem::musicFile() const {
    return itemMusicFile;
}

const xMusicDirectory& xPlayerMusicLibraryWidgetItem::musicDirectory() const {
    return itemMusicDirectory;
}

QString xPlayerMusicLibraryWidgetItem::description() const {
    QString desc;
    if (itemMusicFile) {
        desc = QString("%1/%2/%3").arg(itemMusicFile->getArtist(), itemMusicFile->getAlbum(), itemMusicFile->getTrackName());
    } else {
        if (itemParent) {
            desc = QString("%1/%2").arg(itemParent->musicDirectory().name(), itemMusicDirectory.name());
        } else {
            desc = itemMusicDirectory.name();
        }
    }
    if (!text(1).isEmpty()) {
        return QString("%1 - %2").arg(desc, text(1));
    } else {
        return desc;
    }
}


xPlayerMusicLibraryWidget::xPlayerMusicLibraryWidget(xMusicLibrary* library, const QString& name, QWidget* parent):
        QGroupBox(name, parent),
        musicLibrary(library),
        musicLibraryReady(false),
        musicLibrarySortBySize(false),
        musicLibraryItemBackground() {

    setFlat(xPlayerUseFlatGroupBox);
    musicLibraryTree = new QTreeWidget(this);
    musicLibraryTree->setColumnCount(2);
    musicLibraryTree->setRootIsDecorated(true);
    musicLibraryTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    musicLibraryTree->header()->setStretchLastSection(false);
    musicLibraryTree->header()->setVisible(false);
    musicLibraryTree->setContextMenuPolicy(Qt::CustomContextMenu);
    musicLibraryTree->setColumnWidth(1, QFontMetrics(QApplication::font()).width("99999.99 GB"));
    auto boxLayout = new QVBoxLayout();
    boxLayout->addWidget(musicLibraryTree);
    setLayout(boxLayout);

    connect(musicLibrary, &xMusicLibrary::scanningFinished, this, &xPlayerMusicLibraryWidget::scanningFinished);
    connect(musicLibraryTree, &QTreeWidget::customContextMenuRequested, this, &xPlayerMusicLibraryWidget::openContextMenu);
    connect(musicLibraryTree, &QTreeWidget::itemClicked, [=](QTreeWidgetItem* item) {
        if (item) {
            if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)) {
                emit treeItemCtrlClicked(reinterpret_cast<xPlayerMusicLibraryWidgetItem *>(item));
            }
        }
    });
}

void xPlayerMusicLibraryWidget::setBaseDirectory(const std::filesystem::path &base) {
    musicLibraryReady = false;
    musicLibrary->setBaseDirectory(base);
}

void xPlayerMusicLibraryWidget::setSortBySize(bool enabled) {
    musicLibrarySortBySize = enabled;
    updateMusicLibraryTree();
}

bool xPlayerMusicLibraryWidget::isReady() const {
    return musicLibraryReady;
}

void xPlayerMusicLibraryWidget::selectItem(const xMusicDirectory& artist) {
    for (int i = 0; i < musicLibraryTree->topLevelItemCount(); ++i) {
        auto artistItem = reinterpret_cast<xPlayerMusicLibraryWidgetItem*>(musicLibraryTree->topLevelItem(i));
        if (artistItem->musicDirectory().name() == artist.name()) {
            musicLibraryTree->setCurrentItem(artistItem);
            musicLibraryTree->scrollToItem(artistItem);
            return;
        }
    }
}

void xPlayerMusicLibraryWidget::selectItem(const xMusicDirectory& artist, const xMusicDirectory& album) {
    for (int i = 0; i < musicLibraryTree->topLevelItemCount(); ++i) {
        auto artistItem = reinterpret_cast<xPlayerMusicLibraryWidgetItem*>(musicLibraryTree->topLevelItem(i));
        if (artistItem->musicDirectory().name() == artist.name()) {
            for (int j = 0; j < artistItem->childCount(); ++j) {
                auto albumItem = reinterpret_cast<xPlayerMusicLibraryWidgetItem*>(artistItem->child(j));
                if (albumItem->musicDirectory().name() == album.name()) {
                    musicLibraryTree->setCurrentItem(albumItem);
                    return;
                }
            }

        }
    }
}

xPlayerMusicLibraryWidgetItem* xPlayerMusicLibraryWidget::itemAt(const QPoint& point) const {
    auto item = musicLibraryTree->itemAt(point);
    if (item) {
        return reinterpret_cast<xPlayerMusicLibraryWidgetItem*>(item);
    } else {
        return nullptr;
    }
}

void xPlayerMusicLibraryWidget::markItems(const std::list<xMusicDirectory>& missingArtists,
                                          const std::map<xMusicDirectory, std::list<xMusicDirectory>>& missingAlbums,
                                          const std::list<xMusicFile*>& missingTracks,
                                          const std::list<xMusicFile*>& differentTracks) {
    // Remove any markings.
    clearItems();
    // Mark the missing artists.
    for (const auto& artist : missingArtists) {
        auto artistItem = mapArtists.find(artist);
        if (artistItem != mapArtists.end()) {
            artistItem->second->mark(QBrush(Qt::red, Qt::Dense4Pattern), true);
        }
    }
    // Mark the missing albums.
    for (const auto& artist : missingAlbums) {
        auto albumsForArtist = mapAlbums.find(artist.first);
        if (albumsForArtist != mapAlbums.end()) {
            for (const auto& album : artist.second) {
                auto albumItem = albumsForArtist->second.find(album);
                if (albumItem != albumsForArtist->second.end()) {
                    // Color the missing albums (recursivly) with a denser pattern.
                    albumItem->second->mark(QBrush(Qt::red, Qt::Dense4Pattern), true);
                    // The corresponding missingArtists are colored with lighter pattern.
                    albumItem->second->artist()->mark(QBrush(Qt::red, Qt::Dense6Pattern));
                }
            }
        }
    }
    // Mark the missing tracks.
    for (auto track : missingTracks) {
        auto trackItem = mapTracks.find(track);
        if (trackItem != mapTracks.end()) {
            // Color the missing missingTracks with a denser pattern.
            trackItem->second->mark(QBrush(Qt::red, Qt::Dense4Pattern));
            // The corresponding artist and album are colored with lighter pattern.
            trackItem->second->artist()->mark(QBrush(Qt::red, Qt::Dense6Pattern));
            trackItem->second->album()->mark(QBrush(Qt::red, Qt::Dense6Pattern));
        }
    }
    // mark the different tracks.
    for (const auto& track : differentTracks) {
        auto trackItem = mapTracks.find(track);
        if (trackItem != mapTracks.end()) {
            // Color the missing missingTracks with a denser pattern.
            trackItem->second->mark(QBrush(Qt::yellow, Qt::Dense4Pattern));
            // The corresponding artist and album are colored with lighter pattern.
            trackItem->second->artist()->mark(QBrush(Qt::red, Qt::Dense6Pattern));
            trackItem->second->album()->mark(QBrush(Qt::red, Qt::Dense6Pattern));
        }
    }
}

void xPlayerMusicLibraryWidget::markExistingItems(const std::map<xMusicDirectory, std::map<xMusicDirectory,
                                                  std::list<xMusicFile*>>>& existingTracks) {
    // Go through the map and mark all items.
    for (const auto& artist : existingTracks) {
        auto albumsForArtist = mapAlbums.find(artist.first);
        if (albumsForArtist != mapAlbums.end()) {
            for (const auto& album : artist.second) {
                auto albumItem = albumsForArtist->second.find(album.first);
                if (albumItem != albumsForArtist->second.end()) {
                    // Color the existing artist and album with a denser pattern.
                    albumItem->second->mark(QBrush(Qt::green, Qt::Dense4Pattern));
                    albumItem->second->artist()->mark(QBrush(Qt::green, Qt::Dense4Pattern));
                    for (auto track : album.second) {
                        auto trackItem = mapTracks.find(track);
                        if (trackItem != mapTracks.end()) {
                            trackItem->second->mark(QBrush(Qt::green, Qt::Dense4Pattern));
                        }
                    }
                }
            }
        }
    }
}

void xPlayerMusicLibraryWidget::clearItems() {
    // Remove all markings.
    for  (int i = 0; i < musicLibraryTree->topLevelItemCount(); ++i) {
        clearItem(musicLibraryTree->topLevelItem(i), true);
    }
}

void xPlayerMusicLibraryWidget::clearItem(QTreeWidgetItem* item, bool clearChildren) {
    // Use mark with default background in order to update save brush.
    auto libraryItem = reinterpret_cast<xPlayerMusicLibraryWidgetItem*>(item);
    libraryItem->mark(musicLibraryItemBackground);
    if (clearChildren) {
        for (int i = 0; i < item->childCount(); ++i) {
            // Clear children.
            auto entryChild = reinterpret_cast<xPlayerMusicLibraryWidgetItem*>(item->child(i));
            entryChild->mark(musicLibraryItemBackground);
            for (int j = 0; j < entryChild->childCount(); ++j) {
                // Clear grand children.
                auto entryGrandChild = reinterpret_cast<xPlayerMusicLibraryWidgetItem*>(entryChild->child(j));
                entryGrandChild->mark(musicLibraryItemBackground);
            }
        }
    }
}

void xPlayerMusicLibraryWidget::clear() {
    // Clear the QTreeWidget.
    musicLibraryTree->clear();
    // Clear all mappings.
    mapArtists.clear();
    mapAlbums.clear();
    mapTracks.clear();
}

void xPlayerMusicLibraryWidget::scanningFinished() {
    // Cleanup everything.
    clear();
    // Create new music library tree.
    auto artists = musicLibrary->getLibraryFiles()->getArtists();
    for (const auto& artist : artists) {
        auto artistItem = new xPlayerMusicLibraryWidgetItem(artist);
        artistItem->updateTotalSize(musicLibrary->getLibraryFiles()->getTotalSize(artist));
        mapArtists[artist] = artistItem;
        std::map<xMusicDirectory, xPlayerMusicLibraryWidgetItem*> mapAlbumsForArtist;
        auto albums = musicLibrary->getLibraryFiles()->getAlbums(artist);
        for (const auto& album : albums) {
            auto albumItem = new xPlayerMusicLibraryWidgetItem(album, artistItem);
            albumItem->updateTotalSize(musicLibrary->getLibraryFiles()->getTotalSize(artist, album));
            artistItem->addChild(albumItem);
            mapAlbumsForArtist[album] = albumItem;
            auto tracks = musicLibrary->getLibraryFiles()->getMusicFiles(artist, album);
            for (auto track : tracks) {
                auto trackItem = new xPlayerMusicLibraryWidgetItem(track, albumItem);
                mapTracks[track] = trackItem;
                albumItem->addChild(trackItem);
            }
            albumItem->setExpanded(false);
        }
        artistItem->setExpanded(true);
        mapAlbums[artist] = mapAlbumsForArtist;
    }
    // Widget now ready for business...
    musicLibraryReady = true;
    updateMusicLibraryTree();
}

void xPlayerMusicLibraryWidget::updateMusicLibraryTree() {
    // Clear music library tree. Do not call clear as it will release the items itself.
    for (int index = musicLibraryTree->topLevelItemCount()-1; index >= 0; --index) {
        // We do not need to store the items as they are already in mapArtists.
        musicLibraryTree->takeTopLevelItem(index);
    }

    if (musicLibrarySortBySize) {
        for (const auto& artist : mapArtists) {
            int insertIndex;
            for (insertIndex = 0; insertIndex < musicLibraryTree->topLevelItemCount(); ++insertIndex) {
                auto insertItem = reinterpret_cast<xPlayerMusicLibraryWidgetItem*>(musicLibraryTree->topLevelItem(insertIndex));
                if (artist.second->getTotalSize() > insertItem->getTotalSize()) {
                    musicLibraryTree->insertTopLevelItem(insertIndex, artist.second);
                    break;
                }
            }
            if (insertIndex >= musicLibraryTree->topLevelItemCount()) {
                musicLibraryTree->addTopLevelItem(artist.second);
            }
            artist.second->setExpanded(true);
        }

    } else {
        for (const auto& artist : mapArtists) {
            musicLibraryTree->addTopLevelItem(artist.second);
            artist.second->setExpanded(true);
        }
    }
    // Save background.
    if (musicLibraryTree->topLevelItemCount() > 0) {
        musicLibraryItemBackground = musicLibraryTree->topLevelItem(0)->background(0);
    }
}

void xPlayerMusicLibraryWidget::openContextMenu(const QPoint &point) {
    // Emit treeItemCtrlClicked.
    if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)) {
        auto item = musicLibraryTree->itemAt(point);
        if (item) {
            emit treeItemCtrlRightClicked(reinterpret_cast<xPlayerMusicLibraryWidgetItem *>(item));
        }
        return;
    }

    QMenu contextMenu;
    // Add section for artist info website.
    contextMenu.addSection(tr("Display Options"));
    contextMenu.addAction(tr("Artists Only"), this, [=] () {
        musicLibraryTree->collapseAll();
    });
    contextMenu.addAction(tr("Artists and Albums"), this, [=] () {
        // Expand only to the level of artist and albums.
        for (int i = 0; i < musicLibraryTree->topLevelItemCount(); ++i) {
            auto artist = musicLibraryTree->topLevelItem(i);
            for (int j = 0; j < artist->childCount(); ++j) {
                artist->child(j)->setExpanded(false);
            }
            artist->setExpanded(true);
        }
    });
    contextMenu.addAction(tr("Artists, Albums and Tracks"), this, [=] () {
        musicLibraryTree->expandAll();
    });
    contextMenu.exec(mapToGlobal(point));
}
