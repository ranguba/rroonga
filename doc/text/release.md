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

### 3-1. Bump version if need

Change `RB_GRN_MAJOR_VERSION`, `RB_GRN_MINOR_VERSION` and `RB_GRN_MICRO_VERSION`
to the same version of the latest Groonga.

    $ $EDITOR ext/groonga/rb-grn.h

### 3-2. Release

    $ rake release

### 3-3. Bump version

Increment `RB_GRN_MICRO_VERSION`.
If current `RB_GRN_MICRO_VERSION` is `9`, increment `RB_GRN_MINOR_VERSION` and reset `RB_GRN_MICRO_VERSION` to `0`.

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

## 5. Update Blogroonga (blog)

Please refer to [Groonga document](https://groonga.org//docs/contribution/development/release.html#blogroonga) 
for how to update Blogroonga.

## 6. Announce

* [GitHub Discussions](https://github.com/ranguba/rroonga/discussions/categories/releases) (English/Japanese)
* [Twitter](https://twitter.com/groonga) (English/Japanese)
* [Facebook](https://ja-jp.facebook.com/groonga/) (English/Japanese)
* ruby-talk (English)
  * e.g.: http://blade.nagaokaut.ac.jp/cgi-bin/scat.rb/ruby/ruby-talk/428992
* ruby-list (Japanese)
  * e.g.: http://blade.nagaokaut.ac.jp/cgi-bin/scat.rb/ruby/ruby-list/50025
