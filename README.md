# README

[![Gem Version](https://badge.fury.io/rb/rroonga.svg)](https://badge.fury.io/rb/rroonga)

## Name

Rroonga

## Description

Ruby bindings for Groonga that provide full text search and
column store features.

Rroonga is an extension library to use Groonga's DB-API
layer. Rroonga provides Rubyish readable and writable API
not C like API. You can use Groonga's fast and highly
functional features from Ruby with Rubyish form.

See the following URL about Groonga.

* [The Groonga official site](https://groonga.org/)

## Authors

* Kouhei Sutou `<kou@clear-code.com>`
* Tasuku SUENAGA `<a@razil.jp>`
* Daijiro MORI `<morita@razil.jp>`
* Yuto HAYAMIZU `<y.hayamizu@gmail.com>`
* SHIDARA Yoji `<dara@shidara.net>`
* yoshihara haruka `<yoshihara@clear-code.com>`

## License

LGPL 2.1. See license/LGPL for details.

(Kouhei Sutou has a right to change the license including
contributed patches.)

## Dependencies

* Ruby
* Groonga

## Install

If you're using Bundler, add `plugin "rubygems-requirements-system"`
and `gem "rroonga"` to your `Gemfile`:

```ruby
plugin "rubygems-requirements-system"

gem "rroonga"
```

If you're not using Bundler, install `rubygems-requirements-system`
and `rroonga`:

```bash
sudo gem install rubygems-requirements-system rroonga
```

## Documents

* [Reference manual in English](https://ranguba.org/rroonga/en/)
* [Reference manual in Japanese](https://ranguba.org/rroonga/ja/)

## Mailing list

* English: [groonga-talk](https://lists.sourceforge.net/mailman/listinfo/groonga-talk)
* Japanese: [groonga-dev](https://lists.sourceforge.jp/mailman/listinfo/groonga-dev)

## Thanks

* Daijiro MORI: sent patches to support the latest Groonga.
* Tasuku SUENAGA: sent patches and bug reports.
* niku: sent bug reports.
* dara:
  * wrote tests.
  * fixed bugs.
* id:mat_aki: sent bug reports.
* @yune_kotomi: sent a bug report.
* m_seki: sent bug reports.
* ono matope: sent bug reports.
* @kamipo: send a bug report.
* ongaeshi: sent a patch to build gem on Windows.
* mallowlabs: send a patch.
