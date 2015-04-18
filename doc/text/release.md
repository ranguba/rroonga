# Release

This document describes how to release Rroonga.

## 1. Summarize the changes for this release

### Show the commits since the latest release

    % git log -p --reverse <the latest release version>..HEAD

For example:

    % git log -p --reverse 4.0.3..HEAD

Or

[Commits on GitHub](https://github.com/ranguba/rroonga/commits/master)

### Extract the commits related to users

#### Including

* Changed specifications
* Added new features
* ...

#### Excluding

* Refactorings
* Tests
* ...

### Categorize

* Improvements: new features, improved behavior, ...
* Fixes: fixed bugs, ...

### Thanks

If a contributor name is in a commit log, create "Thanks" group and write the name to the group.

### Template for a new release for news.textile

    ## X.X.X: YYYY-MM-DD {#version-x-x-x}

    ### Improvements

    * ...

    ### Fixes

    * ...

    ### Thanks

    * ...

## 2. Translate

### 2-1. Update PO

Update PO files (e.g.: ja.po) by the following command:

    $ rake reference:po:update

### 2-2. Edit PO

Then, edit PO files:

* Edit msgid and msgstr.
* Search fuzzy.
  * Edit if necessary.
  * Then, remove fuzzy.

## 3. Upload gem to RubyGems.org

### 3-1. For Linux

    $ rake release

### 3-2. For Windows

    $ rake build:windows
    $ gem push pkg/rroonga-<the latest release version>-x86-mingw32.gem
    $ gem push pkg/rroonga-<the latest release version>-x64-mingw32.gem

### 3-3. Bump version

    $ $EDITOR ext/groonga/rb-grn.h

## 4. Update ranguba.org

### 4-1. Update the latest version and release date

Update version and release date in ranguba.org (index.html、ja/index.html).

First, clone ranguba.org repository by the following command:

    $ cd ..
    $ git clone git@github.com:ranguba/ranguba.org.git
    $ cd ranguba.org

Or

    $ cd ../ranguba.org
    $ git pull

Second, update the latest version and release date:

    $ $EDITOR _config.yml

### 4-2. Update reference manual

First, copy references to reference html directory in ranguba.org:

    $ cd ../rroonga
    $ rake release:references:upload

Second, commit and push the html directory:

    $ cd ../ranguba.org
    $ git add .
    $ git commit
    $ git push

## 5. Announce in mailing lists

* groonga-talk (English)
  * e.g.: http://sourceforge.net/p/groonga/mailman/message/33144993/
* groonga-dev (Japanese)
  * e.g.: http://sourceforge.jp/projects/groonga/lists/archive/dev/2014-December/003014.html
* ruby-talk (English) (optional)
  * e.g.: http://blade.nagaokaut.ac.jp/cgi-bin/scat.rb/ruby/ruby-talk/428992
* ruby-list (Japanese) (optional)
  * e.g.: http://blade.nagaokaut.ac.jp/cgi-bin/scat.rb/ruby/ruby-list/50025
