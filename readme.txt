Droper 0.4.1
------------

Droper is a Symbian Dropbox client that allows Dropbox users to access and
modify their synced files and folders.

Copyright 2011 Omar Lawand Dalatieh <lawand87@gmail.com>.

http://coinonedge.com/droper/


Changelog
---------

0.4.1
*Under Development*

0.4.0
Rewritten the UI to support S60v3 devices (non-touch devices), better integrate
with Symbian and stabilize kinetic scrolling, but stop supporting desktop
platforms.
Added an option to get the Dropbox referral link.
Added an option to get public links to files in the public folder.

0.3.4
Updated the readme file.

0.3.1
Added an option to upload files, made lots of GUI enhancements including an
experimental kinetic scrolling feature for Symbian, added an option to view
file properties which include the date of last modification.


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

Although Dropbox allows uploading files up to 300MB in size, Droper currently
allows uploading small files only (few MBs, specific amount depends on free RAM
memory on device). This is a known issue and is supposed to be fixed in later
releases.


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

If you try to compile Droper, you'll notice that the consumerdata.cpp file is
missing, it is a simple file that contains the values of the consumer key and
secret. This file wasn't uploaded to the repository for security reasons. There
is a template consumerdata.cpp file in the repository called
"consumerdata_template.cpp", you can check it out to get an idea about this
file. For more info search for and learn about OAuth and Dropbox API.

You can ignore build warnings when building Droper.

The only version of Qt for Symbian that was tested is 4.7.3, which is
shipped with Qt SDK 1.1.2.

Distributing a custom build for Symbian requires renaming the application and
specifying a different UID in the project file (so that it can be installed
along with Droper).


Legal Notes
-----------

Copyright 2011 Omar Lawand Dalatieh.

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


Credits
-------

Droper was programmed using Qt4.

The Silk Icon Set by Mark James is used by Droper.

Thanks to all Dropbox Forums members for helping with development.

