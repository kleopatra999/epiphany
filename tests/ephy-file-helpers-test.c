/* vim: set sw=2 ts=2 sts=2 et: */
/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * ephy-file-helpers-test.c
 * This file is part of Epiphany
 *
 * Copyright © 2012 - Igalia S.L.
 *
 * Epiphany is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Epiphany is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Epiphany; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include "config.h"
#include "ephy-debug.h"
#include "ephy-file-helpers.h"
#include "ephy-settings.h"

#include <glib.h>
#include <gtk/gtk.h>

typedef struct {
  const char *dir;
  gboolean private;
  gboolean keep_tmp;
} FileInitTest;

static const FileInitTest private_tests[] =
{
  { NULL, TRUE, FALSE },
  { NULL, TRUE, TRUE }
};

static void
test_ephy_file_helpers_init ()
{
  int i;

  for (i = 0; i < G_N_ELEMENTS (private_tests); i++) {
    FileInitTest test;

    char *tmp_dir = NULL;
    char *dot_dir = NULL;

    test = private_tests[i];
    g_test_message ("INIT: dir: %s; private: %s; keep-tmp: %s;",
                    test.dir,
                    test.private ? "TRUE" : "FALSE",
                    test.keep_tmp ? "TRUE" : "FALSE");

    g_assert (ephy_dot_dir () == NULL);
    g_assert (ephy_file_helpers_init (NULL, test.private, test.keep_tmp, NULL));

    tmp_dir = g_strdup (ephy_file_tmp_dir ());
    dot_dir = g_strdup (ephy_dot_dir ());

    g_assert (tmp_dir != NULL);
    g_assert (dot_dir != NULL);

    /* These should always exist after init. */
    g_assert (g_file_test (tmp_dir, G_FILE_TEST_EXISTS));
    g_assert (g_file_test (dot_dir, G_FILE_TEST_EXISTS));

    ephy_file_helpers_shutdown ();

    /* Private profiles have their dot_dir inside tmp_dir. */
    g_assert (g_file_test (tmp_dir, G_FILE_TEST_EXISTS) == test.keep_tmp);
    g_assert (g_file_test (dot_dir, G_FILE_TEST_EXISTS) == test.keep_tmp);

    /* Cleanup tmp-dir left behind. */
    if (test.keep_tmp) {
      GFile *file;

      file = g_file_new_for_path (tmp_dir);
      /* As a safety measure, only try recursive delete on paths
       * prefixed with /tmp. */
      if (g_str_has_prefix (tmp_dir, "/tmp"))
        g_assert (ephy_file_delete_dir_recursively (file, NULL));
      else
        g_warning ("INIT: dangerous path returned as tmp_dir: %s", tmp_dir);

      g_object_unref (file);
    }

    g_free (tmp_dir);
    g_free (dot_dir);
  }
}

typedef struct {
  const char *key_value;
  const char *expected;
  GUserDirectory user_dir;
} DownloadsDirTest;

static const DownloadsDirTest downloads_tests[] =
{
  { "Desktop", NULL, G_USER_DIRECTORY_DESKTOP },

  { "Downloads", NULL, G_USER_DIRECTORY_DOWNLOAD },
  { "invalid-keyword", NULL, G_USER_DIRECTORY_DOWNLOAD },

  { "/tmp/Downloads", "/tmp/Downloads", -1 },
};

static void
test_ephy_file_get_downloads_dir ()
{
  int i;

  ephy_file_helpers_init (NULL, TRUE, FALSE, NULL);

  for (i = 0; i < G_N_ELEMENTS (downloads_tests); i++) {
    DownloadsDirTest test;
    char *downloads_dir = NULL;
    const char *expected_dir = NULL;

    test = downloads_tests[i];

    if (test.expected != NULL)
      expected_dir = test.expected;
    else
      expected_dir = g_get_user_special_dir (test.user_dir);

    g_test_message ("DOWNLOADS: key_value: %s; expected: %s;",
                    test.key_value,
                    expected_dir);

    g_settings_set_string (EPHY_SETTINGS_STATE,
                           EPHY_PREFS_STATE_DOWNLOAD_DIR,
                           test.key_value);

    downloads_dir = ephy_file_get_downloads_dir ();
    g_assert_cmpstr (downloads_dir, ==, expected_dir);

    g_free (downloads_dir);
  }

  ephy_file_helpers_shutdown ();
}

typedef struct {
  const char *dir;
  gboolean exists;
  gboolean can_create;
  gboolean can_delete;
} DirTest;

