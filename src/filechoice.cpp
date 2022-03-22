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

#include "filechoice.h"
#include "strhelpers.h"
#include "menu.h"
#include "theme.h"
#include "message_dialog.h"
#include "libopenui_conf.h"
#include <algorithm>

#if !defined(STR_SDCARD)
  #define STR_SDCARD "SD"
#endif

#if !defined(STR_NO_FILES_ON_SD)
  #define STR_NO_FILES_ON_SD "No files on SD Card!"
#endif

FileChoice::FileChoice(FormGroup *parent, const rect_t &rect,
                       std::string folder, const char *extension, int maxlen,
                       std::function<std::string()> getValue,
                       std::function<void(std::string)> setValue,
                       bool stripExtension) :
    ChoiceBase(parent, rect, CHOICE_TYPE_FOLDER),
    folder(std::move(folder)),
    extension(extension),
    maxlen(maxlen),
    getValue(std::move(getValue)),
    setValue(std::move(setValue)),
    stripExtension(stripExtension)
{
}

std::string FileChoice::getLabelText()
{
  return getValue();
}


bool FileChoice::openMenu()
{
  OpenUiFileInfo fno;
  OpenUiDir dir;
  std::list<std::string> files;
  const char *fnExt;
  uint8_t fnLen, extLen;

  OpenUiFsRetType res = openUiOpenDir(&dir, folder.c_str());  // Open the directory
  if (res == OPENUI_FS_OK) {
    for (;;) {
      res = openUiReadDir(&dir, &fno);
      if (res != OPENUI_FS_OK || openUiFsGetName(&fno).length() == 0)
        break;                             // break on error or end of dir
      if (openUiFsIsDir(&fno))        continue;  // skip subfolders
      if (openUiFsIsHiddenFile(&fno)) continue;  // skip hidden files
      if (openUiFsIsSystemFile(&fno)) continue;  // skip system files

      fnExt = openUiGetFileExtension(openUiFsGetName(&fno).c_str(), 0, 0, &fnLen, &extLen);

      if (extension && (!fnExt || !openUiIsFileExtensionMatching(fnExt, extension, nullptr)))
        continue;  // wrong extension

      if (stripExtension) fnLen -= extLen;

      if (!fnLen || fnLen > maxlen) continue;  // wrong size

      // eject duplicates
      std::string newFile = fno.getName();
      if (std::find(files.begin(), files.end(), newFile) != files.end())
        continue;

      files.emplace_back(newFile);
    }
    openUiCloseDir(&dir);

    if (!files.empty()) {
      // sort files
      files.sort(compare_nocase);
      files.push_front("");

      auto menu = new Menu(this);
      int count = 0;
      int current = -1;
      std::string value = getValue();
      for (const auto &file : files) {
        menu->addLine(file, [=]() { setValue(file); });
        // TRACE("%s %d %s %d", value.c_str(), value.size(), file.c_str(),
        // file.size());
        if (value.compare(file) == 0) {
          // TRACE("OK");
          current = count;
        }
        ++count;
      }

      if (current >= 0) {
        menu->select(current);
      }

      menu->setCloseHandler([=]() {
        editMode = false;
        setFocus(SET_FOCUS_DEFAULT);
      });

      return true;
    }
  }

  new MessageDialog(this, STR_SDCARD, STR_NO_FILES_ON_SD);

  return false;
}

#if defined(HARDWARE_KEYS)
void FileChoice::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    if (openMenu()) {
      editMode = true;
      invalidate();
    }
  }
  else {
    FormField::onEvent(event);
  }
}
#endif

#if defined(HARDWARE_TOUCH)
bool FileChoice::onTouchEnd(coord_t, coord_t)
{
  setFocus(SET_FOCUS_DEFAULT);
  setEditMode(true);
  openMenu();
  return true;
}
#endif
