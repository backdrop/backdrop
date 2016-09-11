# CSS Guidelines

[See Backdrop's CSS Standards](https://api.backdropcms.org/css-standards)

## Files
The types of CSS files in the theme are:
* Base - Using a normalize stylesheet to ensure consistent rendering
* Fonts - `@font-face` rules
* Layout - Component layout, some overrides for `layout` extension styles
* Component - CSS files that
* Skin - The colors, fonts and aesthetic CSS

Basis is made with more granular CSS files so that a sub theme can override
only the files it needs to, and inherit the rest.

To override a CSS file, declare it in your .info file with the same file name,
the file in Basis with the same name will not be loaded _at all_ in favor of the
one in the active theme.

A good way to think about overriding files:
 * Start by overriding skin.css with the colors and fonts you'd prefer
 * If there are other changes to be made, copy over the files one by one and
   override as needed