static const DirTest dir_tests[] =
{
  { "/tmp", TRUE, FALSE, FALSE },
  { "/tmp/ephy-test-dir", FALSE, TRUE, TRUE },
  /* Unreadable */
  { "/root/ephy-test-dir", FALSE, FALSE, FALSE },
  /* Unwritable */
  { "/non-existant/ephy-test-dir", FALSE, FALSE, FALSE },
};

static void
test_ephy_file_create_delete_dir ()
{
  int i;

  ephy_file_helpers_init (NULL, TRUE, FALSE, NULL);

  for (i = 0; i < G_N_ELEMENTS (dir_tests); i++) {
    GFile *file = NULL;
    DirTest test;

    test = dir_tests[i];

    g_test_message ("DIR: testing for %s", test.dir);

    g_assert (g_file_test (test.dir, G_FILE_TEST_EXISTS) == test.exists);
    g_assert (ephy_ensure_dir_exists (test.dir, NULL) == (test.exists || test.can_create));
    g_assert (g_file_test (test.dir, G_FILE_TEST_EXISTS) == (test.exists || test.can_create));

    file = g_file_new_for_path (test.dir);

    g_assert (ephy_file_delete_dir_recursively (file, NULL) == test.can_delete);

    if (test.exists)
      g_assert (g_file_test (test.dir, G_FILE_TEST_EXISTS) != test.can_delete);

    g_object_unref (file);
  }

  ephy_file_helpers_shutdown ();
}

static void
test_ephy_file_desktop_dir ()
{
  char *desktop_dir;
  const char *xdg_desktop;

  ephy_file_helpers_init (NULL, TRUE, FALSE, NULL);

  desktop_dir = ephy_file_desktop_dir ();
  xdg_desktop = g_get_user_special_dir (G_USER_DIRECTORY_DESKTOP);
  g_assert_cmpstr (desktop_dir, ==, xdg_desktop);

  g_free (desktop_dir);

  ephy_file_helpers_shutdown ();
}

static void
test_ephy_file_create_delete_tmp ()
{
  char *tmp_file = NULL;
  char *tmp_path = NULL;
  char *tmp_uri = NULL;
  char *tmp_path_prefix = NULL;

  ephy_file_helpers_init (NULL, TRUE, FALSE, NULL);

  /* Fails because filename has no "XXXXXX" keyword. */
  tmp_file = ephy_file_tmp_filename ("test-ephy", NULL);
  tmp_path = g_build_filename (ephy_file_tmp_dir (), "test-ephy", NULL);

  g_assert (tmp_file == NULL);
  g_assert (g_file_test (tmp_path, G_FILE_TEST_EXISTS) == FALSE);

  g_free (tmp_path);
  g_free (tmp_file);

  /* Simple creation. */
  tmp_file = ephy_file_tmp_filename ("test-ephy-XXXXXX", NULL);
  tmp_path = g_build_filename (ephy_file_tmp_dir (), tmp_file, NULL);

  g_test_message ("TMP: %s", tmp_path);
  g_assert (g_file_test (tmp_path, G_FILE_TEST_EXISTS) == FALSE);
  g_assert (g_file_set_contents (tmp_path, "test", -1, NULL));
  g_assert (g_file_test (tmp_path, G_FILE_TEST_EXISTS));

  tmp_uri = g_filename_to_uri (tmp_path, NULL, NULL);
  ephy_file_delete_uri (tmp_uri);

  g_assert (g_file_test (tmp_path, G_FILE_TEST_EXISTS) == FALSE);

  g_free (tmp_uri);
  g_free (tmp_path);
  g_free (tmp_file);

  /* Creation with suffix. */
  tmp_file = ephy_file_tmp_filename ("test-ephy-XXXXXX", "test-ext");
  tmp_path = g_build_filename (ephy_file_tmp_dir (), tmp_file, NULL);

  g_assert (g_file_test (tmp_path, G_FILE_TEST_EXISTS) == FALSE);
  g_assert (g_file_set_contents (tmp_path, "test", -1, NULL));
  g_assert (g_file_test (tmp_path, G_FILE_TEST_EXISTS));

  tmp_path_prefix = g_build_filename (ephy_file_tmp_dir (), "test-ephy-", NULL);

  g_test_message ("TMP: %s", tmp_path);
  g_assert (g_str_has_suffix (tmp_path, "test-ext"));
  g_assert (g_str_has_prefix (tmp_path, tmp_path_prefix));

  tmp_uri = g_filename_to_uri (tmp_path, NULL, NULL);
  ephy_file_delete_uri (tmp_uri);

  g_assert (g_file_test (tmp_file, G_FILE_TEST_EXISTS) == FALSE);

  g_free (tmp_uri);
  g_free (tmp_path_prefix);
  g_free (tmp_path);
  g_free (tmp_file);

  ephy_file_helpers_shutdown ();
}

