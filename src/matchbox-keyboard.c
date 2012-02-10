/*
 *  Matchbox Keyboard - A lightweight software keyboard.
 *
 *  Authored By Matthew Allum <mallum@o-hand.com>
 *
 *  Copyright (c) 2005-2012 Intel Corp
 *  Copyright (c) 2012 Vernier Software & Technology
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms and conditions of the GNU Lesser General Public License,
 *  version 2.1, as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 *  more details.
 *
 */

#include "matchbox-keyboard.h"

#define FONT_FAMILY_LEN 100

static void
mb_kbd_usage (char *progname)
{
  fprintf(stderr, "Usage:\n   %s [Options ] [ Layout Variant ]\n", progname);
  fprintf(stderr, "\nOptions are;\n"
	  "   -xid,--xid            Print window ID to stdout ( for embedding )\n"
	  "   -d,--daemon           Run in 'daemon' mode (for remote control)\n"
	  "   -o,--orientation <portrait|landscape>\n"
          "                         Use to limit visibility with screen orientation \n"
          "   --fontfamily <font family>\n"
          "                         Colon (:) delimited list of font family descriptor to use (ie. dejavu:sans)\n"
          "   --fontptsize <integer>\n"
          "                         Base font point size to use\n"
          "   --fontvariant <variant1:variant2>\n"
          "                         Colon (:) delimited list of Font variants to apply (ie. bold:mono:italic)\n");
  fprintf(stderr, "\nmatchbox-keyboard %s \nCopyright (C) 2007 OpenedHand Ltd.\n", VERSION);

  exit(-1);
}

MBKeyboard*
mb_kbd_new (int argc, char **argv)
{
  MBKeyboard *kb = NULL;
  char       *variant = NULL; 
  Bool        want_embedding = False, want_daemon = False;
  int         i;
  MBKeyboardDisplayOrientation orientation = MBKeyboardDisplayAny;

  kb = util_malloc0(sizeof(MBKeyboard));

  kb->key_border = 1;
  kb->key_pad    = 0;
  kb->key_margin = 0;

  kb->col_spacing = 0;
  kb->row_spacing = 0;

  kb->font_family  = strdup("sans");
  kb->font_pt_size = 8;
  kb->font_variant = strdup("bold");

  for (i = 1; i < argc; i++) 
    {
      if (streq ("-xid", argv[i]) || streq ("--xid", argv[i])) 
	{
	  want_embedding = True;
	  continue;
	}

      if (streq ("-d", argv[i]) || streq ("--daemon", argv[i])) 
	{
	  want_daemon = True;
	  continue;
	}

      if (streq ("--fontfamily", argv[i])) 
        {
          if (++i>=argc)
            {
              mb_kbd_usage (argv[0]);
            }
          else
            {
              char family[FONT_FAMILY_LEN];
              char *cp = strdup (argv[i]);
              const char *delimiter = ":";
              char *token = strtok (cp, delimiter);

              memset (family, 0, FONT_FAMILY_LEN);

              while (NULL != token)
                {
                  strncat (family, token, (FONT_FAMILY_LEN - strlen (family)));
                  token = strtok (NULL, delimiter);
                  if (NULL != token)
                    strncat (family, " ", (FONT_FAMILY_LEN - strlen (family)));
                }

              kb->font_family  = strndup(family, 100);
              free (cp);
            }
          continue;
        }
      
      if (streq ("--fontptsize", argv[i])) 
        {
          if (++i>=argc) mb_kbd_usage (argv[0]);
          kb->font_pt_size = strtol(argv[i], NULL, 0);
          continue;
        }

      if (streq ("--fontvariant", argv[i])) 
        {
          if (++i>=argc) mb_kbd_usage (argv[0]);
          kb->font_variant = strdup(argv[i]);
          continue;
        }
      
      if (streq ("-o", argv[i]) || streq ("--orientation", argv[i]))
	{
	  if (++i>=argc) mb_kbd_usage (argv[0]);

	  if (streq(argv[i], "portrait"))
	    {
	      orientation = MBKeyboardDisplayPortrait;
	    }
	  else if (streq(argv[i], "landscape"))
	    {
	      orientation = MBKeyboardDisplayLandscape;
	    }
	  else
	    mb_kbd_usage (argv[0]);

	  continue;
	}

      if (i == (argc-1) && argv[i][0] != '-')
	variant = argv[i];
      else
	mb_kbd_usage(argv[0]);
    }

  if (variant == NULL)
    variant = getenv("MB_KBD_VARIANT");

  if (!mb_kbd_ui_init(kb))
    return NULL;

  if (!mb_kbd_config_load(kb, variant))
    return NULL;

  kb->selected_layout 
    = (MBKeyboardLayout *)util_list_get_nth_data(kb->layouts, 0);

  if (want_embedding)
    mb_kbd_ui_set_embeded (kb->ui, True);

  if (want_daemon)
    {
      mb_kbd_ui_set_daemon (kb->ui, True);
      if (orientation != MBKeyboardDisplayAny)
	mb_kbd_ui_limit_orientation (kb->ui, orientation);
    }

  if (!mb_kbd_ui_realize(kb->ui))
    return NULL;

  if (want_embedding)
    mb_kbd_ui_print_window (kb->ui);

  //fprintf (stderr, "***** Settings: font_family: %s font_pt_size: %d font_variant: %s\n", kb->font_family, kb->font_pt_size, kb->font_variant);

  return kb;
}

