/*
 * Copyright (C) OpenTX
 *
 * Source:
 *  https://github.com/opentx/libopenui
 *
 * This file is a part of libopenui library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#include "textedit.h"
#include "font.h"
#include "libopenui_globals.h"
#include "libopenui_config.h"

#if !defined(STR_EDIT)
#define STR_EDIT "Edit"
#endif

#if !defined(STR_CLEAR)
#define STR_CLEAR "Clear"
#endif

#if defined(HARDWARE_TOUCH)
#include "keyboard_text.h"
#endif

#if defined(HARDWARE_KEYS)
#include "menu.h"
#endif

#if defined(CLIPBOARD)
#include "clipboard.h"
#endif


static lv_obj_t *virtual_kb = nullptr;

static void ta_event_cb(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *ta = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED) {
    if (virtual_kb == nullptr) {
      virtual_kb = lv_keyboard_create(lv_scr_act());
      lv_obj_set_size(virtual_kb, LV_HOR_RES, LV_VER_RES / 2);
    }

    if (virtual_kb != NULL) {
      lv_keyboard_set_textarea(virtual_kb, ta);
      lv_obj_add_state(ta, LV_STATE_EDITED);
    }
  } else if (code == LV_EVENT_DEFOCUSED || code == LV_EVENT_DELETE) {
    if (virtual_kb != nullptr) {
      lv_obj_del(virtual_kb);
      virtual_kb = nullptr;
    }
  }
}

static lv_style_t style_main;
static lv_style_t style_edit;
static LvglWidgetFactory textEditFactory = { lv_textarea_create, nullptr };

TextEdit::TextEdit(Window *parent, const rect_t &rect, char *value,
                   uint8_t length, LcdFlags windowFlags,
                   const char *_extra_chars) :
    FormField(parent, {rect.x, rect.y, rect.w, rect.h - 2}, windowFlags, 0,
              &textEditFactory),
    value(value),
    length(length)
{
  extra_chars = (_extra_chars) ? _extra_chars : extra_chars_default;

  // callbacks
  lv_obj_add_event_cb(lvobj, ta_event_cb, LV_EVENT_ALL, NULL);

  // properties
  lv_obj_set_scrollbar_mode(lvobj, LV_SCROLLBAR_MODE_OFF);
  lv_textarea_set_password_mode(lvobj, false);
  lv_textarea_set_one_line(lvobj, true);
  lv_textarea_set_text(lvobj, value);
  lv_textarea_set_placeholder_text(lvobj, "---");
  lv_textarea_set_max_length(lvobj, length);

  // LV_PART_MAIN
  lv_style_init(&style_main);
  lv_style_set_border_width(&style_main, 1);
  lv_style_set_border_color(&style_main, makeLvColor(COLOR_THEME_SECONDARY2));
  lv_style_set_bg_color(&style_main, makeLvColor(COLOR_THEME_PRIMARY2));
  lv_style_set_bg_opa(&style_main, LV_OPA_COVER);
  //lv_style_set_radius(&style_main, 0);
  lv_style_set_text_font(&style_main, &lv_font_montserrat_12);
  lv_style_set_text_color(&style_main, makeLvColor(COLOR_THEME_SECONDARY1));
  lv_obj_add_style(lvobj, &style_main, LV_PART_MAIN);

  // LV_STATE_FOCUSED
  lv_obj_set_style_bg_color(lvobj, makeLvColor(COLOR_THEME_FOCUS),
                            LV_PART_MAIN | LV_STATE_FOCUSED);

  lv_obj_set_style_text_color(lvobj, makeLvColor(COLOR_THEME_PRIMARY2),
                              LV_PART_MAIN | LV_STATE_FOCUSED);

  // Hide cursor
  lv_obj_set_style_opa(lvobj, 0, LV_PART_CURSOR);

  // Show Cursor in "Edit" mode
  lv_style_init(&style_edit);
  lv_style_set_opa(&style_edit, LV_OPA_COVER);
  lv_style_set_bg_opa(&style_edit, LV_OPA_COVER);
  lv_style_set_pad_left(&style_edit, (lv_coord_t)-(FIELD_PADDING_LEFT+2));
  lv_style_set_pad_top(&style_edit, (lv_coord_t)-(FIELD_PADDING_TOP+2));
  lv_obj_add_style(lvobj, &style_edit, LV_PART_CURSOR | LV_STATE_EDITED);

  // Text padding
  auto label = lv_textarea_get_label(lvobj);
  lv_obj_set_style_pad_left(label, FIELD_PADDING_LEFT, LV_PART_MAIN);
  lv_obj_set_style_pad_top(label, FIELD_PADDING_TOP, LV_PART_MAIN);
}

void TextEdit::paint(BitmapBuffer * dc)
{
//   FormField::paint(dc);

//   if (editMode) {
//     dc->drawSizedText(FIELD_PADDING_LEFT, FIELD_PADDING_TOP, value, length,
//                       COLOR_THEME_PRIMARY2);
//     coord_t left = (cursorPos == 0 ? 0 : getTextWidth(value, cursorPos));
// #if defined(SOFTWARE_KEYBOARD)
//     dc->drawSolidFilledRect(left + 2, 2, 2, height() - 4, COLOR_THEME_PRIMARY2);
// #else
//     char s[] = {value[cursorPos], '\0'};
//     dc->drawSolidFilledRect(FIELD_PADDING_LEFT + left - 1,
//                             FIELD_PADDING_TOP - 1, getTextWidth(s, 1) + 1,
//                             height() - 2, COLOR_THEME_PRIMARY2);
//     dc->drawText(FIELD_PADDING_LEFT + left, FIELD_PADDING_TOP, s,
//                  COLOR_THEME_SECONDARY1);
// #endif
//   } else {
//     const char * displayedValue = value;
//     LcdFlags textColor = COLOR_THEME_PRIMARY2;
//     if (hasFocus()) {
//       if (strlen(value) == 0) {
//         displayedValue = "---";
//       }
//       textColor = COLOR_THEME_PRIMARY2;
//     }
//     else {
//       if (strlen(value) == 0) {
//         displayedValue = "---";
//         textColor = COLOR_THEME_DISABLED;
//       }
//       else {
//         textColor = COLOR_THEME_SECONDARY1;
//       }
//     }
//     dc->drawSizedText(FIELD_PADDING_LEFT, FIELD_PADDING_TOP, displayedValue, length, textColor);
//   }
}

void TextEdit::trim()
{
  for (int i = length - 1; i >= 0; i--) {
    if (value[i] == ' ' || value[i] == '\0')
      value[i] = '\0';
    else
      break;
  }
}

void TextEdit::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

#if defined(HARDWARE_TOUCH)
  if (IS_VIRTUAL_KEY_EVENT(event)) {
    uint8_t c = event & 0xFF;
    if (c == (uint8_t)KEYBOARD_BACKSPACE[0]) {
      if (cursorPos > 0) {
        memmove(value + cursorPos - 1, value + cursorPos, length - cursorPos);
        value[length - 1] = '\0';
        --cursorPos;
        invalidate();
        changed = true;
      }
    }
    else if (c == (uint8_t)KEYBOARD_ENTER[0]) {
      changeEnd();
    }
    else if (cursorPos < length) {
      memmove(value + cursorPos + 1, value + cursorPos, length - cursorPos - 1);
      value[cursorPos++] = c;
      invalidate();
      changed = true;
    }
  }
#endif

#if defined(HARDWARE_KEYS)
  if (editMode) {
    int c = value[cursorPos];
    int v = c;

    switch (event) {
      case EVT_ROTARY_RIGHT:
        for (int i = 0; i < ROTARY_ENCODER_SPEED(); i++) {
          v = getNextChar(v);
        }
        break;

      case EVT_ROTARY_LEFT:
        for (int i = 0; i < ROTARY_ENCODER_SPEED(); i++) {
          v = getPreviousChar(v);
        }
        break;

      case EVT_KEY_BREAK(KEY_LEFT):
        if (cursorPos > 0) {
          cursorPos--;
          invalidate();
        }
        break;

      case EVT_KEY_BREAK(KEY_RIGHT):
        if (cursorPos < length - 1 && value[cursorPos + 1] != '\0') {
          cursorPos++;
          invalidate();
        }
        break;

      case EVT_KEY_BREAK(KEY_ENTER):
        if (cursorPos < length - 1) {
          if (value[cursorPos] == '\0') {
            value[cursorPos] = ' ';
            changed = true;
          }
          cursorPos++;
          if (value[cursorPos] == '\0') {
            value[cursorPos] = ' ';
            changed = true;
          }
          invalidate();
        }
        else {
          changeEnd();
          FormField::onEvent(event);
        }
        break;

      case EVT_KEY_BREAK(KEY_EXIT):
        changeEnd();
        FormField::onEvent(event);
        
#if defined(SOFTWARE_KEYBOARD)
        Keyboard::hide();
#endif
        setFocus(SET_FOCUS_DEFAULT, this);
        break;

      case EVT_KEY_LONG(KEY_ENTER):
      {
        killEvents(event);
        auto menu = new Menu(this);
        menu->setTitle(STR_EDIT);
#if defined(CLIPBOARD)
        menu->addLine(STR_COPY, [=] {
          clipboard.write((uint8_t *)value, length, Clipboard::CONTENT_TEXT);
        });
        if (clipboard.contentType == Clipboard::CONTENT_TEXT) {
          menu->addLine(STR_PASTE, [=] {
            clipboard.read((uint8_t *)value, length);
            changeEnd(true);
          });
        }
#endif
        menu->addLine(STR_CLEAR, [=] {
          memset(value, 0, length);
          changeEnd(true);
        });
        break;
      }

      case EVT_KEY_BREAK(KEY_UP):
        v = toggleCase(v);
        break;

      case EVT_KEY_LONG(KEY_LEFT):
      case EVT_KEY_LONG(KEY_RIGHT):
        v = toggleCase(v);
        if (event == EVT_KEY_LONG(KEY_LEFT)) {
          killEvents(KEY_LEFT);
        }
        break;

      case EVT_KEY_BREAK(KEY_PGDN):
        if (cursorPos < length) {
          memmove(&value[cursorPos], &value[cursorPos + 1], length - cursorPos - 1);
          value[length - 1] = '\0';
          changed = true;
          if (cursorPos > 0 && value[cursorPos] == '\0') {
            cursorPos = cursorPos - 1;
          }
          invalidate();
        }
        break;
    }

    if (c != v) {
      // TRACE("value[%d] = %d", cursorPos, v);
      value[cursorPos] = v;
      invalidate();
      changed = true;
    }
  }
  else {
    cursorPos = 0;
    FormField::onEvent(event);
  }
#endif
}

#if defined(HARDWARE_TOUCH)
bool TextEdit::onTouchEnd(coord_t x, coord_t y)
{
  if (!isEnabled()) {
    return true;
  }

  if (!hasFocus()) {
    setFocus(SET_FOCUS_DEFAULT);
  }

// #if defined(SOFTWARE_KEYBOARD)
//   TextKeyboard::show(this);
// #endif

  coord_t rest = x;
  for (cursorPos = 0; cursorPos < length; cursorPos++) {
    char c = value[cursorPos];
    if (c == '\0')
      break;
    uint8_t w = getCharWidth(c, fontspecsTable[0]) + 1;
    if (rest < w)
      break;
    rest -= w;
  }

  invalidate();
  return true;
}
#endif

void TextEdit::onFocusLost()
{
// #if defined(SOFTWARE_KEYBOARD)
//   TextKeyboard::hide();
// #endif

  changeEnd();
  FormField::onFocusLost();
}
