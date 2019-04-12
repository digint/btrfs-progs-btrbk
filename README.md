Btrfs-progs-btrbk
=================

This is a patchset on top of btrfs-progs, allowing to build distinct
binaries for specific btrfs subcommands, e.g. "btrfs-subvolume-show"
which would be identical to "btrfs subvolume show".

This is especially useful for **[btrbk]**, using the `backend
btrbk-progs-btrbk` [configuration option].

License: GPLv2.

  [btrbk]: https://digint.ch/btrbk
  [configuration option]: https://digint.ch/btrbk/doc/btrbk.conf.5.html#_system_options


Motivation
----------

While btrfs-progs offer the all-inclusive "btrfs" command, it gets
pretty cumbersome to restrict privileges to the subcommands. Common
approaches are to either setuid root for "/sbin/btrfs" (which is not
recommended at all), or to write sudo rules for each subcommand.

Separating the subcommands into distinct binaries makes it easy to set
elevated privileges using capabilities(7) or setuid. A typical use
case where this is needed is when it comes to automated scripts,
e.g. btrbk creating snapshots and send/receive them via ssh.


Installation
------------

This is a short overview for the cmds-separated-fscaps patchset,
please refer to the INSTALL documentation for more information.

    $ ./autogen.sh
    $ ./configure --enable-btrfs-separated --enable-setcap-install
    $ make separated

This will build all separated binaries `btrfs-*.separated`. If you
want to install the binaries with setuid, use
`--enable-setuid-install` instead.

If you want to install all separated binaries, type:

    $ make SETCAP_GROUP=btrfs install-separated

This will install the binaries along with elevated file capabilities
(setcap) for users in the `btrfs` group.

Alternatively, select specific binaries to install:

    $ make SETCAP_GROUP=btrfs \
           install-btrfs-subvolume-show.separated \
           install-btrfs-subvolume-list.separated \
           install-btrfs-send.separated \
           [...]

The result should be something like this:

    $ sudo getcap -r /usr/local/bin/
    /usr/local/bin/btrfs-send = cap_dac_read_search,cap_fowner,cap_sys_admin+ep
    /usr/local/bin/btrfs-receive = cap_chown,cap_dac_override,cap_dac_read_search,cap_fowner,cap_sys_admin,cap_mknod+ep
    /usr/local/bin/btrfs-subvolume-delete = cap_dac_override,cap_sys_admin+ep
    /usr/local/bin/btrfs-subvolume-list = cap_dac_read_search,cap_fowner,cap_sys_admin+ep
    /usr/local/bin/btrfs-subvolume-show = cap_dac_read_search,cap_fowner,cap_sys_admin+ep
    /usr/local/bin/btrfs-subvolume-snapshot = cap_dac_override,cap_dac_read_search,cap_fowner,cap_sys_admin+ep

Note that for **btrbk**, you will need:

| Purpose  | Binaries                                       |
| :------- | :--------------------------------------------- |
| always   | `btrfs-subvolume-show`, `btrfs-subvolume-list` |
| source   | `btrfs-subvolume-snapshot`, `btrfs-send`       |
| target   | `btrfs-receive`                                |
| delete   | `btrfs-subvolume-delete`                       |
| info     | `btrfs-filesystem-usage`                       |


### Gentoo Linux

If you're on gentoo, grab the digint portage overlay from:
`https://dev.tty0.ch/portage/digint-overlay.git`

Install selected binaries, e.g. for backup source:

    $ echo sys-fs/btrfs-progs-btrbk filecaps btrbk-source >> /etc/portage/package.use
    $ emerge sys-fs/btrfs-progs-btrbk


Development
-----------

If you would like to contribute or have found bugs:

  * Visit [digint/btrfs-progs-btrbk on GitHub] and use the [issues
    tracker] there.
* Talk to us on Freenode in `#btrbk`.

  [digint/btrfs-progs-btrbk on GitHub]: https://github.com/digint/btrfs-progs-btrbk
  [issues tracker]: https://github.com/digint/btrfs-progs-btrbk/issues


References
----------

* [btrbk](https://digint.ch/btrbk)
* [btrfs-progs](https://github.com/kdave/btrfs-progs)
