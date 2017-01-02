Drivers add implementation for low-level services in Backdrop.  Specifically,
drivers add new implementations for databases, configuration storage, and
caching.

These three classes of code all need to be loaded before the bulk of Backdrop
is ready, and well before Backdrop can find the list of active modules.

Drivers are like modules, but a little bit special.  Drivers don't have the
concept of being enabled or disabled, installed or uninstalled.  If they are
present in this folder, they are active.
