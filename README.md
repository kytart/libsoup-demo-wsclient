# Libsoup WebSocket Client

Simple project to demonstrate how to write a websocket client using libsoup.

## Requirements

	* meson
	* glib
	* libsoup

## How to build and run

```bash
meson builddir
meson compile -C builddir
builddir/client <websocket_url>
```
