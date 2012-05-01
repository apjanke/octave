/* OctaveGUI - A graphical user interface for Octave
 * Copyright (C) 2011 Jacob Dawid (jacob.dawid@googlemail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ResourceManager.h"
#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include <QSettings>

SettingsDialog::SettingsDialog (QWidget * parent):
QDialog (parent), ui (new Ui::SettingsDialog)
{
  ui->setupUi (this);

  QSettings *settings = ResourceManager::instance ()->settings ();
  ui->useCustomFileEditor->setChecked (settings->value ("useCustomFileEditor").toBool ());
  ui->customFileEditor->setText (settings->value ("customFileEditor").toString ());
  ui->editor_showLineNumbers->setChecked (settings->value ("editor/showLineNumbers",true).toBool () );
  ui->editor_highlightCurrentLine->setChecked (settings->value ("editor/highlightCurrentLine",true).toBool () );
  ui->editor_codeCompletion->setChecked (settings->value ("editor/codeCompletion",true).toBool () );
  ui->editor_fontName->setCurrentFont (QFont (settings->value ("editor/fontName","Courier").toString()) );
  ui->editor_fontSize->setValue (settings->value ("editor/fontSize",10).toInt ());  
  ui->editor_longWindowTitle->setChecked (settings->value ("editor/longWindowTitle",true).toBool ());
  ui->terminal_fontName->setCurrentFont (QFont (settings->value ("terminal/fontName","Courier").toString()) );
  ui->terminal_fontSize->setValue (settings->value ("terminal/fontSize",10).toInt ());
  ui->showFilenames->setChecked (settings->value ("showFilenames").toBool());
  ui->showFileSize->setChecked (settings->value ("showFileSize").toBool());
  ui->showFileType->setChecked (settings->value ("showFileType").toBool());
  ui->showLastModified->setChecked (settings->value ("showLastModified").toBool());
  ui->showHiddenFiles->setChecked (settings->value ("showHiddenFiles").toBool());
  ui->useAlternatingRowColors->setChecked (settings->value ("useAlternatingRowColors").toBool());
  ui->useProxyServer->setChecked (settings->value ("useProxyServer").toBool ());
  ui->proxyHostName->setText (settings->value ("proxyHostName").toString ());

  int currentIndex = 0;
  QString proxyTypeString = settings->value ("proxyType").toString ();
  while ( (currentIndex < ui->proxyType->count ()) && (ui->proxyType->currentText () != proxyTypeString))
    {
      currentIndex++;
      ui->proxyType->setCurrentIndex (currentIndex);
    }

  ui->proxyPort->setText (settings->value ("proxyPort").toString ());
  ui->proxyUserName->setText (settings->value ("proxyUserName").toString ());
  ui->proxyPassword->setText (settings->value ("proxyPassword").toString ());

  // Short cuts
  QStringList headerLabels;
  headerLabels << "Modifier" << "Key" << "Action";
  ui->shortcutTableWidget->setColumnCount (3);
  ui->shortcutTableWidget->setRowCount (10);
  ui->shortcutTableWidget->horizontalHeader ()->setStretchLastSection (true);
  ui->shortcutTableWidget->setHorizontalHeaderLabels (headerLabels);
  ui->shortcutTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->shortcutTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

  /*
  newAction->setShortcut(QKeySequence::New);
  openAction->setShortcut(QKeySequence::Open);
  saveAction->setShortcut(QKeySequence::Save);
  saveAsAction->setShortcut(QKeySequence::SaveAs);
  undoAction->setShortcut(QKeySequence::Undo);
  redoAction->setShortcut(QKeySequence::Redo);
  m_copyAction->setShortcut(QKeySequence::Copy);
  m_cutAction->setShortcut(QKeySequence::Cut);
  pasteAction->setShortcut(QKeySequence::Paste);
  runAction->setShortcut(Qt::Key_F5);
  nextBookmarkAction->setShortcut(Qt::Key_F2);
  prevBookmarkAction->setShortcut(Qt::SHIFT + Qt::Key_F2);
  toggleBookmarkAction->setShortcut(Qt::Key_F7);
  commentSelectedAction->setShortcut(Qt::CTRL + Qt::Key_R);
  uncommentSelectedAction->setShortcut(Qt::CTRL + Qt::Key_T);*/
}

SettingsDialog::~SettingsDialog ()
{
  QSettings *settings = ResourceManager::instance ()->settings ();
  settings->setValue ("useCustomFileEditor", ui->useCustomFileEditor->isChecked ());
  settings->setValue ("customFileEditor", ui->customFileEditor->text ());
  settings->setValue ("editor/showLineNumbers", ui->editor_showLineNumbers->isChecked ());
  settings->setValue ("editor/highlightCurrentLine", ui->editor_highlightCurrentLine->isChecked ());
  settings->setValue ("editor/codeCompletion", ui->editor_codeCompletion->isChecked ());
  settings->setValue ("editor/fontName", ui->editor_fontName->currentFont().family());
  settings->setValue ("editor/fontSize", ui->editor_fontSize->value());
  settings->setValue ("editor/longWindowTitle", ui->editor_longWindowTitle->isChecked());
  settings->setValue ("terminal/fontSize", ui->terminal_fontSize->value());
  settings->setValue ("terminal/fontName", ui->terminal_fontName->currentFont().family());
  settings->setValue ("showFilenames", ui->showFilenames->isChecked ());
  settings->setValue ("showFileSize", ui->showFileSize->isChecked ());
  settings->setValue ("showFileType", ui->showFileType->isChecked ());
  settings->setValue ("showLastModified", ui->showLastModified->isChecked ());
  settings->setValue ("showHiddenFiles", ui->showHiddenFiles->isChecked ());
  settings->setValue ("useAlternatingRowColors", ui->useAlternatingRowColors->isChecked ());
  settings->setValue ("useProxyServer", ui->useProxyServer->isChecked ());
  settings->setValue ("proxyType", ui->proxyType->currentText ());
  settings->setValue ("proxyHostName", ui->proxyHostName->text ());
  settings->setValue ("proxyPort", ui->proxyPort->text ());
  settings->setValue ("proxyUserName", ui->proxyUserName->text ());
  settings->setValue ("proxyPassword", ui->proxyPassword->text ());
  delete ui;
}
