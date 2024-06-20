Installation profiles provide site features and functions for a specific type 
of site as a single download containing Backdrop CMS core, contributed modules, 
themes, and pre-defined configuration. They make it possible to quickly set up a 
complex, use-specific site in fewer steps than if installing and configuring 
elements individually. 

What to place in this directory?
--------------------------------

A great way to get started with installation profiles for Backdrop CMS would be
to copy an existing installation profile from the core/profiles directory and 
begin to edit it to meet your sepcific needs. 

After copying an existing installation profile into your profiles directory,
make sure to rename the installation profile as well as the files and functions 
to match the name of your custom installation profile. 


Multisite configuration
-----------------------

In multisite configuration, profiles found in this directory are available to
all sites. To restrict profiles to a specific site instance, place profiles in a
directory following the pattern sites/your_site_name/profiles.