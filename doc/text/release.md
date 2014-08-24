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

    h2(#x-x-x). x.x.x: YYYY-MM-DD

    h3. Improvements

    * ...

    h3. Fixes

    * ...

    h3. Thanks

    * ...
