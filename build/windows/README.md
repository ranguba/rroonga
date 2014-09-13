Vagrant box for Cross compiling Rroonga
===

Vagrant box for cross compiling Rroonga.

```bash
$ cd build/windows
$ vagrant up
```

or simply execute in Rroonga root directory:

```bash
$ rake build:windows
```

Then, cross compiling for a long time....

Cross compiled gems are copied under \<rroonga\_base\_dir\>/pkg when it executes via `rake build:windows`.
