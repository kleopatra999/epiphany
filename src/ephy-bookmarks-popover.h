/*
 * Copyright (C) 2016 Iulian-Gabriel Radu <iulian.radu67@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EPHY_BOOKMARKS_POPOVER_H
#define EPHY_BOOKMARKS_POPOVER_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define EPHY_TYPE_BOOKMARKS_POPOVER (ephy_bookmarks_popover_get_type())

G_DECLARE_FINAL_TYPE (EphyBookmarksPopover, ephy_bookmarks_popover, EPHY, BOOKMARKS_POPOVER, GtkPopover)

EphyBookmarksPopover*       ephy_bookmarks_popover_new      (void);

G_END_DECLS

#endif /* EPHY_BOOMARKS_POPOVER_H */