# How to use Flatpak with Kiwix

## Caution

__By default `flatpak` requires to be root to install and remove runtimes and apps, however flatpak can be forced to run in user mode by adding the `--user` option. All of the following commands are described using the default mode (`system`).__

Also, only the `org.kiwix.Client.yml` manifest is needed to build the app, it does not depend on other files in this repository.

## Requirements

### Install Flatpak and add Flathub repository

Follow the instructions on the [Flatpak][1] website according to your Linux distribution. In addition the tool `flatpak-builder` is required.

### Install Kde Platform and Sdk runtimes

```shell
$ flatpak install flathub org.kde.Platform//5.11 org.kde.Sdk//5.11
```

## Build and install Kiwix

### Build and deploy it in a local repo

```shell
$ flatpak-builder --ccache --force-clean --repo=repo builddir org.kiwix.Client.yml
```

### Add the local repository and install the app

```shell
$ flatpak remote-add --no-gpg-verify kiwix-repo repo
$ flatpak install kiwix-repo org.kiwix.Client
```

### Run it

```shell
$ flatpak run org.kiwix.Client
```

## Appendix

### Build bundle and install it

It's possible to build a standalone image of an application and install it. __This image does not contain the runtime.__

```shell
$ flatpak build-bundle repo org.kiwix.Client.flatpak org.kiwix.Client
$ flatpak instal org.kiwix.Client.flatpak
```

<!-- Links -->

[1]: https://flatpak.org/
