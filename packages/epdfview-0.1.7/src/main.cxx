// ePDFView - A lightweight PDF Viewer.
// Copyright (C) 2006, 2007, 2009 Emma's Software.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include <config.h>
#include <stdlib.h>
#include <gettext.h>
#include <locale.h>
#include <gtk/gtk.h>
#include "epdfview.h"
#include <MainView.h>

using namespace ePDFView;

struct LoadFileInfo
{
    gchar *fileName;
    MainPter *mainPter;
    PDFDocument *document;
};

static int
loadFileFromCommandLine (gpointer data)
{
    LoadFileInfo *info = static_cast<LoadFileInfo *> (data);

    info->mainPter->setOpenState (info->fileName, FALSE);
    info->document->load (info->fileName, NULL);

    g_free (info->fileName);
    delete info;

    return FALSE;
}

int
main (int argc, char **argv)
{
    // Enable NLS.
    setlocale (LC_ALL, "");
    bindtextdomain (PACKAGE, LOCALEDIR);
    bind_textdomain_codeset (PACKAGE, "UTF-8");
    textdomain (PACKAGE);
    // Create the command line options context.
    GOptionContext *optionContext = 
        g_option_context_new (_("[FILE] - view PDF documents"));
    g_option_context_add_group (optionContext, gtk_get_option_group (TRUE));
    GError *error = NULL;
    if ( !g_option_context_parse (optionContext, &argc, &argv, &error) )
    {
        g_critical ("Error parsing command line options: %s\n", error->message);
        g_error_free (error);
        exit (EXIT_FAILURE);
    }
    // Initialise the working thread.
    IJob::init ();
    // Initialise the GTK library.
    gtk_init (&argc, &argv);
    g_set_application_name (_("PDF Viewer"));
    // Create the main presenter.
    PDFDocument *document = new PDFDocument;
    MainPter *mainPter = new MainPter (document);
    // Create the main view.
    MainView *mainView = new MainView (mainPter);
    // Let know to the presenter which is its view.
    mainPter->setView (mainView);
    // Now check if we have additional parameters. Any additional parameter
    // will be a file name to open.
    if ( argc > 1 )
    {
        LoadFileInfo *info = new LoadFileInfo;
        info->mainPter = mainPter;
        info->document = document;
        info->fileName = g_strdup (argv[1]);
        g_idle_add (loadFileFromCommandLine, info);
    }

    gtk_main();

    // There's no need for us to delete the main view, as it's
    // the presenter's responsibility.
    delete mainPter;

    // Save the configuration.
    Config::getConfig().save ();

    g_option_context_free (optionContext);

    // All done!.
    return EXIT_SUCCESS;
}
