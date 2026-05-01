# orig/ — original binary location

This directory should contain symlinks (or copies, but symlinks are
preferred — saves disk and avoids accidental commits) to the five
shipped FFXIV 1.23b binaries:

```
orig/ffxivgame.exe       -> ../ffxiv-install-environment/.../ffxivgame.exe
orig/ffxivboot.exe       -> ...
orig/ffxivconfig.exe     -> ...
orig/ffxivlogin.exe      -> ...
orig/ffxivupdater.exe    -> ...
```

Populate them with:

```sh
../tools/symlink_orig.sh
```

`.gitignore` excludes everything in this directory except this README
— do not commit the binaries; they're copyright Square Enix.
