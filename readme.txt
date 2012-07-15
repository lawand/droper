Droper 0.4.5
------------

Droper is a Symbian Dropbox client that allows Dropbox users to access and
modify their synced files and folders.

Copyright 2011, 2012 Omar Lawand Dalatieh <lawand87@gmail.com>.

http://coinonedge.com/droper/


Changelog
---------

0.4.5
*Under Development*

0.4.4 (2012-03-30)
Added an option for shareable links.
Made some GUI updates and fixed some bugs.

0.4.2 (2012-03-18)
Changed upload method to allow larger file sizes.
Switched to version 1 of the Dropbox API.
Made some GUI updated and fixed some bugs.

0.4.0 (2011-08-30)
Rewritten the UI to support S60v3 devices (non-touch devices), better integrate
with Symbian and stabilize kinetic scrolling, but stop supporting desktop
platforms.
Added an option to get the Dropbox referral link.
Added an option to get public links to files in the public folder.

0.3.4 (2011-08-11)
Updated the readme file.

0.3.1 (2011-04-14)
Added an option to upload files, made lots of GUI enhancements including an
experimental kinetic scrolling feature for Symbian, added an option to view
file properties which include the date of last modification.

0.2.1 (2011-03-03)
Added Symbian support.

0.1 (2011-02-21)
Initial version.


Installation Notes
------------------

Droper is compatible with most Nokia Symbian smartphones running S60v3, S60v5
or Symbian^3 (in other words, most Nokia Symbian smartphones manufactured after
2006). For more info about your device, check out Forum Nokia:
http://www.forum.nokia.com/Devices/

Note: S60v3 devices are those equipped with a non-touch screen, whereas S60v5
and Symbian^3 devices are those equipped with a touch screen.

Qt 4.7.3 should be installed before Droper can be used. On S60v5 and
Symbian^3 devices, Qt 4.7.3 gets automatically downloaded and installed when
installing Droper but on S60v3 devices you have to install Qt 4.7.3 manually.
You can get Qt 4.7.3 here: http://coinonedge.com/qt-symbian-installer/


General Usage Tips
------------------

When downloading or uploading files, you can close the progress dialog and the
operation won't be interrupted, and you can check the progress later by
selecting the Active Transfer > Active Download or the
Active Transfer > Active Upload option.

You can block Droper's access to your account (in case you forget to remove the
authentication information when using Droper on a friend's device, for
instance) by logging into your Dropbox account in dropbox.com and selecting the
"My Apps" option in the account's settings and removing Droper's access. You
can give the access back by logging in again using Droper.

Uninstalling Droper doesn't remove the login information, you must do so
manually, using the Account > Sign Out option.

There currently seem to be some issues when dealing with some files and folders
which names contain these five symbols ; + ~ # %

The current version isn't optimized for minimal network traffic. The use of
WLAN networks when using Droper is recommended to avoid data transfer costs.


Keyboard Shortcuts
------------------

Keyboard shortcuts for actions are presented in the following. Note that they
are similar to those used in the X-Plore mobile application to make Droper
easier to use for those who use X-Plore.

For smart phones with a touch screen (S60v5 and newer) and desktop platforms:

Delete          - Backspace
Copy            - Ctrl+C
Cut             - Ctrl+X
Paste           - Ctrl+V
Download        - Ctrl+D
Properties      - Ctrl+E
Rename          - Ctrl+R
Upload          - Ctrl+U
Create Folder   - Ctrl+F
Refresh         - Ctrl+Space
Public Link     - Ctrl+P
Shareable Link  - Ctrl+S

For smart phones without a touch screen (S60v3):

Delete          - Backspace
Copy            - 1
Cut             - 2
Paste           - 3
Download        - 4
Properties      - 5
Rename          - 7
Upload          - 8
Create Folder   - 9
Refresh         - 0
Public Link     - *
Shareable Link  - #


General Notes for Developers
----------------------------

Droper is a C++/Qt4 application.

Droper can only be compiled for and is only tested with Symbian.

Droper version numbers follow this scheme: Major.Minor.Patch (e.g 0.8.2).
Released versions (which are git-tagged) always have an even patch number
(starting with 0.3.4) for example: 1.7.2 not 1.7.1 or 1.7.3. Odd patch numbers
are given to code edits that take place between two releases (e.g. 1.7.3 is
between 1.7.2 and the next released version which could be 1.7.4 and could be
1.8.0 and 2.0.0 depending on the severity of the changes made to the code).

You can ignore build warnings when building Droper.

The only version of Qt for Symbian that was tested is 4.7.3 (for Symbian^1),
which is shipped with Qt SDK 1.1.2.

Distributing a custom build for Symbian requires renaming the application and
specifying a different UID in the project file (so that it can be installed
along with Droper).


Translating Droper
------------------

You can contribute to Droper by translating it to your native language.
If you interested, please contact the maintainer(s) of Droper.


Legal Notes
-----------

Copyright 2011, 2012 Omar Lawand Dalatieh.

Droper is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

Droper is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
Droper. If not, see <http://www.gnu.org/licenses/>.

For more information, check out the GNU General Public license found in the
Copying file.

All trademarks are property of their respective owners.

This application uses a third party library named "qt-json". For more info, see
the qt-json Copyright Notice section below. You can also checkout the following URL:
https://github.com/ereilin/qt-json

This application uses a third party library named "QsKineticScroller". For more
info, see the QsKineticScroller Copyright Notice section below.


qt-json Copyright Notice
------------------------

Copyright 2011 Eeli Reilin. All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ''AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL EELI REILIN OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation
are those of the authors and should not be interpreted as representing
official policies, either expressed or implied, of Eeli Reilin.


QtScroller Copyright Notice
---------------------------

Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
All rights reserved.
Contact: Nokia Corporation (qt-info@nokia.com)


Credits
-------

Droper was programmed using Qt4.

The Silk Icon Set by Mark James is used by Droper.

Thanks to all Dropbox Forums members for helping with development.

