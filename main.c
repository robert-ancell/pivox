/*
 * Copyright (C) 2018 Robert Ancell
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version. See http://www.gnu.org/copyleft/gpl.html the full text of the
 * license.
 */

#include "pv-application.h"

int
main (int argc, char **argv)
{
    PvApplication *app = pv_application_new ();
    return g_application_run (G_APPLICATION (app), argc, argv);
}
