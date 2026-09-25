# Bundled Sodium keys

Any file placed in this directory with a `.pub` extension is loaded as an
additional trusted public key for verification, on top of any profiles
configured through the admin UI.

Each file must contain a single base64-encoded Ed25519 public key (the same
format stored in a profile's `public_key` field), and nothing else:

```
H9w2gWiaHDZsC/L63Nr4OSrIiuZ+tNs7r/V5/tiZzcs=
```

Bundled keys are verification-only trust anchors. They are never returned by
`CodeSignSodiumEngine::getPrivateKeychain()` and cannot be used to sign data
- only a profile's own keypair can do that. Do not place a private/secret
key in this directory or anywhere else in the module's source tree.