int
mb_kbd_row_spacing(MBKeyboard *kb)
{
  return kb->row_spacing;
}

int
mb_kbd_col_spacing(MBKeyboard *kb)
{
  return kb->col_spacing;
}

int
mb_kbd_keys_border(MBKeyboard *kb)
{
  return kb->key_border;
}

int
mb_kbd_keys_pad(MBKeyboard *kb)
{
  return kb->key_pad;
}

int
mb_kbd_keys_margin(MBKeyboard *kb)
{
  return kb->key_margin;
}

void
mb_kbd_add_state(MBKeyboard *kbd, MBKeyboardStateType state)
{
  kbd->keys_state |= state;
}

void
mb_kbd_toggle_state(MBKeyboard *kbd, MBKeyboardStateType state)
{
  kbd->keys_state ^= state;
}

boolean
mb_kbd_has_state(MBKeyboard *kbd, MBKeyboardStateType state)
{
  return (kbd->keys_state & state);
}

boolean
mb_kbd_has_any_state(MBKeyboard *kbd)
{
  return  (kbd->keys_state > 0);
}

void
mb_kbd_remove_state(MBKeyboard *kbd, MBKeyboardStateType state)
{
  kbd->keys_state &= ~(state);
}

MBKeyboardKeyStateType
mb_kbd_keys_current_state(MBKeyboard *kbd)
{
  if (mb_kbd_has_state(kbd, MBKeyboardStateShifted))
    return MBKeyboardKeyStateShifted;

  if (mb_kbd_has_state(kbd, MBKeyboardStateMod1))
    return MBKeyboardKeyStateMod1;

  if (mb_kbd_has_state(kbd, MBKeyboardStateMod2))
    return MBKeyboardKeyStateMod2;
  
  if (mb_kbd_has_state(kbd, MBKeyboardStateMod3))
    return MBKeyboardKeyStateMod3;
  
  return MBKeyboardKeyStateNormal;
}

void
mb_kbd_redraw(MBKeyboard *kb)
{
  mb_kbd_ui_redraw(kb->ui);
}

void
mb_kbd_redraw_key(MBKeyboard *kb, MBKeyboardKey *key)
{
  mb_kbd_ui_redraw_key(kb->ui, key);
  mb_kbd_ui_swap_buffers(kb->ui);
}

MBKeyboardKey*
mb_kbd_locate_key(MBKeyboard *kb, int x, int y)
{
  MBKeyboardLayout *layout;
  List             *row_item, *key_item;

  layout = mb_kbd_get_selected_layout(kb);

  row_item = mb_kbd_layout_rows(layout);

  while (row_item != NULL)
    {
      MBKeyboardRow *row = row_item->data;

      if (x >= mb_kbd_row_x(row) 
	  && x <= mb_kbd_row_x(row) + mb_kbd_row_width(row) 
	  && y >= mb_kbd_row_y(row)
	  && y <= mb_kbd_row_y(row) + mb_kbd_row_height(row) )
	{
	  mb_kbd_row_for_each_key(row, key_item) 
	    {
	      MBKeyboardKey *key = key_item->data;

	      if (!mb_kbd_is_extended(kb) 
		      && mb_kbd_key_get_extended(key))
		continue;

	      if (!mb_kbd_key_is_blank(key)
		  && x >= mb_kbd_key_abs_x(key)
		  && x <= mb_kbd_key_abs_x(key) + mb_kbd_key_width(key))
		return key;

	    }
	 
	  return NULL;
	}

      row_item = util_list_next(row_item);
    }
  return NULL;
}

void
mb_kbd_add_layout(MBKeyboard *kb, MBKeyboardLayout *layout)
{
  kb->layouts = util_list_append(kb->layouts, (pointer)layout);
}

MBKeyboardLayout*
mb_kbd_get_selected_layout(MBKeyboard *kb)
{
  return kb->selected_layout;
}

void
mb_kbd_set_held_key(MBKeyboard *kb, MBKeyboardKey *key)
{
  kb->held_key = key;
}

MBKeyboardKey *
mb_kbd_get_held_key(MBKeyboard *kb)
{
  return kb->held_key;
}

void
mb_kbd_set_extended(MBKeyboard *kb, boolean extend)
{
  kb->extended = extend;
}

boolean
mb_kbd_is_extended(MBKeyboard *kb)
{
  return kb->extended;
}


void
mb_kbd_run(MBKeyboard *kb)
{
  mb_kbd_ui_event_loop(kb->ui);
}


int 
main(int argc, char **argv)
{
  MBKeyboard *kb;

  kb = mb_kbd_new(argc, argv);

  if (kb) mb_kbd_run(kb);

  return 0;
}
