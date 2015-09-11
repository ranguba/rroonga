# Install

This document describes how to install Rroonga.

You can install Rroonga by RubyGems. It is the standard way for Ruby
libraries.

Rroonga is depends on Groonga. So you need to install both Groonga and
Rroonga. You can't install Groonga by RubyGems because it isn't Ruby
library. But don't worry. Rroonga provides the following options for
easy to install:

* Rroonga downloads, builds and installs Groonga automatically. You
  don't need to do them explicitly.
* Rroonga uses Groonga installed by your packaging system.

The following sections describe the above in detail.

## Install with auto Groonga build

Rroonga searches Groonga on install. If Rroonga can't find Groonga,
Rroonga downloads, builds and installs Groonga automatically.

Type the following command to install Rroonga and Groonga. You don't
need to install Groonga explicitly:

    % gem install rroonga

## Install with Groonga package

You can use Groonga package on you packaging system instead of building
Groonga by yourself. There are the following advantages for this option:

* It reduces installation time.
* It doesn't fail on building Groonga.

### Windows

Rroonga gem for Windows includes both pre-compiled Rroonga and Groonga
in the gem. So what you need to do is you just install rroonga gem.

Type the following command on Ruby console:

    > gem install rroonga

This document assumes that you're using [RubyInstaller for
Windows](http://rubyinstaller.org/) .

### OS X

There are Groonga packages for OS X environment.

#### MacPorts

If you're using [MacPorts](http://www.macports.org/) , type the
following
commands on your terminal:

    % sudo port install groonga
    % sudo gem install rroonga

#### Homebrew

If you're using [Homebrew](http://brew.sh/) , type the following
commands
on your terminal:

    % brew install groonga
    % gem install rroonga

### Debian GNU/Linux

You can install the Groonga package by apt. See [Groonga
documentation](http://groonga.org/docs/install/debian.html) how to set
apt-line up.

Type the following commands on your terminal after you finish to set
apt-line up.

    % sudo apt-get install -y libgroonga-dev
    % sudo gem install rroonga

### Ubuntu

You can install the Groonga package by apt. See [Groonga
documentation](http://groonga.org/docs/install/ubuntu.html) how to set
apt-line up.

Type the following commands on your terminal after you finish to set
apt-line up.

    % sudo apt-get install -y libgroonga-dev
    % sudo gem install rroonga

### CentOS

You can install the Groonga package by yum. See [Groonga
documentation](http://groonga.org/docs/install/centos.html) how to set
yum repository up.

But you need to install Ruby 1.9.3 or later by yourself. Both CentOS 5
and 6 ship Ruby 1.8. Rroonga doesn't support Ruby 1.8.

Type the following commands on your terminal after you finish to set
yum
repository up and installing Ruby 1.9.3 or later.

    % sudo yum install groonga-devel -y
    % gem install rroonga

### Fedora

You can install the Groonga package by yum. The Groonga package is
included in the official Fedora repository.

    % sudo yum install groonga-devel -y
    % sudo gem install rroonga

## Links

* [2. Install - Groonga documentation](http://groonga.org/docs/install.html)