static void
test_ephy_file_switch_temp_file ()
{
  char *tmp_file;

  GFile *orig;
  char *orig_path;

  GFile *dest;
  char *dest_path;
  char *file_cont = NULL;

  ephy_file_helpers_init (NULL, TRUE, FALSE, NULL);

  /* Empty dest */
  tmp_file = ephy_file_tmp_filename ("test-dest-XXXXXX", NULL);
  dest_path = g_build_filename (ephy_file_tmp_dir (), tmp_file, NULL);
  g_free (tmp_file);

  g_assert (g_file_test (dest_path, G_FILE_TEST_EXISTS) == FALSE);
  dest = g_file_new_for_path (dest_path);

  tmp_file = ephy_file_tmp_filename ("test-orig-XXXXXX", NULL);
  orig_path = g_build_filename (ephy_file_tmp_dir (), tmp_file, NULL);
  g_free (tmp_file);

  g_assert (g_file_test (orig_path, G_FILE_TEST_EXISTS) == FALSE);
  orig = g_file_new_for_path (orig_path);

  g_file_set_contents (orig_path, "orig", -1, NULL);
  g_assert (g_file_test (orig_path, G_FILE_TEST_EXISTS));

  g_test_message ("SWITCH: %s to %s", orig_path, dest_path);

  g_assert (ephy_file_switch_temp_file (dest, orig));
  g_assert (g_file_test (orig_path, G_FILE_TEST_EXISTS) == FALSE);
  g_assert (g_file_test (dest_path, G_FILE_TEST_EXISTS));

  g_assert (g_file_get_contents (dest_path, &file_cont, NULL, NULL));
  g_assert_cmpstr ("orig", ==, file_cont);
  g_free (file_cont);

  ephy_file_delete_uri (g_file_get_uri (dest));
  g_assert (g_file_test (dest_path, G_FILE_TEST_EXISTS) == FALSE);

  /* Full replace */
  g_file_set_contents (dest_path, "dest", -1, NULL);
  g_assert (g_file_test (dest_path, G_FILE_TEST_EXISTS));

  g_file_set_contents (orig_path, "orig", -1, NULL);
  g_assert (g_file_test (orig_path, G_FILE_TEST_EXISTS));

  g_test_message ("SWITCH REPLACE: %s to %s", orig_path, dest_path);
  g_assert (ephy_file_switch_temp_file (dest, orig));
  g_assert (g_file_test (dest_path, G_FILE_TEST_EXISTS));
  g_assert (g_file_test (orig_path, G_FILE_TEST_EXISTS) == FALSE);

  g_assert (g_file_get_contents (dest_path, &file_cont, NULL, NULL));
  g_assert_cmpstr ("orig", ==, file_cont);
  g_free (file_cont);

  g_free (orig_path);
  g_free (dest_path);

  g_object_unref (orig);
  g_object_unref (dest);

  ephy_file_helpers_shutdown ();
}

int
main (int argc, char *argv[])
{
  int ret;

  /* This should affect only this test, we use it so we can test for
   * default directory changes. */
  g_setenv ("GSETTINGS_BACKEND", "memory", TRUE);
  /* Set our custom user-dirs.dirs, to control the output of
   * g_get_user_special_dir. The values there are the ones we should
   * check for in the test. */
  g_setenv ("XDG_CONFIG_HOME", TOP_SRC_DIR "/tests/data/", TRUE);

  gtk_test_init (&argc, &argv);

  ephy_debug_init ();

  g_test_add_func ("/lib/ephy-file-helpers/init",
                   test_ephy_file_helpers_init);

  g_test_add_func ("/lib/ephy-file-helpers/get_downloads_dir",
                   test_ephy_file_get_downloads_dir);

  g_test_add_func ("/lib/ephy-file-helpers/create_delete_dir",
                   test_ephy_file_create_delete_dir);

  g_test_add_func ("/lib/ephy-file-helpers/desktop_dir",
                   test_ephy_file_desktop_dir);

  g_test_add_func ("/lib/ephy-file-helpers/create_delete_tmp",
                   test_ephy_file_create_delete_tmp);

  g_test_add_func ("/lib/ephy-file-helpers/switch_temp_file",
                   test_ephy_file_switch_temp_file);

  ret = g_test_run ();

  return ret;
}