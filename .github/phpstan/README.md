# PHPStan usage in Backdrop

[PHPStan](https://phpstan.org/) is run in a GitHub workflow.

But you can also run it on your local dev instance.

You can install PHPStan via composer or you can also install the phar file
locally.

Then cd to the main directory (for instance /var/www/backdrop/), and run:

```
phpstan analyze -c .github/phpstan/phpstan.neon
```

If you're working on a bugfix in a single module, you can override the path
to check:

```
phpstan analyze -c .github/phpstan/phpstan.neon core/modules/file
```

There are two baseline files:

- .github/phpstan/phpstan-baseline-includes.neon
- .github/phpstan/core/phpstan-baseline-modules.neon

These files are generated with phpstan to cover lots of legacy code, that
should get improved later.

Once some of the problems got fixed, you might want to generate new baseline
files, for instance:

```
phpstan analyze -c .github/phpstan/phpstan.neon core/modules/ --generate-baseline=.github/phpstan/phpstan-baseline-modules.neon
```

Be warned, on weak hardware you might run out of memory, as analysis
consumes lots of it. Make sure, you have several GB available.

If that's not possible, you can (temporarily) tweak the settings to run less
jobs in parallel.

See the documentation regarding parallel processing:

https://phpstan.org/config-reference#parallel-processing
