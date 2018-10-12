Installation profiles define additional steps that run after the base
installation provided by Backdrop core when Backdrop is first installed.

Installation profiles are generally provided as part of distributions, and only
have an effect during the installation phase of a Backdrop website. Installation
profiles do not have any effect on a preexisting website.

This directory contains core installation profiles. Backdrop may be extended
with additional installation profiles, which should be placed in a "profiles"
directory in the root of your Backdrop installation.

What to place in this directory?
--------------------------------

Do not place any non-core files in this directory. Create a "profiles" directory
in the root of your installation. And place installation profiles there instead.

Download additional distributions
---------------------------------

Contributed distributions from the Backdrop community may be available at
https://github.com/backdrop-contrib.

Multisite configuration
-----------------------

In multisite configurations installation profiles found in this directory are
available to all sites during their initial site installation.
