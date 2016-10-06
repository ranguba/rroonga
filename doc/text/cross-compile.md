# How to Cross compile Rroonga

## For rake-compiler-dock

`rake-compiler-dock` depends `docker` and some platform requires docker client such as `docker-machine`.
Please install `docker` and docker client before cross compiling with `rake-compiler-dock`.

### cross compiling with rake-compiler-dock

execute following rake task:

```bash
$ bundle exec rake build:windows
```

Then, `pkg` directory is created. And cross compiled gems move into `pkg` directory.

## Manual procedure for Debian GNU/Linux like Linux distribution Users

This is the manual procedure without rake-compiler-dock.

### apt

* mingw-w64
* build-essential

### prepare ruby and rubygems

```bash
$ rbenv install 1.9.3-p547
$ gem install rubygems-update
$ update_rubygems
$ gem install bundler
$ rbenv install 2.0.0-p576
$ gem install bundler
$ rbenv install 2.1.3
$ gem install bundler
```

### bundle install

```bash
$ rbenv local 1.9.3-p547
$ bundle install [--path vendor/bundle]
$ rbenv local 2.0.0-p576
$ bundle install [--path vendor/bundle]
$ rbenv local 2.1.3
$ bundle install [--path vendor/bundle]
```

### rake-compiler

```bash
$ rbenv local 1.9.3-p547
$ bundle exec rake-compiler cross-ruby HOST=i686-w64-mingw32 VERSION=1.9.3-p547 [EXTS=--without-extensions]
$ bundle exec rake-compiler cross-ruby HOST=x86_64-w64-mingw32 VERSION=1.9.3-p547 [EXTS=--without-extensions]
$ rbenv local 2.0.0-p576
$ bundle exec rake-compiler cross-ruby HOST=i686-w64-mingw32 VERSION=2.0.0-p576 [EXTS=--without-extensions]
$ bundle exec rake-compiler cross-ruby HOST=x86_64-w64-mingw32 VERSION=2.0.0-p576 [EXTS=--without-extensions]
[$ rbenv local 2.1.3]
$ bundle exec rake-compiler cross-ruby HOST=i686-w64-mingw32 VERSION=2.1.3 [EXTS=--without-extensions]
$ bundle exec rake-compiler cross-ruby HOST=x86_64-w64-mingw32 VERSION=2.1.3 [EXTS=--without-extensions]
```

### cross compile

```bash
$ rbenv local 1.9.3-p547
$ bundle exec rake:build
$ bundle exec rake RUBY_CC_VERSION=1.9.3:2.0.0:2.1.3 cross clean native gem
$ bundle exec rake clean:groonga
$ bundle exec rake RUBY_CC_VERSION=1.9.3:2.0.0:2.1.3 cross RROONGA_USE_GROONGA_X64=true clean native gem
```
