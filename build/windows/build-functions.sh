#!/bin/bash

function old_ruby_install_with_bundler() {
    ruby_version=$1
    rbenv install $ruby_version
    rbenv global $ruby_version
    gem update --system
    gem install bundler
}

function ruby_install_with_bundler() {
    ruby_version=$1
    rbenv install $ruby_version
    rbenv global $ruby_version
    gem install bundler
}

function bundle_install() {
    ruby_version=$1
    rbenv local $ruby_version
    bundle install --path vendor/bundle
}

function install_rake_compiler_cross_ruby() {
    ruby_version=$1
    rbenv local $ruby_version
    bundle exec rake-compiler cross-ruby HOST=i686-w64-mingw32 VERSION=$ruby_version
    bundle exec rake-compiler cross-ruby HOST=x86_64-w64-mingw32 VERSION=$ruby_version
}

function cross_compile() {
    bundle exec rake clean:groonga
    rbenv local $RUBY19
    bundle exec rake RUBY_CC_VERSION=$CROSS_TERGET MAKE_N_JOBS=$CPU_NUMS cross clean native gem
    bundle exec rake clean:groonga
    bundle exec rake RUBY_CC_VERSION=$CROSS_TERGET MAKE_N_JOBS=$CPU_NUMS RROONGA_USE_GROONGA_X64=true cross clean native gem
}

function cp_pkg_gems() {
    mkdir -p $PKG_GEMS
    cp pkg/*.gem $PKG_GEMS
}
